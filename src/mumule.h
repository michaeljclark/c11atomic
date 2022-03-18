#pragma once

#undef NDEBUG
#include <stddef.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <string.h>
#include <threads.h>
#include <time.h>
#include <assert.h>

#include "mulog.h"
#include "muclock.h"

#if defined(_MSC_VER)
#define ALIGNED(x) __declspec(align(x))
#elif defined(__GNUC__)
#define ALIGNED(x) __attribute__((aligned(x)))
#else
#define ALIGNED(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef long long llong;

struct mu_mule;
typedef struct mu_mule mu_mule;
struct mu_thread;
typedef struct mu_thread mu_thread;

/*
 * mumule thread pool:
 *
 * - `mule_init(mule, nthreads, kernel, userdata)` to initialize the queue
 * - `mule_start(mule)` to start threads
 * - `mule_stop(mule)` to stop threads
 * - `mule_submit(mule,n)` to queue work
 * - `mule_sync(mule)` to quench the queue
 * - `mule_reset(mule)` to clear counters
 *
 * mumule example program:
 *
 * {
 *     mu_mule mule;
 *
 *     mule_init(&mule, 2, w1, NULL);
 *     mule_submit(&mule, 8);
 *     mule_start(&mule);
 *     mule_sync(&mule);
 *     mule_stop(&mule);
 *     mule_destroy(&mule);
 * }
 *
 */

typedef void(*mumule_work_fn)(void *arg, size_t thr_idx, size_t item_idx);

static void mule_init(mu_mule *mule, size_t num_threads, mumule_work_fn kernel, void *userdata);
static size_t mule_submit(mu_mule *mule, size_t count);
static int mule_start(mu_mule *mule);
static int mule_sync(mu_mule *mule);
static int mule_reset(mu_mule *mule);
static int mule_stop(mu_mule *mule);
static int mule_destroy(mu_mule *mule);

enum {
    mumule_max_threads = 8,

    /*
     * condition revalidation timeouts - time between revalidation of the
     * work available condition for worker threads is 10 ms (100Hz).
     * if workers are busy they will only perform an atomic increment,
     * dispatching thread has a shorter timeout in mule_sync. timeouts are
     * only necessary if thread is pre-empted before calling cnd_timedwait.
     */
    mumule_revalidate_work_available_ns = 10000000, /* 10 milliseconds */
    mumule_revalidate_queue_complete_ns = 1000000,  /* 1 millisecond */
};

struct mu_thread { mu_mule *mule; size_t idx; thrd_t thread; };

struct mu_mule
{
    mtx_t            mutex;
    cnd_t            wake_dispatcher;
    cnd_t            wake_worker;
    void*            userdata;
    mumule_work_fn   kernel;
    size_t           num_threads;
    _Atomic size_t   running;
    _Atomic size_t   threads_running;

    mu_thread        threads[mumule_max_threads];

    ALIGNED(64) _Atomic size_t  queued;
    ALIGNED(64) _Atomic size_t  processing;
    ALIGNED(64) _Atomic size_t  processed;
};

/*
 * mumule implementation
 */

static inline struct timespec _timespec_add(struct timespec abstime, llong reltime)
{
    llong tv_nsec = abstime.tv_nsec + reltime;
    abstime.tv_nsec = tv_nsec % 1000000000;
    abstime.tv_sec += tv_nsec / 1000000000;
    return abstime;
}

static inline const char* _timespec_string(char *buf, size_t buf_size, struct timespec abstime)
{
    struct tm ttm;
    time_t tsec = abstime.tv_sec;
    const char tfmt[] = "%Y-%m-%dT%H:%M:%S.";
#if defined(HAVE_GMTIME_R)
    gmtime_r(&tsec, &ttm);
#elif defined(HAVE_GMTIME_S)
    gmtime_s(&tsec, &ttm);
#elif defined(HAVE_GMTIME_S_REVERSED)
    gmtime_s(&ttm, &tsec);
#else
    #error gmtime not available
#endif
    size_t len = strftime(buf, buf_size, tfmt, &ttm);
    snprintf(buf+len, buf_size-len, "%lldZ", (llong)abstime.tv_nsec);
    return buf;
}

static void mule_init(mu_mule *mule, size_t num_threads, mumule_work_fn kernel, void *userdata)
{
    memset(mule, 0, sizeof(mu_mule));
    mule->userdata = userdata;
    mule->kernel = kernel;
    mule->num_threads = num_threads;
    mtx_init(&mule->mutex, mtx_plain);
    cnd_init(&mule->wake_worker);
    cnd_init(&mule->wake_dispatcher);
}

static int mule_thread(void *arg)
{
    mu_thread *thread = (mu_thread*)arg;
    mu_mule *mule = thread->mule;
    void* userdata = mule->userdata;
    const size_t thread_idx = thread->idx;
    size_t queued, processing, processed, workitem_idx;
    char tstr[32];

    const muclock_definition *clk = muclock_get_default();

    debugf("mule_thread-%zu: worker-started\n", thread_idx);
    atomic_fetch_add_explicit(&mule->threads_running, 1, __ATOMIC_RELAXED);

    for (;;) {
        struct timespec abstime = { 0 };
	// fixme: muclock is relative so incorrect epoch for cnd_timewait
        // assert(!clock_gettime(CLOCK_REALTIME, &abstime));
	muclock_get_time_ts(clk, &abstime);
        abstime = _timespec_add(abstime, mumule_revalidate_work_available_ns);

        /* find out how many items still need processing */
        queued = atomic_load_explicit(&mule->queued, __ATOMIC_ACQUIRE);
        processing = atomic_load_explicit(&mule->processing, __ATOMIC_ACQUIRE);

        /* sleep on condition if queue empty or exit if asked to stop */
        if (processing == queued)
        {
            tracef("mule_thread-%zu: queue-empty (t=%s)\n",
                thread_idx, _timespec_string(tstr, sizeof(tstr), abstime));

            mtx_lock(&mule->mutex);
            if (!atomic_load(&mule->running)) {
                mtx_unlock(&mule->mutex);
                break;
            }

            /*
             * +
             * |
             * | [queue-empty] -> [queue-processing]
             * |
             * | [worker-lost-wakeup] condition change missed by
             * | the worker if pre-empted before cnd_wait so we
             * | use cond_timedwait and loop to recheck the condition.
             *  \
             *   +
             */
            tracef("mule_thread-%zu: queue-empty\n", thread_idx);
            cnd_timedwait(&mule->wake_worker, &mule->mutex, &abstime);
            tracef("mule_thread-%zu: worker-woke\n", thread_idx);
            mtx_unlock(&mule->mutex);

            continue;
        }

        /* dequeue work-item using compare-and-swap, run, update processed */
        workitem_idx = processing + 1;
        if (!atomic_compare_exchange_weak(&mule->processing, &processing,
            workitem_idx)) continue;
        atomic_thread_fence(__ATOMIC_ACQUIRE);
        (mule->kernel)(userdata, thread_idx, workitem_idx);
        atomic_thread_fence(__ATOMIC_RELEASE);
        processed = atomic_fetch_add_explicit(&mule->processed, 1, __ATOMIC_SEQ_CST);

        /* signal dispatcher precisely when the last item is processed */
        if (processed + 1 == queued) {
            tracef("mule_thread-%zu: queue-complete\n", thread_idx);
            /*
             *   +
             *  /
             * | [dispatcher-lost-wakeup] condition change missed by
             * | the dispatcher if pre-empted before cnd_wait so we
             * | use cond_timedwait and loop to recheck the condition.
             * |
             * | [queue-processing] -> [queue-complete]
             * |
             * +
             */
            cnd_signal(&mule->wake_dispatcher);
        }
    }

    atomic_fetch_add_explicit(&mule->threads_running, -1, __ATOMIC_RELAXED);
    debugf("mule_thread-%zu: worker-exiting\n", thread_idx);

    return 0;
}

static size_t mule_submit(mu_mule *mule, size_t count)
{
    debugf("mule_submit: queue-start\n");
    size_t idx = atomic_fetch_add_explicit(&mule->queued, count, __ATOMIC_SEQ_CST);
    cnd_broadcast(&mule->wake_worker);
    return idx + count;
}

static int mule_start(mu_mule *mule)
{
    mtx_lock(&mule->mutex);
    if (atomic_load(&mule->running)) {
        mtx_unlock(&mule->mutex);
        return 0;
    }

    debugf("mule_start: starting-threads\n");
    for (size_t idx = 0; idx < mule->num_threads; idx++) {
        mule->threads[idx].mule = mule;
        mule->threads[idx].idx = idx;
    }
    atomic_store(&mule->running, 1);
    atomic_thread_fence(__ATOMIC_SEQ_CST);

    for (size_t i = 0; i < mule->num_threads; i++) {
        assert(!thrd_create(&mule->threads[i].thread, mule_thread, &mule->threads[i]));
    }
    mtx_unlock(&mule->mutex);
    return 0;
}

static int mule_sync(mu_mule *mule)
{
    size_t queued, processed;
    char tstr[32];

    const muclock_definition *clk = muclock_get_default();

    debugf("mule_sync: quench-queue\n");
    cnd_broadcast(&mule->wake_worker);

    /* wait for queue to quench */
    mtx_lock(&mule->mutex);
    for (;;) {
        struct timespec abstime = { 0 };
	// fixme: muclock is relative so incorrect epoch for cnd_timewait
        // assert(!clock_gettime(CLOCK_REALTIME, &abstime));
	muclock_get_time_ts(clk, &abstime);
        abstime = _timespec_add(abstime, mumule_revalidate_queue_complete_ns);

        queued = atomic_load_explicit(&mule->queued, __ATOMIC_ACQUIRE);
        processed = atomic_load_explicit(&mule->processed, __ATOMIC_ACQUIRE);
        if (processed < queued) {
            /*
             * +
             * |
             * | [queue-processing] -> [queue-complete]
             * |
             * | [dispatcher-lost-wakeup] condition change missed by
             * | the dispatcher if pre-empted before cnd_wait so we
             * | use cond_timedwait and loop to recheck the condition.
             *  \
             *   +
             */
            tracef("mule_sync: queue-processing (t=%s)\n",
                _timespec_string(tstr, sizeof(tstr), abstime));
            cnd_timedwait(&mule->wake_dispatcher, &mule->mutex, &abstime);
            tracef("mule_sync: dispatcher-woke\n");
        } else {
            break;
        }
    };
    mtx_unlock(&mule->mutex);

    debugf("mule_sync: queue-complete\n");

    return 0;
}

static int mule_reset(mu_mule *mule)
{
    mule_sync(mule);

    atomic_store(&mule->queued, 0);
    atomic_store(&mule->processing, 0);
    atomic_store(&mule->processed, 0);

    cnd_broadcast(&mule->wake_worker);

    return 0;
}

static int mule_stop(mu_mule *mule)
{
    mtx_lock(&mule->mutex);
    if (!atomic_load(&mule->running)) {
        mtx_unlock(&mule->mutex);
        return 0;
    }

    /* shutdown workers */
    debugf("mule_stop: stopping-threads\n");

    atomic_store_explicit(&mule->running, 0, __ATOMIC_RELEASE);
    mtx_unlock(&mule->mutex);
    cnd_broadcast(&mule->wake_worker);

    /* join workers */
    for (size_t i = 0; i < mule->num_threads; i++) {
        int res;
        assert(!thrd_join(mule->threads[i].thread, &res));
    }

    return 0;
}

static int mule_destroy(mu_mule *mule)
{
    mule_stop(mule);

    mtx_destroy(&mule->mutex);
    cnd_destroy(&mule->wake_worker);
    cnd_destroy(&mule->wake_dispatcher);

    return 0;
}

#ifdef __cplusplus
}
#endif

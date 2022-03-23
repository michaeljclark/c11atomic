/*
 * atomic memory interlock latency benchmark (public domain)
 * author: Michael Clark <michaeljclark@mac.com>
 * c++ -pthread -O2 atomic_latency.cc -o atomic_latency
 */

#undef NDEBUG
#include <stdio.h>
#include <threads.h>
#include <stdatomic.h>
#include <assert.h>

#include "muclock.h"
#include "mucpu.h"

typedef unsigned uint;

#if defined(_MSC_VER)
#define ALIGN(N) __declspec(align(N))
#elif defined(__GNUC__)
#define ALIGN(N) __attribute__((aligned(N)))
#else
#define ALIGN(N)
#endif

const size_t iters = 10000000;

static ALIGN(64) atomic_uint sctr;
static ALIGN(64) atomic_uint rctr;

static thrd_t sthr, rthr;
static muclock_type st[2], rt[2];

static const muclock_definition *clk;

static int sloop(void *arg)
{
    report_cpuid_node_topology();

    st[0] = muclock_gettime_ns(clk);
    for (size_t i = 0; i < iters; i++) {
	uint y, x = atomic_load_explicit(&sctr, memory_order_acquire);
	do {
	    y = atomic_load_explicit(&rctr, memory_order_acquire);
	} while (x != y);
	atomic_store_explicit(&sctr, x+1, memory_order_release);
    }
    st[1] = muclock_gettime_ns(clk);

    return 0;
}

static int rloop(void *arg)
{
    report_cpuid_node_topology();

    rt[0] = muclock_gettime_ns(clk);
    for (size_t i = 0; i < iters; i++) {
	uint y, x = atomic_load_explicit(&rctr, memory_order_acquire);
	do {
	    y = atomic_load_explicit(&sctr, memory_order_acquire);
	} while (x == y);
	atomic_store_explicit(&rctr, y, memory_order_release);
    }
    rt[1] = muclock_gettime_ns(clk);

    return 0;
}

void run()
{
    int sres, rres;
    clk = muclock_get_default();
    assert(!thrd_create(&sthr, sloop, 0));
    assert(!thrd_create(&rthr, rloop, 0));
    assert(!thrd_join(sthr, &sres));
    assert(!thrd_join(rthr, &rres));
    uint count = atomic_load(&rctr);
    double t = (double)(rt[1] - rt[0]);
    printf("%s: received: %u in %.2f sec, %.f req/sec, %.2f ns roundtrip\n",
	   count == iters ? "PASS" : "FAIL",
	   count, t / 1e9, count * (1e9 / t), t / count);
}

int main(int argc, const char **argv)
{
    run();
}

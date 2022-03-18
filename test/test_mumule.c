#undef NDEBUG
#include <stdio.h>
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include "mumule.h"

int debug = 2;

_Atomic size_t counter = 0;

void w1(void *arg, size_t thr_idx, size_t item_idx)
{
    debugf("arg=%p thr_idx=%zu item_idx=%zu\n", arg, thr_idx, item_idx);
    atomic_fetch_add_explicit(&counter, 1, __ATOMIC_SEQ_CST);
}

void t1()
{
    mu_mule mule;
    mule_init(&mule, 2, w1, NULL);
    mule_submit(&mule, 8);
    mule_start(&mule);
    mule_sync(&mule);
    mule_stop(&mule);
    mule_destroy(&mule);
    assert(atomic_load(&counter) == 8);
}

int main(int argc, const char **argv)
{
    if (argc == 2 && strcmp(argv[1], "-v") == 0) {
        mu_set_debug(1);
    } else if (argc == 2 && strcmp(argv[1], "-vv") == 0) {
        mu_set_debug(2);
    }

    t1();

    debugf("test-complete");
}

#undef NDEBUG
#include <assert.h>
#include <stdatomic.h>

/* tests atomic_int. needs to be templated for other types. */

void t1() {
    _Atomic int v = 0;
    assert(atomic_exchange((_Atomic int*)&v,5) == 0);
    assert(atomic_exchange((_Atomic int*)&v,0) == 5);
}
void t2() {
    _Atomic int v = 0;
    int o = 0;
    assert(atomic_compare_exchange_strong((_Atomic int*)&v,&o,5));
    assert(!atomic_compare_exchange_strong((_Atomic int*)&v,&o,5));
}
void t3() {
    _Atomic int v = 0;
    assert(atomic_fetch_add((_Atomic int*)&v,5) == 0);
    assert(atomic_fetch_add((_Atomic int*)&v,5) == 5);
}
void t4() {
    _Atomic int v = 0;
    assert(atomic_fetch_sub((_Atomic int*)&v,5) == 0);
    assert(atomic_fetch_sub((_Atomic int*)&v,5) == -5);
}
void t5() {
    _Atomic int v = 0;
    assert(atomic_fetch_and((_Atomic int*)&v,5) == 0);
    assert(atomic_fetch_and((_Atomic int*)&v,5) == 0);
}
void t6() {
    _Atomic int v = 0;
    assert(atomic_fetch_or((_Atomic int*)&v,5) == 0);
    assert(atomic_fetch_or((_Atomic int*)&v,5) == 5);
}
void t7() {
    _Atomic int v = 0;
    assert(atomic_fetch_xor((_Atomic int*)&v,5) == 0);
    assert(atomic_fetch_xor((_Atomic int*)&v,5) == 5);
}
void t8() {
    atomic_flag f = ATOMIC_FLAG_INIT;
    assert(atomic_flag_test_and_set(&f) == 0);
    assert(atomic_flag_test_and_set(&f) == 1);
    atomic_flag_clear(&f);
}
void t9() {
    _Atomic int x = 0;
    assert(atomic_is_lock_free(&x));
}

int main(int argc, const char **argv)
{
    t1();
    t2();
    t2();
    t4();
    t5();
    t6();
    t7();
    t8();
    t9();
}

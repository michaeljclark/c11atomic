#include <stdatomic.h>

/* generates atomic_int. needs to be templated for other types. */

_Atomic int v;
atomic_flag f = ATOMIC_FLAG_INIT;

int f1() {
    return atomic_exchange((_Atomic int*)&v,5);
}
_Bool f2() {
    int n = 0;
    return atomic_compare_exchange_strong((_Atomic int*)&v,&n,5);
}
int f3() {
    return atomic_fetch_add((_Atomic int*)&v,5);
}
int f4() {
    return atomic_fetch_sub((_Atomic int*)&v,5);
}
int f5() {
    return atomic_fetch_and((_Atomic int*)&v,5);
}
int f6() {
    return atomic_fetch_or((_Atomic int*)&v,5);
}
int f7() {
    return atomic_fetch_xor((_Atomic int*)&v,5);
}
int f8() {
    return atomic_flag_test_and_set(&f);
}

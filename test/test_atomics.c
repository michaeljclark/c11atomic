#undef NDEBUG
#include <assert.h>
#include <stdatomic.h>

typedef long long llong;

#define concat2(a,b) a ## _ ## b
#define concat3(a,b,c) a ## _ ## b ## _ ## c

#define gen_atomic_exchange(type) void concat2(test_atomic_exchange,type)() { \
    _Atomic type v = 0; \
    assert(atomic_exchange((_Atomic type*)&v,5) == 0); \
    assert(atomic_exchange((_Atomic type*)&v,0) == 5); }

#define gen_atomic_compare_exchange(type) void concat2(test_atomic_compare_exchange,type)() { \
    _Atomic type v = 0; type o = 0; \
    assert(atomic_compare_exchange_strong((_Atomic type*)&v,&o,5)); \
    assert(!atomic_compare_exchange_strong((_Atomic type*)&v,&o,5)); }

#define gen_atomic_fetch_add(type) void concat2(test_atomic_fetch_add,type)() { \
    _Atomic type v = 0; \
    assert(atomic_fetch_add((_Atomic type*)&v,5) == 0); \
    assert(atomic_fetch_add((_Atomic type*)&v,5) == 5); }

#define gen_atomic_fetch_sub(type) void concat2(test_atomic_fetch_sub,type)() { \
    _Atomic type v = 0; \
    assert(atomic_fetch_sub((_Atomic type*)&v,5) == 0); \
    assert(atomic_fetch_sub((_Atomic type*)&v,5) == -5); }

#define gen_atomic_fetch_and(type) void concat2(test_atomic_fetch_and,type)() { \
    _Atomic type v = 0; \
    assert(atomic_fetch_and((_Atomic type*)&v,5) == 0); \
    assert(atomic_fetch_and((_Atomic type*)&v,5) == 0); }

#define gen_atomic_fetch_or(type) void concat2(test_atomic_fetch_or,type)() { \
    _Atomic type v = 0; \
    assert(atomic_fetch_or((_Atomic type*)&v,5) == 0); \
    assert(atomic_fetch_or((_Atomic type*)&v,5) == 5); }

#define gen_atomic_fetch_xor(type) void concat2(test_atomic_fetch_xor,type)() { \
    _Atomic type v = 0; \
    assert(atomic_fetch_xor((_Atomic type*)&v,5) == 0); \
    assert(atomic_fetch_xor((_Atomic type*)&v,5) == 5); }

#define gen_atomic_is_lock_free(type) void concat2(test_atomic_is_lock_free,type)() \
    { _Atomic type x = 0; assert(atomic_is_lock_free(&x)); }

gen_atomic_exchange(char)
gen_atomic_exchange(short)
gen_atomic_exchange(int)
gen_atomic_exchange(long)
gen_atomic_exchange(llong)

gen_atomic_compare_exchange(char)
gen_atomic_compare_exchange(short)
gen_atomic_compare_exchange(int)
gen_atomic_compare_exchange(long)
gen_atomic_compare_exchange(llong)

gen_atomic_fetch_add(char)
gen_atomic_fetch_add(short)
gen_atomic_fetch_add(int)
gen_atomic_fetch_add(long)
gen_atomic_fetch_add(llong)

gen_atomic_fetch_sub(char)
gen_atomic_fetch_sub(short)
gen_atomic_fetch_sub(int)
gen_atomic_fetch_sub(long)
gen_atomic_fetch_sub(llong)

gen_atomic_fetch_and(char)
gen_atomic_fetch_and(short)
gen_atomic_fetch_and(int)
gen_atomic_fetch_and(long)
gen_atomic_fetch_and(llong)

gen_atomic_fetch_or(char)
gen_atomic_fetch_or(short)
gen_atomic_fetch_or(int)
gen_atomic_fetch_or(long)
gen_atomic_fetch_or(llong)

gen_atomic_fetch_xor(char)
gen_atomic_fetch_xor(short)
gen_atomic_fetch_xor(int)
gen_atomic_fetch_xor(long)
gen_atomic_fetch_xor(llong)

gen_atomic_is_lock_free(char)
gen_atomic_is_lock_free(short)
gen_atomic_is_lock_free(int)
gen_atomic_is_lock_free(long)
gen_atomic_is_lock_free(llong)

void test_atomic_flag_test_and_set()
{
    atomic_flag f = ATOMIC_FLAG_INIT;
    assert(atomic_flag_test_and_set(&f) == 0);
    assert(atomic_flag_test_and_set(&f) == 1);
    atomic_flag_clear(&f);
}

void test_atomic_flag_clear()
{
    atomic_flag f = ATOMIC_FLAG_INIT;
    assert(atomic_flag_test_and_set(&f) == 0);
    atomic_flag_clear(&f);
    assert(atomic_flag_test_and_set(&f) == 0);
}

int main(int argc, const char **argv)
{
    test_atomic_exchange_char();
    test_atomic_exchange_short();
    test_atomic_exchange_int();
    test_atomic_exchange_long();
    test_atomic_exchange_llong();

    test_atomic_compare_exchange_char();
    test_atomic_compare_exchange_short();
    test_atomic_compare_exchange_int();
    test_atomic_compare_exchange_long();
    test_atomic_compare_exchange_llong();

    test_atomic_fetch_add_char();
    test_atomic_fetch_add_short();
    test_atomic_fetch_add_int();
    test_atomic_fetch_add_long();
    test_atomic_fetch_add_llong();

    test_atomic_fetch_sub_char();
    test_atomic_fetch_sub_short();
    test_atomic_fetch_sub_int();
    test_atomic_fetch_sub_long();
    test_atomic_fetch_sub_llong();

    test_atomic_fetch_and_char();
    test_atomic_fetch_and_short();
    test_atomic_fetch_and_int();
    test_atomic_fetch_and_long();
    test_atomic_fetch_and_llong();

    test_atomic_fetch_or_char();
    test_atomic_fetch_or_short();
    test_atomic_fetch_or_int();
    test_atomic_fetch_or_long();
    test_atomic_fetch_or_llong();

    test_atomic_fetch_xor_char();
    test_atomic_fetch_xor_short();
    test_atomic_fetch_xor_int();
    test_atomic_fetch_xor_long();
    test_atomic_fetch_xor_llong();

    test_atomic_is_lock_free_char();
    test_atomic_is_lock_free_short();
    test_atomic_is_lock_free_int();
    test_atomic_is_lock_free_long();
    test_atomic_is_lock_free_llong();

    test_atomic_flag_test_and_set();
    test_atomic_flag_clear();
}

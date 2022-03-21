#include <stdatomic.h>

typedef long long llong;

atomic_char v_char;
atomic_short v_short;
atomic_int v_int;
atomic_long v_long;
atomic_llong v_llong;

atomic_flag f = ATOMIC_FLAG_INIT;

#define concat2(a,b) a ## _ ## b
#define concat3(a,b,c) a ## _ ## b ## _ ## c

#define gen_atomic_exchange(type) type concat2(test_atomic_exchange,type)() \
    { return atomic_exchange((_Atomic type*)&concat2(v,type),5); }

#define gen_atomic_compare_exchange(type) _Bool concat2(test_atomic_compare_exchange,type)() \
    { type n = 0; return atomic_compare_exchange_strong((_Atomic type*)&concat2(v,type),&n,5); }

#define gen_atomic_fetch_op(op,type) type concat3(test_atomic_fetch,op,type)() \
    { return concat2(atomic_fetch,op)((_Atomic type*)&concat2(v,type),5); }

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

gen_atomic_fetch_op(add,char)
gen_atomic_fetch_op(add,short)
gen_atomic_fetch_op(add,int)
gen_atomic_fetch_op(add,long)
gen_atomic_fetch_op(add,llong)

gen_atomic_fetch_op(sub,char)
gen_atomic_fetch_op(sub,short)
gen_atomic_fetch_op(sub,int)
gen_atomic_fetch_op(sub,long)
gen_atomic_fetch_op(sub,llong)

gen_atomic_fetch_op(and,char)
gen_atomic_fetch_op(and,short)
gen_atomic_fetch_op(and,int)
gen_atomic_fetch_op(and,long)
gen_atomic_fetch_op(and,llong)

gen_atomic_fetch_op(or,char)
gen_atomic_fetch_op(or,short)
gen_atomic_fetch_op(or,int)
gen_atomic_fetch_op(or,long)
gen_atomic_fetch_op(or,llong)

gen_atomic_fetch_op(xor,char)
gen_atomic_fetch_op(xor,short)
gen_atomic_fetch_op(xor,int)
gen_atomic_fetch_op(xor,long)
gen_atomic_fetch_op(xor,llong)

int test_atomic_flag_test_and_set()
{
    return atomic_flag_test_and_set(&f);
}

void test_atomic_flag_clear()
{
    atomic_flag_clear(&f);
}

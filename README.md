# c11atomic

C11 <threads.h> and <stdatomic.h> compatibility headers for MSVC++.

- [C11 <threads.h> emulation library](https://cgit.freedesktop.org/mesa/mesa/log/include/c11)
is distributed under the Boost Software License, Version 1.0.
- C11 <stdatomic.h> emulation library is distributed under the PLEASE LICENSE.

## <stdatomic.h>

the heeader currently supports a subset of the atomic types.

- `atomic_flag`, `atomic_bool`
- `atomic_char`, `atomic_uchar`
- `atomic_short`, `atomic_ushort`
- `atomic_int`, `atomic_uint`
- `atomic_long`, `atomic_ulong`
- `atomic_llong`, `atomic_ullong`
- `atomic_ptr` _(non-standard)_

the header currently uses strong ordering for all primitives.

- [x] `atomic_load`, `atomic_load_explicit`
- [x] `atomic_store`, `atomic_store_explicit`
- [x] `atomic_exchange`, `atomic_exchange_explicit`
- [x] `atomic_compare_exchange_weak`, `atomic_compare_exchange_weak_explicit`
- [x] `atomic_compare_exchange_strong`, `atomic_compare_exchange_strong_explicit`
- [x] `atomic_fetch_add`, `atomic_fetch_add_explicit`
- [x] `atomic_fetch_sub`, `atomic_fetch_sub_explicit`
- [x] `atomic_fetch_and`, `atomic_fetch_and_explicit`
- [x] `atomic_fetch_or`, `atomic_fetch_or_explicit`
- [x] `atomic_fetch_xor`, `atomic_fetch_xor_explicit`
- [x] `atomic_flag_test_and_set`, `atomic_flag_test_and_set_explicit`
- [x] `atomic_flag_clear`, `atomic_flag_clear_explicit`
- [x] `atomic_thread_fence`, `atomic_signal_fence`
- [x] `atomic_is_lock_free`

## limitations

### custom pointer types

custom pointer types are not supported due to limitations with `_Generic`
dispatch as all types need to be specified in advance. GCC and Clang
implement atomics using polymorphic compiler builtin functions so they are
not exposed to the limitations of `_Generic` dispatch. as a work around
`atomic_ptr` has been added which is a void* pointer type that can be used
with casts to support atomic operations on pointers to custom types.

```
typedef void * _Atomic atomic_ptr;
```

## tests

### integration tests

simple compile, link and code generation tests. this is not intended
to be a full test suite.

- `test_threads` - tests <threads.h> thread creation.
- `test_atomics` - tests <stdatomic.h> interlocked operations.
- `codegen_atomics` - is for inspection of <stdatomic.h> code generation.

### latency tests

two threads exchange counter bumps to measure inter-thread latency
using the memory system.

- C11 &lt;threads.h> and &lt;stdatomic.h> tests:
  - `latency_atomic_acquire_release_c` - sender and receiver
    loop using acquire and release memory ops
  - `latency_atomic_relaxed_barrier_c` - sender and receiver
    loop using relaxed memory ops and barriers

- C++11 &lt;threads>, &lt;atomic>, and &lt;chrono> tests:
  - `latency_atomic_acquire_release_cxx` - sender and receiver
    loop using acquire and release memory ops
  - `latency_atomic_relaxed_barrier_cxx` - sender and receiver
    loop using relaxed memory ops and barriers

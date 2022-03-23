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

### thread pool tests

[mumule](https://github.com/michaeljclark/mumule) is a simple thread pool
implementation using the C11 thread support library and C11 atomics.

- `test_mumule` - minimal thread pool dispatcher using queue counters

```
mule_submit: queue-start
mule_start: starting-threads
mule_sync: quench-queue
mule_thread-0: worker-started
mule_thread-1: worker-started
arg=(nil) thr_idx=1 item_idx=2
arg=(nil) thr_idx=1 item_idx=3
arg=(nil) thr_idx=1 item_idx=4
arg=(nil) thr_idx=1 item_idx=5
arg=(nil) thr_idx=1 item_idx=6
arg=(nil) thr_idx=1 item_idx=7
arg=(nil) thr_idx=1 item_idx=8
mule_sync: queue-processing (t=1970-01-01T00:00:01.249553791Z)
arg=(nil) thr_idx=0 item_idx=1
mule_thread-0: queue-complete
mule_sync: dispatcher-woke
mule_sync: queue-complete
mule_stop: stopping-threads
mule_thread-1: queue-empty (t=1970-01-01T00:00:01.258748034Z)
mule_thread-1: worker-exiting
mule_thread-0: queue-empty (t=1970-01-01T00:00:01.258818636Z)
mule_thread-0: worker-exiting
```

### clock tests

portable replacement for `clock_gettime` using platform specific clock
APIs on Windows, Linux, and macOS, plus x86 TSC using `rdtsc`, and a
fallback to the lower resolution C Standard Library clock API.

- `test_muclock` - clock busy loop overshoot, jitter and latency histogram

```
# test_meta
clock    : clock-std        freq     : 1000000
clock    : clock-posix      freq     : 1000000000
clock    : clock-x86        freq     : 2600000000
clk: clock-x86

# test_busy
wait_1ms :    2600034 cycles (elapsed 1000013.077 ns, overshoot   13.077 ns)

# test_jitter
dmin     :         48 cycles (  18.462 ns)
dmax     :        122 cycles (  46.923 ns)
drange   :         74 cycles (  28.462 ns)

# histogram
      48 (  18.462 ns)     82 ███████████▉
      52 (  20.000 ns)    345 ██████████████████████████████████████████████████
      54 (  20.769 ns)     41 █████▉
      56 (  21.538 ns)    125 ██████████████████
      58 (  22.308 ns)     82 ███████████▉
      60 (  23.077 ns)    143 ████████████████████▋
      62 (  23.846 ns)     41 █████▉
      64 (  24.615 ns)    163 ███████████████████████▌
      72 (  27.692 ns)      1 ▏
     122 (  46.923 ns)      1 ▏
```

### cpuid tests

interface to x86 `cpuid` on Windows, Linux, and macOS to query processor
topology (cpuid leaf _0x1f_ or _0x0b_) and report SMT thread placement.

- `test_mucpu` - reports x86 processor model name, apic_id, core_id and smt_id

```
Intel(R) Core(TM) i9-7980XE CPU @ 2.60GHz (apic_id:17 core_id:8 smt_id:1)
```

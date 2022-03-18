#include <stdio.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <malloc.h>
#define alloca _alloca
#else
#include <alloca.h>
#endif

#include "mucpu.h"
#include "muclock.h"

#define array_size(a) (sizeof(a)/sizeof(a[0]))

#ifdef __APPLE__

#include <mach/mach.h>
#include <mach/mach_time.h>

static const muclock_type muclock_resolution = 1000000000ll;

static void muclock_mach_init()
{
    // do nothing
}

static const char* muclock_mach_name()
{
    return "clock-mach";
}

static muclock_type muclock_mach_frequency()
{
    mach_timebase_info_data_t ti;
    mach_timebase_info(&ti);
    return ti.denom / ti.numer * muclock_resolution;
}

static muclock_type muclock_mach_time()
{
    return (muclock_type)mach_absolute_time();
}

static const muclock_definition muclock_mach_def = {
    muclock_mach_init,
    muclock_mach_name,
    muclock_mach_frequency,
    muclock_mach_time
};

#else
static const muclock_definition muclock_mach_def = { 0 };
#endif

#if defined(__linux__) || defined(__GLIBC__)

static const muclock_type muclock_resolution = 1000000000ll;

static void muclock_posix_init()
{
    // do nothing
}

static const char* muclock_posix_name()
{
    return "clock-posix";
}

static muclock_type muclock_posix_frequency()
{
    return muclock_resolution;
}

static muclock_type muclock_posix_time()
{
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return (muclock_type)res.tv_nsec + (muclock_type)res.tv_sec * muclock_resolution;
}

static const muclock_definition muclock_posix_def = {
    muclock_posix_init,
    muclock_posix_name,
    muclock_posix_frequency,
    muclock_posix_time
};

#else
static const muclock_definition muclock_posix_def = { 0 };
#endif

static void muclock_std_init()
{
    // do nothing
}

static const char* muclock_std_name()
{
    return "clock-std";
}

static muclock_type muclock_std_frequency()
{
    return CLOCKS_PER_SEC;
}

static muclock_type muclock_std_time()
{
    return (muclock_type)clock();
}

static const muclock_definition muclock_std_def = {
    muclock_std_init,
    muclock_std_name,
    muclock_std_frequency,
    muclock_std_time
};

#ifdef _WIN32

static void muclock_win32_init()
{
    // do nothing
}

static const char* muclock_win32_name()
{
    return "clock-win32";
}

static muclock_type muclock_win32_frequency()
{
    LARGE_INTEGER f = {};
    QueryPerformanceFrequency(&f);
    return f.QuadPart;
}

static muclock_type muclock_win32_time()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
}

static const muclock_definition muclock_win32_def = {
    muclock_win32_init,
    muclock_win32_name,
    muclock_win32_frequency,
    muclock_win32_time
};

#else
static const muclock_definition muclock_win32_def = { 0 };
#endif

#if ( defined __GNUC__ && (defined __i386__ || defined __x86_64__) ) || \
    ( defined _MSC_VER && (defined _M_IX86 || defined _M_X64) )

static muclock_type base_freq;

static void muclock_x86_init()
{
    int cpuid_16[4] = { 0 };
    int cpuid_vm[4] = { 0 };
    enum { eax, ecx, edx, ebx };

    __cpuidex(cpuid_vm, 0x40000010, 0);
    __cpuidex(cpuid_16, 0x16,       0);

    if (cpuid_16[eax]) {
        base_freq = cpuid_16[eax] & 0xffff;
    }
    else if (cpuid_vm[eax] > 100000) {
        base_freq = cpuid_vm[eax] / 1000;
    }
    else {
        base_freq = 0;
    }
}

static const char* muclock_x86_name()
{
    return "clock-x86";
}

static muclock_type muclock_x86_frequency()
{
    return base_freq * 1000000ll;
}

static muclock_type muclock_x86_time()
{
    return __rdtsc();
}

static const muclock_definition muclock_x86_def = {
    muclock_x86_init,
    muclock_x86_name,
    muclock_x86_frequency,
    muclock_x86_time
};

#else
static const muclock_definition muclock_x86_def = { 0 };
#endif

/* architecture independent clock interface */

static const muclock_definition muclock_impls[] = {
    muclock_std_def,
    muclock_mach_def,
    muclock_posix_def,
    muclock_win32_def,
    muclock_x86_def,
};
static uint32_t muclock_inited;
static const muclock_definition *chosen_clock;

void muclock_get_definitions(const muclock_definition **defs, size_t *nclocks)
{
    size_t n = 0;
    for (intptr_t i = 0; i < array_size(muclock_impls); i++) {
        const muclock_definition *impl = muclock_impls + i;
        if (!(impl->init && impl->init && impl->init)) continue;
        if ((muclock_inited & (1 << n)) == 0) {
            impl->init();
            muclock_inited |= (1 << n);
        }
        if (defs && nclocks && n < *nclocks) {
            defs[n] = impl;
        }
        n++;
    }
    if (nclocks) {
        *nclocks = n;
    }
}

static const muclock_definition* choose_clock()
{
    intptr_t best_idx = -1;
    muclock_type best_freq = 0;

    const muclock_definition **available_clocks = (const muclock_definition**)
	alloca(array_size(muclock_impls) * sizeof(muclock_definition*));
    size_t nclocks = array_size(muclock_impls);
    muclock_get_definitions(available_clocks, &nclocks);

    for (intptr_t i = 0; i < (intptr_t)nclocks; i++) {
        const muclock_definition *impl = available_clocks[i];
        muclock_type freq = impl->freq();
        if (freq == 0) continue;
        if (freq > best_freq) {
            best_freq = freq;
            best_idx = i;
        }
    }
    return best_idx < 0 ? NULL : available_clocks[best_idx];
}

const muclock_definition* muclock_get_default()
{
    if (chosen_clock == NULL) {
        chosen_clock = choose_clock();
    }
    return chosen_clock;
}

uint64_t muclock_get_time_ns(const muclock_definition *clk)
{
    muclock_type f = clk->freq(), t = clk->time();
    return t * 1000000000ull / f; /* this could overflow */
}

void muclock_get_time_ts(const muclock_definition *clk, struct timespec *ts)
{
    uint64_t ns = muclock_get_time_ns(clk);
    ts->tv_sec = ns / 1000000000ull;
    ts->tv_nsec = ns % 1000000000ull;
}

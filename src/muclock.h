#pragma once

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t muclock_type;

typedef void (*muclock_init_fn)();
typedef const char* (*muclock_name_fn)();
typedef muclock_type (*muclock_frequency_fn)();
typedef muclock_type (*muclock_time_fn)();

typedef struct muclock_definition
{
    muclock_init_fn init;
    muclock_name_fn name;
    muclock_frequency_fn freq;
    muclock_time_fn time;
} muclock_definition;

void muclock_get_definitions(const muclock_definition **defs, size_t *nclocks);
const muclock_definition* muclock_get_default();
uint64_t muclock_gettime_ns(const muclock_definition *clk);
void muclock_gettime_ts(const muclock_definition *clk, struct timespec *ts);

#ifdef __cplusplus
}
#endif

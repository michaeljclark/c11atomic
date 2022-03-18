#pragma once

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Debug
 */

extern int debug;

static void mu_set_debug(int level) { debug = level; }
static void log_printf(const char* fmt, ...);
#define debugf(...) if(debug > 0) log_printf(__VA_ARGS__)
#define tracef(...) if(debug > 1) log_printf(__VA_ARGS__)


/*
 * logging implementation
 */

static void log_printf(const char* fmt, ...)
{
    int len;
    char buf[128];
    char *pbuf = buf;
    char *hbuf = NULL;
    va_list ap;
    va_start(ap, fmt);
    len = vsnprintf(pbuf, sizeof(buf)-1, fmt, ap);
    pbuf[sizeof(buf)-1] = '\0';
    va_end(ap);
    if (len >= sizeof(buf)) {
        pbuf = hbuf = (char*)malloc(len + 1);
        va_start(ap, fmt);
        len = vsnprintf(pbuf, len + 1, fmt, ap);
        pbuf[len] = '\0';
        va_end(ap);
    }
    fwrite(pbuf, 1, len, stderr);
    if (hbuf) free(hbuf);
}

#ifdef __cplusplus
}
#endif

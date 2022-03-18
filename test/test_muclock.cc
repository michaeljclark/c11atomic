#include <stdio.h>
#include <limits.h>
#include <map>
#include <string>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "muclock.h"

void test_meta()
{
    static const size_t max_clocks = 16;
    size_t nclocks = max_clocks;
    const muclock_definition *available_clocks[max_clocks];

    printf("\n# test_meta\n");
    muclock_get_definitions(available_clocks, &nclocks);

    for (size_t i = 0; i < nclocks; i++) {
        const muclock_definition *clk = available_clocks[i];
        const char* name = clk->name();
        muclock_type f = clk->freq();
        printf("clock    : %-16s freq     : %lld\n",
               name, (long long)f);
    }
}

void test_busy()
{
    const muclock_definition *clk = muclock_get_default();
    printf("clk: %s\n", clk->name());
    muclock_type f = clk->freq();
    muclock_type wait = f/1000;

    printf("\n# test_busy\n");

    muclock_type begin = 0, end = 0, now;
    do {
        now = clk->time();
        if (begin == 0) {
            begin = now;
            end = now + wait;
        }
    } while (now < end);
    printf("wait_1ms :%11lld cycles (elapsed %8.3f ns, overshoot %8.3f ns)\n",
        (long long)(now-begin),
        (double)(now-begin)/(double)f*1e9,
        (double)(now-begin-wait)/(double)f*1e9);
}

std::string bar(size_t value, size_t max, size_t width)
{
#ifndef _WIN32
    // ▏▎▍▌▋▊▉█
    static const char* a[] = { " ", "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█" };

    size_t v = value * width * 8 / max;
    std::string s;
    while (v > 8) { s.append(a[8]); v-=8; }
    s.append(a[v]);
#else
    size_t v = value * width / max;
    std::string s;
    while (v-- > 0) s.append("X");
#endif
    return s;
}

void print_histogram(uint16_t *hist, intptr_t hist_size, muclock_type f)
{
    std::map<int,int> bins;
    for (intptr_t i = 0; i < hist_size; i++) {
        bins[hist[i]]++;
    }
    size_t hist_max = 0;
    for (auto &bin : bins) {
        if (bin.second > hist_max) {
            hist_max = bin.second;
        }
    }
    for (auto &bin : bins) {
        printf("%8d (%8.3f ns) %6d %s\n",
            bin.first, (double)bin.first/(double)f*1e9, bin.second,
            bar(bin.second, hist_max, 50).c_str());
    }
}

void test_jitter()
{
    printf("\n# test_jitter\n");
    const muclock_definition *clk = muclock_get_default();
    muclock_type t, l, f, d;
    muclock_type dmin = LLONG_MAX, dmax = 0, drange = 0;
    f = clk->freq();
    const intptr_t hist_size = 1024;
    uint16_t *hist = new uint16_t[hist_size]();
    for (intptr_t i = -1; i < hist_size; i++) {
        t = clk->time();
        if (i >= 0) {
            hist[i] = (uint16_t)(t - l);
        }
        l = t;
    }
    for (intptr_t i = 0; i < hist_size; i++) {
        d = hist[i];
        if (d < dmin) dmin = d;
        if (d > dmax) dmax = d;
    }
    drange = dmax - dmin;
    printf("dmin     :%11lld cycles (%8.3f ns)\n"
           "dmax     :%11lld cycles (%8.3f ns)\n"
           "drange   :%11lld cycles (%8.3f ns)\n",
        (long long)dmin, (double)dmin/(double)f*1e9,
        (long long)dmax, (double)dmax/(double)f*1e9,
        (long long)drange, (double)drange/(double)f*1e9);
    printf("\n# histogram\n");
    print_histogram(hist, hist_size, f);
    delete [] hist;
}

int main(int argc, char **argv)
{
    test_meta();
    test_busy();
    test_jitter();
}

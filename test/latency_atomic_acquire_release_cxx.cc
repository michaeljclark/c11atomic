/*
 * atomic memory interlock latency benchmark (public domain)
 * author: Michael Clark <michaeljclark@mac.com>
 * c++ -pthread -O2 atomic_latency.cc -o atomic_latency
 */

#include <cstdio>
#include <thread>
#include <atomic>
#include <chrono>

#include "mucpu.h"

using namespace std::chrono;

typedef unsigned uint;
typedef time_point<high_resolution_clock> time_type;

struct atomic_latency
{
    const size_t iters = 10000000;

    alignas(64) std::atomic<uint> sctr;
    alignas(64) std::atomic<uint> rctr;

    std::thread sthr, rthr;
    time_type st[2], rt[2];

    atomic_latency() : sctr(0), rctr(0),
        sthr(&atomic_latency::sloop, this),
        rthr(&atomic_latency::rloop, this) {}

    void sloop()
    {
        report_cpuid_node_topology();
        st[0] = high_resolution_clock::now();
        for (size_t i = 0; i < iters; i++) {
            uint y, x = sctr.load(std::memory_order_acquire);
            do {
                y = rctr.load(std::memory_order_acquire);
            } while (x != y);
            sctr.store(x+1, std::memory_order_release);
        }
        st[1] = high_resolution_clock::now();
    }

    void rloop()
    {
        report_cpuid_node_topology();
        rt[0] = high_resolution_clock::now();
        for (size_t i = 0; i < iters; i++) {
            uint y, x = rctr.load(std::memory_order_acquire);
            do {
                y = sctr.load(std::memory_order_acquire);
            } while (x == y);
            rctr.store(y, std::memory_order_release);
        }
        rt[1] = high_resolution_clock::now();
    }

    void run()
    {
        sthr.join();
        rthr.join();
        auto count = rctr.load();
        double t = (double)duration_cast<nanoseconds>(rt[1] - rt[0]).count();
        printf("%s: received: %u in %.2f sec, %.f req/sec, %.2f ns roundtrip\n",
	       count == iters ? "PASS" : "FAIL",
	       count, t / 1e9, count * (1e9 / t), t / count);
    }
};

int main(int argc, const char **argv)
{
    atomic_latency app;
    app.run();
}

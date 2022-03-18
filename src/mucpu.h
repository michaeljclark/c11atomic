#pragma once

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#if defined __GNUC__ && (defined __i386__ || defined __x86_64__)
#include <cpuid.h>
#include <x86intrin.h>
#ifndef __cpuidex
#define __cpuidex(inf,ax,cx) __cpuid_count(ax,cx,inf[0],inf[1],inf[2],inf[3])
#endif
#endif

#if defined _MSC_VER && (defined _M_IX86 || defined _M_X64)
#include <windows.h>
#endif

enum { eax, ebx, ecx, edx };

typedef enum {
    cpuid_level_type_smt = 1,
    cpuid_level_type_core = 2,
    cpuid_level_type_invalid = 0,
} cpuid_level_type;

typedef struct cpuid_node_topology {
    char nlevels;
    int x2apicid[4];
    short lvlprocs[4];
    char lvlbits[4];
    char lvlnum[4];
    char lvltype[4];
} cpuid_node_topology;

static inline bool check_cpuid_topoology_leaf(int leaf)
{
    int cpuid[4];
    __cpuidex(cpuid, leaf, 0);
    int lvltype = (cpuid[ecx] >> 8) & 0xff;
    int lvlnum = cpuid[ecx] & 0xff;
    return lvlnum == 0 && lvltype == cpuid_level_type_smt;
}

static inline cpuid_node_topology get_cpuid_node_topology()
{
    cpuid_node_topology node = { 0 };
    int i = 0, leaf;

    if (check_cpuid_topoology_leaf(0x1f)) {
        leaf = 0x1f;
    } else if (check_cpuid_topoology_leaf(0x0b)) {
        leaf = 0x0b;
    } else {
        goto out;
    }

    do {
        int cpuid[4];
        __cpuidex(cpuid, leaf, i);
        node.lvlbits[i] = cpuid[eax] & 0x1f;
        node.lvlprocs[i] = cpuid[ebx] & 0xffff;
        node.lvlnum[i] = cpuid[ecx] & 0xff;
        node.lvltype[i] = (cpuid[ecx] >> 8) & 0xff;
        node.x2apicid[i] = cpuid[edx];
    } while (node.lvltype[i] != 0 && ++i < 100);

    node.nlevels = i;

 out:
    return node;
}

static inline void dump_cpuid_node_topology(cpuid_node_topology node)
{
    for (size_t i = 0; i < node.nlevels; i++) {
        printf("cpuid.0BH: lvlbits: %d lvlprocs: %d lvlnum: %d lvltype: %d x2apicid: %d (%d)\n",
               node.lvlbits[i], node.lvlprocs[i], node.lvlnum[i], node.lvltype[i],
               node.x2apicid[i], node.x2apicid[i] >> node.lvlbits[i]);
    }
}

static inline void report_cpuid_node_topology() {
    cpuid_node_topology node = get_cpuid_node_topology();
    int apic_id = 0, core_id = 0, smt_id = 0;
    for (size_t i = 0; i < node.nlevels; i++) {
        if (node.lvltype[i] == cpuid_level_type_smt) {
            core_id = node.x2apicid[i] >> node.lvlbits[i];
            smt_id = node.x2apicid[i] & ((1 << node.lvlbits[i]) - 1);
            apic_id = node.x2apicid[i];
        }
    }

    int cpuid_0[4];
    int cpuid_8_2[4];
    int cpuid_8_3[4];
    int cpuid_8_4[4];

    int model;
    char name[49] = { 0 };
    char vendor[13] = { 0 };

    __cpuidex(cpuid_0,   0,          0);
    __cpuidex(cpuid_8_2, 0x80000002, 0);
    __cpuidex(cpuid_8_3, 0x80000003, 0);
    __cpuidex(cpuid_8_4, 0x80000004, 0);

    memset(vendor, 0, sizeof(vendor));
    memmove(vendor + 0, &cpuid_0[1], 4);
    memmove(vendor + 4, &cpuid_0[3], 4);
    memmove(vendor + 8, &cpuid_0[2], 4);
    model = cpuid_0[0];

    memset(name, 0, sizeof(name));
    memmove(name + 0x00, cpuid_8_2, 0x10);
    memmove(name + 0x10, cpuid_8_3, 0x10);
    memmove(name + 0x20, cpuid_8_4, 0x10);

    printf("%s (apic_id:%d core_id:%d smt_id:%d)\n", name, apic_id, core_id, smt_id);
}

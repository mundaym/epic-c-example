// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "htif.h"

#ifdef KERNEL
volatile uint64_t tohost __attribute__((section(".htif")));
volatile uint64_t fromhost __attribute__((section(".htif")));
#define TOHOST   tohost
#define FROMHOST fromhost
#else
#define TOHOST   (*(volatile uint64_t*)0x0002000)
#define FROMHOST (*(volatile uint64_t*)0x0002008)
#endif

uintptr_t syscall(uintptr_t n, uintptr_t a0, uintptr_t a1, uintptr_t a2,
                  uintptr_t a3, uintptr_t a4, uintptr_t a5, uintptr_t a6)
{
    uint64_t th = TOHOST;
    if(th) {
        __builtin_trap();
        while(1) {}
    }

    static volatile uint64_t htif_mem[8];
    htif_mem[0] = n;
    htif_mem[1] = a0;
    htif_mem[2] = a1;
    htif_mem[3] = a2;
    htif_mem[4] = a3;
    htif_mem[5] = a4;
    htif_mem[6] = a5;
    htif_mem[7] = a6;
    TOHOST = (uintptr_t)htif_mem;

    while(1) {
        uint64_t fh = FROMHOST;
        if(fh) {
            FROMHOST = 0;
            break;
        }
    }

    return htif_mem[0];
}

void shutdown(int code) {
    syscall(SYS_exit, code, 0, 0, 0, 0, 0, 0);
    while(1) {}
}

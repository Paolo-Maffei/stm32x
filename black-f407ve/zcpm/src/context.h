#ifndef __CONTEXT_INCLUDED__
#define __CONTEXT_INCLUDED__

#include "z80emu.h"
#include <stdint.h>

#define MAINMEM ((uint8_t*) 0x10000000) // CCM: special 64K area in F407

typedef struct {
	Z80_STATE	state;
	uint8_t		done;
        uint8_t         bank;
        uint8_t*        split;
        uint32_t        offset [16];
	uint8_t 	mem [0x18000]; // 96 KB extra for banked mem
} Context;

extern void SystemCall (Context *ctx, int request);

static uint8_t* mapMem (void* cp, uint16_t addr) {
    uint8_t* ptr = MAINMEM + addr;
#if 1
    Context* ctx = (Context*) cp;
    if (ptr >= ctx->split)
        ptr += ctx->offset[ctx->bank];
#endif
    return ptr;
}

#endif

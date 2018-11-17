/* zextest.h
 * Header for zextest example.
 *
 * Copyright (c) 2012, 2016 Lin Ke-Fong
 *
 * This code is free, do whatever you want with it.
 */

#ifndef __ZEXTEST_INCLUDED__
#define __ZEXTEST_INCLUDED__

#include "z80emu.h"
#include <stdint.h>

typedef struct {

	Z80_STATE	state;
	int 		done;
	uint8_t 	bankMem [0x18000];

} Context;

extern void     SystemCall (Context *zextest, int request);

static uint8_t* mapMem (uint16_t addr) {
    return (uint8_t*) 0x10000000 + addr;
}

#endif

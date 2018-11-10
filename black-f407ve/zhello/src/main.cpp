// Run a minimal "hello" example with z80 emu, to illustrate running z80 code

#include <jee.h>
#include <string.h>

extern "C" {
#include "zextest.h"
#include "z80emu.h"
}

const uint8_t rom [] = {
#include "hello.h";
};

UartDev< PinA<9>, PinA<10> > console;

ZEXTEST zex;

void SystemCall (ZEXTEST*) {
    for (int i = zex.state.registers.word[Z80_DE]; zex.memory[i] != '$'; i++)
        console.putc(zex.memory[i & 0xffff]);
}

int main() {
    console.init();

    memcpy(zex.memory, rom, sizeof rom);
    Z80Reset(&zex.state);

    while (!zex.is_done)
        Z80Emulate(&zex.state, 4000000, &zex);

    while (1) {}
}

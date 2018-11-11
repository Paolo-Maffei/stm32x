// Minimal "hello" example with z80 emu, using internal flash.

#include <jee.h>
#include <string.h>

const uint8_t* FlashBase = (const uint8_t*) 0x40000;

extern "C" {
#include "zextest.h"
#include "z80emu.h"
}

const uint8_t rom [] = {
#include "hello.h";
};

UartDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

ZEXTEST zex;

void SystemCall (ZEXTEST*) {
    for (int i = zex.state.registers.word[Z80_DE]; zex.memory[i] != '$'; i++)
        console.putc(zex.memory[i & 0xffff]);
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);

    // erase boot block and store some example code in it
    printf("erasing...\n");
    Flash::erasePage(FlashBase);

    printf("%08x %08x\n", ((const uint32_t*) FlashBase)[0],
                          ((const uint32_t*) FlashBase)[1]);

    printf("writing...\n");
    for (int i = 0; i < sizeof rom; i += 4)
        Flash::write32(FlashBase + i, ((const uint32_t*) rom)[i>>2]);

    printf("%08x %08x\n", ((const uint32_t*) FlashBase)[0],
                          ((const uint32_t*) FlashBase)[1]);

    // now emulate a boot loader which loads the first "disk" block at 0x0000
    memcpy(zex.memory, FlashBase, 128);
    Z80Reset(&zex.state);

    while (!zex.is_done)
        Z80Emulate(&zex.state, 4000000, &zex);

    while (1) {}
}

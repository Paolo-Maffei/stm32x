// Try out the F4's CRC-32 hardware calculation unit.

#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

int main() {
    console.init();
    printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    MMIO32(Periph::rcc + 0x30) |= (1<<12);  // CRCEN, p.181
    constexpr uint32_t crc = 0x40023000;

    MMIO32(crc+0x08) = 1; // reset
    printf("%08x\n", MMIO32(crc));

    MMIO32(crc) = 0x12345678;
    MMIO32(crc) = 0x87654321;
    MMIO32(crc) = 0xDEADBEEF;
    MMIO32(crc) = 0x11223344;
    MMIO32(crc) = 0x55667788;
    MMIO32(crc) = 0x00000000;

    printf("%08x\n", MMIO32(crc));

    MMIO32(crc) = MMIO32(crc);  // the resulting CRC should be 0

    printf("%08x\n", MMIO32(crc));

    while (1) {}
}

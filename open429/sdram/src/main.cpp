#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;
#include <../../common.h>

int main() {
	console.init();
	fullSpeedClock();
	printf("hello\n");

	// FMC I/O pin mode: 5432109876543210
    Port<'B'>::modeMap(0b0000000001100000, Pinmode::alt_out, 12);
    Port<'C'>::modeMap(0b0000000000000001, Pinmode::alt_out, 12);
    Port<'D'>::modeMap(0b1100011100000011, Pinmode::alt_out, 12);
    Port<'E'>::modeMap(0b1111111110000011, Pinmode::alt_out, 12);
    Port<'F'>::modeMap(0b1111100000111111, Pinmode::alt_out, 12);
    Port<'G'>::modeMap(0b1000000100110011, Pinmode::alt_out, 12);

    MMIO32(Periph::rcc + 0x38) = (1<<0);  // enable FSMC

	// SDRAM timing
    constexpr uint32_t fmc = 0xA0000000;
    MMIO32(fmc + 0x140) =
    MMIO32(fmc + 0x144) = (1<<13) | (0<<12) | (2<<10) | (0<<9) | (3<<7) |
							(1<<6) | (1<<4) | (1<<2) | (0<<0);
    MMIO32(fmc + 0x148) =
    MMIO32(fmc + 0x14C) = (1<<24) | (1<<20) | (1<<16) | (6<<12) |
							(3<<8) | (6<<4) | (1<<0);

	// SDRAM commands
    while (MMIO32(fmc + 0x158) & (1<<5)) {}
    MMIO32(fmc + 0x150) = (0<<9) | (0<<5) | (1<<3) | (1<<0);
	wait_ms(10);
    while (MMIO32(fmc + 0x158) & (1<<5)) {}
    MMIO32(fmc + 0x150) = (0<<9) | (0<<5) | (1<<3) | (2<<0);
    while (MMIO32(fmc + 0x158) & (1<<5)) {}
    MMIO32(fmc + 0x150) = (0<<9) | (3<<5) | (1<<3) | (3<<0);
    while (MMIO32(fmc + 0x158) & (1<<5)) {}
    MMIO32(fmc + 0x150) = (0<<9) | (3<<5) | (1<<3) | (3<<0);
    while (MMIO32(fmc + 0x158) & (1<<5)) {}
    MMIO32(fmc + 0x150) = (0x0231<<9) | (0<<5) | (1<<3) | (4<<0);
    while (MMIO32(fmc + 0x158) & (1<<5)) {}
    MMIO32(fmc + 0x154) = (1386<<1);

    constexpr uint32_t sdram = 0xD0000000;
    MMIO32(sdram+0) = 0x11223344;
    MMIO32(sdram+4) = 0x12345678;

    while (1) {
		printf("%08x %08x\n", MMIO32(sdram+0), MMIO32(sdram+4));
		wait_ms(1000);
	}
}

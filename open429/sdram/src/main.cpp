#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;
#include <../../common.h>

int main() {
	console.init();
	fullSpeedClock();
	printf("hello\n");

	initFmcSdram();

    MMIO32(sdram+0) = 0x11223344;
    MMIO32(sdram+4) = 0x12345678;

    while (1) {
		printf("%08x %08x %d\n", MMIO32(sdram+0), MMIO32(sdram+4), ticks);
		wait_ms(1000);
	}
}

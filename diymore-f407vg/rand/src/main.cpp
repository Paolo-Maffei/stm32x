// Try out the F4's Random Number Generator hardware.

#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    Periph::bit(Periph::rcc+0x34, 6) = 1;  // RNGEN, p.244
    constexpr uint32_t rng = 0x50060800;

    MMIO32(rng+0x0) = (1<<2); // RNGEN

    while (1) {
        while ((MMIO32(rng+0x4) & (1<<0)) == 0) {}
        printf("%08x %d\n", MMIO32(rng+0x8), ticks);
        wait_ms(1000);
    }
}

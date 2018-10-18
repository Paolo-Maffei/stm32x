#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinG<13> led;

static void enableClkAt160MHz () {
    constexpr uint32_t rcc   = Periph::rcc;

    MMIO32(Periph::flash + 0x00) = 0x705; // flash acr, 5 wait states
    MMIO32(rcc + 0x00) = (1<<16); // HSEON
    while ((MMIO32(rcc + 0x00) & (1<<17)) == 0) ; // wait for HSERDY
    MMIO32(rcc + 0x08) = 1; // switch to HSE
    MMIO32(rcc + 0x04) = (7<<24) | (1<<22) | (0<<16) | (160<<6) | (4<<0);
    MMIO32(rcc + 0x00) |= (1<<24); // PLLON
    while ((MMIO32(rcc + 0x00) & (1<<25)) == 0) ; // wait for PLLRDY
    MMIO32(rcc + 0x08) = (4<<13) | (5<<10) | (2<<0);
}

int main() {
	console.init();
#if 0
	enableSysTick();
#elif 1
	fullSpeedClock();
#else
    const int hz = 160000000;
	enableClkAt160MHz();
    enableSysTick(hz/1000);
	console.baud(115200, hz/2);
#endif

    led.mode(Pinmode::out);

	// result @ 16 = 1878, @ 160 = 187, @ 168 = 178
	int t = ticks;
	for (int i = 0; i < 10000000; ++i) __asm("");
	printf("loop %d ms\n", ticks - t);

    while (1) {
        printf("hello %d\n", ticks);

        led = 1;
        wait_ms(100);
        led = 0;
        wait_ms(400);
    }
}

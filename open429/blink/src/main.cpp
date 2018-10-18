#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;
#include <../../common.h>

PinG<13> led;

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

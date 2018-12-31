// Demo of the RTC code in the JeeH library.

#include <jee.h>

UartBufDev< PinA<2>, PinA<3> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinB<3> led;

RTC rtc;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock());
    led.mode(Pinmode::out);

    rtc.init();

    //rtc.set(RTC::DateTime{18,2,28,23,59,55});

    while (true) {
        RTC::DateTime dt = rtc.get();
        printf("%d %02d/%02d/%02d %02d:%02d:%02d\n", ticks,
                dt.yr, dt.mo, dt.dy, dt.hh, dt.mm, dt.ss);
        led = 1;
        wait_ms(100);
        led = 0;
        wait_ms(900);
    }
}

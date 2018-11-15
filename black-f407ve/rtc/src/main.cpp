// Demo of the RTC code in the JeeH library.

#include <jee.h>

UartDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinA<6> led;

RTC rtc;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    led.mode(Pinmode::out);

    rtc.init();

    //rtc.set(RTC::DateTime{18,2,28,23,59,55});

    while (1) {
        RTC::DateTime dt = rtc.get();
        printf("%d %02d/%02d/%02d %02d:%02d:%02d\n", ticks,
                dt.yr, dt.mo, dt.dy, dt.hh, dt.mm, dt.ss);
        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(400);
    }
}

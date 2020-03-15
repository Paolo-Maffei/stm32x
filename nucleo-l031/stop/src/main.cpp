// Power down demo, using stop mode and periodic wakeup events from the RTC

#include <jee.h>

UartDev< PinA<2>, PinA<3> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinB<3> led;

RTC rtc;

int main() {
    console.init();
    //enableSysTick();
    led.mode(Pinmode::out);

    rtc.init();
    //rtc.set(RTC::DateTime{18,2,28,23,59,55});
    rtc.wakeup(3*37000/16); // approx 3s, based on 37 kHz LSI clock

    int i = 0;
    while (true) {
        led.toggle();

        RTC::DateTime dt = rtc.get();
        printf("%d %02d/%02d/%02d %02d:%02d:%02d\n", ++i,
                dt.yr, dt.mo, dt.dy, dt.hh, dt.mm, dt.ss);
        for (int j = 0; j < 10000; ++j) asm ("");

        rtc.arm();
        powerDown(false);
    }
}

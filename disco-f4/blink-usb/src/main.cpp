#include <jee.h>
#include <arch/stm32f4-usb.h>

UsbDev console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinD<12> led;

int main() {
    fullSpeedClock();
    console.init();

    led.mode(Pinmode::out);
    led = 0;

    while (1) {
        printf("%d\n", ticks);
        led = 1;
        for (int i = 0; i < 10; ++i) { wait_ms(10); console.poll(); }
        led = 0;
        for (int i = 0; i < 40; ++i) { wait_ms(10); console.poll(); }
    }
}

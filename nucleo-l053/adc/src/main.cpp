#include <jee.h>

UartBufDev< PinA<2>, PinA<3> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<5> led;
PinA<0> ain;
ADC adc;
int aVal;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock());
    led.mode(Pinmode::out);

    adc.init();
    adc.window(1000, 3000); // define initial analog watchdog window

    // enable the analog watchdog, by passing in an interrupt handler
    // once outside range, change the range to avoid infinite interrupts
    adc.watch([]() {
        led = 1000 <= aVal && aVal <= 3000;

        if (aVal < 1000)
            adc.window(0, 999);     // low, trigger when it's in range again
        else if (aVal > 3000)
            adc.window(3001, 4095); // high, trigger when it's in range again
        else
            adc.window(1000, 3000); // trigger when it goes out of range

        MMIO32(adc.isr) = (1<<7);   // clear AWD interrupt flag
    });

    while (true) {
        aVal = adc.read(ain);
        printf("%d %d\n", ticks, aVal);
        wait_ms(500);
    }
}

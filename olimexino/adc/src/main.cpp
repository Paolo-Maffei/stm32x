#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<5> led;
PinC<0> ain;
ADC<1> adc;

int main() {
    (void) powerDown;
    (void) enableClkAt8MHz;
    console.init();
    console.baud(115200, fullSpeedClock());
    led.mode(Pinmode::out);

    adc.init();

    while (true) {
        int aVal = adc.read(ain);
#if 0
        printf("%d %d\n", ticks, aVal);
#else
        for (int i = 0; i < aVal; i += 60)
            console.putc('-');
        printf("+\n");
#endif
        wait_ms(10);
    }
}

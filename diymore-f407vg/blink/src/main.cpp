// Simple on-board LED blink demo, with periodic messages to the serial port.

#include <jee.h>

UartDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinE<0> led;

SysTick<168000000> tick;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    led.mode(Pinmode::out);

    // dummy loop timing test
    uint32_t t = tick.micros();
    for (int i = 0; i < 1000000; ++i) __asm("");
    uint32_t ns = tick.micros() - t;
    uint32_t ns2 = 1000000000/168000;
    printf("%d ps, %d ps, %d x1000, %d MHz\n",
            ns, ns2, 1000*ns/ns2, 1000000/ns);

    while (1) {
        printf("%d\n", ticks);
        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(400);
    }
}

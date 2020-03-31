#include <jee.h>

#if 0
UartDev< PinD<8>, PinD<9> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}
#endif

PinB<0> led;

int main() {
    //console.init();
    enableSysTick(); // no HSE crystal
    led.mode(Pinmode::out);

    while (true) {
        //printf("%d\n", ticks);
        led = 1;
        wait_ms(100);
        //for (int i = 0; i < 10000000; ++i) asm ("");
        led = 0;
        wait_ms(400);
        //for (int i = 0; i < 10000000; ++i) asm ("");
    }
}

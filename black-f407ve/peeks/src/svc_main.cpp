#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<6> led;
//PinA<7> led2;

int main () {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    led.mode(Pinmode::out);

// https://falstaff.agner.ch/2013/02/18/cortex-m3-supervisor-call-svc-using-gcc/
// ... but I'm not sure it's valid, since the code size *increases* 8 bytes
//  VTableRam().sv_call = []() __attribute__ (( naked )) {
    VTableRam().sv_call = []() {
        printf("%d\n", ticks);
    };

    while (true) {
        __asm("svc #0");
        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(400);
    }
}

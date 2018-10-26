#include <jee.h>

UartBufDev< PinA<2>, PinA<3> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinB<3> led;
CanDev can;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock());
    printf("hello\n");

    led.mode(Pinmode::out);

    can.init();
    can.filterInit(0);

    uint32_t last = 0;
    while (1) {
        if (ticks / 500 != last) {
            last = ticks / 500;
            printf("T %d\n", ticks);
            can.transmit(0x654, "a1b2c3d4", 8);
        }

        int len, id, dat[2];
        len = can.receive(&id, dat);
        if (len >= 0) {
            printf("R %d @%x #%d: %08x %08x\n", ticks, id, len, dat[0], dat[1]);
            led.toggle();
        }
    }
}

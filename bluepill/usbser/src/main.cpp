#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinC<13> led;
UsbDev< PinA<12> > usb;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock());
    led.mode(Pinmode::out);

    usb.init();

    while (true) {
        if (console.readable() && usb.writable())
            usb.putc(console.getc());
        if (usb.readable() && console.writable())
            console.putc(usb.getc());
    }
}

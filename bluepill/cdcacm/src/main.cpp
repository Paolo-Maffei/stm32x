#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

#include "usb.h"

int main () {
    // avoid "unused" warnings
    (void) enableClkAt8MHz;
    (void) powerDown;

    console.init();
    console.baud(115200, fullSpeedClock());

    // pulse PA12 low to force re-enumeration
    PinA<12> usbPin;
    usbPin.mode(Pinmode::out);
    wait_ms(2);
    usbPin.mode(Pinmode::in_float);

    usb.init();
    while (true) {
        if (console.readable() && usb.writable())
            usb.putc(console.getc());
        if (usb.readable() && console.writable())
            console.putc(usb.getc());
    }
}

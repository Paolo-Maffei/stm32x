// Simple bridge test for the mean-and-lean USB serial driver

#include <jee.h>
#include <jee/usb.h>

UartBufDev< PinA<9>, PinA<10> > console;
UsbDev usb;

int main() {
    console.init();
    console.baud(921600, fullSpeedClock()/2);
    usb.init();

    while (1) {
        if (usb.readable() && console.writable())
            console.putc(usb.getc());

        if (console.readable() && usb.writable())
            usb.putc(console.getc());
    }
}   

#include <jee.h>

UartDev< PinA<2>, PinA<15> > console;

PinB<3> led;

int main() {
    console.init();
    console.putc('+');
    led.mode(Pinmode::out);

    while (true) {
        console.putc(console.getc());
        led.toggle();
    }
}

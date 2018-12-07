// Toggle a GPIO pin as fast as possible.

#include <jee.h>

int main() {
    //fullSpeedClock();

    MMIO32(Periph::flash+0x00) |= 0b111<<8;

    PinB<13> led;
    led.mode(Pinmode::out_100mhz);

    while (false)
        led.toggle();

    while (false) {
        led.toggle();
        led.toggle();
        led.toggle();
        led.toggle();
        led.toggle();
        led.toggle();
        led.toggle();
        led.toggle();
    }

    while (false) {
        led = 0;
        led = 1;
    }

    while (true) {
        led = 1;
        led = 0;
        led = 1;
        led = 0;
        led = 1;
        led = 0;
        led = 1;
        led = 0;
        led = 1;
        led = 0;
        led = 1;
        led = 0;
        led = 1;
        led = 0;
        led = 1;
        led = 0;

        //for (int i = 0; i < 1000; ++i) __asm("");
    }
}

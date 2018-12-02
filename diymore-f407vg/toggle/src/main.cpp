// Toggle a GPIO pin as fast as possible.

#include <jee.h>

int main() {
    fullSpeedClock();

    PinB<13> pin;
    Pinmode m = (Pinmode) ((int)Pinmode::out + (int)Pinmode::ospeed_very_high);
    pin.mode(m);

    while (1) {
        pin = 1;
        pin = 0;
        pin = 1;
        pin = 0;
        pin = 1;
        pin = 0;
        pin = 1;
        pin = 0;
        pin = 1;
        pin = 0;
        pin = 1;
        pin = 0;
        pin = 1;
        pin = 0;
        pin = 1;
        pin = 0;
        for (int i = 0; i < 1000; ++i) __asm("");
    }
}

#include <jee.h>

Timer<3> timer;

int main() {
    fullSpeedClock();

    // generate a 9 MHz signal with 50% duty cycle on PB0, using TIM3
    PinB<0>::mode(Pinmode::alt_out);
    //PinB<0>::mode(Pinmode::alt_out_50mhz);
    timer.init(8);
    timer.pwm(4);

    while (true) {}
}

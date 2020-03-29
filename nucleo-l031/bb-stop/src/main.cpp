#include <jee.h>
#include <jee/spi-rf69.h>

SpiHw< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;
RF69< decltype(spi) > rf;
RTC rtc;

int main () {
    rtc.init(true);

    spi.init(0); // div=0 @ 2 MHz: 1 Mhz
    rf.init(63, 42, 8683);  // node 63, group 42, 868.3 MHz

    PinA<4> p4; p4=1; p4.mode(Pinmode::in_pullup);
    PinA<5> p5; p5=1; p5.mode(Pinmode::in_pullup);
    PinA<6> p6; p6=1; p6.mode(Pinmode::in_pullup);
    PinA<7> p7; p7=1; p7.mode(Pinmode::in_pullup);

    PinB<1> pR; pR=0; pR.mode(Pinmode::out); // rfm reset, active high

    powerDown(false);

    while (true) {} // should never get here
}

#include <jee.h>
#include <jee/spi-rf69.h>

SpiHw< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;
RF69< decltype(spi) > rf;
RTC rtc;

int main () {
    rtc.init(true);

    // the RFM's reset is not used here, it does not need to be initialised
    //PinB<1> rst; rst=0; rst.mode(Pinmode::out); // rfm reset, active high

    spi.init(0); // div=0 @ 2 MHz: 1 Mhz
    rf.init(63, 42, 8683);  // node 63, group 42, 868.3 MHz

    // don't let the MISO pin float, it will draw over 50 µA @ 2.7V (!)
    // this will need to be restored to Pinmode::out_od after resuming
    PinA<6> miso; miso=1; miso.mode(Pinmode::in_pullup);

    powerDown(false);

    while (true) {} // should never get here
}

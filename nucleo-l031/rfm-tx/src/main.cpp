#include <jee.h>
#include <jee/spi-rf69.h>

SpiHw< PinB<5>, PinB<4>, PinB<3>, PinA<11> > spi;  // for Nucleo-32
RF69< decltype(spi) > rf;

RTC rtc;

int main() {
    spi.init(0); // div=0 @ 2 MHz: 1 Mhz
    rf.init(63, 42, 8683);  // node 63, group 42, 868.3 MHz
    rf.txPower(0);

    rtc.init();
    rtc.wakeup(37000/16); // approx 1s, based on 37 kHz LSI clock

    while (true) {
        rtc.arm();
        powerDown(false);

        static int seq = 0;
        rf.send(0, &seq, sizeof seq);
        ++seq;
    }
}

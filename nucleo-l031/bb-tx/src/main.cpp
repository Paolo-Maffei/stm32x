#include <jee.h>
#include <jee/spi-rf69.h>

SpiHw< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;
RF69< decltype(spi) > rf;

RTC rtc;
PinA<1> led;

int main() {
    led.mode(Pinmode::out);
    spi.init(0); // div=0 @ 2 MHz: 1 Mhz
    rf.init(63, 42, 8683);  // node 63, group 42, 868.3 MHz
    rf.txPower(0);

    rtc.init();
    rtc.wakeup(37000/16); // approx 1s, based on 37 kHz LSI clock

    while (true) {
        rtc.arm();
        powerDown(false);

        led = 1;
        static int seq = 0;
        rf.send(0, &seq, sizeof seq);
        ++seq;
        led = 0;
    }
}

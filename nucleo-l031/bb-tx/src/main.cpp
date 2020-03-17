#include <jee.h>
#include <jee/spi-rf69.h>

SpiHw< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;
RF69< decltype(spi) > rf;

RTC rtc;

int main() {
    spi.init(0); // div=0 @ 2 MHz: 1 Mhz
    rf.init(63, 42, 8683);  // node 63, group 42, 868.3 MHz
    rf.txPower(0);

    rtc.init();

    int seq = 0;

    while (true) {
        rtc.wakeup(37000/16/4); // approx 0.25s, based on 37 kHz LSI clock

        // 25s of transmissions, one every 0.25s
        for (int i = 0; i < 100; ++i) {
            rtc.arm();
            powerDown(false);

            rf.send(0, &seq, sizeof seq);
            ++seq;
        }

        rtc.wakeup(37000/16*25); // approx 25s, based on 37 kHz LSI clock
        rtc.arm();
        powerDown(false);
    }
}

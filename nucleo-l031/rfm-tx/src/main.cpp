#include <jee.h>
#include <jee/spi-rf69.h>

PinA<0> dio0;

SpiHw< PinB<5>, PinB<4>, PinB<3>, PinA<11> > spi;  // for Nucleo-32
RF69< decltype(spi) > rf;

RTC rtc;

int main() {
    dio0.mode(Pinmode::in_float);

#if 0
    // RFM69 hard reset, so this code always starts from a known state
    PinB<1> rfrst;
    rfrst.mode(Pinmode::out);
    //rfrst = 1; wait_ms(2); rfrst = 0; wait_ms(10);
    rfrst = 1;
    rfrst = 0;
    for (int i = 0; i < 1000; ++i) asm ("");
#endif

    spi.init(0); // div=0 @ 16 MHz: 8 Mhz
    rf.init(63, 42, 8683);  // node 63, group 42, 868.3 MHz
    rf.txPower(3);

    rtc.init();
    rtc.wakeup(37000/16); // approx 1s, based on 37 kHz LSI clock

    MMIO32(Periph::exti+0x04) |= (1<<0); // EMR, unmask events PA0
    MMIO32(Periph::exti+0x0C) |= (1<<0); // FTSR, rising edge events

    while (true) {
        rtc.arm();
        powerDown(false);

        static int seq = 0;
        rf.send(0, &seq, sizeof seq);
        ++seq;

        powerDown(false);
        MMIO32(Periph::exti+0x14) = (1<<0); // clear events on PA0

        rf.sleep();
    }
}

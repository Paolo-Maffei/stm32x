#include <jee.h>

PinA<7> ledG;   PinB<7> keyG;
PinA<6> led1;   PinB<6> key1;
PinA<5> led2;   PinB<5> key2;
PinA<4> led3;   PinB<4> key3;
PinA<3> led4;   PinB<3> key4;
PinA<2> led5;
PinA<1> led6;

struct SingleWireCanDev : public CanDev {
    static void init () {
        MMIO32(Periph::afio + 0x04) |= (2<<13); // CAN remap to B9+B8
        Pin<'B',8>::mode(Pinmode::in_float);
        Pin<'B',9>::mode(Pinmode::alt_out_od);
        MMIO32(Periph::rcc + 0x1C) |= (1<<25);  // enable CAN1

        MMIO32(mcr) &= ~(1<<1); // exit sleep
        MMIO32(mcr) |= (1<<6) | (1<<0); // set ABOM, init req
        while ((MMIO32(msr) & (1<<0)) == 0) {}
        MMIO32(btr) = (6<<20) | (9<<16) | (59<<0); // 33.3 KBps
        MMIO32(mcr) &= ~(1<<0); // init leave
        while (MMIO32(msr) & (1<<0)) {}
        MMIO32(fmr) &= ~(1<<0); // ~FINIT
    }
};

SingleWireCanDev swcan;

int main() {
    enableSysTick();

    ledG.mode(Pinmode::out); ledG = 0;  keyG.mode(Pinmode::out); keyG = 0;
    led1.mode(Pinmode::out);            key1.mode(Pinmode::in_pullup);
    led2.mode(Pinmode::out);            key2.mode(Pinmode::in_pullup);
    led3.mode(Pinmode::out);            key3.mode(Pinmode::in_pullup);
    led4.mode(Pinmode::out);            key4.mode(Pinmode::in_pullup);
    led5.mode(Pinmode::out);
    led6.mode(Pinmode::out);

    // disable JTAG in AFIO-MAPR to release PB3, PB4, and PA15
    constexpr uint32_t afio = 0x40010000;
    MMIO32(afio + 0x04) |= 1 << 25; // disable JTAG, keep SWD enabled

    swcan.init();
    swcan.filterInit(0);

    uint32_t last = 0;
    while (1) {
        // send key state once every 50 ms
        if (ticks / 50 != last) {
            last = ticks / 50;

            uint8_t keys = (!key1<<0) | (!key2<<1) | (!key3<<2) | (!key4<<3);
            swcan.transmit(0x123, &keys, 1);

            led5.toggle();
        }

        // process incoming messages
        int id;
        uint8_t dat[8];
        if (swcan.receive(&id, dat) == 1) {
            led1 = (dat[0] >> 0) & 1;
            led2 = (dat[0] >> 1) & 1;
            led3 = (dat[0] >> 2) & 1;
            led4 = (dat[0] >> 3) & 1;

            led6.toggle();
        }
    }
}

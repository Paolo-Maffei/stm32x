#include <jee.h>

// choose breadboard version (1) or 4-colour test board (0)
#if 1
constexpr auto pullMode = Pinmode::in_pullup;
PinA<7> ledG;   PinB<7> keyG;
PinA<6> led1;   PinB<6> key1;
PinA<5> led2;   PinB<5> key2;
PinA<4> led3;   PinB<4> key3;
PinA<3> led4;   PinB<3> key4;
#else
constexpr auto pullMode = Pinmode::in_pulldown;
PinB<10> ledG;  PinB<11> keyG;  // unused
PinA<0> led1;   PinA<4> key1;
PinA<1> led2;   PinA<5> key2;
PinA<2> led3;   PinA<6> key3;
PinA<3> led4;   PinA<7> key4;
#endif

struct SingleWireCanDev : public CanDev {
    static void swInit () {
        Pin<'A',11>::mode(Pinmode::in_float);
        Pin<'A',12>::mode(Pinmode::alt_out_od_2mhz);
        MMIO32(Periph::rcc + 0x1C) |= (1<<25);  // enable CAN1

        MMIO32(mcr) &= ~(1<<1); // exit sleep
        MMIO32(mcr) |= (1<<6) | (1<<0); // set ABOM, init req
        while ((MMIO32(msr) & (1<<0)) == 0) {}
        MMIO32(btr) = (6<<20) | (9<<16) | (1<<0); // 83.3 KBps
        MMIO32(mcr) &= ~(1<<0); // init leave
        while (MMIO32(msr) & (1<<0)) {}
        MMIO32(fmr) &= ~(1<<0); // ~FINIT
    }
};

SingleWireCanDev can;

int main() {
    fullSpeedClock();

    ledG.mode(Pinmode::out); ledG = 0;  keyG.mode(Pinmode::out); keyG = 0;
    led1.mode(Pinmode::out);            key1.mode(pullMode);
    led2.mode(Pinmode::out);            key2.mode(pullMode);
    led3.mode(Pinmode::out);            key3.mode(pullMode);
    led4.mode(Pinmode::out);            key4.mode(pullMode);

    // disable JTAG in AFIO-MAPR to release PB3, PB4, and PA15
    constexpr uint32_t afio = 0x40010000;
    MMIO32(afio + 0x04) |= 1 << 25; // disable JTAG, keep SWD enabled

    // test board has a slider to select CAN (PB0=0) or SW-CAN (PB0=1)
    // whereas the breadboard version always uses SW-CAN
    PinB<0> single;
    single.mode(Pinmode::in_pullup);

    if (single)
        can.swInit(); // PA11+PA12
    else
        can.init(true); // PB8+PB9
    can.filterInit(0);

    uint32_t last = 0;
    while (1) {
        // send key state once every 500 ms
        if (ticks / 500 != last) {
            last = ticks / 500;

            uint8_t keys = (!key1<<0) | (!key2<<1) | (!key3<<2) | (!key4<<3);
            can.transmit(0x123, &keys, 1);
        }

        // process incoming messages
        int id;
        uint8_t dat[8];
        if (can.receive(&id, dat) == 1) {
            led1 = (dat[0] >> 0) & 1;
            led2 = (dat[0] >> 1) & 1;
            led3 = (dat[0] >> 2) & 1;
            led4 = (dat[0] >> 3) & 1;
        }
    }
}

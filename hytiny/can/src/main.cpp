#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<1> led;

struct CanDev {
    constexpr static uint32_t base = 0x40006400;

    constexpr static uint32_t mcr  = base + 0x000;
    constexpr static uint32_t msr  = base + 0x004;
    constexpr static uint32_t tsr  = base + 0x008;
    constexpr static uint32_t rfr  = base + 0x00C;
    constexpr static uint32_t btr  = base + 0x01C;
    constexpr static uint32_t tir  = base + 0x180;
    constexpr static uint32_t tdtr = base + 0x184;
    constexpr static uint32_t tdlr = base + 0x188;
    constexpr static uint32_t tdhr = base + 0x18C;
    constexpr static uint32_t rir  = base + 0x1B0;
    constexpr static uint32_t rdtr = base + 0x1B4;
    constexpr static uint32_t rdlr = base + 0x1B8;
    constexpr static uint32_t rdhr = base + 0x1BC;
    constexpr static uint32_t fmr  = base + 0x200;
    constexpr static uint32_t fsr  = base + 0x20C;
    constexpr static uint32_t far  = base + 0x21C;
    constexpr static uint32_t fr1  = base + 0x240;
    constexpr static uint32_t fr2  = base + 0x244;

    static void init (bool alt =false) {
		if (alt) {
			MMIO32(Periph::afio + 0x04) |= (2<<13); // CAN remap to B9+B8
			PinB<8>::mode(Pinmode::in_float);
			PinB<9>::mode(Pinmode::alt_out);
		} else {
			PinA<11>::mode(Pinmode::in_float);
			PinA<12>::mode(Pinmode::alt_out);
		}
        MMIO32(Periph::rcc + 0x1C) |= (1<<25);  // enable CAN1

        MMIO32(mcr) &= ~(1<<1); // exit sleep
        MMIO32(mcr) |= (1<<6) | (1<<0); // set ABOM, init req
        while ((MMIO32(msr) & (1<<0)) == 0) {}
        //MMIO32(btr) = (7<<20) | (5<<16) | (2<<0); // 1 MBps
        MMIO32(btr) = (6<<20) | (9<<16) | (1<<0); // 1 MBps
        MMIO32(mcr) &= ~(1<<0); // init leave
        while (MMIO32(msr) & (1<<0)) {}
        MMIO32(fmr) &= ~(1<<0); // ~FINIT
    }

    static void filterInit (int num, int id =0, int mask =0) {
        MMIO32(far) &= ~(1<<num); // ~FACT
        MMIO32(fsr) |= (1<<num); // FSC 32b
        MMIO32(fr1 + 8 * num) = id;
        MMIO32(fr2 + 8 * num) = mask;
        MMIO32(far) |= (1<<num); // FACT
    }

    static void transmit (int id, const void* ptr, int len) {
        if (MMIO32(tsr) & (1<<26)) { // TME0
            MMIO32(tir) = (id<<21);
            MMIO32(tdtr) = (len<<0);
            // this assumes that misaligned word access works
            MMIO32(tdlr) = ((const uint32_t*) ptr)[0];
            MMIO32(tdhr) = ((const uint32_t*) ptr)[1];

            MMIO32(tir) |= (1<<0); // TXRQ
        }
    }

    static int receive (int* id, void* ptr) {
        int len = -1;
        if (MMIO32(rfr) & (3<<0)) { // FMP
            *id = MMIO32(rir) >> 21;
            len = MMIO32(rdtr) & 0x0F;
            ((uint32_t*) ptr)[0] = MMIO32(rdlr);
            ((uint32_t*) ptr)[1] = MMIO32(rdhr);
            MMIO32(rfr) |= (1<<5); // RFOM
        }
        return len;
    }
};

CanDev can;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock());
    led.mode(Pinmode::out);
    printf("hello\n");

    can.init();
    can.filterInit(0);

    uint32_t last = 0;
    while (1) {
        if (ticks / 500 != last) {
            last = ticks / 500;
            printf("T %d\n", ticks);
            can.transmit(0x456, "a1b2c3d4", 8);
        }

        int len, id, dat[2];
        len = can.receive(&id, dat);
        if (len >= 0) {
            printf("R %d @%x #%d: %08x %08x\n", ticks, id, len, dat[0], dat[1]);
            led.toggle();
        }
    }
}

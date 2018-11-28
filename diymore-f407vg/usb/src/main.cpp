// My feeble attempt to get USB going on F4 ...

#include <jee.h>

UartBufDev< PinA<9>, PinA<10>, 5000 > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinE<0> led;

namespace Periph {
    constexpr uint32_t usb   = 0x50000000;
}

namespace USB {
    constexpr uint32_t base      = 0x50000000;
    constexpr uint32_t GAHBCFG   = base + 0x008;  // p.1275
    constexpr uint32_t GUSBCFG   = base + 0x00C;  // p.1276
    constexpr uint32_t GINTSTS   = base + 0x014;  // p.1280
    constexpr uint32_t GINTMSK   = base + 0x018;  // p.1284
    constexpr uint32_t GRXSTSP   = base + 0x020;  // p.1287
    constexpr uint32_t GRXFSIZ   = base + 0x024;  // p.1288
    constexpr uint32_t DIEPTXF0  = base + 0x028;  // p.1289
    constexpr uint32_t GCCFG     = base + 0x038;  // p.1290
    constexpr uint32_t DCFG      = base + 0x800;  // p.1303
    constexpr uint32_t DCTL      = base + 0x804;  // p.1304
    constexpr uint32_t DSTS      = base + 0x808;  // p.1305
    constexpr uint32_t DAINT     = base + 0x818;  // p.1305
    constexpr uint32_t DIEPCTL0  = base + 0x900;  // p.1310
    constexpr uint32_t DIEPTSIZ0 = base + 0x910;  // p.1321
    constexpr uint32_t DOEPCTL0  = base + 0xB00;  // p.1316
    constexpr uint32_t DOEPTSIZ0 = base + 0xB10;  // p.1323
    constexpr uint32_t PCGCCTL   = base + 0xE00;  // p.1326

    struct EndPointBase {
        static uint16_t limit;

        
    };

    uint16_t EndPointBase::limit;

    void init () {
        Port<'A'>::modeMap(0b0001100000000000, Pinmode::alt_out, 10);

        MMIO32(Periph::rcc + 0x34) |= (1<<7);  // OTGFSEN
        wait_ms(2); // added, because otherwise GINTSTS is still zero
        printf("00 GINTSTS %08x\n", MMIO32(GINTSTS));

        MMIO32(GCCFG) |= (1<<21) | (1<<16);  // NOVBUSSENS, PWRDWN
        MMIO32(GUSBCFG) |= (1<<30);  // FDMOD
        MMIO32(DCFG) |= (3<<0);  // DSPD

        //while ((MMIO32(GINTSTS) & (1<<12)) == 0) {}  // USBRST
        //while ((MMIO32(GINTSTS) & (1<<13)) == 0) {}  // ENUMDNE

        printf("10 GINTSTS %08x DSTS %08x DCTL %08x\n",
                MMIO32(GINTSTS), MMIO32(DSTS), MMIO32(DCTL));
    }

    void poll () {
        uint32_t irq = MMIO32(GINTSTS) & ~0x04008028;
        MMIO32(GINTSTS) = irq;  // clear all interrupts
        if (irq)
            printf("irq %08x\n", irq);

        if (irq & (1<<13)) {  // ENUMDNE
#if 0
            printf("11 GINTSTS %08x DSTS %08x DCTL %08x\n",
                    MMIO32(GINTSTS), MMIO32(DSTS), MMIO32(DCTL));
#endif
            // see p.1354
            MMIO32(DIEPCTL0) = 0;
            MMIO32(DOEPCTL0) = (1<<15);  // USBAEP

            MMIO32(DOEPCTL0 + 0x20) |= (1<<27);  // SNAK ep1
            MMIO32(DOEPCTL0 + 0x40) |= (1<<27);  // SNAK ep2
            MMIO32(DOEPCTL0 + 0x60) |= (1<<27);  // SNAK ep3

            MMIO32(DIEPTSIZ0) = 64;
            MMIO32(DIEPCTL0) |= (1<<31) | (1<<27);  // EPENA, SNAK

            MMIO32(DOEPTSIZ0) = (3<<29) | (1<<19) | 64;  // STUPCNT, PKTCNT
            MMIO32(DOEPCTL0) |= (1<<31) | (1<<27);  // EPENA, SNAK

            MMIO32(GRXFSIZ) = 512/4;  // 512b for receive FIFO
            MMIO32(DIEPTXF0) = (64/4<<16) | 512;  // then 64b for TX0
#if 0
            printf("12 GINTSTS %08x DSTS %08x\n",
                    MMIO32(GINTSTS), MMIO32(DSTS));
#endif
            return;
        }

        if (irq & (1<<4)) {
            printf("rx GINTSTS %08x DSTS %08x\n",
                    MMIO32(GINTSTS), MMIO32(DSTS));
            uint32_t rx = MMIO32(GRXSTSP);
            uint32_t rxtyp = (rx>>17) & 0xF;
            printf("GRXSTSP %08x type %x\n", rx, rxtyp);

            static union {
                struct {
                    uint8_t typ;
                    uint8_t req;
                    uint16_t val;
                    uint16_t idx;
                    uint16_t len;
                };
                uint32_t buf [2];
            } setup;
            
            switch (rxtyp) {
                case 0b0110:  // SETUP
                    setup.buf[0] = MMIO32(base + 0x1000);
                    setup.buf[1] = MMIO32(base + 0x1000);
                    printf("setup %08x %08x\n", setup.buf[0], setup.buf[1]);
                    break;
                case 0b0100:  // SETUP complete
                    printf("setup complete %d %d %d %d %d\n",
                        setup.typ, setup.req, setup.val, setup.idx, setup.len);
                    if (setup.req == 5) {  // set address
                        MMIO32(DCFG) &= ~(0x7F<<4);
                        MMIO32(DCFG) |= (setup.val<<4);
                    }
                    break;
            }

            MMIO32(DOEPTSIZ0) = 64;
            MMIO32(DOEPCTL0) |= (1<<31) | (1<<26);  // EPENA, CNAK

            static int i = 30;
            if (--i <= 0)
                while (1) {}
        }
    }
};

using namespace USB;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    init();

    led.mode(Pinmode::out);
    while (1) {
        led = (ticks/100) % 10; // 100 ms on, 900 ms off, inverted logic
        poll();
    }
}

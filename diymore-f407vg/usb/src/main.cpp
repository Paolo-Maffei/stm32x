// My feeble attempt to get USB going on F4 ...

#include <jee.h>

UartBufDev< PinA<9>, PinA<10>, 5000 > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

//#define printf(...)

PinE<0> led;

namespace Periph {
    constexpr uint32_t usb   = 0x50000000;
}

namespace USB {
    constexpr uint32_t base      = 0x50000000;
    constexpr uint32_t GOTGINT   = base + 0x004;  // p.1273
    constexpr uint32_t GAHBCFG   = base + 0x008;  // p.1275
    constexpr uint32_t GUSBCFG   = base + 0x00C;  // p.1276
    constexpr uint32_t GINTSTS   = base + 0x014;  // p.1280
    constexpr uint32_t GINTMSK   = base + 0x018;  // p.1284
    constexpr uint32_t GRXSTSP   = base + 0x020;  // p.1287
    constexpr uint32_t GRXFSIZ   = base + 0x024;  // p.1288
    constexpr uint32_t DIEPTXF0  = base + 0x028;  // p.1289
    constexpr uint32_t GCCFG     = base + 0x038;  // p.1290
    constexpr uint32_t DIEPTXF1  = base + 0x104;  // p.1292
    constexpr uint32_t DCFG      = base + 0x800;  // p.1303
    constexpr uint32_t DCTL      = base + 0x804;  // p.1304
    constexpr uint32_t DSTS      = base + 0x808;  // p.1305
    constexpr uint32_t DAINT     = base + 0x818;  // p.1305
    constexpr uint32_t DIEPCTL0  = base + 0x900;  // p.1310
    constexpr uint32_t DIEPINT0  = base + 0x908;  // p.1319
    constexpr uint32_t DIEPTSIZ0 = base + 0x910;  // p.1321
    constexpr uint32_t DOEPCTL0  = base + 0xB00;  // p.1316
    constexpr uint32_t DOEPINT0  = base + 0xB08;  // p.1319
    constexpr uint32_t DOEPTSIZ0 = base + 0xB10;  // p.1323
    constexpr uint32_t PCGCCTL   = base + 0xE00;  // p.1326

    const uint8_t devDesc [] = {
        18, 1, 0, 2, 2, 0, 0, 64,
        /* vendor: */ 0x83, 0x04, /* product: */ 0x40, 0x57,
        0, 2, 0, 0, 0, 1,
    };

    const uint8_t cnfDesc [] = { // total length = 67 bytes
        9, 2, 67, 0, 2, 1, 0, 192, 50, // USB Configuration
        9, 4, 0, 0, 1, 2, 2, 0, 0,     // Interface
        5, 36, 0, 16, 1,               // Header Functional
        5, 36, 1, 0, 1,                // Call Management Functional
        4, 36, 2, 2,                   // ACM Functional
        5, 36, 6, 0, 1,                // Union Functional
        7, 5, 130, 3, 8, 0, 255,       // Endpoint 2
        9, 4, 1, 0, 2, 10, 0, 0, 0,    // Data class interface
        7, 5, 1, 2, 64, 0, 0,          // Endpoint 1 out
        7, 5, 129, 2, 64, 0, 0,        // Endpoint 1 in
    };

    union {
        struct { uint8_t typ, req; uint16_t val, idx, len; };
        uint32_t buf [2];
    } setupPkt;

    volatile uint32_t& fifo (int ep) {
        return MMIO32(base + (ep+1) * 0x1000);
    }

    void sendEp0 (void const* ptr, uint32_t len) {
        if (len > setupPkt.len)
            len = setupPkt.len;

        MMIO32(DIEPTSIZ0) = len;
        MMIO32(DIEPCTL0) |= (1<<31) | (1<<26);  // EPENA, CNAK

        uint32_t const* wptr = (uint32_t const*) ptr;
        for (uint32_t i = 0; i < len; i += 4)
            fifo(0) = *wptr++;
    }

    void setConfig () {
        MMIO32(DOEPTSIZ0 + 0x20) = 64;  // accept 64b on RX ep1
        MMIO32(DOEPTSIZ0 + 0x40) = 64;  // accept 64b on RX ep2

        MMIO32(DOEPCTL0 + 0x20) = (3<<18) | (1<<15) | 64  // BULK ep1
                                | (1<<31) | (1<<26);      // EPENA, CNAK
        MMIO32(DOEPCTL0 + 0x40) = (2<<18) | (1<<15) | 64  // INTR ep2
                                | (1<<31) | (1<<26);      // EPENA, CNAK
    }

    void init () {
        PinA<12> usbPin;
        usbPin.mode(Pinmode::out_od);
        wait_ms(3);

        Port<'A'>::modeMap(0b0001100000000000, Pinmode::alt_out, 10);

        MMIO32(Periph::rcc + 0x34) |= (1<<7);  // OTGFSEN
        wait_ms(2); // added, because otherwise GINTSTS is still zero
        printf("00 GINTSTS %08x\n", MMIO32(GINTSTS));

        MMIO32(GCCFG) |= (1<<21) | (1<<16);  // NOVBUSSENS, PWRDWN
        MMIO32(GUSBCFG) |= (1<<30);  // FDMOD
        MMIO32(DCFG) &= ~(0x7F<<4);  // clear DAD
        MMIO32(DCFG) |= (3<<0);  // DSPD

        printf("10 GINTSTS %08x DSTS %08x DCTL %08x\n",
                MMIO32(GINTSTS), MMIO32(DSTS), MMIO32(DCTL));
    }

    void poll () {
        uint32_t irq = MMIO32(GINTSTS);
        if (irq & ~0x04008028)
            printf("irq %08x\n", irq);
        MMIO32(GINTSTS) = irq;  // clear all interrupts

        if (irq & (1<<7)) {  // GONAKEFF
            printf("GONAKEFF: DCTL %08x\n", MMIO32(DCTL));
            MMIO32(DCTL) |= (1<<10);  // CGONAK
        }

        if (irq & (1<<2)) {  // OTGINT, needed?
            printf("GOTGINT %08x\n", MMIO32(GOTGINT));
            MMIO32(GOTGINT) = MMIO32(GOTGINT);
        }

        if (irq & (1<<12))  // USBRST
            printf("usbrst\n");

        if (irq & (1<<13)) {  // ENUMDNE
            printf("enumdne\n");
            printf("11 GINTSTS %08x DSTS %08x DCTL %08x DOEPINT0 %08x\n",
                    MMIO32(GINTSTS), MMIO32(DSTS),
                    MMIO32(DCTL), MMIO32(DOEPINT0));

            // see p.1354
            MMIO32(DIEPCTL0 + 0x20) = (1<<22)| (3<<18) | (1<<15) | 64;
            MMIO32(DOEPCTL0 + 0x20) |= (1<<27);     // SNAK ep1
            MMIO32(DOEPCTL0 + 0x40) |= (1<<27);     // SNAK ep2

            MMIO32(DOEPTSIZ0) = (3<<29) | 64;       // STUPCNT, XFRSIZ
            MMIO32(DOEPCTL0) = (1<<31) | (1<<15) | (1<<27); // EPENA, SNAK

            MMIO32(GRXFSIZ)    = 512/4;                     // 512b for RX all
            MMIO32(DIEPTXF0)   = (128/4<<16) | 512;         // 128b for TX ep0
            MMIO32(DIEPTXF1 + 0) = (512/4<<16) | (512+128); // 512b for TX ep1
            MMIO32(DIEPTXF1 + 4) = (128/4<<16) | (512+640); // 128b for TX ep2
        }

        if (irq & (1<<18))  // IEPINT
            printf("iepint DAINT %08x\n", MMIO32(DAINT));
        if (irq & (1<<19))  // OEPINT
            printf("oepint DAINT %08x\n", MMIO32(DAINT));

        const char* isep = "";
        for (int i = 0; i < 4; ++i) {
            uint32_t v = MMIO32(DIEPINT0 + 0x20*i);
            if (v & 1) {
                if (*isep == 0)
                    printf("DIEPINT0:");
                printf("   %d %08x", i, v);
                isep = "\n";
                MMIO32(DIEPINT0 + 0x20*i) = v;
            }
        }
        printf(isep);

        const char* osep = "";
        for (int i = 0; i < 4; ++i) {
            uint32_t v = MMIO32(DOEPINT0 + 0x20*i);
            if (v & 1) {
                if (*osep == 0)
                    printf("DOEPINT0:");
                printf("   %d %08x", i, v);
                osep = "\n";
                MMIO32(DOEPINT0 + 0x20*i) = v;

                // FIXME hard-coded hack, to restart accepting out packets
                MMIO32(DOEPCTL0 + 0x20) |= (3<<18) | (1<<15) | 64;
                MMIO32(DOEPCTL0 + 0x20) |= (1<<31) | (1<<26);  // EPENA, CNAK
            }
        }
        printf(osep);

        if (irq & (1<<4)) {
            //printf("rx GINTSTS %08x DSTS %08x\n",
            //        MMIO32(GINTSTS), MMIO32(DSTS));
            int rx = MMIO32(GRXSTSP), typ = (rx>>17) & 0xF,
                ep = rx & 0x0F, cnt = (rx>>4) & 0x7FF;
            printf("rx %08x typ %d cnt %d ep %d\n", rx, typ, cnt, ep);

            switch (typ) {
                case 0b0010:  // OUT
                    printf("out %d\n", ep);
                    break;
                case 0b0011:  // OUT complete
                    printf("out complete %d\n", ep);
                    break;
                case 0b0110:  // SETUP
                    setupPkt.buf[0] = fifo(0);
                    setupPkt.buf[1] = fifo(0);
                    cnt -= 8;
                    //printf("setup %08x %08x\n",
                    //        setupPkt.buf[0], setupPkt.buf[1]);
                    break;
                case 0b0100:  // SETUP complete
                    printf("setup complete t %d r %d v %d i %d l %d\n",
                                setupPkt.typ, setupPkt.req,
                                setupPkt.val, setupPkt.idx, setupPkt.len);
                    switch (setupPkt.req) {
                        case 5:  // set address
                            printf("set address %d\n", setupPkt.val);
                            MMIO32(DCFG) &= ~(0x7F<<4);  // clear DAD
                            MMIO32(DCFG) |= (setupPkt.val<<4);
                            sendEp0(0, 0);
                            break;
                        case 6:  // get descriptor
                            printf("get descriptor v %04x i %d #%d\n",
                                    setupPkt.val, setupPkt.len, setupPkt.len);
                            switch (setupPkt.val) {
                                case 0x100:  // device desc
                                    sendEp0(devDesc, sizeof devDesc);
                                    break;
                                case 0x200:  // configuration desc
                                    sendEp0(cnfDesc, sizeof cnfDesc);
                                    break;
                                default:
                                    // TODO makes no difference?
                                    MMIO32(DIEPCTL0) |= (1<<21);  // STALL
                                    sendEp0(0, 0);
                                    break;
                            }
                            break;
                        case 9:  // set configuration
                            setConfig();
                        case 34:  // set control line state
                            printf("rts/dtr %02b\n", setupPkt.val);
                        case 32:  // set line coding
                        //default:
                            sendEp0(0, 0);
                            break;
                    }
                    break;
            }

            for (int i = 0; i < cnt; i += 4) {
                uint32_t x = fifo(0);
                printf("drop %d %08x\n", i, x);
                //console.putc('?');
            }

            MMIO32(DOEPTSIZ0) = (3<<29) | 64;  // STUPCNT, XFRSIZ
            MMIO32(DOEPCTL0) |= (1<<31) | (1<<26);  // EPENA, CNAK
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

        if (console.readable()) {
            int c = console.getc();
            printf("got %d\n", c);

            MMIO32(DIEPTSIZ0 + 0x20) = 4;
            MMIO32(DIEPCTL0 + 0x20) |= (1<<31) | (1<<26);  // EPENA, CNAK
            fifo(1) = 0x41424344;
            printf("DIEPTSIZ0 %08x\n", MMIO32(DIEPTSIZ0));
        }
    }
}

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
    constexpr uint32_t GOTGINT   = base + 0x004;  // p.1273
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

    static struct {
        uint8_t len, typ; uint16_t usb;
        uint8_t dclass, dsub, proto, size;
        uint16_t vendor, product, device;
        uint8_t smfct, sprod, serial, confs;
    } devDesc = {
        sizeof devDesc, 1, 0x0200,
        2, 0, 0, 64,
        0x0483, 0x5740, 0x0200,
        0, 0, 0, 1,
    };

    const uint8_t confDesc [] = { // total length = 67 bytes
        // USB Configuration Descriptor
        9,   // bLength: Configuration Descriptor size
        0x02, // USB_CONFIGURATION_DESCRIPTOR_TYPE
        67,  // VIRTUAL_COM_PORT_SIZ_CONFIG_DESC
        0,
        2,   // bNumInterfaces: 2 interface
        1,   // bConfigurationValue
        0,   // iConfiguration
        0xC0, // bmAttributes: self powered
        0x32, // MaxPower 0 mA
        // Interface Descriptor
        9,   // bLength: Interface Descriptor size
        0x04, // USB_INTERFACE_DESCRIPTOR_TYPE
        0x00, // bInterfaceNumber: Number of Interface
        0x00, // bAlternateSetting: Alternate setting
        0x01, // bNumEndpoints: One endpoints used
        0x02, // bInterfaceClass: Communication Interface Class
        0x02, // bInterfaceSubClass: Abstract Control Model
        0x01, // bInterfaceProtocol: Common AT commands
        0x00, // iInterface:
        // Header Functional Descriptor
        5,   // bLength: Endpoint Descriptor size
        0x24, // bDescriptorType: CS_INTERFACE
        0x00, // bDescriptorSubtype: Header Func Desc
        0x10, // bcdCDC: spec release number
        0x01,
        // Call Management Functional Descriptor
        5,   // bFunctionLength
        0x24, // bDescriptorType: CS_INTERFACE
        0x01, // bDescriptorSubtype: Call Management Func Desc
        0x00, // bmCapabilities: D0+D1
        0x01, // bDataInterface: 1
        // ACM Functional Descriptor
        4,   // bFunctionLength
        0x24, // bDescriptorType: CS_INTERFACE
        0x02, // bDescriptorSubtype: Abstract Control Management desc
        0x02, // bmCapabilities
        // Union Functional Descriptor
        5,   // bFunctionLength
        0x24, // bDescriptorType: CS_INTERFACE
        0x06, // bDescriptorSubtype: Union func desc
        0x00, // bMasterInterface: Communication class interface
        0x01, // bSlaveInterface0: Data Class Interface
        // Endpoint 2 Descriptor
        7,   // bLength: Endpoint Descriptor size
        0x05, // USB_ENDPOINT_DESCRIPTOR_TYPE
        0x82, // bEndpointAddress: (IN2)
        0x03, // bmAttributes: Interrupt
        8,   // VIRTUAL_COM_PORT_INT_SIZE
        0,
        0xFF, // bInterval:
        // Data class interface descriptor
        9,   // bLength: Endpoint Descriptor size
        0x04, // USB_INTERFACE_DESCRIPTOR_TYPE
        0x01, // bInterfaceNumber: Number of Interface
        0x00, // bAlternateSetting: Alternate setting
        0x02, // bNumEndpoints: Two endpoints used
        0x0A, // bInterfaceClass: CDC
        0x00, // bInterfaceSubClass:
        0x00, // bInterfaceProtocol:
        0x00, // iInterface:
        // Endpoint 3 Descriptor
        7,   // bLength: Endpoint Descriptor size
        0x05, // USB_ENDPOINT_DESCRIPTOR_TYPE
        0x03, // bEndpointAddress: (OUT3)
        0x02, // bmAttributes: Bulk
        64,  // VIRTUAL_COM_PORT_DATA_SIZE
        0,
        0x00, // bInterval: ignore for Bulk transfer
        // Endpoint 1 Descriptor
        7,   // bLength: Endpoint Descriptor size
        0x05, // USB_ENDPOINT_DESCRIPTOR_TYPE
        0x81, // bEndpointAddress: (IN1)
        0x02, // bmAttributes: Bulk
        64,  // VIRTUAL_COM_PORT_DATA_SIZE
        0,
        0x00, // bInterval
    };

    void init () {
        PinA<12> usbPin;
        usbPin.mode(Pinmode::out_od);
        usbPin = 0;
        wait_ms(5);
        usbPin = 1;
        wait_ms(5);

        Port<'A'>::modeMap(0b0001100000000000, Pinmode::alt_out, 10);

        MMIO32(Periph::rcc + 0x34) |= (1<<7);  // OTGFSEN
        wait_ms(2); // added, because otherwise GINTSTS is still zero
        printf("00 GINTSTS %08x\n", MMIO32(GINTSTS));

        MMIO32(GCCFG) |= (1<<21) | (1<<16);  // NOVBUSSENS, PWRDWN
        MMIO32(GUSBCFG) |= (1<<30);  // FDMOD
        MMIO32(DCFG) &= ~(0x7F<<4);  // clear DAD
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

        if (irq & (1<<2)) { //  needed?
            printf("GOTGINT %08x\n", MMIO32(GOTGINT));
            MMIO32(GOTGINT) = MMIO32(GOTGINT);
        }

        if (irq & (1<<13)) {  // ENUMDNE
            printf("enumdne\n");
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
                    uint8_t typ, req;
                    uint16_t val, idx, len;
                };
                uint32_t buf [2];
            } setup;
    
            switch (rxtyp) {
                case 0b0010:  // IN
                    printf("in\n");
                    break;
                case 0b0011:  // IN complete
                    printf("in complete\n");
                    break;
                case 0b0110:  // SETUP
                    setup.buf[0] = MMIO32(base + 0x1000);
                    setup.buf[1] = MMIO32(base + 0x1000);
                    printf("setup %08x %08x\n", setup.buf[0], setup.buf[1]);
                    break;
                case 0b0100:  // SETUP complete
                    printf("setup complete t %d r %d v %d i %d l %d\n",
                        setup.typ, setup.req, setup.val, setup.idx, setup.len);
                    switch (setup.req) {
                        case 0:  // get status
                            printf("get status\n");
            MMIO32(DIEPTSIZ0) = 2;
            MMIO32(DIEPCTL0) |= (1<<31) | (1<<26);  // EPENA, CNAK
            MMIO32(base + 0x1000) = 0;
                            break;
                        case 1:  // clear feature
                            printf("clear feature\n");
            MMIO32(DIEPTSIZ0) = 0;
            MMIO32(DIEPCTL0) |= (1<<31) | (1<<26);  // EPENA, CNAK
                            break;
                        case 5:  // set address
                            printf("set address %d\n", setup.val);
                            //MMIO32(DCFG) &= ~(0x7F<<4);
                            MMIO32(DCFG) |= (setup.val<<4);
            MMIO32(DIEPTSIZ0) = 0;
            MMIO32(DIEPCTL0) |= (1<<31) | (1<<26);  // EPENA, CNAK
                            break;
                        case 6:  // get descriptor
                            printf("get descriptor v %04x i %d #%d\n",
                                    setup.val, setup.len, setup.len);
                            switch (setup.val) {
                                case 0x100:  // device desc
            MMIO32(DIEPTSIZ0) = sizeof devDesc;
            MMIO32(DIEPCTL0) |= (1<<31) | (1<<26);  // EPENA, CNAK
            for (uint32_t i = 0; i < sizeof devDesc; i += 4)
                MMIO32(base + 0x1000) = ((uint32_t*) &devDesc)[i/4];
                                    break;
                                default:
            MMIO32(DIEPTSIZ0) = 0;
            MMIO32(DIEPCTL0) |= (1<<31) | (1<<26);  // EPENA, CNAK
                                    break;
                            }
                            break;
                    }
                    break;
            }

            //MMIO32(DOEPTSIZ0) = 64;
            MMIO32(DOEPTSIZ0) = (3<<29) | (1<<19) | 64;  // STUPCNT, PKTCNT
            MMIO32(DOEPCTL0) |= (1<<31) | (1<<26);  // EPENA, CNAK

            static int i = 50;
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
    wait_ms(1000);

    init();

    led.mode(Pinmode::out);
    while (1) {
        led = (ticks/100) % 10; // 100 ms on, 900 ms off, inverted logic
        poll();
    }
}

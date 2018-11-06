#include <jee.h>
#include <string.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinA<7> ledG;   PinB<7> keyG;
PinA<6> led1;   PinB<6> key1;
PinA<5> led2;   PinB<5> key2;
PinA<4> led3;   PinB<4> key3;
PinA<3> led4;   PinB<3> key4;

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

template< typename BUS >
class Node {
    uint8_t myHwId [8];
    uint16_t mySlot;
    uint32_t whoisClick;

    //enum { WHOIS = 0x7A0, ASSIGN = 0x7C0, UPLOAD = 0x7E0,
    //        QUERY = 0x7FF, VERSION = 0x7FF };

    static constexpr uint16_t WHOIS   = 0x7A0; // node -> central
    static constexpr uint16_t ASSIGN  = 0x7C0; // central -> node
    static constexpr uint16_t UPLOAD  = 0x7E0; // central -> node
    static constexpr uint16_t QUERY   = 0x7FF; // central -> all
    static constexpr uint16_t VERSION = 0x7FF; // node -> central

    void sendVersion () {
        printf("VERSION\n");
        transmit(VERSION, "abcd", 4);
    }

    void assignSlot (int slot) {
        printf("ASSIGN %d\n", slot);
        mySlot = slot;
        BUS::filterInit(3, mySlot, 0x1F);
    }

    void handleUpload (const void* ptr, int len) {
        printf("UPLOAD #%d\n", len);
    }

public:
    Node () : mySlot (0), whoisClick (~0) {}

    void init () {
        BUS::filterInit(1, QUERY, 0x7FF);
        BUS::filterInit(2, ASSIGN, 0x7E0);
    }

    void poll () {
        uint32_t click = ticks / 3000; // 240000;
        if (whoisClick != click) {
            whoisClick = click;
            transmit(WHOIS + mySlot, myHwId, sizeof myHwId);
        }
    }

    void transmit (int id, const void* ptr, int len) {
        BUS::transmit(id, ptr, len);
    }

    int receive (int* pid, void* ptr) {
        while (true) {
            int len = BUS::receive(pid, ptr);
            if (len < 0)
                break;

            printf("%03x #%d\n", *pid, len);

            if (*pid == QUERY && len == 1 && *(uint8_t*) ptr == mySlot)
                sendVersion();
            else if ((*pid & 0x7E0) == ASSIGN && len == sizeof myHwId &&
                        *pid != ASSIGN && memcmp(ptr, myHwId, len) == 0)
                assignSlot(*pid & 0x1F);
            else if (*pid == (UPLOAD | mySlot) && mySlot != 0)
                handleUpload(ptr, len);
            else
                return len;
        }

        return -1;
    }
};

Node< decltype(can)> node;

int main() {
    console.init();
	console.baud(115200, fullSpeedClock());

    wait_ms(500);
    printf("hello\n");

    ledG.mode(Pinmode::out); ledG = 0;  keyG.mode(Pinmode::out); keyG = 0;
    led1.mode(Pinmode::out);            key1.mode(Pinmode::in_pullup);
    led2.mode(Pinmode::out);            key2.mode(Pinmode::in_pullup);
    led3.mode(Pinmode::out);            key3.mode(Pinmode::in_pullup);
    led4.mode(Pinmode::out);            key4.mode(Pinmode::in_pullup);

    // disable JTAG in AFIO-MAPR to release PB3, PB4, and PA15
    constexpr uint32_t afio = 0x40010000;
    MMIO32(afio + 0x04) |= 1 << 25; // disable JTAG, keep SWD enabled

    can.swInit(); // PA11+PA12
    can.filterInit(0, 0x123, 0x7FF);
    node.init();

    uint32_t last = 0;
    while (true) {
        node.poll();

        // send key state once every 500 ms
        if (ticks / 500 != last) {
            last = ticks / 500;

            uint8_t keys = (!key1<<0) | (!key2<<1) | (!key3<<2) | (!key4<<3);
            can.transmit(0x123, &keys, 1);
        }

        // process incoming messages
        int id;
        uint8_t dat[8];
        if (node.receive(&id, dat) == 1) {
            led1 = (dat[0] >> 0) & 1;
            led2 = (dat[0] >> 1) & 1;
            led3 = (dat[0] >> 2) & 1;
            led4 = (dat[0] >> 3) & 1;
        }
    }
}

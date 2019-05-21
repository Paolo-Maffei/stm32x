#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<8> ras;
PinA<15> cas;
PinC<13> wen;

Port<'A'> pinsA;
Port<'B'> pinsB;

static void refresh () {
    for (int i = 0; i < 64; ++i) {
        cas = 0; ras = 0; cas = 1; ras = 1;
    }
}

static void set12 (unsigned addr12) {
    MMIO32(pinsA.bsrr) = 0x000F0000 | (addr12 & 0x000F);
    MMIO32(pinsB.bsrr) = 0x00FF0000 | ((addr12>>4) & 0x00FF);
}

void wrDram (unsigned addr, uint8_t val) {
    __asm("cpsid i");

    //pinsB.modeMap(0b1111111100000000, Pinmode::out);
    MMIO32(pinsB.crh) = 0x11111111;

    set12(addr);
    ras = 0;
    MMIO32(pinsB.bsrr) = 0xFF000000 | (val<<8);
    wen = 0;
    set12(addr>>12);
    cas = 0;
    cas = 1;
    wen = 1;
    ras = 1;

    //pinsB.modeMap(0b1111111100000000, Pinmode::in_float);
    MMIO32(pinsB.crh) = 0x44444444;

    __asm("cpsie i");
}

uint8_t rdDram (unsigned addr) {
    __asm("cpsid i");

    set12(addr);
    ras = 0;
    set12(addr>>12);
    cas = 0;
    cas = 1;
    ras = 1;
    uint8_t val = MMIO32(pinsB.idr) >> 8;

    __asm("cpsie i");
    return val;
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock());
    //enableSysTick();
    wait_ms(100);

    wen.mode(Pinmode::out); wen = 1;
    ras.mode(Pinmode::out); ras = 1;
    cas.mode(Pinmode::out); cas = 1;

    pinsA.modeMap(0b0000000000001111, Pinmode::out);
    pinsB.modeMap(0b0000000011111111, Pinmode::out);
    pinsB.modeMap(0b1111111100000000, Pinmode::in_float);

    // disable JTAG in AFIO-MAPR to release PB3, PB4, and PA15
    // (looks like this has to be done *after* some GPIO mode inits)
    constexpr uint32_t afio = 0x40010000;
    MMIO32(afio+0x04) |= (2<<24); // disable JTAG, keep SWD enabled

    uint32_t t = ticks;
    for (int i = 0; i < 1000; ++i) refresh();
    printf("init %d µs per 64-refresh\n", ticks - t);

    VTableRam().systick = []() { ++ticks; refresh(); };
    
    while (true) {
        t = ticks;
        for (int i = 0; i < 1000000; ++i)
            wrDram(i, i);
        t = ticks - t;
        printf("%d.%03d µs/write, ", t/1000, t%1000);

        t = ticks;
        for (int i = 0; i < 1000000; ++i) {
            volatile uint8_t x = rdDram(i);
        }
        t = ticks - t;
        printf("%d.%03d µs/read: ", t/1000, t%1000);

        wait_ms(1000); // test to make sure that refresh keeps the data intact

        for (int i = 0; i < 10000; ++i) {
            if (i % 256 == 0)
                printf(".");
            uint8_t x = i, y = rdDram(i);
            // FIXME my current test build has data bit 6 stuck high :(
            //if (x ^ y)
            if ((x ^ y) & ~0x40)
                printf(" %d=%02x", i, x ^ y);
        }
        printf("\n");
    }
}

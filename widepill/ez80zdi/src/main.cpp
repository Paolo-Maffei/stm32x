#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;
UartBufDev< PinA<2>, PinA<3> > serial;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<1> led;
Timer<3> timer;

PinB<0> XIN;
PinB<2> ZDA;
PinB<4> ZCL;
PinB<8> RST;

const uint8_t hello [] = {
    0x06, 0x00, 0x0E, 0xA5, 0x3E, 0x03, 0xED, 0x79, 0x0E, 0xC3, 0x3E, 0x80,
    0xED, 0x79, 0x0E, 0xC0, 0x3E, 0x1A, 0xED, 0x79, 0x0E, 0xC3, 0x3E, 0x03,
    0xED, 0x79, 0x0E, 0xC2, 0x3E, 0x06, 0xED, 0x79, 0x21, 0x39, 0xE0, 0x7E,
    0xA7, 0x28, 0x10, 0x0E, 0xC5, 0xED, 0x78, 0xE6, 0x20, 0x28, 0xF8, 0x0E,
    0xC0, 0x7E, 0xED, 0x79, 0x23, 0x18, 0xEC, 0x18, 0xFE, 0x48, 0x65, 0x6C,
    0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x21, 0x0A, 0x0D, 0x00,
};

const uint8_t flash [] = {
    0x01, 0xF5, 0x00, 0x3E, 0xB6, 0xED, 0x79, 0x3E, 0x49, 0xED, 0x79, 0x0E, 
    0xF9, 0x3E, 0x29, 0xED, 0x79, 0x0E, 0xF5, 0x3E, 0xB6, 0xED, 0x79, 0x3E, 
    0x49, 0xED, 0x79, 0x0E, 0xFA, 0x3E, 0x00, 0xED, 0x79, 0x0E, 0xFF, 0x3E, 
    0x01, 0xED, 0x79, 0x18, 0xFE, 
};

void ezInit () {
    // initialise all the main control pins
    RST.mode(Pinmode::out_od);
    XIN.mode(Pinmode::alt_out); // XXX alt_out_50mhz
    ZDA.mode(Pinmode::out);
    ZCL.mode(Pinmode::out); // XXX out_50mhz

    // disable JTAG in AFIO-MAPR to release PB3, PB4, and PA15
    // (looks like this has to be done *after* the GPIO mode inits)
    constexpr uint32_t afio = 0x40010000;
    MMIO32(afio+0x04) |= (2<<24); // disable JTAG, keep SWD enabled

#define SLOW 40
#if SLOW
    // generate a 4 MHz signal with 50% duty cycle on PB0, using TIM3
    timer.init(18);
    timer.pwm(9);
#else
    // generate a 36 MHz signal with 50% duty cycle on PB0, using TIM3
    timer.init(2);
    timer.pwm(1);
#endif
}

void ezReset () {
    RST = 0;
    ZCL = 1; // p.257
    ZDA = 0; // p.243
    wait_ms(2);
    RST = 1;
}

static void delay () {
    for (int i = 0; i < SLOW; ++i) __asm(""); // prevents optimisation
}

static void zcl (int f) {
    delay(); ZCL = f; delay();
}

static void zdiSet (bool f) {
    zcl(0); ZDA = f; zcl(1);
}

static uint8_t zdiInBits (bool last =0) {
    uint8_t b = 0;
    for (int i = 0; i < 8; ++i) {
        zcl(0); zcl(1);
        b <<= 1;
        b |= ZDA & 1;
    }
    zdiSet(last);
    return b;
}

static void zdiOutBits (uint8_t b, bool last =0) {
    for (int i = 0; i < 8; ++i) {
        zdiSet((b & 0x80) != 0);
        b <<= 1;
    }
    zdiSet(last);
}

static void zdiStart (uint8_t b, int rw) {
    ZDA = 0;
    zdiOutBits((b<<1) | rw);
}

static uint8_t zdiIn (uint8_t addr) {
    zdiStart(addr, 1);
    ZDA.mode(Pinmode::in_pullup);
    uint8_t b = zdiInBits(1);
    ZDA.mode(Pinmode::out);
    return b;
}

static void zdiOut (uint8_t addr, uint8_t val) {
    zdiStart(addr, 0);
    zdiOutBits(val, 1);
}

void zIns (uint8_t v0) {
    zdiOut(0x25, v0);
}

void zIns (uint8_t v0, uint8_t v1) {
    zdiStart(0x24, 0);
    zdiOutBits(v1);
    zdiOutBits(v0, 1);
}

void zIns (uint8_t v0, uint8_t v1, uint8_t v2) {
    zdiStart(0x23, 0);
    zdiOutBits(v2);
    zdiOutBits(v1);
    zdiOutBits(v0, 1);
}

void zIns (uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3) {
    zdiStart(0x22, 0);
    zdiOutBits(v3);
    zdiOutBits(v2);
    zdiOutBits(v1);
    zdiOutBits(v0, 1);
}

void zIns (uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4) {
    zdiStart(0x21, 0);
    zdiOutBits(v4);
    zdiOutBits(v3);
    zdiOutBits(v2);
    zdiOutBits(v1);
    zdiOutBits(v0, 1);
}

void zCmd (uint8_t cmd) {
    zdiOut(0x16, cmd);
}

uint8_t getMbase () {
    zCmd(0x08); // set ADL
    zCmd(0x00); // read MBASE
    uint8_t b = zdiIn(0x12); // get U
    return b;
}

void setMbase (uint8_t b) {
    //zCmd(0x08); // set ADL
#if 0
    zCmd(0x00); // read MBASE
    zdiOut(0x13, zdiIn(0x10)); // keep L
    zdiOut(0x14, zdiIn(0x11)); // keep H
    zdiOut(0x15, b); // set U
    zCmd(0x80); // write MBASE
#else
    zCmd(0x00); // read MBASE
    zdiOut(0x13, b); // set L
    zCmd(0x80); // write MBASE
    zIns(0xED, 0x6D); // ld mb,a
#endif
}

void setPC (uint32_t addr) {
    zdiOut(0x13, addr);
    zdiOut(0x14, addr >> 8);
    zdiOut(0x15, addr >> 16); // must be in ADL mode
    zCmd(0x87); // write PC
}

uint32_t getPC () {
    zCmd(0x07); // read PC
    uint8_t l = zdiIn(0x10);
    uint8_t h = zdiIn(0x11);
    uint8_t u = zdiIn(0x12);
    return (u<<16) | (h<<8) | l;
}

void readMem (uint32_t addr, void *ptr, unsigned len) {
    if (len > 0) {
        setPC(--addr); // p.255 start reading one byte early
        zdiStart(0x20, 1);
        ZDA.mode(Pinmode::in_pullup);
        zdiInBits(0); // ignore first read
        for (unsigned i = 0; i < len; ++i)
            ((uint8_t*) ptr)[i] = zdiInBits(i >= len-1);
        ZDA.mode(Pinmode::out);
    }
}

void writeMem (uint32_t addr, const void *ptr, unsigned len) {
    if (len > 0) {
        setPC(addr);
        zdiStart(0x30, 0);
        for (unsigned i = 0; i < len; ++i)
            zdiOutBits(((const uint8_t*) ptr)[i], i >= len-1);
    }
}

void dumpReg () {
    static const char* regs [] = {
        "AF", "BC", "DE", "HL", "IX", "IY", "SP", "PC"
    };

    for (int i = 0; i < 8; ++i) {
        zCmd(i);
        uint8_t l = zdiIn(0x10);
        uint8_t h = zdiIn(0x11);
        uint8_t u = zdiIn(0x12);
        printf("  %s = %02x:%02x%02x", regs[i], u, h, l);
        if (i % 4 == 3)
            printf("\n");
    }
}

void jump (uint32_t addr) {
    setPC(addr);
    zdiOut(0x10, 0x00);
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock());
    led.mode(Pinmode::out);
    wait_ms(500);
    printf("\n---\n");

    serial.init();
    serial.baud(19200, 72000000);

    ezInit();
    ezReset();

    printf("v%02x", zdiIn(1));
    printf(".%02x", zdiIn(0));
    printf(".%02x\n", zdiIn(2));

#if 0
    printf("s%02x ", zdiIn(3));
    zCmd(0x08);                     // set ADL
    printf("s%02x\n", zdiIn(3));

    //printf("s%02x ", zdiIn(3));
    //zIns(0x76);                     // halt
    printf("s%02x ", zdiIn(3));
    zdiOut(0x10, 0x00);             // continue
    printf("s%02x ", zdiIn(3));
    zdiOut(0x10, 0x80);             // break
    printf("s%02x\n", zdiIn(3));
#endif

    while (true) {
        uint8_t stat = zdiIn(3);
        printf("s%02x %02x: ", stat, getMbase());
        if ((stat & 0x10) == 0)
            zCmd(0x09); // reset ADL

        while (!console.readable()) {
            if (serial.readable())
                console.putc(serial.getc());
        }
        led.toggle();

        int ch = console.getc();
        if (ch != '\n')
            printf("%c\n", ch);

        switch (ch) {
            case 'b': zdiOut(0x10, 0x80); break; // break
            case 'c': zdiOut(0x10, 0x00); break; // continue
            case 'h': zIns(0x76);         break; // halt
            case 'n': zIns(0x00);         break; // nop
            case 'R': zdiOut(0x11, 0x80); break; // reset
            case 'H': ezReset();          break; // hardware reset
            case 'a': zCmd(0x08);         break; // set ADL
            case 'z': zCmd(0x09);         break; // reset ADL
            case 'r': dumpReg();          break; // register dump

            case 'm':
            {
                uint8_t buf [16];
                for (unsigned addr = 0; addr < 64; addr += 16) {
                    readMem(0xFFE000 + addr, buf, sizeof buf);
                    for (unsigned i = 0; i < sizeof buf; ++i)
                        printf(" %02x", buf[i]);
                    printf("\n");
                }
            }
            break;

            case '0': setMbase(0x00); break;
            case '1': setMbase(0x20); break;
            case '2': setMbase(0xFF); break;

            case 'f': writeMem(0xFFE000, flash, sizeof flash); break;
            case 'w': writeMem(0xFFE000, hello, sizeof hello); break;
            case 'j': setPC(0xFFE000); break; 
            default: printf("?\n");
        }
    }
}

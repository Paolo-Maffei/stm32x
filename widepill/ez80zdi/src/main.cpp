#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

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

void ezInit () {
    // initialise all the main control pins
    RST.mode(Pinmode::out_od);
    XIN.mode(Pinmode::alt_out_50mhz); // XXX alt_out_50mhz
    ZDA.mode(Pinmode::out);
    ZCL.mode(Pinmode::out);

    // disable JTAG in AFIO-MAPR to release PB3, PB4, and PA15
    // (looks like this has to be done *after* the GPIO mode inits)
    constexpr uint32_t afio = 0x40010000;
    MMIO32(afio+0x04) |= (2<<24); // disable JTAG, keep SWD enabled

    // generate a 36 MHz signal with 50% duty cycle on PB0, using TIM3
    timer.init(2);
    timer.pwm(1);
}

void ezReset () {
    RST = 0;
    ZCL = 1; // p.257
    ZDA = 1;
    wait_ms(2);
    RST = 1;
}

void zclDelay () {
     __asm("nop");
}

void zcl (int f) {
    zclDelay();
    ZCL = f;
    zclDelay();
}

void zdiSet (bool f) {
    zcl(0); ZDA = f; zcl(1); ZDA = 1;
}

void zdiStart (uint8_t b, int rw) {
    ZDA.mode(Pinmode::out);
    ZDA = 0;
    for (int i = 0; i < 7; ++i) {
        zdiSet((b & 0x40) != 0);
        b <<= 1;
    }
    zdiSet(rw); zdiSet(0);
}

uint8_t zdiInBits (bool last =0) {
    uint8_t r = 0;
    for (int i = 0; i < 8; ++i) {
        zcl(0); zcl(1);
        r <<= 1;
        r |= ZDA & 1;
    }
    zdiSet(last);
    return r;
}

uint8_t zdiIn (uint8_t addr) {
    zdiStart(addr, 1);
    ZDA.mode(Pinmode::out_od);
    uint8_t b = zdiInBits(1);
}

void zdiOutBits (uint8_t b, bool last =0) {
    for (int i = 0; i < 8; ++i) {
        zdiSet((b & 0x80) != 0);
        b <<= 1;
    }
    zdiSet(last);
}

void zdiOut (uint8_t addr, uint8_t val) {
    zdiStart(addr, 0);
    zdiOutBits(val, 1);
}

void zIns (uint8_t v0) {
    zdiStart(0x25, 0);
    zdiOutBits(v0, 1);
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
    //zCmd(0x08); // set ADL
    zCmd(0x00); // read MBASE
    uint8_t b = zdiIn(0x12); // get U
    //zCmd(0x09); // reset ADL
    return b;
}

void setMbase (uint8_t b) {
    //zCmd(0x08); // set ADL
    zCmd(0x00); // read MBASE
    zdiOut(0x15, b); // set U
    //zdiOut(0x13, b); // set U
    zCmd(0x80); // write MBASE
    //zIns(0xED, 0x6D);
    //zCmd(0x09); // reset ADL
}

void readMem (uint32_t addr, void* ptr, unsigned len) {
    zdiOut(0x13, addr);
    zdiOut(0x14, addr >> 8);
    zdiOut(0x15, addr >> 16);
    zCmd(0x87); // write PC

    zdiStart(0x20, 1);
    ZDA.mode(Pinmode::out_od);
    uint8_t b = zdiInBits(1);
    for (int i = 0; i < len; ++i)
        ((uint8_t*) ptr)[i] = zdiInBits(i < len-1);
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock());
    led.mode(Pinmode::out);
    wait_ms(500);
    printf("\n---\n");

    ezInit();
    ezReset();

    printf("v%02x", zdiIn(0x00));
    printf(".%02x", zdiIn(0x01));
    printf(".%02x\n", zdiIn(0x02));

    printf("s%02x ", zdiIn(3));
    zCmd(0x08);                     // set ADL
    printf("s%02x ", zdiIn(3));
    zIns(0x76);                     // halt
    printf("s%02x ", zdiIn(3));
    zdiOut(0x10, 0x80);             // break
    printf("s%02x\n", zdiIn(3));

    printf("b%02x ", getMbase());
    setMbase(0x45);
    printf("b%02x\n", getMbase());

    printf("s%02x ", zdiIn(3));
    zdiOut(0x13, 0x98);             // set L
    printf("l%02x ", zdiIn(0x10));
    zdiOut(0x13, 0x76);             // set L
    printf("l%02x ", zdiIn(0x10));
    printf("s%02x ", zdiIn(3));
    zdiOut(0x13, 0x54);             // set L
    printf("l%02x\n", zdiIn(0x10));

    wait_ms(1000);

    uint8_t buf [16];
    printf("s%02x\n", zdiIn(3));
    readMem(0x000000, buf, sizeof buf);
    printf("s%02x\n", zdiIn(3));
    for (unsigned i = 0; i < sizeof buf; ++i)
        printf(" %02x", buf[i]);
    printf("\n");
    printf("s%02x\n", zdiIn(3));

    while (true) {
        led.toggle();
        wait_ms(500);
    }
}

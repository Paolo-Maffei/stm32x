#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

Timer<3> timer;

PinB<0> XIN;
PinB<2> ZDA;
PinB<4> ZCL;
PinB<8> RST;

void ezInit () {
    // initialise all the main control pins
    RST.mode(Pinmode::out_od);
    XIN.mode(Pinmode::alt_out); // XXX alt_out_50mhz
    ZDA.mode(Pinmode::out_od);
    ZCL.mode(Pinmode::out);

    // disable JTAG in AFIO-MAPR to release PB3, PB4, and PA15
    // (looks like this has to be done *after* the GPIO mode inits)
    constexpr uint32_t afio = 0x40010000;
    MMIO32(afio+0x04) |= (2<<24); // disable JTAG, keep SWD enabled

    // generate a 9 MHz signal with 50% duty cycle on PB0, using TIM3
    timer.init(4);
    timer.pwm(2);
}

void ezReset () {
    RST = 0;
    ZCL = 1; // p.257
    ZDA = 1;
    wait_ms(2);
    RST = 1;
}

void zclDelay () {
    for (int i = 0; i < 5; ++i) __asm("");
}

void zcl (int f) {
    zclDelay();
    ZCL = f;
    zclDelay();
}

#if 0
: zdi! ( f -- )  zcl-lo  ZDA io!  zcl-hi  ZDA ios! ;

: zdi-start ( u -- )
  ( zcl-hi ) ZDA ioc!
  OMODE-PP ZDA io-mode!
  7 0 do
    dup $40 and zdi!  shl
  loop  drop ;

: zdi> ( addr -- val )
  zdi-start  1 zdi!  1 zdi!
  OMODE-OD ZDA io-mode!
  0  8 0 do
    zcl-lo  zcl-hi
    shl  ZDA io@ 1 and or
  loop
  zcl-lo ZDA ios! zcl-hi ;

: >zdi ( val addr -- )
  zdi-start  0 zdi!  1 zdi!
  8 0 do
    dup $80 and zdi!  shl
  loop  drop
  zcl-lo ZDA ios! zcl-hi
  OMODE-OD ZDA io-mode! ;

: ins1 ( u1 -- )              $25 >zdi ;
: ins2 ( u1 u2 -- )           $24 >zdi ins1 ;
: ins3 ( u1 u2 u3 -- )        $23 >zdi ins2 ;
: ins4 ( u1 u2 u3 u4 -- )     $22 >zdi ins3 ;
: ins5 ( u1 u2 u3 u4 u5 -- )  $21 >zdi ins4 ;
#endif

void zdiSet (int f) {
    zcl(0); ZDA = f; zcl(1); ZDA = 1;
}

void zdiStart (uint8_t b, int rw) {
    ZDA = 0;
    ZDA.mode(Pinmode::out);
    for (int i = 0; i < 7; ++i) {
        zdiSet((b & 0x40) != 0);
        b <<= 1;
    }
    zdiSet(rw); zdiSet(1);
}

void zdiInN (uint8_t addr, uint8_t* ptr, int n) {
    zdiStart(addr, 1);
    ZDA.mode(Pinmode::out_od);
    while (--n >= 0) {
        uint8_t r = 0;
        for (int i = 0; i < 8; ++i) {
            zcl(0); zcl(1);
            r <<= 1;
            r |= ZDA & 1;
        }
        *ptr++ = r;
        zdiSet(1);
    }
}

uint8_t zdiIn (uint8_t addr) {
    uint8_t r = 0;
    zdiInN(addr, &r, 1);
    return r;
}

void zdiOutN (uint8_t addr, const uint8_t* ptr, int n) {
    zdiStart(addr, 0);
    while (--n >= 0) {
        uint8_t b = *ptr++;
        for (int i = 0; i < 8; ++i) {
            zdiSet((b & 0x80) != 0);
            b <<= 1;
        }
        zdiSet(1);
    }
    ZDA.mode(Pinmode::out_od);
}

void zdiOut (uint8_t addr, uint8_t val) {
    zdiOutN(addr, &val, 1);
}

void zIns (uint8_t v0) {
    zdiOut(0x25, v0);
}

void zIns (uint8_t v0, uint8_t v1) {
    uint8_t buf [] = { v1, v0 };
    zdiOutN(0x24, buf, sizeof buf);
}

void zIns (uint8_t v0, uint8_t v1, uint8_t v2) {
    uint8_t buf [] = { v2, v1, v0 };
    zdiOutN(0x23, buf, sizeof buf);
}

void zIns (uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3) {
    uint8_t buf [] = { v3, v2, v1, v0 };
    zdiOutN(0x22, buf, sizeof buf);
}

void zIns (uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4) {
    uint8_t buf [] = { v4, v3, v2, v1, v0 };
    zdiOutN(0x21, buf, sizeof buf);
}

#if 0
: mb> ( -- u )            \ get MBASE
  $08 $16 >zdi            \ set adl mode
  $00 $16 >zdi  $12 zdi>  \ mbase => <u>
  $09 $16 >zdi ;          \ set z80 mode

: >mb ( u -- )  \ set MBASE
  $08 $16 >zdi            \ set adl mode
  $13 >zdi  $80 $16 >zdi  \ ld a,<u>
  $ED $6D ins2            \ ld mb,a
  $09 $16 >zdi ;          \ set z80 mode
#endif

uint8_t getMbase () {
    zdiOut(0x16, 0x08); // set ADL
    zdiOut(0x16, 0x00); // read MBASE
    uint8_t b = zdiIn(0x12); // get U
    zdiOut(0x16, 0x09); // reset ADL
    return b;
}

void setMbase (uint8_t b) {
    zdiOut(0x16, 0x08); // set ADL
    zdiOut(0x15, b); // set U
    zdiOut(0x16, 0x80); // write MBASE
    //zIns(0xED, 0x6D);
    //zdiOut(0x24, 0x6D);
    //zdiOut(0x25, 0xED);
    zdiOut(0x16, 0x09); // reset ADL
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock());
    wait_ms(500);
    printf("\n---\n");

    ezInit();
    ezReset();

    printf("v %02x", zdiIn(0x00));
    printf(".%02x", zdiIn(0x01));
    printf(".%02x\n", zdiIn(0x02));

    printf("status %02x\n", zdiIn(0x03));
    zdiOut(0x16, 0x09); // reset ADL
    printf("status %02x\n", zdiIn(0x03));

    printf("b1 %02x", getMbase());
    setMbase(0x45);
    printf(" --> ");
    printf("b2 %02x\n", getMbase());

    while (true) {}
}

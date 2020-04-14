#include <jee.h>
#include <jee/spi-rf69.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<5> led;
PinC<0> vac;
PinC<1> ct1;
PinC<2> ct2;
PinC<3> ct3;

ADC<1> adc;
Timer<3> timer;

constexpr int NSAMP = 2000, NCHAN = 4;
uint16_t adcData [NSAMP][NCHAN];

SpiHw< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;
RF69< decltype(spi) > rf;

constexpr static uint32_t dma1  = 0x40020000;

static void dmaInit () {
    // config ADC for DMA, avoid triggering a conversion right away FIXME
    MMIO32(adc.cr2) = (1<<20) | (4<<17) | (1<<8) | (1<<0);

    // configure 4-chan acquisition PC0/PC1/PC2/PC3, analog channels 10..13
    MMIO32(adc.smpr1) = (2<<9) | (2<<6) | (2<<3) | (6<<0); // 71.5/13.5 cycles
    MMIO32(adc.sqr3) = (13<<15) | (12<<10) | (11<<5) | (10<<0);
    MMIO32(adc.base+0x2C) = (NCHAN-1)<<20;    // SQR1
    Periph::bit(adc.cr1, 8) = 1;              // SCAN

    Periph::bit(Periph::rcc+0x14, 0) = 1;     // DMA1EN
    MMIO32(dma1+0x14) = (uint32_t) adcData;   // CMAR1
    MMIO32(dma1+0x10) = adc.dr;               // CPAR1
    MMIO32(dma1+0x08) = (1<<10) | (1<<8) | (1<<7); // CCR1
}

static void dmaCapture () {
    MMIO32(dma1+0x0C) = NSAMP*NCHAN;          // CNDTR1
    Periph::bit(dma1+0x08, 0) = 1;            // EN
    Periph::bit(adc.cr2, 0) = 1;              // ADEN

    // wait for the DMA capture to complete
    while (Periph::bit(dma1+0x00, 1) == 0) {} // TCIF1 in ISR

    Periph::bit(dma1+0x04, 0) = 1;            // CGIF1 in IFCR
    Periph::bit(dma1+0x08, 0) = 0;            // ~EN
    Periph::bit(adc.cr2, 0) = 0;              // ~ADEN
}

int main() {
    (void) powerDown;
    (void) enableClkAt8MHz;

    console.init();
    console.baud(115200, fullSpeedClock());
    led.mode(Pinmode::out);
    wait_ms(100);

    spi.init();
    //rf.init(63, 42, 8683);
    //rf.txPower(0);

    timer.init(16*36); // 16 µs & 36 MHz APB1
    adc.init(); // init all the I/O pins to analog
    adc.read(vac); adc.read(ct1); adc.read(ct2); adc.read(ct3);
    dmaInit();
    dmaCapture(); // ignore first mis-aligned capture

    while (true) {
        wait_ms(1000);

        uint32_t t = ticks;
        dmaCapture();
        t = ticks - t;
        for (int i = 0; i < NSAMP; ++i)
            for (int j = 0; j < NCHAN; ++j)
                printf("%d%c", adcData[i][j], j < NCHAN-1 ? ',' : '\n');
        printf("%d\n", t);
    }
}

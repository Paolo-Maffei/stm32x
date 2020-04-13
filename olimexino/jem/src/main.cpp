#include <jee.h>
#include <jee/spi-rf69.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<5> led;
PinC<0> ct1;
PinC<1> ct2;
PinC<2> ct3;
PinA<0> acin;

ADC<1> adc;
Timer<3> timer;

constexpr int NSAMP = 2000, NCHAN = 4;
uint16_t adcData [NSAMP][NCHAN];

SpiHw< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;
RF69< decltype(spi) > rf;

void dmaCapture () {
    constexpr static uint32_t dma1  = 0x40020000;

    Periph::bit(Periph::rcc+0x14, 0) = 1;     // DMA1EN
    MMIO32(dma1+0x0C) = NSAMP*NCHAN;          // CNDTR1
    MMIO32(dma1+0x14) = (uint32_t) adcData;   // CMAR1
    MMIO32(dma1+0x10) = adc.dr;               // CPAR1
    MMIO32(dma1+0x08) = (1<<10) | (1<<8) | (1<<7) | (1<<5) | (1<<0); // CCR1

    MMIO32(adc.cr2) = (1<<20) | (4<<17) | (1<<8) | (1<<0); // config for DMA

    // configure 4-chan acquisition
    MMIO32(adc.smpr1) = (3<<9) | (3<<6) | (3<<3) | (3<<0); // 28.5+12.5 cycles
    MMIO32(adc.base+0x2C) = (3<<20);                               // SQR1
    MMIO32(adc.base+0x34) = (0<<15) | (12<10) | (11<<5) | (10<<0); // SQR3
    Periph::bit(adc.cr1, 8) = 1;                                   // SCAN

    // wait for the DMA capture to complete
    while (Periph::bit(dma1+0x00, 1) == 0) {} // TCIF1 in ISR
    Periph::bit(dma1+0x04, 0) = 1;            // CGIF1 in IFCR

    Periph::bit(Periph::rcc+0x14, 0) = 0;     // DMA1EN
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

    timer.init(16, 36-1); // 16 µs, 36 MHz APB1
    adc.init();

    while (true) {
        wait_ms(1000);

        uint32_t t = ticks;
        dmaCapture();
        t = ticks - t;
        for (int i = 0; i < 10; ++i)
            for (int j = 0; j < NCHAN; ++j)
                printf("%d%c", adcData[i][j], j < NCHAN-1 ? ',' : '\n');
        printf("%d\n", t);
    }
}

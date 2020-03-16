#include <jee.h>
#include <jee/spi-rf69.h>

UartDev< PinA<2>, PinA<15> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<0> dio0;
PinA<8> dio3;

SpiHw< PinB<5>, PinB<4>, PinB<3>, PinA<11> > spi;  // for Nucleo-32
RF69< decltype(spi) > rf;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock(false)); // HSI16, no PLL
    MMIO32(Periph::rcc+0x0C) |= (1<<15); // use HSI16 after stop mode
    //enableSysTick();

    dio0.mode(Pinmode::in_float);
    dio3.mode(Pinmode::in_float);

#if 1
    // RFM69 hard reset, so this code always starts from a known state
    PinB<1> rfrst;
    rfrst.mode(Pinmode::out);
    rfrst = 1; wait_ms(2); rfrst = 0; wait_ms(10);
#endif

    spi.init(0); // div=0 @ 16 MHz: 8 Mhz
    rf.init(63, 42, 8683);  // node 63, group 42, 868.3 MHz

    // prepare DIO pins for GPIO polling
    constexpr int DIO0 = 0, DIO2 = 3, DIO3 = 2, DIO5 = 3;
    rf.writeReg(0x25, (DIO0<<6) | (DIO2<<2) | DIO3);
    rf.writeReg(0x26, (DIO5<<4) | 0b101); // clkout/32

    MMIO32(Periph::exti+0x04) |= (1<<0) | (1<<8); // EMR, unmask events PA0+PA8
    MMIO32(Periph::exti+0x08) |= (1<<0) | (1<<8); // RTSR, rising edge events

    rf.listen();

    while (true) {
        MMIO32(Periph::exti+0x14) = (1<<0) | (1<<8); // clear events
        while (!dio3)
            powerDown(false);

        rf.rssiCapture();

        MMIO32(Periph::exti+0x14) = (1<<0) | (1<<8); // clear events
        while (dio3)
            powerDown(false);

        uint8_t rxBuf [66];
        int rxLen = rf.receive(rxBuf, sizeof rxBuf);

        if (rxLen >= 0) {
#if 1
            printf("RF69 #%d: ", rxLen);
            for (int i = 0; i < rxLen; ++i) {
                printf("%02x", rxBuf[i]);
                if (i < 2)
                    printf(" ");
            }
            printf(" r%4d l%2d a %d (%d)\n", rf.rssi, rf.lna, rf.afc, ticks);
#else
            printf("(%d)\n", ticks);
#endif
            for (int i = 0; i < 1000; ++i) asm (""); // let usart tx drain
        }
    }
}

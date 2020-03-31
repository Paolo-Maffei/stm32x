#include <jee.h>
#include <jee/spi-rf69.h>

UartDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<0> dio0;
PinA<1> dio1;
PinA<2> dio2;
PinA<3> dio3;
PinB<11> dio5;
PinB<10> rfrst;
PinC<13> led;

SpiHw< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;
RF69< decltype(spi) > rf;

int main() {
    PinA<9> tx; tx = 1; tx.mode(Pinmode::in_pullup);

    console.init();
    enableSysTick();
    led.mode(Pinmode::out);
    //MMIO32(Periph::rcc+0x0C) |= (1<<15); // use HSI16 after stop mode

    dio0.mode(Pinmode::in_float);
    dio1.mode(Pinmode::in_float);
    dio2.mode(Pinmode::in_float);
    dio3.mode(Pinmode::in_float);
    dio5.mode(Pinmode::in_float);

#if 1
    // RFM69 hard reset, so this code always starts from a known state
    rfrst.mode(Pinmode::out);
    rfrst = 1; wait_ms(2); rfrst = 0; wait_ms(10);
#endif

    spi.init();
    rf.init(63, 42, 8683);  // node 63, group 42, 868.3 MHz

    // prepare DIO pins for GPIO polling
    constexpr int DIO0 = 0, DIO2 = 3, DIO3 = 2, DIO5 = 3;
    rf.writeReg(0x25, (DIO0<<6) | (DIO2<<2) | DIO3);
    rf.writeReg(0x26, (DIO5<<4) | 0b101); // clkout/32

    MMIO32(Periph::exti+0x04) |= (1<<0) | (1<<3); // EMR, unmask events PA0+PA3
    MMIO32(Periph::exti+0x08) |= (1<<0) | (1<<3); // RTSR, rising edge events

    rf.listen();
    printf("\nReset.\n");
    wait_ms(3);

    Iwdg watchdog;

    while (true) {
        led.toggle();
        watchdog.kick();

        MMIO32(Periph::exti+0x14) = (1<<0) | (1<<3); // clear events
        while (!dio3)
            powerDown(false);

        rf.rssiCapture();

        MMIO32(Periph::exti+0x14) = (1<<0) | (1<<3); // clear events
        while (dio3)
            powerDown(false);

        uint8_t rxBuf [66];
        int rxLen = rf.receive(rxBuf, sizeof rxBuf);

        if (rxLen >= 0) {
            printf("RF69 #%d: ", rxLen);
            for (int i = 0; i < rxLen; ++i) {
                printf("%02x", rxBuf[i]);
                if (i < 2)
                    printf(" ");
            }
            printf(" r%4d l%2d a %d (%d)\n", rf.rssi, rf.lna, rf.afc, ticks);
            for (int i = 0; i < 1000; ++i) asm (""); // let usart tx drain
        }
    }
}

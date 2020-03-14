#include <jee.h>
#include <jee/spi-rf69.h>

UartBufDev< PinA<2>, PinA<15> > console;

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
    console.baud(115200, fullSpeedClock());
    //enableSysTick();

    dio0.mode(Pinmode::in_float);
    dio3.mode(Pinmode::in_float);

    // RFM69 hard reset, so this code always starts from a known state
    PinB<1> rfrst;
    rfrst.mode(Pinmode::out);
    rfrst = 1; wait_ms(2); rfrst = 0; wait_ms(10);

    spi.init();
    rf.init(63, 42, 8683);  // node 63, group 42, 868.3 MHz
    rf.txPower(3);

    // prepare DIO pins for GPIO polling, instead of the RF69's SPI polling
    constexpr int DIO0 = 1, DIO1 = 3, DIO2 = 3, DIO3 = 2, DIO5 = 3;//1;
    rf.writeReg(0x25, (DIO0<<6) | (DIO1<<4) | (DIO2<<2) | DIO3);
    rf.writeReg(0x26, (DIO5<<4) | 0b101); // clkout/32

    // the following loop is a bit convoluted, because it is mostly checking
    // for new incoming packets, but it also sends out a packet once a second

    while (true) {
        // when waiting for a new incoming packet, don't use SPI polling
        // instead, poll the DIO0 and DIO3 pins (will use interrupts later)

        while (!dio3 && ticks % 1000 != 0) {} // wait for DIO3 0 => 1

        if (!dio3 && ticks % 1000 == 0) {
            static int seq = 0;
            rf.send(0, &seq, sizeof seq);
            ++seq;
            while (!dio0) {} // wait for packet sent to start
            while (dio0) {} // wait for packet sent to complete
            rf.resume();
            continue;
        }

        rf.rssiCapture();

        while (dio3 && !dio0) {} // wait for DIO3 == 1 && DIO0 1 => 0

        uint8_t rxBuf [66];
        int rxLen = rf.receive(rxBuf, sizeof rxBuf);

        if (rxLen >= 0) {
            printf("RF69 #%d: ", rxLen);
            for (int i = 0; i < rxLen; ++i) {
                printf("%02x", rxBuf[i]);
                if (i < 2)
                    printf(" ");
            }
            printf(" r%4d l%2d a %d\n", rf.rssi, rf.lna, rf.afc);
        }
    }
}

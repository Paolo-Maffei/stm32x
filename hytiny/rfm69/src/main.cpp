#include <jee.h>
#include <jee/spi-rf69.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<1> led;

SpiGpio< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;  // default SPI1 pins
RF69< decltype(spi) > rf;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock());
    led.mode(Pinmode::out); led = 1;

    spi.init();
    rf.init(63, 42, 8683);  // node 63, group 42, 868.3 MHz
    rf.txPower(5);

    int seq = 0;
    while (true) {
        uint8_t rxBuf [64];
        auto rxLen = rf.receive(rxBuf, sizeof rxBuf);

        if (ticks % 1000 == 0) {
            wait_ms(1);
            rf.send(0, &seq, sizeof seq);
            ++seq;
        }

        if (rxLen >= 0) {
            led = 0;

            printf("RF69 #%d: ", rxLen);
            for (int i = 0; i < rxLen; ++i) {
                printf("%02x", rxBuf[i]);
                if (i < 2)
                    printf(" ");
            }
            printf(" r%4d l%2d a%4d\n", rf.rssi, rf.lna, rf.afc);

            led = 1;
        }
    }
}

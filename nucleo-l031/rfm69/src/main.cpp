#include <jee.h>
#include <jee/spi-rf69.h>

UartBufDev< PinA<2>, PinA<15> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

//PinB<3> led; // pin not available, as the on-board LED is tied to SCK

SpiGpio< PinB<5>, PinB<4>, PinB<3>, PinA<11> > spi;  // for Nucleo-32
RF69< decltype(spi) > rf;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock());
    //led.mode(Pinmode::out);

    spi.init();
    rf.init(63, 42, 8683);  // node 63, group 42, 868.3 MHz

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
            //led = 0;

            printf("RF69 #%d: ", rxLen);
            for (int i = 0; i < rxLen; ++i) {
                printf("%02x", rxBuf[i]);
                if (i < 2)
                    printf(" ");
            }
            printf(" r%4d l%2d a%4d\n", rf.rssi, rf.lna, rf.afc);

            //led = 1;
        }
    }
}

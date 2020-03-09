#include <jee.h>

// enter sleep mode while waiting for send() to complete, see spi-rf69.h
#define Yield() wait_ms(1)
#include <jee/spi-rf69.h>

UartBufDev< PinA<2>, PinA<15> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<0> dio0;
PinA<8> dio3;

SpiGpio< PinB<5>, PinB<4>, PinB<3>, PinA<11> > spi;  // for Nucleo-32
RF69< decltype(spi) > rf;

int main() {
    console.init();
    enableSysTick();

    dio0.mode(Pinmode::in_float);
    dio3.mode(Pinmode::in_float);

    spi.init();
    rf.init(63, 42, 8683);  // node 63, group 42, 868.3 MHz
    rf.txPower(6);

    // prepare DIO pins for GPIO polling, instead of the RF69's SPI polling
    rf.writeReg(0x25, 0b01000001); // dio0 mode 1, dio3 mode 1

    // the following loop is a bit convoluted, because it is mostly checking
    // for new incoming packets, but it also sends out a packet once a second

    int seq = 0;
    while (true) {
        if (rf.mode == rf.MODE_RECEIVE) {
            // when waiting for a new incoming packet, don't use SPI polling
            // instead, poll the DIO0 pin until PayloadReady sets it
            // but while waiting for that, check DIO3 to fetch RSSI and AFC

            while (!dio0) {
                if (dio3)
                    rf.receive(0, 0); // will fetch rssi, lnd, and afc

                wait_ms(1); // put the µC in sleep mode until the next tick

                if (ticks % 1000 == 0) {
                    rf.send(0, &seq, sizeof seq);
                    ++seq;
                    break;
                }
            }
        }

        // receive does the usual SPI polling, but because of the above logic,
        // it'll only be reached once the radio is known to have a new packet

        uint8_t rxBuf [64];
        auto rxLen = rf.receive(rxBuf, sizeof rxBuf);

        if (rxLen >= 0) {
            printf("RF69 #%d: ", rxLen);
            for (int i = 0; i < rxLen; ++i) {
                printf("%02x", rxBuf[i]);
                if (i < 2)
                    printf(" ");
            }
            printf(" r%4d l%2d a%4d\n", rf.rssi, rf.lna, rf.afc);
        }
    }
}

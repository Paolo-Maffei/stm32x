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

SpiHw< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;
RF69< decltype(spi) > rf;

int main() {
    (void) powerDown;
    (void) enableClkAt8MHz;

    console.init();
    console.baud(115200, fullSpeedClock());
    led.mode(Pinmode::out);

    adc.init();
    spi.init();
    //rf.init(63, 42, 8683);
    //rf.txPower(0);

    while (true) {
        int aVal = adc.read(ct1);
#if 0
        printf("%d %d\n", ticks, aVal);
#else
        for (int i = 0; i < aVal; i += 60)
            console.putc('-');
        printf("+\n");
#endif
        wait_ms(10);
    }
}

#include <jee.h>
#include <jee/i2c-sht2x.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinC<13> led;

I2cBus< PinB<7>, PinB<6>, 2 > bus;
SHT2x< decltype(bus) > ambient;

template< typename T >
void detectI2c (T bus) {
    for (int i = 0; i < 128; i += 16) {
        printf("%02x:", i);
        for (int j = 0; j < 16; ++j) {
            int addr = i + j;
            if (0x08 <= addr && addr <= 0x77) {
                bool ack = bus.start(addr<<1);
                bus.stop();
                printf(ack ? " %02x" : " --", addr);
            } else
                printf("   ");
        }
        printf("\n");
    }
}

int main() {
    console.init();
    //console.baud(115200, fullSpeedClock());
    enableSysTick();
    led.mode(Pinmode::out);

    detectI2c(bus);

    ambient.init();

    while (true) {
        //bool ack = bus.start(0x40<<1);
        //bus.stop();

        int temp = ambient.temp100();
        int humi = ambient.humidity10();
        printf("%d t %d h %d\n", ticks, temp, humi);

        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(900);
    }
}

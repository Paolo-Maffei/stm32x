// see [1] https://jeelabs.org/ref/STM32F4-RM0090.pdf

#include <jee.h>

UartDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

#include "mem-ili9341.h"

ILI9341< 0x60080000 > lcd;

static void initFsmcLcd () {
    MMIO32(Periph::rcc + 0x38) |= (1<<0);  // enable FSMC [1] p.245

    Port<'D'>::modeMap(0b1110011110110011, Pinmode::alt_out, 12);
    Port<'E'>::modeMap(0b1111111110000000, Pinmode::alt_out, 12);

    constexpr uint32_t bcr1 = Periph::fsmc;
    constexpr uint32_t btr1 = bcr1 + 0x04;
    MMIO32(bcr1) = (1<<12) | (1<<7) | (1<<4);
    MMIO32(btr1) = (1<<20) | (3<<8) | (1<<4) | (1<<0);
    MMIO32(bcr1) |= (1<<0);
}

PinA<6> led;
PinB<1> backlight;

int main () {
    //backlight.mode(Pinmode::out); // turn backlight off

    console.init();
    console.baud(115200, fullSpeedClock()/2);
    printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    led.mode(Pinmode::out);

    initFsmcLcd();
    lcd.init();
    uint32_t start = ticks;
    for (int i = 0; i < 100; ++i)
        lcd.clear();
    printf("%d ms\n", ticks - start);

    //backlight = 1;

    //lcd.fill(0, 0, 240, 160, 0xF800);
    //lcd.fill(0, 0, 240, 320, 0x001F);
    //lcd.fill(0, 0, 240, 320, 0x07E0);

    while (true) {
        printf("hello %d\n", ticks);
        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(400);

        static uint16_t colour = 0xF000;
        lcd.fill(0, 0, 140, 300, colour);
        lcd.fill(200, 10, 10, 200, colour);
        colour = ~colour;

        static uint8_t s = 0;
        lcd.vscroll(s);
        s += 8;
    }
}

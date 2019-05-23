// see [1] https://jeelabs.org/ref/STM32F4-RM0090.pdf

#include <jee.h>
#include <jee/mem-ili9486.h>

ILI9486<0x60100000> lcd;

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

static void initFsmcLcd () {
    MMIO32(Periph::rcc + 0x38) |= (1<<0);  // enable FSMC [1] p.245

    Port<'D'>::modeMap(0b1100000010110011, Pinmode::alt_out, 12);
    Port<'E'>::modeMap(0b0000011110010000, Pinmode::alt_out, 12);

    constexpr uint32_t bcr1 = Periph::fsmc;
    constexpr uint32_t btr1 = bcr1 + 0x04;
    MMIO32(bcr1) = (1<<12) | (1<<7);
    MMIO32(btr1) = (5<<8); // a slight delay is needed to avoid signal errors
    MMIO32(bcr1) |= (1<<0);
}

PinE<5> lcd_rst;

int main () {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    //enableSysTick();

    lcd_rst.mode(Pinmode::out);
    lcd_rst = 1;

    initFsmcLcd();
    lcd.init();

    uint32_t t = ticks;
    lcd.clear();
    printf("clear %d ms\n", ticks - t);

    for (int i = 0; i < 200; ++i)
        lcd.pixel(i, 2*i, 0xF800); // draw some red pixels

    while (true) {}
}

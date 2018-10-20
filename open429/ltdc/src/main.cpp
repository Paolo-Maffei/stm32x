#include <jee.h>
#include <jee/spi-ili9341.h>
#include <string.h>

UartBufDev< PinA<9>, PinA<10> > console;
#include <../../common.h>

SpiGpio< PinF<9>, PinF<8>, PinF<7>, PinC<2> > spi;
PinD<13> wrx;
ILI9341< decltype(spi), decltype(wrx) > lcd;

int main() {
	console.init();
	fullSpeedClock();
	printf("hello\n");

	initFmcSdram();

#if 0
	// LCD_CtrlLinesConfig
	PinC<2> ncs;
	PinD<13> wrx;
	ncs.mode(Pinmode::out);
	wrx.mode(Pinmode::out);
    ncs = 1;

    ncs = 0; // enable

	// LCD_SPIConfig
    //MMIO32(Periph::rcc + 0x44) = (1<<20);  // enable SPI5
	SpiGpio< PinF<9>, PinF<8>, PinF<7>, SlowPin< PinC<2>, 10> > spiLcd;
	spiLcd.init();

	// LCD_PowerOn
#endif

	spi.init();
	lcd.init();

#error "Actual LTDC code missing, this is an EARLY attempt"

	for (int x = 0; x < 240; ++x)
		for (int y = 0; y < 320; ++y)
			lcd.pixel(x, y, 0xF800);

	for (int i = 0; i < 256; ++i) {
		printf("%d %d\n", i, ticks);
		memset((void*) sdram, 0, 8 * 1024 * 1024);
		wait_ms(1000);
	}

    while (1) {
		printf("%08x %08x %d\n", MMIO32(sdram+0), MMIO32(sdram+4), ticks);
		wait_ms(1000);
	}
}

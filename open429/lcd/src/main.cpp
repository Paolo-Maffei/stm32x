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

	spi.init();
	lcd.init();

	for (int x = 0; x < 240; ++x)
		for (int y = 0; y < 320; ++y)
			lcd.pixel(x, y, 0xFFE0);

    while (1) {}
}

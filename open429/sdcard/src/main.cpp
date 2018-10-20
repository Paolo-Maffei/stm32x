#include <jee.h>
#include <jee/spi-sdcard.h>

UartBufDev< PinA<9>, PinA<10> > console;
#include <../../common.h>

SpiGpio< PinD<2>, PinC<8>, PinC<12>, PinC<11> > spi;
SdCard< decltype(spi) > sd;

int main() {
	console.init();
	fullSpeedClock();
	printf("hello\n");

	spi.init();
	printf("sd %d sdhc %d\n", sd.init(), sd.sdhc);

    while (1) {}
}

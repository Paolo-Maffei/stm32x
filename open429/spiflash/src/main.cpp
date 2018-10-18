#include <jee.h>
#include <jee/spi-flash.h>

UartBufDev< PinA<9>, PinA<10> > console;
#include <../../common.h>

SpiGpio< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;
SpiFlash< decltype(spi) > flash;

int main() {
	console.init();
	fullSpeedClock();

	spi.init();
	flash.init();
	printf("flash %x\n", flash.devId());

    while (1) {}
}

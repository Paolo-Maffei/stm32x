#include <jee.h>
#include <jee/spi-max7219.h>

PinC<13> led;

SpiGpio< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;
MAX7219< decltype(spi) > display;
CanDev can;

static void hexDigit (int pos, int val) {
	const uint8_t segMap [] = {
		0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70,
		0x7F, 0x73, 0x77, 0x1F, 0x4E, 0x3D, 0x4F, 0x47,
	};
	display.sendOne(0, 8 - pos, segMap[val & 0x0F]);
}

int main() {
    fullSpeedClock();

	led.mode(Pinmode::out);
	led = 1;

    can.init(true);
    can.filterInit(0);

	spi.init();
	display.init(1);

    while (1) {
        int len, id, dat[2];
        len = can.receive(&id, dat);
        if (len >= 0) {
			display.clear();

			// show 11-bit packet id
			hexDigit(0, id >> 8);
			hexDigit(1, id >> 4);
			hexDigit(2, id);

			// show first payload byte(s)
			if (len > 1) {
				hexDigit(6, dat[0] >> 12);
				hexDigit(7, dat[0] >> 8);
			}
			if (len > 0) {
				hexDigit(4, dat[0] >> 4);
				hexDigit(5, dat[0]);
			}

#if 0
			// show a dot to indicate packet length
			if (len > 0)
				display.pixel(7, len - 1, 1);
#endif

            led.toggle();
        }
    }
}

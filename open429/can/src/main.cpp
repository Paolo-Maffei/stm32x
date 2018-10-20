#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;
#include <../../common.h>

// both CAN drivers have to be wired together and terminated 2x
CanDev<0> can1;
CanDev<1> can2;

int main() {
	console.init();
	fullSpeedClock();
	printf("hello!\n");

	can1.init();
	can2.init();

	can1.filterInit(14); // always set filters via can1

	uint32_t last = 0;
    while (1) {
		if (ticks / 500 != last) {
			last = ticks / 500;
			//printf("%d\n", ticks);

			can1.transmit();
		}

		if (can2.rxPending()) {
			printf("R %d\n", ticks);
			can2.rxClear();
		}
	}
}

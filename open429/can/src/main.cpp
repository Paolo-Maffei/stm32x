#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;
#include <../../common.h>

template< int N >
struct CanDev {
    constexpr static uint32_t base = N == 0 ? 0x40006400 : 0x40006800;

    constexpr static uint32_t mcr  = base + 0x000;
    constexpr static uint32_t msr  = base + 0x004;
    constexpr static uint32_t tsr  = base + 0x008;
    constexpr static uint32_t btr  = base + 0x01C;
    constexpr static uint32_t tir  = base + 0x180;
    constexpr static uint32_t tdtr = base + 0x184;
    constexpr static uint32_t tdlr = base + 0x188;
    constexpr static uint32_t tdhr = base + 0x18C;

	static void init () {
		if (N == 0) {
			// alt mode CAN1:    5432109876543210
			Port<'B'>::modeMap(0b0000001100000000, Pinmode::alt_out, 9);
			MMIO32(Periph::rcc + 0x40) |= (1<<25);  // enable CAN1
		} else {
			// alt mode CAN2:    5432109876543210
			Port<'B'>::modeMap(0b0000000001100000, Pinmode::alt_out, 9);
			MMIO32(Periph::rcc + 0x40) |= (1<<26);  // enable CAN2
		}
		MMIO32(mcr) &= ~(1<<1); // exit sleep
		MMIO32(mcr) |= (1<<0); // init req
		while ((MMIO32(msr) & (1<<0)) == 0) {}
		MMIO32(btr) = (7<<20) | (5<<16) | (2<<0); // 1 MBps
		MMIO32(mcr) &= ~(1<<0); // init leave
		while (MMIO32(msr) & (1<<0)) {}
	}

	static void transmit () {
		if (MMIO32(tsr) & (1<<26)) { // TME0
			printf("T %d\n", ticks);

			MMIO32(tir) = (0x321<<21);
			MMIO32(tdtr) = (8<<0);
			MMIO32(tdlr) = 0x11223344;
			MMIO32(tdhr) = 0x05060708;

			MMIO32(tir) |= (1<<0); // TXRQ
		}
	}
};

CanDev<0> can1;
CanDev<1> can2;

void initCan1 () {
	// alt mode CAN1:    5432109876543210
    Port<'B'>::modeMap(0b0000001100000000, Pinmode::alt_out, 9);
    MMIO32(Periph::rcc + 0x40) = (1<<25);  // enable CAN1
}

int main() {
	console.init();
	fullSpeedClock();
	printf("hello!\n");

printf("11\n");
	can1.init();
printf("22\n");
	can2.init();
printf("33\n");
	can1.transmit();

	uint32_t last = 0;
    while (1) {
		if (ticks / 500 != last) {
			last = ticks / 500;
			//printf("%d\n", ticks);

			can1.transmit();
		}
	}
}

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
            printf("T %d\n", ticks);
            can1.transmit(0x123, "abcd1234", 8);
        }

        int len, id, dat[2];
        len = can2.receive(&id, dat);
        if (len >= 0) {
            printf("R %d @%x #%d: %08x %08x\n", ticks, id, len, dat[0], dat[1]);
        }
    }
}

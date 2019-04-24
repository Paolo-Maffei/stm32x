// splitting an app in two:
//  - lomem loads at 0x08000000 with ram at 0x20000000 (same as usual)
//  - himem loads at 0x08004000 with ram at 0x10000000 (lee himem.ld)
//  - both lomem.ld and himem.ld define memory sizes of only 10K each
//  - that way, everything stays out of each other's way, including ram clear
// when started, the app in lomem jumps to the main entry of the app in himem

#include <jee.h>

PinB<9> led;

int main() {
    led.mode(Pinmode::out);

#if LOMEM
    const uint32_t* himem = (const uint32_t*) 0x08004000;
    void (*start)() = (void (*)()) himem[1];
    start();
#else
    while (true) {
        led = 0;
        for (int i = 0; i < 1000000; ++i) __asm("");
        led = 1;
        for (int i = 0; i < 1000000; ++i) __asm("");
    }
#endif
}

// splitting an app in two:
//  - lomem loads at 0x08000000 with ram at 0x20000000 (same as usual)
//  - himem loads at 0x08004000 with ram at 0x10000000 (lee himem.ld)
//  - both lomem.ld and himem.ld define memory sizes of only 10K each
//  - that way, everything stays out of each other's way, including ram clear
// when started, the app in lomem jumps to the main entry of the app in himem

#include <jee.h>

void(**linkArea)() = (void(**)()) 0x10004000;

#if LOMEM
UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinB<9> led;

static void toggleLed () {
    led.toggle();
}

int main() {
    console.init();
    enableSysTick();
    led.mode(Pinmode::out);

    *linkArea = toggleLed;

    const uint32_t* himem = (const uint32_t*) 0x08004000;
    void (*start)() = (void (*)()) himem[1];
    start();
}

#else

// this code calls back into lomem to toggle the LED
int main() {
    while (true) {
        (*linkArea)(); // led.toggle()
        for (int i = 0; i < 1000000; ++i) __asm("");
    }
}

// disabling SystemInit() allows interrupts in lomem to continue working
// see https://arm-software.github.io/CMSIS_5/Core_A/html/group__system__init__gr.html
extern "C" void SystemInit () {}

#endif

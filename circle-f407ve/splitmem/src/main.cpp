// splitting an app in two:
//  - lomem loads at 0x08000000 with ram at 0x20000000 (same as usual)
//  - himem loads at 0x20010000 with ram at 0x20014000 (see himem.ld)
//  - both lomem.ld and himem.ld define memory sizes of only 10K each
//  - that way, everything stays out of each other's way, including ram clear
//  - there's a "link area" at 0x2000FFF0 for himem to call lomem code
// when started, the app in lomem jumps to the main entry of the app in himem

#include <stdint.h>

typedef struct {
    void (*toggleLed)();
    int (*printf)(const char* fmt, ...);
    void (*wait_ms)(uint32_t);
} LowCalls;

LowCalls* linkArea = (LowCalls*) 0x2000FFF0;

#if LOMEM
#include <jee.h>

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
    console.baud(115200, fullSpeedClock()/2);
    led.mode(Pinmode::out);

    linkArea->toggleLed = toggleLed;
    linkArea->printf = printf;
    linkArea->wait_ms = wait_ms;

    const uint32_t* himem = (const uint32_t*) 0x20010000;
    void (*start)() = (void (*)()) himem[1];

    printf("jump to himem\n");
    start();
    printf("returned ?\n");
}

#else

#define toggleLed   linkArea->toggleLed
#define printf      linkArea->printf
#define wait_ms     linkArea->wait_ms

// this code calls back into lomem to toggle the LED
int main () {
    printf("now in himem\n");
    for (int n = 0; n < 10; ++n) {
        toggleLed(); // led.toggle()
        wait_ms(500);
        printf("%d\n", n);
    }
    while (true) {}  // can't return
}

// disabling SystemInit() allows interrupts in lomem to continue working
// see https://arm-software.github.io/CMSIS_5/Core_A/html/group__system__init__gr.html
extern "C" void SystemInit () {}

#endif

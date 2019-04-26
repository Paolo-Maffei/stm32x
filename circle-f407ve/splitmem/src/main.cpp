// splitting an app in two:
//  - lomem loads at 0x08000000 with ram at 0x20000000 (same as usual)
//  - himem loads at 0x08004000 with ram at 0x10000000 (lee himem.ld)
//  - both lomem.ld and himem.ld define memory sizes of only 10K each
//  - that way, everything stays out of each other's way, including ram clear
// when started, the app in lomem jumps to the main entry of the app in himem

#include <jee.h>
#include <setjmp.h>

typedef struct {
    void (*toggleLed)();
    int (*printf)(const char* fmt, ...);
    void (*jumpInfo)();
} LowCalls;

LowCalls*& linkArea = *(LowCalls**) 0x10004000;

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

// This code helps figure out how to re-use the setjmp/longjmp mechanism for
// task switching - all we need are the offsets where SP and PC are stored.

jmp_buf jb;

void dumpJump (bool immediate) {
    if (immediate) {
        setjmp(jb); // fill in jb

        uint32_t sp = (uint32_t) &immediate; // the real SP is slightly lower
        uint32_t pc = (uint32_t) &dumpJump;  // the real PC is slightly higher
        printf("'setjmp' layout guess: sp < %08x, pc > %08x:", sp, pc);

        for (int i = 0; i < sizeof jb / 4; ++i) {
            if (i % 8 == 0)
                printf("\n%6d: ", i);
            printf(" %08x", ((uint32_t*) &jb)[i]);
        }
        printf("\n");

        // find the most plausible location of sp in the jb buffer
        for (int i = 0; i < sizeof jb / 4; ++i)
            if (sp - ((uint32_t*) &jb)[i] < 10)
                printf("  best guess for SP is word #%d (%d off)\n",
                        i, sp - ((uint32_t*) &jb)[i]);

        // find the most plausible location of sp in the jb buffer
        for (int i = 0; i < sizeof jb / 4; ++i)
            if (((uint32_t*) &jb)[i] - pc < 30)
                printf("  best guess for PC is word #%d (%d off)\n",
                        i, ((uint32_t*) &jb)[i] - pc);
        return;
    }
    
    // else use recursion to add an extra stack frame
    dumpJump(!immediate);
}

void jumpInfo () {
    printf("FIRST ");
    dumpJump(true);
    printf("SECOND ");
    dumpJump(false);
    printf("sizeof (jmp_buf) = %d bytes\n", sizeof (jmp_buf));
}

LowCalls lowCalls = {
    toggleLed,
    printf,
    jumpInfo,
};

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    led.mode(Pinmode::out);

    wait_ms(500);
    jumpInfo();

    linkArea = &lowCalls;

    const uint32_t* himem = (const uint32_t*) 0x08004000;
    void (*start)() = (void (*)()) himem[1];
    start();
}

#else

#define toggleLed   linkArea->toggleLed
#define printf      linkArea->printf
#define jumpInfo    linkArea->jumpInfo

// this code calls back into lomem to toggle the LED
int main() {
    printf("control transferred to himem\n");
    jumpInfo();
    int n = 0;
    while (true) {
        toggleLed(); // led.toggle()
        for (int i = 0; i < 10000000; ++i) __asm("");
        printf("%d\n", ++n);
    }
}

// disabling SystemInit() allows interrupts in lomem to continue working
// see https://arm-software.github.io/CMSIS_5/Core_A/html/group__system__init__gr.html
extern "C" void SystemInit () {}

#endif

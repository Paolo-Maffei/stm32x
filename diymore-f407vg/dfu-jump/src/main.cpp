// Jump into the DFU boot loader from running app

#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

void resetClocks () {  // HAL_RCC_DeInit
    Periph::bit(Periph::rcc+0x00, 0) = 1; // HSION
    while (Periph::bit(Periph::rcc+0x00, 1) == 0) {} // wait for HSIRDY
    MMIO32(Periph::rcc+0x08) = 0; // CFGR
    MMIO32(Periph::rcc+0x00) &= ~(1<<26) & ~(1<<24) & ~(1<<19) & ~(1<<16);
    Periph::bit(Periph::rcc+0x00, 18) = 0; // HSEBYP
    MMIO32(Periph::rcc+0x04) = 0; // PLLCFGR
    MMIO32(Periph::rcc+0x04) = 0x24003010;
    MMIO32(Periph::rcc+0x84) = 0; // PLLI2SCFGR
    MMIO32(Periph::rcc+0x84) = 0x20003000;
    MMIO32(Periph::rcc+0x0C) = 0; // CIR

    constexpr static uint32_t tick = 0xE000E010; // SysTick
    MMIO32(tick + 0x00) = 0;
    MMIO32(tick + 0x04) = 0;
    MMIO32(tick + 0x08) = 0;
}

void resetPeriphs () {  // HAL_DeInit
    MMIO32(Periph::rcc+0x20) = 0xFFFFFFFF; // APB1RSTR
    MMIO32(Periph::rcc+0x20) = 0;
    MMIO32(Periph::rcc+0x24) = 0xFFFFFFFF; // APB2RSTR
    MMIO32(Periph::rcc+0x24) = 0;
    MMIO32(Periph::rcc+0x10) = 0xFFFFFFFF; // AHB1RSTR
    MMIO32(Periph::rcc+0x10) = 0;
    MMIO32(Periph::rcc+0x14) = 0xFFFFFFFF; // AHB2RSTR
    MMIO32(Periph::rcc+0x14) = 0;
    MMIO32(Periph::rcc+0x18) = 0xFFFFFFFF; // AHB3RSTR
    MMIO32(Periph::rcc+0x18) = 0;
}

#define RAMFUNC __attribute__ ((noinline, long_call, section (".data")))

void jumpToBootLoader () {
    __asm volatile ("movs r3, #0\n"
                    "MSR primask, r3" : : : "r3");

#if 1
    constexpr uint32_t syscfg = 0x40013800;
    MMIO32(syscfg) = 1;  // memory remap mode 1, i.e. system flash

    __asm volatile ("movs r3, #0\n"
                    "ldr r3, [r3, #0]\n"
                    "MSR msp, r3\n"
        : : : "r3", "sp");

    ((void (*)(void)) MMIO32(0x00000004))();
#else
//  __asm volatile ("movw r3, #0x0000\n"
//                  "movt r3, #0x1FF0\n"
//                  "ldr r3, [r3, #0]\n"
//                  "MSR msp, r3\n"
//      : : : "r3", "sp");
    __asm volatile ("movw r3, #0x2000\n"
                    "movt r3, #0x2000\n"
                    "MSR msp, r3\n"
        : : : "r3", "sp");

    ((void (*)(void)) MMIO32(0x1FFF0004))();
#endif
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    PinE<0> led;
    led.mode(Pinmode::out);

    for (int i = 0; i < 5; ++i) {
        printf("%d\n", ticks);
        led = 1;
        wait_ms(400);
        led = 0;
        wait_ms(100);
    }

    printf("%08x %08x %08x %08x\n", MMIO32(0xE000ED08), MMIO32(0x1FFF0000),
                                    MMIO32(0x1FFF0004), jumpToBootLoader);
    wait_ms(5); // slight delay to flush TX

    resetClocks();
    //resetPeriphs();
    jumpToBootLoader();

    while (1) {}
}

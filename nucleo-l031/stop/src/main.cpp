#include <jee.h>

PinA<1> led;

RTC rtc;

int main() {
    Port<'A'>::modeMap(0b1111111111111111, Pinmode::in_analog);
    led.mode(Pinmode::out_od);

    rtc.init();
    rtc.wakeup(2000);

    constexpr uint32_t scr = 0xE000ED10;
    MMIO32(scr) |= (1<<2); // set SLEEPDEEP

    // FWU, ULP, CWUF, LPSDSR
    MMIO32(Periph::pwr) |= (1<<10) | (1<<9) | (1<<2) | (1<<0);

    while (true) {
        led = 0;
        for (int i = 0; i < 5; ++i) __asm("");
        led = 1;

        MMIO32(rtc.isr) &= ~(1<<10);  // clear WUTF

        __asm("wfe");
    }
}

extern "C" void SystemInit () {}

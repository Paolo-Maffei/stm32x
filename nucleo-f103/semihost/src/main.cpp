// semihosting setup:
//
// $ openocd -f board/st_nucleo_f103rb.cfg
//
// or: openocd -f interface/stlink-v2-1.cfg -f target/stm32f1x_stlink.cfg
//
// $ arm-none-eabi-gdb .pioenvs/nucleo/firmware.elf
// (gdb) tar rem :3333
// (gdb) mon arm semihosting enable
// (gdb) load
// (gdb) c

#include <jee.h>

static char semiBuf [100], *semiFill;

void semiCmd (int command, void* message) {
    asm("mov r0, %[cmd];"
            "mov r1, %[msg];"
            "bkpt #0xAB"
            :
            : [cmd] "r" (command), [msg] "r" (message)
            : "r0", "r1", "memory");
}

int printf(const char* fmt, ...) {
    semiFill = semiBuf;

    va_list ap;
    va_start(ap, fmt);
    veprintf([](int c) {
        if (semiFill < semiBuf + sizeof semiBuf)
            *semiFill++ = c;
    }, fmt, ap);
    va_end(ap);

    uint32_t m[] = { 2 /*stderr*/, (uint32_t) semiBuf, semiFill - semiBuf };
    semiCmd(5, m);
    return 0;
}

PinA<5> led;

int main() {
    enableSysTick(); // no HSE crystal on Nucleo
    led.mode(Pinmode::out);

    while (1) {
        printf("%d\n", ticks);
        led = 1;
        wait_ms(100);
        led = 0;
        wait_ms(400);
    }
}

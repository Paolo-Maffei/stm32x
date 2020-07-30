// JTAG via Olimex USB Tiny-H:
//
// $ openocd -f interface/ftdi/olimex-arm-usb-tiny-h.cfg -f target/stm32f4x.cfg
//
// $ arm-none-eabi-gdb .pioenvs/gold_dragon/firmware.elf
// [...]
// (gdb) tar rem :3333
// Remote debugging using :3333
// (gdb) load
// (gdb) c
//
// or: USB-MiniJTAG by Haoyu with "upload_protocol = jlink"

#include <jee.h>

UartBufDev< PinC<10>, PinC<11> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinE<0> led;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock() / 4);
    led.mode(Pinmode::out);

    while (true) {
        printf("%d\n", ticks);
        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(400);
    }
}

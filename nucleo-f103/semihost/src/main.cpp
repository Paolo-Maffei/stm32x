#include <jee.h>

static char semiBuf [100], *semiFill;

void send_command(int command, void *message) {
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
	uint32_t m[] = { 2/*stderr*/, (uint32_t) semiBuf, semiFill - semiBuf };
	send_command(0x05/* some interrupt ID */, m);
	return 0;
}

PinA<5> led;

int main() {
	enableSysTick(); // no HSE crystal
	led.mode(Pinmode::out);

    while (1) {
        printf("%d\n", ticks);
        led = 1;
        wait_ms(100);
        led = 0;
        wait_ms(400);
    }
}

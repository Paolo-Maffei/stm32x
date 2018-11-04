#include <jee.h>
#include <string.h>

UartDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

static int rxReady () { return console.readable(); }
static int rxReceive () { return console.getc(); }
static int txReady () { return console.writable(); }
static void txSend (char ch) { console.putc(ch); }

#include "pdp8.h"

uint8_t store [] = {
#include "rom.h"
};

int main () {
    console.init();
    run();
    return 0;
}

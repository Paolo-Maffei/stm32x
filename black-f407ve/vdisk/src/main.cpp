#include <jee.h>
#include <jee/spi-flash.h>
#include <jee/parse-cmd.h>

// FIXME input not working for interrupt-driven UartBufDev !
UartDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinA<6> led;

SpiGpio< PinB<5>, PinB<4>, PinB<3>, PinB<0> > spi;
SpiFlash< decltype(spi) > spif;

Command cmd;
int drive, track, sector;
uint8_t buffer [128];

static void dump (const uint8_t* buf, int len) {
    for (int i = 0; i < len; i += 16) {
        printf("%02x:", i);
        for (int j = 0; j < 16; ++j) {
            if (j % 4 == 0)
                printf(" ");
            printf(" %02x", buf[j]);
        }
        printf("\n");
        buf += 16;
    }
}

static uint32_t voffset (int drive, int track, int sector) {
    uint32_t offset = 128 * (sector + 26 * track) + 256 * 1024 * drive;
    printf("offset d %d t %d s %d => %06x\n", drive, track, sector, offset);
    return offset;
}

static void vread (int drive, int track, int sector) {
    uint32_t pos = voffset(drive, track, sector);
    spif.read(pos, buffer, sizeof buffer);
    dump(buffer, 128);
}

static void vwrite (int drive, int track, int sector) {
    uint32_t pos = voffset(drive, track, sector);
    if (pos % 4096 == 0)
        spif.erase(pos >> 8);
    spif.write(pos, buffer, 128);
}

int main() {
    console.init();
    //console.baud(115200, fullSpeedClock()/2);
    printf("\n-------------------------------------------------------------\n");
    led.mode(Pinmode::out);

    spi.init();
    spif.init();

    while (1) {
        if (console.readable()) {
            char c = console.getc();
            console.putc(c);
            if ('a' <= c && c <= 'z')
                printf("\n");

            switch (cmd.parse(c)) {
                case 'h':
                    printf("hello\n");
                    break;
                case 'i':
                    printf("spif id %x, %d kB\n", spif.devId(), spif.size());
                    break;
                case 'd':
                    drive = cmd.argc > 0 ? cmd.args[0] : 0;
                    break;
                case 't':
                    track = cmd.argc > 0 ? cmd.args[0] : 0;
                    sector = 0;
                    break;
                case 's':
                    sector = cmd.argc > 0 ? cmd.args[0] : 0;
                    break;
                case 'r':
                    vread(drive, track, sector);
                    break;
                case 'w':
                    vwrite(drive, track, sector);
                    break;
            }
        }
    }
}

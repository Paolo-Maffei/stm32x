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

static void vread () {
    uint32_t pos = voffset(drive, track, sector);
    spif.read(pos, buffer, sizeof buffer);
    dump(buffer, 128);
}

static void vwrite () {
    uint32_t pos = voffset(drive, track, sector);
    if (pos % 4096 == 0)
        spif.erase(pos >> 8);
    spif.write(pos, buffer, 128);
}

static void showMap (int d) {
    uint32_t pos = 256 * 1024 * d;
    printf("[%06x] %d:", pos, d);
    for (int i = 0; i < 64; ++i) {
        char mark = '0';
        for (int j = 0; j < 4096; j += 128) {
            spif.read(pos + j, buffer, sizeof buffer);
            for (int k = 0; k < sizeof buffer; ++k)
                if (buffer[k] != 0xFF) {
                    ++mark;
                    break;
                }
            if (mark > '9')
                break;
        }
        pos += 4096;
        if (i % 16 == 0)
            printf(" ");
        printf("%c", mark < '1' ? '.' : mark > '9' ? '+' : mark);
    }
    printf("\n");
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
                case 'h': // hello
                    printf("hello\n"); break;
                case 'i': // info
                    printf("id %x, %d kB\n", spif.devId(), spif.size()); break;
                case 'd': // drive
                    drive = cmd.argc > 0 ? cmd.args[0] : 0; break;
                case 't': // track
                    track = cmd.argc > 0 ? cmd.args[0] : 0; break;
                case 's': // sector
                    sector = cmd.argc > 0 ? cmd.args[0] : 0; break;
                case 'r': // read
                    vread(); break;
                case 'w': // write
                    vwrite(); break;
                case 'm': // map
                    for (int i = 0; i < 8; ++i) showMap(i); break;
            }
        }
    }
}

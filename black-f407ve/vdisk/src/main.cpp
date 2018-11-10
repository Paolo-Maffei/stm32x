#include <jee.h>
#include <jee/spi-flash.h>
#include <jee/parse-cmd.h>
#include <jee/text-ihex.h>

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

static void loadBuffer () {
    IntelHex<128> ihex;
    ihex.init();
    while (!ihex.parse(console.getc())) {}
    printf("\n");
    if (ihex.check != 0)
        printf("checksum error\n");
    else if (ihex.type == 0)
        for (int i = 0; i < ihex.len; ++i)
            buffer[(ihex.addr + i) % sizeof buffer] = ihex.data[i];
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
                case 'h': // help
                    printf(
                      "  i - Show Info, <n> [dts] - Set Drive/Track/Sector\n"
                      "  r - Read, w - Write, m - Show Map, b - Show Buffer\n"
                      "  :... - Fill 128-byte buffer with Intel HEX bytes\n");
                    break;
                case 'i': // info
                    printf("spif %x %dK, d %d t %d s %d\n",
                        spif.devId(), spif.size(), drive, track, sector); break;
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
                case ':': // intel hex
                    loadBuffer(); break;
                case 'b': // dump buffer contents
                    dump(buffer, sizeof buffer); break;
            }
        }
    }
}

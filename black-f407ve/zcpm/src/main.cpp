// Launch CP/M, with virtual disks on SPI flash and SD card.

#include <jee.h>
#include <jee/spi-flash.h>
#include <jee/spi-sdcard.h>
#include <jee/mem-ili9341.h>
#include <jee/text-font.h>
#include <string.h>
#include "spi-wear.h"
#include "flashwear.h"
#include "drive.h"
#include "cpmdate.h"

extern "C" {
#include "context.h"
#include "z80emu.h"
#include "macros.h"
}

const uint8_t rom [] = {
#include "rom.h"
};

const uint8_t ram [] = {
#include "ram.h"
};

ILI9341<0x60080000> lcd;
TextLcd< decltype(lcd) > text;
Font5x7< decltype(text) > screen;

UartBufDev< PinA<9>, PinA<10> > console;

PinE<4> key0;
PinE<3> key1;
PinB<1> backlight;

static void putcBoth (int c) {
    console.putc(c);
    screen.putc(c);
}

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(putcBoth, fmt, ap); va_end(ap);
	return 0;
}

RTC rtc;
Context context;
Drive drives [8];

static void setBankSplit (uint8_t page) {
    context.split = MAINMEM + (page << 8);
    memset(context.offset, 0, sizeof context.offset);
#if NBANKS > 1
    static uint8_t bankedMem [96*1024]; // additional memory banks on F407
    uint8_t* base = bankedMem;
    for (int i = 1; i < NBANKS; ++i) {
        uint8_t* limit = base + (page << 8);
        if (limit > bankedMem + sizeof bankedMem)
            break; // no more complete banks left
        context.offset[i] = base - MAINMEM;
        base = limit;
    }
#endif
}

static void initFsmcLcd () {
    MMIO32(Periph::rcc + 0x38) |= (1<<0);  // enable FSMC [1] p.245

    Port<'D'>::modeMap(0b1110011110110011, Pinmode::alt_out, 12);
    Port<'E'>::modeMap(0b1111111110000000, Pinmode::alt_out, 12);

    constexpr uint32_t bcr1 = Periph::fsmc;
    constexpr uint32_t btr1 = bcr1 + 0x04;
    MMIO32(bcr1) = (1<<12) | (1<<7) | (1<<4);
    MMIO32(btr1) = (1<<20) | (3<<8) | (1<<4) | (1<<0);
    MMIO32(bcr1) |= (1<<0);
}

void SystemCall (Context* z, int req) {
    Z80_STATE* state = &(z->state);
    //printf("req %d A %d\n", req, A);
    switch (req) {
        case 0: // coninst
            A = console.readable() ? 0xFF : 0x00;
            break;
        case 1: // conin
            A = console.getc();
            break;
        case 2: // conout
            putcBoth(C);
            break;
        case 3: // constr
            for (uint16_t i = DE; *mapMem(&context, i) != 0; i++)
                putcBoth(*mapMem(&context, i));
            break;
        case 4: { // read/write
            //  ld a,(sekdrv)
            //  ld b,1 ; +128 for write
            //  ld de,(seksat)
            //  ld hl,(dmaadr)
            //  in a,(4)
            //  ret
            bool out = (B & 0x80) != 0;
            uint8_t sec = DE, trk = DE >> 8, dsk = A, cnt = B & 0x7F;

            A = 0;
            for (int i = 0; i < cnt; ++i) {
                // TODO careful with wrapping across paged memory boundary!!!
                void* mem = mapMem(&context, HL + 128 * i);
                if (out)
                    drives[dsk].write(trk, sec+i, mem);
                else
                    drives[dsk].read(trk, sec+i, mem);
            }
            break;
        }
        case 5: { // time get/set
            if (C == 0) {
                RTC::DateTime dt = rtc.get();
                //printf("mdy %02d/%02d/20%02d %02d:%02d:%02d (%d ms)\n",
                //        dt.mo, dt.dy, dt.yr, dt.hh, dt.mm, dt.ss, ticks);
                uint8_t* ptr = mapMem(&context, HL);
                int t = date2dr(dt.yr, dt.mo, dt.dy);
                ptr[0] = t;
                ptr[1] = t>>8;
                ptr[2] = dt.hh + 6*(dt.hh/10); // hours, to BCD
                ptr[3] = dt.mm + 6*(dt.mm/10); // minutes, to BCD
                ptr[4] = dt.ss + 6*(dt.ss/10); // seconcds, to BCD
            } else {
                RTC::DateTime dt;
                uint8_t* ptr = mapMem(&context, HL);
                // TODO set clock date & time
                dr2date(*(uint16_t*) ptr, &dt);
                dt.hh = ptr[2] - 6*(ptr[2]>>4); // hours, from BCD
                dt.mm = ptr[3] - 6*(ptr[3]>>4); // minutes, from BCD
                dt.ss = ptr[4] - 6*(ptr[4]>>4); // seconds, from BCD
                rtc.set(dt);
            }
            break;
        }
        case 6: // banked memory config
            setBankSplit(A);
            break;
        case 7: // selmem
            context.bank = A % NBANKS;
            break;
        case 8: { // for use in xmove, inter-bank copying of 1..256 bytes
            uint8_t *src = MAINMEM + DE, *dst = MAINMEM + HL;
            // never map above the split, i.e. in the common area
            if (dst < context.split)
                dst += context.offset[B % NBANKS];
            if (src < context.split)
                src += context.offset[C % NBANKS];
            // TODO careful, this won't work across the split!
            int n = 256 - A; // e.g. 0 -> 256b, 128 -> 128b
            memcpy(dst, src, n);
            DE += n;
            HL += n;
            break;
        }
        default:
            printf("syscall %d @ %04x ?\n", req, state->pc);
            while (1) {}
    }
}

int main() {
    backlight.mode(Pinmode::out); // turn backlight off as soon as possible

    console.init();
    console.baud(115200, fullSpeedClock()/2);

    initFsmcLcd();
    lcd.init();
    lcd.clear();

    printf("\r\n"); // this goes to lcd as well as serial

    key0.mode(Pinmode::in_pullup); // inverted logic
    key1.mode(Pinmode::in_pullup); // inverted logic

    rtc.init();

    // check internal flash before it gets inited (by Drive::assign() below)
    bool flashIsEmpty = ! iflash.valid();

    static char names [][12] = {
        "        1  ",  // A:
#if 1
        "        11 ",  // B:
        "CPMA    F  ",  // C:
#else
        "        2  ",  // B:
        "        3  ",  // C:
#endif
        "T1      F  ",  // D:
        "T2      FD ",  // E:
        "T3      DSK",  // F:
        "T4      DSK",  // G:
        "T5      DSK",  // H:
    };

    for (int i = 0; i < 8; ++i)
        if (names[i][0] == ' ')
            drives[i].assign(names[i]);

    if (flashIsEmpty) {
        printf("set up system tracks and directories\n");
        // copy system rom to system tracks
        for (uint32_t i = 0; i < sizeof rom; i += 128)
            drives[0].write(0, i / 128, rom + i);
        // reformat drives to clear all directories
        uint8_t buf [128];
        memset(buf, 0xE5, sizeof buf);
        for (int i = 0; i < 16; ++i)
            drives[0].write(2, i, buf);
        for (int i = 0; i < 16; ++i)
            drives[1].write(2, i, buf);
        for (int i = 0; i < 16; ++i)
            drives[2].write(2, i, buf);
    }

    spi2.init();
    if (sdCard.init()) {
        //printf("[sd card: sdhc %d]\n", sdCard.sdhc);
        fatFs.init();
#if 0
        printf("base %d spc %d rdir %d rmax %d data %d clim %d\n",
                                    fatFs.base, fatFs.spc, fatFs.rdir,
                                    fatFs.rmax, fatFs.data, fatFs.clim);
#endif
        listSdFiles();

        for (int i = 0; i < 8; ++i)
            if (names[i][0] != ' ')
                drives[i].assign(names[i]);
    }

    // The "K0" and "K1" buttons are checked on power-up and reset:
    //  - both pressed: wipe all, re-install system and clear directory on 1
    //  - left "K0" button pressed: clear directory on 1
    //  - right "K1" button pressed: re-install system on 1
    // Disk 1 is the first 256 KB of spi flash, normally mounted as A:

    if (!key0 && !key1) { // wipe entire spi flash
        printf("[erasing SPI flash] ");
        spiFlash.wipe();
    }

    if (!key1) { // set up system tracks on A:
        printf("[updating system tracks] ");
        for (uint32_t i = 0; i < sizeof rom; i += 128)
            drives[0].write(0, i / 128, rom + i);
    }

    if (!key0) { // set up empty directory blocks on A:
        printf("[clearing boot directory] ");
        uint8_t buf [128];
        memset(buf, 0xE5, sizeof buf);
        for (int i = 0; i < 16; ++i)
            // TODO start track depends on disk parameters
            drives[0].write(2, i, buf);
    }

    // wait for both keys to be released
    while (!key0 || !key1)
        wait_ms(100); // debounce

    // emulate a boot loader which loads the first block of A: at 0x0000
    drives[0].read(0, 0, mapMem(&context, 0));
    // and leave a copy of HEXSAVE.COM in the TPA for saving in CP/M
    memcpy(mapMem(&context, 0x0100), ram, sizeof ram);

    lcd.clear();
    wait_ms(20); // avoids screen flash
    backlight = 1;

    Z80Reset(&context.state);

    do {
        Z80Emulate(&context.state, 4000000, &context);
        if (*mapMem(&context, context.state.pc-1) == 0x76)
            break; // HALT instruction

        static uint32_t last;
        if (last != ticks/3000) {
            last = ticks/3000; // approx every 3s ...
            reBlock128();      // ... flush pending changes to SD card
        }
    } while (!context.done);

    printf("\nhalted at %04xh\n", context.state.pc-1);

    while (1) {}
}

// Launch CP/M, with virtual disks on SPI flash and SD card.

#include <jee.h>
#include <jee/spi-flash.h>
#include <jee/spi-sdcard.h>
#include <jee/mem-ili9341.h>
#include <jee/text-font.h>
#include <string.h>
#include "spi-wear.h"
#include "flashwear.h"

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

UartDev< PinA<9>, PinA<10> > console;
PinE<4> key0;
PinE<3> key1;

static void putcBoth (int c) {
    console.putc(c);
    screen.putc(c);
}

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(putcBoth, fmt, ap); va_end(ap);
	return 0;
}

// chip select needs a minute slowdown to work at 168 MHz
SpiGpio< PinB<5>, PinB<4>, PinB<3>, SlowPin< PinB<0>, 2 > > spi1;
SpiFlash< decltype(spi1) > spiFlash;
SpiWear< decltype(spiFlash), PinA<6> > spiWear;

SpiGpio< PinD<2>, PinC<8>, PinC<12>, PinC<11> > spi2;
SdCard< decltype(spi2) > sdCard;
FatFS< decltype(sdCard) > fatFs;

FlashWear iflash;

// TODO yucky init
typedef FileMap< decltype(fatFs), 257 > DiskMap; // 8M = 256 fat entries x 32K
DiskMap disks [] = {fatFs,fatFs,fatFs,fatFs,fatFs,fatFs,fatFs,fatFs};
DiskMap* currDisk;

RTC rtc;

Context context;

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

static void* reBlock128 (DiskMap* dmp =0, int blk =0, bool dirty =false) {
    static int currSect;
    static bool currDirty;
    static uint8_t currBuf [512];

    int sect = blk / 4;  // CP/M blocks are 128b, SD card sectors are 512b
    if (dmp != currDisk || sect != currSect) {
        if (currDirty && !currDisk->ioSect(true, currSect, currBuf))
            printf("WR-ERR: disk %d sect %d blk %d\n", dmp-disks, sect, blk);
        currDirty = false;
        if (dmp == 0)
            return 0; // it's a flush request, return value will be ignored
        currDisk = dmp;
        currSect = sect;
        if (currDisk != 0 && !currDisk->ioSect(false, currSect, currBuf))
            printf("RD-ERR: disk %d sect %d blk %d\n", dmp-disks, sect, blk);
    }
    currDirty |= dirty;
    return currBuf + (blk*128) % 512;
}

// see simh/dosplus/FDATE.MAC
//
// PROCEDURE drtodate(thedate : integer; VAR yr, mo, day : integer);
// (* 1 Jan 1978 corresponds to Digital Research date = 1  *)
// (* BUG - cannot handle negative values for dates > 2067 *)
//
//   VAR
//     i, y1        : integer;
//     dayspermonth : ARRAY[1..12] OF 1..31;
//
//   BEGIN (* drtodate *)
//   FOR i := 1 TO 12 DO dayspermonth[i] := 31;
//   dayspermonth[4] := 30; dayspermonth[6] := 30;
//   dayspermonth[9] := 30; dayspermonth[11] := 30;
//   IF thedate > 731 THEN BEGIN (* avoid overflows *)
//     yr := 1980; thedate := thedate - 731; END
//   ELSE BEGIN
//     thedate := thedate + 730; yr := 1976; END;
//   (* 0..365=y0; 366..730=y1; 731..1095=y2; 1096..1460=y3 *)
//   i := thedate DIV 1461; thedate := thedate MOD 1461;
//   y1 := (thedate-1) DIV 365; yr := yr + y1 + 4*i;
//   IF y1 = 0 THEN (* leap year *) dayspermonth[2] := 29
//   ELSE BEGIN
//     thedate := thedate - 1; (* 366 -> 365 -> 1 Jan *)
//     dayspermonth[2] := 28; END;
//   day := thedate - 365*y1 + 1; mo := 1;
//   WHILE day > dayspermonth[mo] DO BEGIN
//     day := day - dayspermonth[mo];
//     mo := succ(mo); END;
//   END; (* drtodate *)
//
// Incorporate (a) in year (c), overflows to century (b)

void dr2date (int date, RTC::DateTime* dt) {
    static uint8_t daysInMonth [] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 
    };

    int yr;
    if (date >731) {
        date -= 731;
        yr = 1980;
    } else {
        date += 730;
        yr = 1976;
    }
    int i = date / 1461;
    date %= 1461;
    int y1 = (date-1) / 365;
    yr += y1 + 4*i;
    if (y1 == 0)
        daysInMonth[1] = 29;
    else {
        --date;
        daysInMonth[1] = 28;
    }
    int day = date - 365*y1 + 1;
    int mon = 0;
    while (day > daysInMonth[mon]) {
        day -= daysInMonth[mon];
        ++mon;
    }
    dt->dy = day;
    dt->mo = mon+1;
    dt->yr = yr%100;
    //printf("dr2date: y %d m %d d %d\n", yr, mon+1, day);
}

// see https://www.oryx-embedded.com/doc/date__time_8c_source.html

uint16_t date2dr (int y, int m, int d) {
   // count Jan and Feb as months 13 and 14 of the previous year
   if(m <= 2) {
      m += 12;
      --y;
   }
   // this should work for at least y = 1..63 (i.e. 2001..2063)
   return 365*y + y/4 - y/100 + y/400 + 30*m + (3*(m+1))/5 + d + 8003;
}

void SystemCall (Context* z, int req) {
    static const uint8_t skewMap [] = {
        1,7,13,19,25,5,11,17,23,3,9,15,21,2,8,14,20,26,6,12,18,24,4,10,16,22
    };

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
            //printf("\nrw128: b%d a%04x o%d n%d d%d t%d s%d -> %d\n",
            //        context.bank, HL, out, cnt, dsk, trk, sec, skewMap[sec]);
            if (0 < dsk && dsk < 4 && dsk != 2)
                sec = skewMap[sec]-1;
            // TODO hard-coded for now, should use value in DPB
            int spt = dsk < 4 ? 26 : dsk < 5 ? 72 : 256;
            int blk = trk * spt + sec;

            A = 0;
            for (int i = 0; i < cnt; ++i) {
                // TODO careful with wrapping across paged memory boundary!!!
                void* mem = mapMem(&context, HL + 128 * i);
                if (dsk == 0) {
                    int off = + blk + i;
                    if (out)
                        spiWear.write128(off, mem);
                    else
                        spiWear.read128(off, mem);
                } else if (dsk == 2) {
                    int off = + blk + i;
                    if (out)
                        iflash.writeSector(off, mem);
                    else
                        iflash.readSector(off, mem);
                } else {
                    void* ptr = reBlock128(disks + dsk - 1, blk + i, out);
                    if (out)
                        memcpy(ptr, mem, 128);
                    else
                        memcpy(mem, ptr, 128);
                }
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

void listSdFiles () {
    for (int i = 0; i < fatFs.rmax; ++i) {
        int off = (i*32) % 512;
        if (off == 0)
            sdCard.read512(fatFs.rdir + i/16, fatFs.buf);
        int length = *(int32_t*) (fatFs.buf+off+28);
        if (length >= 0 && '!' < fatFs.buf[off] &&
                fatFs.buf[off] <= '~' && fatFs.buf[off+6] != '~') {
            uint8_t attr = fatFs.buf[off+11];
            printf("   %s\t", attr & 8 ? "vol:" : attr & 16 ? "dir:" : "");
            for (int j = 0; j < 11; ++j) {
                int c = fatFs.buf[off+j];
                if (j == 8)
                    printf(".");
                printf("%c", c);
            }
            printf(" %7d b\n", length);
        }
    }
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);

    initFsmcLcd();
    lcd.init();
    lcd.clear();

    printf("\r\n");

    key0.mode(Pinmode::in_pullup); // inverted logic
    key1.mode(Pinmode::in_pullup); // inverted logic

    rtc.init();

    int nsec = iflash.init();
    printf("iflash %d sectors\n", nsec);

    if (0) { // set up empty directory blocks
        printf("clearing iflash directory");
        for (int i = 0; i < 16; ++i) {
            uint8_t buf [128];
            memset(buf, 0xE5, sizeof buf);
            iflash.writeSector(2*26 + i, buf);
        }
    }

    spi1.init();
    spiWear.init();

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

        disks[0].open("CPMA    CPM"); // B:
        disks[1].open("ZORK1   CPM"); // C:
        disks[2].open("T1      DSK"); // D:
        disks[3].open("T2      DSK"); // E:
        disks[4].open("T3      DSK"); // F:
        disks[5].open("T4      DSK"); // G:
        disks[6].open("T5      DSK"); // H:
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

    if (!key1) { // set up system tracks on disk 1
        printf("[updating system tracks] ");
        for (uint32_t i = 0; i < sizeof rom; i += 128)
            spiWear.write128(i / 128, rom + i);
    }

    if (!key0) { // set up empty directory blocks on disk 1
        printf("[clearing boot directory] ");
        for (int i = 0; i < 16; ++i) {
            uint8_t buf [128];
            memset(buf, 0xE5, sizeof buf);
            spiWear.write128(2*26 + i, buf);
        }
    }

    // wait for both keys to be released
    while (!key0 || !key1)
        wait_ms(100); // debounce

    // emulate a boot loader which loads the first block of A: at 0x0000
    spiWear.read128(0, mapMem(&context, 0));
    // and leave a copy of HEXSAVE.COM in the TPA for saving in CP/M
    memcpy(mapMem(&context, 0x0100), ram, sizeof ram);

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

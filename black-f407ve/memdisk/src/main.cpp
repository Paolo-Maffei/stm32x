// Launch CP/M, with virtual disks on SPI flash and SD card.

#include <jee.h>
#include <jee/spi-flash.h>
#include <jee/spi-sdcard.h>
#include <string.h>
#include "spi-wear.h"
#include "flashwear.h"

UartBufDev< PinA<9>, PinA<10> > console;
PinE<4> key0;
PinE<3> key1;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

// chip select needs a minute slowdown to work at 168 MHz
SpiGpio< PinB<5>, PinB<4>, PinB<3>, SlowPin< PinB<0>, 2 > > spi1;
SpiFlash< decltype(spi1) > spiFlash;
SpiWear< decltype(spiFlash), PinA<6> > spiWear;

SpiGpio< PinD<2>, PinC<8>, PinC<12>, PinC<11> > spi2;
SdCard< decltype(spi2) > sdCard;
FatFS< decltype(sdCard) > fatFs;

// TODO yucky init
typedef FileMap< decltype(fatFs), 257 > DiskMap; // 8M = 256 fat entries x 32K
DiskMap* currDisk;

static void* reBlock128 (DiskMap* dmp =0, int blk =0, bool dirty =false) {
    static int currSect;
    static bool currDirty;
    static uint8_t currBuf [512];

    int sect = blk / 4;  // CP/M blocks are 128b, SD card sectors are 512b
    if (dmp != currDisk || sect != currSect) {
        if (currDirty && !currDisk->ioSect(true, currSect, currBuf))
            printf("WR-ERR: sect %d blk %d\n", sect, blk);
        currDirty = false;
        if (dmp == 0)
            return 0; // it's a flush request, return value will be ignored
        currDisk = dmp;
        currSect = sect;
        if (currDisk != 0 && !currDisk->ioSect(false, currSect, currBuf))
            printf("RD-ERR: sect %d blk %d\n", sect, blk);
    }
    currDirty |= dirty;
    return currBuf + (blk*128) % 512;
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

struct VirtualDisk {
    static constexpr uint32_t BUFLEN = 128;
    static constexpr uint32_t DISK_TINY = 77*26*1;   // 8" SSSD
    static constexpr uint32_t DISK_NORM = 160*8*4;   // 3.5" HD
    static constexpr uint32_t DISK_HUGE = 255*256*1; // 8 MB hd

    VirtualDisk () {}
    virtual ~VirtualDisk () {}

    virtual uint32_t init (char const [11]) =0;
    virtual uint32_t size () =0;
    virtual void read (uint32_t pos, void* buf) =0;
    virtual void write (uint32_t pos, void const* buf) =0;
};

static FlashWear iflash;

class OnChipDisk : public VirtualDisk {
    int offset = -1;

public:
    OnChipDisk () {}
    virtual ~OnChipDisk () {}

    virtual uint32_t init (char const def [11]) {
        if (def[8] >= '1' && def[9] == ' ') {
            static uint32_t totalSectors = 0;
            if (totalSectors == 0)
                totalSectors = iflash.init();

            unsigned num = def[8] - '1';
            offset = num * DISK_TINY;
            if (offset + DISK_TINY <= totalSectors)
                return DISK_TINY;
        }

        offset = -1;
        return 0;
    }

    virtual uint32_t size () {
        return DISK_TINY;
    }

    virtual void read (uint32_t pos, void* buf) {
        if (offset >= 0 && pos < DISK_TINY)
            iflash.readSector(offset + pos, buf);
    }

    virtual void write (uint32_t pos, void const* buf) {
        if (offset >= 0 && pos < DISK_TINY)
            iflash.writeSector(offset + pos, buf);
    }
};

class SpiFlashDisk : public VirtualDisk {
    uint16_t offset, limit = 0;

public:
    SpiFlashDisk () {}
    virtual ~SpiFlashDisk () {}

    virtual uint32_t init (char const def [11]) {
        if (def[8] == '0' && def[9] >= '1' && def[10] == ' ') {
            static uint32_t kbSize = 0;
            if (kbSize == 0) {
                spi1.init();
                spiWear.init();
                kbSize = spiFlash.size();
            }

            unsigned num = def[9] - '1';

            //  2 MB = 2x 252 + 1x 1440 + 0x 8160
            //  4 MB = 2x 252 + 2x 1440 + 0x 8160
            //  8 MB = 2x 252 + 5x 1440 + 0x 8160
            // 16 MB = 2x 252 + 5x 1440 + 1x 8160

            static uint16_t diskSizes [] = {
                DISK_TINY, DISK_TINY,
                DISK_NORM, DISK_NORM, DISK_NORM, DISK_NORM, DISK_NORM,
                DISK_HUGE,
            };

            if (num < sizeof diskSizes / sizeof *diskSizes) {
                offset = 0;
                for (unsigned i = 0; i < num; ++i)
                    offset += diskSizes[i];
                limit = diskSizes[num];
                if (limit <= kbSize * 1024 / BUFLEN)
                    return limit;
            }
        }

        limit = 0;
        return 0;
    }

    virtual uint32_t size () {
        return limit;
    }

    virtual void read (uint32_t pos, void* buf) {
        if (pos < limit)
            spiWear.read128(offset + pos, buf);
    }

    virtual void write (uint32_t pos, void const* buf) {
        if (pos < limit)
            spiWear.write128(offset + pos, buf);
    }
};

class SdCardDisk : public VirtualDisk {
    DiskMap diskMap;
    uint32_t limit = 0;

public:
    SdCardDisk () : diskMap (fatFs) {}
    virtual ~SdCardDisk () {}

    virtual uint32_t init (char const def [11]) {
        limit = diskMap.open(def) / BUFLEN;
        return limit;
    }

    virtual uint32_t size () {
        return limit;
    }

    virtual void read (uint32_t pos, void* buf) {
        if (pos < limit) {
            void* ptr = reBlock128(&diskMap, pos, false);
            memcpy(buf, ptr, BUFLEN);
        }
    }

    virtual void write (uint32_t pos, void const* buf) {
        if (pos < limit) {
            void* ptr = reBlock128(&diskMap, pos, true);
            memcpy(ptr, buf, BUFLEN);
        }
    }
};

constexpr int NUMDRIVES = 8;

struct Drive {
    OnChipDisk onChipDisk;
    SpiFlashDisk spiFlashDisk;
    SdCardDisk sdCardDisk;
    VirtualDisk* current = 0;

    Drive () {}

    uint32_t assign (char const name [11]) {
        uint32_t size = onChipDisk.init(name);
        if (size > 0)
            current = &onChipDisk;
        else {
            size = spiFlashDisk.init(name);
            if (size > 0)
                current = &spiFlashDisk;
            else {
                size = sdCardDisk.init(name);
                if (size > 0)
                    current = &sdCardDisk;
                else
                    current = 0;
            }
        }
        printf("current %08x size %d\n", current, size);
        return size;
    }

    int size () {
        return current != 0 ? current->size() : -1;
    }

    void read (uint32_t pos, void* buf) {
        if (current != 0)
            current->read(pos, buf);
    }

    void write (uint32_t pos, void const* buf) {
        if (current != 0)
            current->write(pos, buf);
    }
};

Drive drives [NUMDRIVES];

void diskCopy (int from, int to, int kb) {
    Drive* df = drives + from;
    Drive* dt = drives + to;

    uint32_t start = ticks;

    uint8_t buf [8][128];
    for (int i = 0; i < kb; ++i) {
        for (int j = 0; j < 8; ++j)
            df->read(8*i + j, buf[j]);
        for (int j = 0; j < 8; ++j)
            df->write(8*i + j, buf[j]);
    }
    reBlock128(); // flush

    uint32_t ms = ticks - start;
    printf("%5d KB: (%d -> %d) %4d ms = %4d KB/sec\n",
            kb, from, to, ms, (1000*kb)/ms);
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    printf("%08x\n", drives);

    //uint32_t t0 = ticks;
    //int nsec = iflash.init();
    //printf("iflash %d ms, %d sectors\n", ticks - t0, nsec);

    //spi1.init();
    //spiWear.init();

    spi2.init();
    if (sdCard.init()) {
        //printf("[sd card: sdhc %d]\n", sdCard.sdhc);

        uint32_t t1 = ticks;
        fatFs.init();
        printf("init %d ms\n", ticks - t1);
#if 0
        printf("base %d spc %d rdir %d rmax %d data %d clim %d\n",
                                    fatFs.base, fatFs.spc, fatFs.rdir,
                                    fatFs.rmax, fatFs.data, fatFs.clim);
#endif
        uint32_t t2 = ticks;
        listSdFiles();
        printf("list %d ms\n", ticks - t2);

        uint32_t t3 = ticks;
        drives[0].assign("        01 "); // A:
        drives[1].assign("CPMA    CPM"); // B:
        drives[2].assign("        1  "); // C:
        drives[3].assign("T1      DSK"); // D:
        drives[4].assign("T2      DSK"); // E:
        drives[5].assign("T3      DSK"); // F:
        drives[6].assign("T4      DSK"); // G:
        drives[7].assign("T5      DSK"); // H:
        printf("open %d ms\n", ticks - t3);

        diskCopy(6, 7, 1);
        diskCopy(6, 7, 10);
        diskCopy(6, 7, 100);
    }

    PinB<1> backlight;
    backlight.mode(Pinmode::out);

    while (1) {}
}

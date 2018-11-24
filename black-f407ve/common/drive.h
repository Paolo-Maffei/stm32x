// Universal disk drive interface to internal flash, spi flash, and sd card.

// chip select needs a minute slowdown to work at 168 MHz
SpiGpio< PinB<5>, PinB<4>, PinB<3>, SlowPin< PinB<0>, 2 > > spi1;
SpiFlash< decltype(spi1) > spiFlash;
SpiWear< decltype(spiFlash), PinA<6> > spiWear;

SpiGpio< PinD<2>, PinC<8>, PinC<12>, PinC<11> > spi2;
SdCard< decltype(spi2) > sdCard;
FatFS< decltype(sdCard) > fatFs;

// 8M = 256 fat entries x 32K
typedef FileMap< decltype(fatFs), 257 > DiskMap;
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

struct VirtualDisk {
    static constexpr uint32_t BUFLEN = 128;
    static constexpr uint32_t DISK_TINY = 77*26*1;   // 8" SSSD
    static constexpr uint32_t DISK_NORM = 160*8*4;   // 3.5" HD
    static constexpr uint32_t DISK_HUGE = 256*256*1; // 8 MB hd

    uint32_t size = 0;

    VirtualDisk () {}
    virtual ~VirtualDisk () {}

    virtual bool init (char const [11]) =0;
    virtual void read (uint32_t pos, void* buf) =0;
    virtual void write (uint32_t pos, void const* buf) =0;

    virtual uint32_t skew (uint32_t /*trk*/, uint32_t sec) { return sec; }

    uint32_t lba (uint32_t trk, uint32_t sec) {
        uint32_t spt = size == DISK_TINY ? 26 :
                       size == DISK_NORM ? 72 : 256;
        return trk * spt + skew(trk, sec);
    }
};

static FlashWear iflash;

class OnChipDisk : public VirtualDisk {
    int offset = -1;

public:
    OnChipDisk () {}
    virtual ~OnChipDisk () {}

    virtual bool init (char const def [11]) {
        if (def[8] >= '1' && def[9] == ' ') {
            static uint32_t totalSectors = 0;
            if (totalSectors == 0)
                totalSectors = iflash.init();

            unsigned num = def[8] - '1';
            offset = num * DISK_TINY;
            size = DISK_TINY;
            if (offset + size <= totalSectors)
                return true;
        }

        size = 0;
        return false;
    }

    virtual void read (uint32_t pos, void* buf) {
        if (pos < DISK_TINY)
            iflash.readSector(offset + pos, buf);
    }

    virtual void write (uint32_t pos, void const* buf) {
        if (pos < DISK_TINY)
            iflash.writeSector(offset + pos, buf);
    }
};

class SpiFlashDisk : public VirtualDisk {
    uint16_t offset;

public:
    SpiFlashDisk () {}
    virtual ~SpiFlashDisk () {}

    virtual bool init (char const def [11]) {
        if (def[8] == '1' && def[9] >= '1' && def[10] == ' ') {
            static uint32_t kbSize = 0;
            if (kbSize == 0) {
                spi1.init();
                spiWear.init();
                kbSize = spiFlash.size();
            }

            unsigned num = def[9] - '1';

            //  2 MB = 2x 250.25 + 1x 1440 + 0x 8192
            //  4 MB = 2x 250.25 + 2x 1440 + 0x 8192
            //  8 MB = 2x 250.25 + 5x 1440 + 0x 8192
            // 16 MB = 2x 250.25 + 5x 1440 + 1x 8192

            static uint32_t diskSizes [] = {
                DISK_TINY, DISK_TINY,
                DISK_NORM, DISK_NORM, DISK_NORM, DISK_NORM, DISK_NORM,
                DISK_HUGE,
            };

            if (num < sizeof diskSizes / sizeof *diskSizes) {
                offset = 0;
                for (unsigned i = 0; i < num; ++i)
                    offset += diskSizes[i];
                size = diskSizes[num];
                if (offset + size <= kbSize * 1024 / BUFLEN)
                    return true;
            }
        }

        size = 0;
        return false;
    }

    virtual void read (uint32_t pos, void* buf) {
        if (pos < size)
            spiWear.read128(offset + pos, buf);
    }

    virtual void write (uint32_t pos, void const* buf) {
        if (pos < size)
            spiWear.write128(offset + pos, buf);
    }
};

class SdCardDisk : public VirtualDisk {
    DiskMap diskMap;
    uint32_t limit;

public:
    SdCardDisk () : diskMap (fatFs) {}
    virtual ~SdCardDisk () {}

    virtual bool init (char const def [11]) {
        size = def[9] == ' ' ? DISK_TINY :
              def[10] == ' ' ? DISK_NORM : DISK_HUGE;

        limit = diskMap.open(def) / BUFLEN;
        return true;
    }

    virtual void read (uint32_t pos, void* buf) {
        if (pos < limit) {
            void* ptr = reBlock128(&diskMap, pos, false);
            memcpy(buf, ptr, BUFLEN);
        }
    }

    virtual void write (uint32_t pos, void const* buf) {
        if (limit < size)
            return; // disk is read-only if it's not a full-size image
        if (pos < limit) {
            void* ptr = reBlock128(&diskMap, pos, true);
            memcpy(ptr, buf, BUFLEN);
        }
    }

    virtual uint32_t skew (uint32_t trk, uint32_t sec) {
        static const uint8_t skewMap [] = {
            0,6,12,18,24,4,10,16,22,2,8,14,20,1,7,13,19,25,5,11,17,23,3,9,15,21
        };

        if (size == DISK_TINY && trk >= 2)
            sec = skewMap[sec];
        return sec;
    }
};

class Drive {
    OnChipDisk   onChipDisk;
    SpiFlashDisk spiFlashDisk;
    SdCardDisk   sdCardDisk;
    VirtualDisk* current = 0;

public:
    Drive () {}

    int assign (char const name [11]) {
        if (onChipDisk.init(name))
            current = &onChipDisk;
        else if (spiFlashDisk.init(name))
            current = &spiFlashDisk;
        else if (sdCardDisk.init(name))
            current = &sdCardDisk;
        else {
            current = 0;
            return -1;
        }
        //printf("current %08x size %d\n", current, current->size);
        return current->size;
    }

    VirtualDisk* operator-> () { return current; }
};

void listSdFiles () {
    for (int i = 0; i < fatFs.rmax; ++i) {
        int off = (i*32) % 512;
        if (off == 0)
            sdCard.read512(fatFs.rdir + i/16, fatFs.buf);
        int length = *(int32_t*) (fatFs.buf+off+28);
        if (length >= 0 &&
                '!' < fatFs.buf[off] && fatFs.buf[off] < '~' &&
                fatFs.buf[off+5] != '~' && fatFs.buf[off+6] != '~') {
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

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
    static constexpr uint32_t DISK_HUGE = 255*256*1; // 8 MB hd

    uint32_t size = 0;

    VirtualDisk () {}
    virtual ~VirtualDisk () {}

    virtual bool init (char const [11]) =0;
    virtual void read (uint32_t pos, void* buf) =0;
    virtual void write (uint32_t pos, void const* buf) =0;
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
                size = diskSizes[num];
                if (size <= kbSize * 1024 / BUFLEN)
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

public:
    SdCardDisk () : diskMap (fatFs) {}
    virtual ~SdCardDisk () {}

    virtual bool init (char const def [11]) {
        size = diskMap.open(def) / BUFLEN;
        return true;
    }

    virtual void read (uint32_t pos, void* buf) {
        if (pos < size) {
            void* ptr = reBlock128(&diskMap, pos, false);
            memcpy(buf, ptr, BUFLEN);
        }
    }

    virtual void write (uint32_t pos, void const* buf) {
        if (pos < size) {
            void* ptr = reBlock128(&diskMap, pos, true);
            memcpy(ptr, buf, BUFLEN);
        }
    }
};

struct Drive {
    OnChipDisk   onChipDisk;
    SpiFlashDisk spiFlashDisk;
    SdCardDisk   sdCardDisk;
    VirtualDisk* current = 0;

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

        printf("current %08x size %d\n", current, current->size);
        return current->size;
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

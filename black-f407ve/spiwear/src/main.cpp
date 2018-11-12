// Testing a wear-leveled wrapper around the SPI-flash driver.

#include <jee.h>
#include <jee/spi-flash.h>
#include <string.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinA<6> led;

// chip select needs a minute slowdown to work at 168 MHz
SpiGpio< PinB<5>, PinB<4>, PinB<3>, SlowPin< PinB<0>, 2 > > spi;
SpiFlash< decltype(spi) > spif;

template< typename SPIF >
class SpiWear {
    constexpr static bool DEBUG = false;
    constexpr static uint32_t groupSize   = 256*1024;
    constexpr static uint32_t pageSize    = 4*1024;
    constexpr static uint32_t blockSize   = 128;
    constexpr static uint32_t groupBlocks = groupSize / blockSize;
    constexpr static uint32_t pageBlocks  = pageSize / blockSize;

    int mapBase = -1;
    uint16_t map [blockSize/2];  // only [1..pageSize] entries actually used
    uint8_t flushBuf [pageSize];

    void loadMap (int blk) {
        // the map is on the first block of the last page of each group
        int base = (blk / groupBlocks + 1) * groupBlocks - pageBlocks;
        if (mapBase != base) {
            mapBase = base;
            readUnmapped(mapBase, map);
        }
    }

    int findFreeSlot () {
        for (uint32_t i = 1; i < pageBlocks; ++i)
            if (map[i] == 0xFFFF)
                return i;  // return first one found
        return 0;          // ... or zero if none
    }

    // find last mapped entry, or use the unmapped one if not found
    int remap (int blk) {
        int actual = blk;
        for (uint32_t i = 1; i < pageBlocks; ++i)
            if (map[i] == blk)
                actual = mapBase + i;
        if (DEBUG && actual != blk) printf("remap %d -> %d\n", blk, actual);
        return actual;
    }

    void writeNewSlot (int n, int blk, const void* buf) {
        if (DEBUG) printf("writeNewSlot n %d blk %d\n", n, blk);
        map[n] = blk;
        // first write the two changed bytes in the map
        SPIF::write(blockSize * mapBase + 2 * n, map + n, 2);
        // then write the block in the freshly allocated slot
        writeUnmapped(mapBase + n, buf);
    }

    void flushMapEntries () {
        int groupBase = (mapBase / groupBlocks) * groupBlocks;
        if (DEBUG) printf("flushMapEntries %d..%d\n", groupBase, mapBase);

        if (DEBUG) {
            printf("map:");
            for (uint32_t i = 0; i < pageBlocks; ++i)
                printf(" %d", map[i]);
            printf("\n");
        }

        // go through all the pages and rewrite them if they have map entries
        for (int g = groupBase; g < mapBase; g += pageBlocks) {
            for (uint32_t slot = 1; slot < pageBlocks; ++slot)
                if (g <= map[slot] && map[slot] < g + pageBlocks) {
                    if (DEBUG) printf("flushing %d..%d\n", g, g+pageBlocks-1);
                    // there is at least one remapped block we need to flush
                    // so first, read all the blocks, with remapping
                    for (uint32_t i = 0; i < pageBlocks; ++i)
                        read128(g + i, flushBuf + blockSize * i);
                    // ... and then, write out all the blocks, unmapped
                    // XXX power loss after this point can lead to data loss
                    // the reason is that the first write will do a page erase
                    for (uint32_t i = 0; i < pageBlocks; ++i)
                        writeUnmapped(g + i, flushBuf + blockSize * i);
                    // XXX end of critical area, power loss is no longer risky
                    break;
                }
        }

        if (DEBUG) printf("flush done, clear map %d\n", mapBase);
        memset(map, 0xFF, sizeof map);
        SPIF::erase(mapBase>>1);
    }

    void readUnmapped (int blk, void* buf) {
        SPIF::read(blockSize * blk, buf, blockSize);
    }

    void writeUnmapped (int blk, const void* buf) {
        if (blk % (pageSize/blockSize) == 0)
            SPIF::erase(blk>>1);
        SPIF::write(blockSize * blk, buf, blockSize);
    }

public:
    void init () {
        led.mode(Pinmode::out);
        led = 1; // off, inverted logic
        SPIF::init();
    }

    void read128 (int blk, void* buf) {
        loadMap(blk);
        readUnmapped(remap(blk), buf);
    }

    void write128 (int blk, const void* buf) {
        led = 0; // on, inverted logic
        loadMap(blk);
        int slot = findFreeSlot();
        if (slot == 0) {
            flushMapEntries();
            slot = 1; // the map is now free again
        }
        writeNewSlot(slot, blk, buf);
        led = 1; // off, inverted logic
    }
};

SpiWear< decltype(spif) > spiw;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    printf("\n-------------------------------------------------------------\n");

    spi.init();
    spiw.init();
    //spif.wipe();

    printf("spif %x, %d kB\n", spif.devId(), spif.size());

    static uint8_t buf [128];
    constexpr int offset = 2048;

    // only run a few times, don't wear out flash while testing
    for (int n = 0; n < 35; ++n) {
        spiw.read128(offset + n % 5, buf);
        int sum = 0;
        for (uint32_t i = 0; i < sizeof buf; ++i)
            sum += buf[i];

        ++buf[0];
        spiw.write128(offset + n % 5, buf);

        printf("block %d ticks %d sum %d buf %02x%02x...\n",
                n % 5, ticks, sum, buf[0], buf[1]);

        wait_ms(100);
    }

    while (true) {}
}

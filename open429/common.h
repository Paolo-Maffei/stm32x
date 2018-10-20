// common code, shared between all projects

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

void enableClkAt160MHz () {
    constexpr uint32_t rcc   = Periph::rcc;

    MMIO32(Periph::flash + 0x00) = 0x705; // flash acr, 5 wait states
    MMIO32(rcc + 0x00) = (1<<16); // HSEON
    while ((MMIO32(rcc + 0x00) & (1<<17)) == 0) ; // wait for HSERDY
    MMIO32(rcc + 0x08) = 1; // switch to HSE
    MMIO32(rcc + 0x04) = (7<<24) | (1<<22) | (0<<16) | (160<<6) | (4<<0);
    MMIO32(rcc + 0x00) |= (1<<24); // PLLON
    while ((MMIO32(rcc + 0x00) & (1<<25)) == 0) ; // wait for PLLRDY
    MMIO32(rcc + 0x08) = (4<<13) | (5<<10) | (2<<0);
}

void enableClkAt180MHz () {
    constexpr uint32_t rcc   = Periph::rcc;

    MMIO32(Periph::flash + 0x00) = 0x705; // flash acr, 5 wait states
    MMIO32(rcc + 0x00) = (1<<16); // HSEON
    while ((MMIO32(rcc + 0x00) & (1<<17)) == 0) ; // wait for HSERDY
    MMIO32(rcc + 0x08) = 1; // switch to HSE
    MMIO32(rcc + 0x04) = (7<<24) | (1<<22) | (0<<16) | (180<<6) | (4<<0);
    MMIO32(rcc + 0x00) |= (1<<24); // PLLON
    while ((MMIO32(rcc + 0x00) & (1<<25)) == 0) ; // wait for PLLRDY
    MMIO32(rcc + 0x08) = (4<<13) | (5<<10) | (2<<0);
}

constexpr uint32_t sdram = 0xD0000000;

void initFmcSdram () {
    // FMC I/O pin mode: 5432109876543210
    Port<'B'>::modeMap(0b0000000001100000, Pinmode::alt_out, 12);
    Port<'C'>::modeMap(0b0000000000000001, Pinmode::alt_out, 12);
    Port<'D'>::modeMap(0b1100011100000011, Pinmode::alt_out, 12);
    Port<'E'>::modeMap(0b1111111110000011, Pinmode::alt_out, 12);
    Port<'F'>::modeMap(0b1111100000111111, Pinmode::alt_out, 12);
    Port<'G'>::modeMap(0b1000000100110011, Pinmode::alt_out, 12);

    MMIO32(Periph::rcc + 0x38) = (1<<0);  // enable FSMC

    // SDRAM timing
    constexpr uint32_t fmc = 0xA0000000;
    MMIO32(fmc + 0x140) =
    MMIO32(fmc + 0x144) = (1<<13) | (0<<12) | (2<<10) | (0<<9) | (3<<7) |
                            (1<<6) | (1<<4) | (1<<2) | (0<<0);
    MMIO32(fmc + 0x148) =
    MMIO32(fmc + 0x14C) = (1<<24) | (1<<20) | (1<<16) | (6<<12) |
                            (3<<8) | (6<<4) | (1<<0);
    // SDRAM commands
    while (MMIO32(fmc + 0x158) & (1<<5)) {}
    MMIO32(fmc + 0x150) = (0<<9) | (0<<5) | (1<<3) | (1<<0);
    wait_ms(10);
    while (MMIO32(fmc + 0x158) & (1<<5)) {}
    MMIO32(fmc + 0x150) = (0<<9) | (0<<5) | (1<<3) | (2<<0);
    while (MMIO32(fmc + 0x158) & (1<<5)) {}
    MMIO32(fmc + 0x150) = (0<<9) | (3<<5) | (1<<3) | (3<<0);
    while (MMIO32(fmc + 0x158) & (1<<5)) {}
    MMIO32(fmc + 0x150) = (0<<9) | (3<<5) | (1<<3) | (3<<0);
    while (MMIO32(fmc + 0x158) & (1<<5)) {}
    MMIO32(fmc + 0x150) = (0x0231<<9) | (0<<5) | (1<<3) | (4<<0);
    while (MMIO32(fmc + 0x158) & (1<<5)) {}
    MMIO32(fmc + 0x154) = (1386<<1);
}

template< int N >
struct CanDev {
    constexpr static uint32_t base = N == 0 ? 0x40006400 : 0x40006800;

    constexpr static uint32_t mcr  = base + 0x000;
    constexpr static uint32_t msr  = base + 0x004;
    constexpr static uint32_t tsr  = base + 0x008;
    constexpr static uint32_t rfr  = base + 0x00C;
    constexpr static uint32_t btr  = base + 0x01C;
    constexpr static uint32_t tir  = base + 0x180;
    constexpr static uint32_t tdtr = base + 0x184;
    constexpr static uint32_t tdlr = base + 0x188;
    constexpr static uint32_t tdhr = base + 0x18C;
    constexpr static uint32_t rir  = base + 0x1B0;
    constexpr static uint32_t rdtr = base + 0x1B4;
    constexpr static uint32_t rdlr = base + 0x1B8;
    constexpr static uint32_t rdhr = base + 0x1BC;
    constexpr static uint32_t fmr  = base + 0x200;
    constexpr static uint32_t fsr  = base + 0x20C;
    constexpr static uint32_t far  = base + 0x21C;
    constexpr static uint32_t fr1  = base + 0x240;
    constexpr static uint32_t fr2  = base + 0x244;

    static void init () {
        if (N == 0) {
            // alt mode CAN1:    5432109876543210
            Port<'B'>::modeMap(0b0000001100000000, Pinmode::alt_out, 9);
            MMIO32(Periph::rcc + 0x40) |= (1<<25);  // enable CAN1
        } else {
            // alt mode CAN2:    5432109876543210
            Port<'B'>::modeMap(0b0000000001100000, Pinmode::alt_out, 9);
            MMIO32(Periph::rcc + 0x40) |= (1<<26);  // enable CAN2
        }

        MMIO32(mcr) &= ~(1<<1); // exit sleep
        MMIO32(mcr) |= (1<<0); // init req
        while ((MMIO32(msr) & (1<<0)) == 0) {}
        MMIO32(btr) = (7<<20) | (5<<16) | (2<<0); // 1 MBps
        MMIO32(mcr) &= ~(1<<0); // init leave
        while (MMIO32(msr) & (1<<0)) {}
    }

    static void filterInit (int num, int id =0, int mask =0) {
        //MMIO32(fmr) |= (1<<0); // FINIT
        //MMIO32(far) &= ~(1<<num); // ~FACT
        MMIO32(fsr) |= (1<<num); // FSC 32b
        MMIO32(fr1 + 8 * num) = id;
        MMIO32(fr2 + 8 * num) = mask;
        MMIO32(far) |= (1<<num); // FACT
        MMIO32(fmr) &= ~(1<<0); // ~FINIT
    }

    static void transmit (int id, const void* ptr, int len) {
        if (MMIO32(tsr) & (1<<26)) { // TME0
            MMIO32(tir) = (id<<21);
            MMIO32(tdtr) = (len<<0);
            // this assumes that misaligned word access works
            MMIO32(tdlr) = ((const uint32_t*) ptr)[0];
            MMIO32(tdhr) = ((const uint32_t*) ptr)[1];

            MMIO32(tir) |= (1<<0); // TXRQ
        }
    }

    static int receive (int* id, void* ptr) {
        int len = -1;
        if (MMIO32(rfr) & (3<<0)) { // FMP
            *id = MMIO32(rir) >> 21;
            len = MMIO32(rdtr) & 0x0F;
            ((uint32_t*) ptr)[0] = MMIO32(rdlr);
            ((uint32_t*) ptr)[1] = MMIO32(rdhr);
            MMIO32(rfr) |= (1<<5); // RFOM
        }
        return len;
    }
};

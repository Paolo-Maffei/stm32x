// common code, shared between all projects

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

void enableClkAt160MHz () {
    MMIO32(Periph::flash + 0x00) = 0x705; // flash acr, 5 wait states
    MMIO32(Periph::rcc + 0x00) = (1<<16); // HSEON
    while ((MMIO32(Periph::rcc + 0x00) & (1<<17)) == 0) ; // wait for HSERDY
    MMIO32(Periph::rcc + 0x08) = (4<<13) | (5<<10) | (1<<0); // prescaler w/ HSE
    MMIO32(Periph::rcc + 0x04) = (7<<24) | (1<<22) | (0<<16) | (160<<6) | (4<<0);
    MMIO32(Periph::rcc + 0x00) |= (1<<24); // PLLON
    while ((MMIO32(Periph::rcc + 0x00) & (1<<25)) == 0) ; // wait for PLLRDY
    MMIO32(Periph::rcc + 0x08) = (4<<13) | (5<<10) | (2<<0);
}

void enableClkAt180MHz () {
    MMIO32(Periph::flash + 0x00) = 0x705; // flash acr, 5 wait states
    MMIO32(Periph::rcc + 0x00) = (1<<16); // HSEON
    while ((MMIO32(Periph::rcc + 0x00) & (1<<17)) == 0) ; // wait for HSERDY
    MMIO32(Periph::rcc + 0x08) = (4<<13) | (5<<10) | (1<<0); // prescaler w/ HSE
    MMIO32(Periph::rcc + 0x04) = (7<<24) | (1<<22) | (0<<16) | (180<<6) | (4<<0);
    MMIO32(Periph::rcc + 0x00) |= (1<<24); // PLLON
    while ((MMIO32(Periph::rcc + 0x00) & (1<<25)) == 0) ; // wait for PLLRDY
    MMIO32(Periph::rcc + 0x08) = (4<<13) | (5<<10) | (2<<0);
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

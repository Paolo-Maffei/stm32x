// common code, shared between all projects

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

static void enableClkAt160MHz () {
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

static void enableClkAt180MHz () {
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

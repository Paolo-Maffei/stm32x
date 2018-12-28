#include <jee.h>

// sine table, with 2500 entries and a swing of +/- 1920 around 2048
// when sampled at 125 kHz, this will produce a pure 50 Hz sine wave
const uint16_t sine [] = {
#include "sine.h"
};

struct DAC {
    constexpr static uint32_t base    = 0x40007400;
    constexpr static uint32_t cr      = base + 0x00;
    constexpr static uint32_t dhr12r1 = base + 0x08;

    static void init () {
        PinA<4>::mode(Pinmode::in_analog);
        MMIO32(Periph::rcc+0x38) |= (1<<29);  // enable DAC
        MMIO32(cr) = (1<<0);  // EN1
    }

    static void write (uint32_t val) {
        MMIO32(dhr12r1) = val;
    }
};

DAC dac;

int main() {
    enableClkAt32mhz();
    dac.init();

    uint32_t i = 0;
    while (true)
        dac.write(sine[i++ % 2500]);
}

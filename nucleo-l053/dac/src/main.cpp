#include <jee.h>

// sine table: has 2500 entries with a swing of +/- 1920 around 2048
// when sampled at 125 kHz, this will produce a pure 50 Hz sine wave
const uint16_t sine [] = {
#include "sine.h"
};

DAC dac;

int main() {
    enableClkAt32mhz();

    PinA<4>::mode(Pinmode::in_analog);
    dac.init();

    dac.dmaWave(sine, 2500, 256); // 32 MHz / 256 = 125 kHz

    while (true) {}
}

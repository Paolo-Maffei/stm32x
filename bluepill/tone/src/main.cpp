// Adapted from embello/explore/1446-lpc810/sine/ - 2018-10-29
//
// Generate a 50 Hz sine wave on pin PB12 of the STM32F103.
// See http://jeelabs.org/2014/11/19/getting-started-episode-3/
//
// Needs a 1 KOhm + 1 uF RC filter to convert PWM to an analog value.
//
// The 1-bit sigma-delta DAC synthesis was adapted from code by Jan Ostman,
// see http://www.hackster.io/janost/micro-virtual-analog-synthesizer

#include <jee.h>

PinB<12> speaker;

// Generated by gensine.go: [0,90) degrees, 256 steps, 16-bit
static uint16_t sineTable [256] = {
    0,201,402,603,804,1005,1206,1407,1608,1809,2009,2210,2411,2611,2811,3012,
    3212,3412,3612,3812,4011,4211,4410,4609,4808,5007,5205,5404,5602,5800,5998,
    6195,6393,6590,6787,6983,7180,7376,7571,7767,7962,8157,8351,8546,8740,8933,
    9127,9319,9512,9704,9896,10088,10279,10469,10660,10850,11039,11228,11417,
    11605,11793,11980,12167,12354,12540,12725,12910,13095,13279,13463,13646,
    13828,14010,14192,14373,14553,14733,14912,15091,15269,15447,15624,15800,
    15976,16151,16326,16500,16673,16846,17018,17190,17361,17531,17700,17869,
    18037,18205,18372,18538,18703,18868,19032,19195,19358,19520,19681,19841,
    20001,20160,20318,20475,20632,20788,20943,21097,21251,21403,21555,21706,
    21856,22006,22154,22302,22449,22595,22740,22884,23028,23170,23312,23453,
    23593,23732,23870,24008,24144,24279,24414,24548,24680,24812,24943,25073,
    25202,25330,25457,25583,25708,25833,25956,26078,26199,26320,26439,26557,
    26674,26791,26906,27020,27133,27246,27357,27467,27576,27684,27791,27897,
    28002,28106,28209,28311,28411,28511,28610,28707,28803,28899,28993,29086,
    29178,29269,29359,29448,29535,29622,29707,29792,29875,29957,30038,30118,
    30196,30274,30350,30425,30499,30572,30644,30715,30784,30853,30920,30986,
    31050,31114,31177,31238,31298,31357,31415,31471,31527,31581,31634,31686,
    31737,31786,31834,31881,31927,31972,32015,32058,32099,32138,32177,32214,
    32251,32286,32319,32352,32383,32413,32442,32470,32496,32522,32546,32568,
    32590,32610,32629,32647,32664,32679,32693,32706,32718,32729,32738,32746,
    32753,32758,32762,32766,32767,
};

uint32_t phase;  // signal phase: bits 16..23 are step, bits 24..25 are quadrant
uint32_t err;    // accumulator for 1-bit DAC error

constexpr uint32_t noteStep = 69433; // noteStep>>16 is approx 12-th root of 2

uint32_t pitch = 1<<16;

int main() {
    speaker.mode(Pinmode::out);

    enableClkAt72MHz();
    enableSysTick(72*2);

    VTableRam().systick = []() {
        if (++ticks > 500000) {
            ticks = 0;
            pitch = pitch * noteStep >> 16;
        }
        phase += pitch;  //. fractional arithmetic, u16.16

        // about 20 us have passed, time to generate the next step
        uint8_t step = phase >> 14;
        // inverted offset in 2nd and 4th quadrant
        if (phase & (1<<22))
            step = ~step; // 0..255 -> 255..0
        // look up the sine value, table only has data for first quadrant
        int ampl = sineTable[step];
        // negative amplitude in 3rd and 4th quadrant
        if (phase & (1<<23))
            ampl = - ampl;
        // calculate the error, this evens out over time
        err = (uint16_t) err - (1<<15) - ampl;
        // set pin 3 if dac > err, else clear pin 3
        speaker = err >> 16;
    };
    
    while (1)
        __asm("wfi");
}

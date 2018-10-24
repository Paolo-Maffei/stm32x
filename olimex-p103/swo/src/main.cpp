// SWO setup via Black Magic Probe:
//
// $ ./bmp_traceswo
// 
// $ arm-none-eabi-gdb
// [...]
// (gdb) tar ext /dev/cu.usbmodemDDE7C6C3
// Remote debugging using /dev/cu.usbmodemDDE7C6C3
// (gdb) mon traceswo
// DDE7C6C3:05:85

#include <jee.h>

#define SWO_FREQ  115200
#define HCLK_FREQ 72000000

#define DBGMCU_CR

#define ITM_PORT0_U8 MMIO8(0xE0000000)
#define ITM_TER      MMIO32(0xE0000E00)
#define ITM_TCR      MMIO32(0xE0000E80)
#define ITM_LAR      MMIO32(0xE0000FB0)

void swoPutc (int c) {
    if ((ITM_TCR & 1) == 0 || (ITM_TER & 1) == 0)
        return;
    while ((ITM_PORT0_U8 & 1) == 0) {}
    ITM_PORT0_U8 = c;
}

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(swoPutc, fmt, ap); va_end(ap);
    return 0;
}

PinC<12> led;

void swoInit () {
    MMIO32(0xE0042004) = (1<<5) | (1<<2) | (1<<1) | (1<<0); // DBGMCU_CR
    MMIO32(0xE000EDFC) |= (1<<24) | (1<<0); // DEMCR: TRCENA & VC_CORERESET

    MMIO32(0xE0040004) = 1; /* port size = 1 bit */
    MMIO32(0xE00400F0) = 1; /* trace port protocol = Manchester */
    MMIO32(0xE0040010) = (HCLK_FREQ / SWO_FREQ) - 1;
    MMIO32(0xE0040304) = 0x100; /* turn off formatter (0x02 bit) */

    ITM_LAR = 0xC5ACCE55; // Unlock Lock Access Registers
    ITM_TCR |= 1<<3; //ITM_TCR_DWTENA_Msk;
    ITM_TCR |= 1<<2; //ITM_TCR_SYNCENA_Msk;
    ITM_TER |= 0x01; // Port 0
    ITM_TCR |= 1<<0; //ITM_TCR_ITMENA_Msk;
}

int main() {
    fullSpeedClock();
    led.mode(Pinmode::out);

    swoInit();

    while (1) {
        printf("%d\n", ticks);
        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(400);
    }
}

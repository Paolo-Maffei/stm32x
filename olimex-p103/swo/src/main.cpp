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

void SWO_PrintChar  (char c);

#define ITM_STIM_U32 (*(volatile unsigned int*)0xE0000000)
#define ITM_STIM_U8  (*(volatile         char*)0xE0000000)
#define ITM_ENA      (*(volatile unsigned int*)0xE0000E00)
#define ITM_TCR      (*(volatile unsigned int*)0xE0000E80)

#define DBGMCU_CR    (*(volatile unsigned int*)0xE0042004)

#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions */
#define     __IO    volatile             /*!< Defines 'read / write' permissions */

/* following defines should be used for structure members */
#define     __IM     volatile const      /*! Defines 'read only' structure member permissions */
#define     __OM     volatile            /*! Defines 'write only' structure member permissions */
#define     __IOM    volatile            /*! Defines 'read / write' structure member permissions */

typedef struct
{
  __OM  union
  {
    __OM  uint8_t    u8;                 /*!< Offset: 0x000 ( /W)  ITM Stimulus Port 8-bit */
    __OM  uint16_t   u16;                /*!< Offset: 0x000 ( /W)  ITM Stimulus Port 16-bit */
    __OM  uint32_t   u32;                /*!< Offset: 0x000 ( /W)  ITM Stimulus Port 32-bit */
  }  PORT [32U];                         /*!< Offset: 0x000 ( /W)  ITM Stimulus Port Registers */
        uint32_t RESERVED0[864U];
  __IOM uint32_t TER;                    /*!< Offset: 0xE00 (R/W)  ITM Trace Enable Register */
        uint32_t RESERVED1[15U];
  __IOM uint32_t TPR;                    /*!< Offset: 0xE40 (R/W)  ITM Trace Privilege Register */
        uint32_t RESERVED2[15U];
  __IOM uint32_t TCR;                    /*!< Offset: 0xE80 (R/W)  ITM Trace Control Register */
        uint32_t RESERVED3[29U];
  __OM  uint32_t IWR;                    /*!< Offset: 0xEF8 ( /W)  ITM Integration Write Register */
  __IM  uint32_t IRR;                    /*!< Offset: 0xEFC (R/ )  ITM Integration Read Register */
  __IOM uint32_t IMCR;                   /*!< Offset: 0xF00 (R/W)  ITM Integration Mode Control Register */
        uint32_t RESERVED4[43U];
  __OM  uint32_t LAR;                    /*!< Offset: 0xFB0 ( /W)  ITM Lock Access Register */
  __IM  uint32_t LSR;                    /*!< Offset: 0xFB4 (R/ )  ITM Lock Status Register */
        uint32_t RESERVED5[6U];
  __IM  uint32_t PID4;                   /*!< Offset: 0xFD0 (R/ )  ITM Peripheral Identification Register #4 */
  __IM  uint32_t PID5;                   /*!< Offset: 0xFD4 (R/ )  ITM Peripheral Identification Register #5 */
  __IM  uint32_t PID6;                   /*!< Offset: 0xFD8 (R/ )  ITM Peripheral Identification Register #6 */
  __IM  uint32_t PID7;                   /*!< Offset: 0xFDC (R/ )  ITM Peripheral Identification Register #7 */
  __IM  uint32_t PID0;                   /*!< Offset: 0xFE0 (R/ )  ITM Peripheral Identification Register #0 */
  __IM  uint32_t PID1;                   /*!< Offset: 0xFE4 (R/ )  ITM Peripheral Identification Register #1 */
  __IM  uint32_t PID2;                   /*!< Offset: 0xFE8 (R/ )  ITM Peripheral Identification Register #2 */
  __IM  uint32_t PID3;                   /*!< Offset: 0xFEC (R/ )  ITM Peripheral Identification Register #3 */
  __IM  uint32_t CID0;                   /*!< Offset: 0xFF0 (R/ )  ITM Component  Identification Register #0 */
  __IM  uint32_t CID1;                   /*!< Offset: 0xFF4 (R/ )  ITM Component  Identification Register #1 */
  __IM  uint32_t CID2;                   /*!< Offset: 0xFF8 (R/ )  ITM Component  Identification Register #2 */
  __IM  uint32_t CID3;                   /*!< Offset: 0xFFC (R/ )  ITM Component  Identification Register #3 */
} ITM_Type;

typedef struct
{
  __IOM uint32_t DHCSR;                  /*!< Offset: 0x000 (R/W)  Debug Halting Control and Status Register */
  __OM  uint32_t DCRSR;                  /*!< Offset: 0x004 ( /W)  Debug Core Register Selector Register */
  __IOM uint32_t DCRDR;                  /*!< Offset: 0x008 (R/W)  Debug Core Register Data Register */
  __IOM uint32_t DEMCR;                  /*!< Offset: 0x00C (R/W)  Debug Exception and Monitor Control Register */
} CoreDebug_Type;

#define ITM_BASE            (0xE0000000UL)                            /*!< ITM Base Address */
#define CoreDebug_BASE      (0xE000EDF0UL)                            /*!< Core Debug Base Address */

#define ITM                 ((ITM_Type       *)     ITM_BASE      )   /*!< ITM configuration struct */
#define CoreDebug           ((CoreDebug_Type *)     CoreDebug_BASE)   /*!< Core Debug configuration struct */

void swoPutc (int c) {
	if ((ITM_TCR & 1) == 0 || (ITM_ENA & 1) == 0)
		return;
	while ((ITM_STIM_U8 & 1) == 0) {}
	ITM_STIM_U8 = c;
}

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(swoPutc, fmt, ap); va_end(ap);
	return 0;
}

PinC<12> led;

#define SWO_FREQ 115200
#define HCLK_FREQ 72000000

#define TPIU_CURRENT_PORT_SIZE *((volatile unsigned *)(0xE0040004))
#define TPIU_ASYNC_CLOCK_PRESCALER *((volatile unsigned *)(0xE0040010))
#define TPIU_SELECTED_PIN_PROTOCOL *((volatile unsigned *)(0xE00400F0))
#define TPIU_FORMATTER_AND_FLUSH_CONTROL *((volatile unsigned *)(0xE0040304))

void swoInit2 () {
	DBGMCU_CR = 0x00000027;
	CoreDebug->DEMCR |= 1<<24; //CoreDebug_DEMCR_TRCENA_Msk;
	CoreDebug->DEMCR |= 1<<0; // CoreDebug_DEMCR_VC_CORERESET_Msk;

	TPIU_CURRENT_PORT_SIZE = 1; /* port size = 1 bit */
	TPIU_SELECTED_PIN_PROTOCOL = 1; /* trace port protocol = Manchester */
	TPIU_ASYNC_CLOCK_PRESCALER = (HCLK_FREQ / SWO_FREQ) - 1;
	TPIU_FORMATTER_AND_FLUSH_CONTROL = 0x100; /* turn off formatter (0x02 bit) */
	
	ITM->LAR = 0xC5ACCE55; // Unlock Lock Access Registers
	ITM->TCR |= 1<<3; //ITM_TCR_DWTENA_Msk;
	ITM->TCR |= 1<<2; //ITM_TCR_SYNCENA_Msk;
	ITM->TER |= 0x01; // Port 0
	ITM->TCR |= 1<<0; //ITM_TCR_ITMENA_Msk;
}

int main() {
	fullSpeedClock();
	led.mode(Pinmode::out);

	swoInit2();

    while (1) {
        printf("%d\n", ticks);
        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(400);
    }
}

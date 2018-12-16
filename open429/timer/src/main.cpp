#include <jee.h>

UartDev< PinA<9>, PinA<10> > console;
#include <../../common.h>

Timer<2> timer;

int main() {
    console.init();
    const int hz = 180000000;
    enableClkAt180MHz();        // the F429 mac clock rate is 180 MHz
    console.baud(115200, hz/2); // APB2 is /2 to stay within 90 MHz max
    enableSysTick(hz/1000);     // recalibrate SysTick to fire every 1 ms

    // generate a 1 MHz signal with 50% duty cycle on PA0, using TIM2
    PinA<0>::mode(Pinmode::alt_out, 1);
    timer.init(90);
    timer.pwm(45);

    // this generates a 3 MHz signal with 40% duty cycle on PA8
    MMIO32(Periph::rcc+0x08) |= (7<<24); // MCO1 div 5
    PinA<8>::mode(Pinmode::alt_out, 0);

    while (true) {}
}

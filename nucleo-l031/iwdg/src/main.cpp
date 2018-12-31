#include <jee.h>

PinB<3> led;
Iwdg dog (1);

int main() {
    led.mode(Pinmode::out);
    led = 1;
    for (int i = 0; i < 10000; ++i) __asm("");
    led = 0;
    
    while (true) {}
}

extern "C" void SystemInit () {}

#include <jee.h>

UartBufDev< PinA<2>, PinA<3> > console;

int printf (const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

int main () {
    console.init();
    console.baud(115200, fullSpeedClock()/4);

    PinD<12> led;
    led.mode(Pinmode::out);

    auto f = led.toggle;
    auto g = wait_ms;
    int on, off;

    auto toggler = [&](int n) { f(); g(n); };
    auto blinker = [&]() { toggler(on); toggler(off); };
    auto printer = [&]() { printf("%d\n", ticks); };

    auto looper = [&](int n, int a, int b) {
        on = a;
        off = b;
        while (--n >= 0) {
            printer();
            blinker();
        }
    };

    looper(5, 100, 400);
    looper(3, 250, 250);
    looper(10, 100, 100);
}

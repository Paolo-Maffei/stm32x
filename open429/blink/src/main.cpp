#include <mbed.h>

Serial console (PA_9, PA_10, 115200);
DigitalOut led (PG_13);


int main() {
    int i = 0;
    while (1) {
        console.printf("hello %d\n", ++i);
        led = 1;
        wait_ms(100);
        led = 0;
        wait_ms(400);
    }
}

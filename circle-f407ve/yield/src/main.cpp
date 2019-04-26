// Task switching demo based on setjmp & longjmp.

#include <jee.h>
#include <setjmp.h>
#include <string.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinB<9> led;

jmp_buf jbSys, jbOne, jbTwo;
uint32_t stackOne [100], stackTwo [100];

void yield (jmp_buf jbp) {
    if (setjmp(jbp) == 0)
        longjmp(jbSys, 1);
}

void taskOne () {
    printf("task 1 start\n");
    while (true) {
        led = 0;
        wait_ms(100);
        printf("yield 1 call\n");
        yield(jbOne);
        printf("yield 1 return\n");
    }
}

void taskTwo () {
    printf("task 2 start\n");
    while (true) {
        led = 1;
        wait_ms(400);
        printf("yield 2 call\n");
        yield(jbTwo);
        printf("yield 2 return\n");
    }
}

void launch (void (*proc)(), uint32_t* stack, jmp_buf jbp) {
    memset(jbp, 0, sizeof jbp);
    ((uint32_t*) jbp)[8] = (uint32_t) (stack + 100);
    ((uint32_t*) jbp)[9] = (uint32_t) proc;
    longjmp(jbp, 1);
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    led.mode(Pinmode::out);

    if (setjmp(jbSys) == 0)
        launch(taskOne, stackOne, jbOne);
    if (setjmp(jbSys) == 0)
        launch(taskTwo, stackTwo, jbTwo);

    while (true) {
        printf("t = %d\n", ticks);
        if (setjmp(jbSys) == 0)
            longjmp(jbOne, 1);
        if (setjmp(jbSys) == 0)
            longjmp(jbTwo, 1);
    }
}

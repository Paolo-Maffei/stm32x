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

// scheduler

jmp_buf *jbCurr;

void launch (void (*proc)(), uint32_t* stack, jmp_buf& jb) {
    memset(&jb, 0, sizeof jb);
    ((uint32_t*) jb)[8] = (uint32_t) (stack + 100);
    ((uint32_t*) jb)[9] = (uint32_t) proc;
}

void yield (jmp_buf& jb) {
    if (setjmp(*jbCurr) == 0) {
        jbCurr = &jb;
        longjmp(jb, 1);
    }
}

// tasks

jmp_buf jbSys, jbOne, jbTwo;
uint32_t stackOne [100], stackTwo [100];

void taskOne () {
    printf("task 1 start\n");
    while (true) {
        led = 0;
        wait_ms(100);
        printf("yield 1 call\n");
        yield(jbSys);
        printf("yield 1 return\n");
    }
}

void taskTwo () {
    printf("task 2 start\n");
    while (true) {
        led = 1;
        wait_ms(400);
        printf("yield 2 call\n");
        yield(jbSys);
        printf("yield 2 return\n");
    }
}

// go!

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    led.mode(Pinmode::out);

    jbCurr = &jbSys;

    launch(taskOne, stackOne, jbOne);
    launch(taskTwo, stackTwo, jbTwo);

    while (true) {
        printf("t = %d\n", ticks);
        yield(jbOne);
        yield(jbTwo);
    }
}

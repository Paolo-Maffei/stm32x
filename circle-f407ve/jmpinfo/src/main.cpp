// This code helps figure out how to re-use the setjmp/longjmp mechanism for
// task switching - all we need are the offsets where SP and PC are stored.

#include <jee.h>
#include <setjmp.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

jmp_buf jb;

void dumpJump (bool immediate) {
    if (immediate) {
        setjmp(jb); // fill in jb

        uint32_t sp = (uint32_t) &immediate; // the real SP is slightly lower
        uint32_t pc = (uint32_t) &dumpJump;  // the real PC is slightly higher
        printf("'setjmp' layout guess: sp < %08x, pc > %08x:", sp, pc);

        for (unsigned i = 0; i < sizeof jb / 4; ++i) {
            if (i % 8 == 0)
                printf("\n%6d: ", i);
            printf(" %08x", ((uint32_t*) &jb)[i]);
        }
        printf("\n");

        // find the most plausible location of sp in the jb buffer
        for (unsigned i = 0; i < sizeof jb / 4; ++i)
            if (sp - ((uint32_t*) &jb)[i] < 10)
                printf("  best guess for SP is word #%d (%d off)\n",
                        i, sp - ((uint32_t*) &jb)[i]);

        // find the most plausible location of sp in the jb buffer
        for (unsigned i = 0; i < sizeof jb / 4; ++i)
            if (((uint32_t*) &jb)[i] - pc < 30)
                printf("  best guess for PC is word #%d (%d off)\n",
                        i, ((uint32_t*) &jb)[i] - pc);
        return;
    }
    
    // else use recursion to add an extra stack frame
    dumpJump(!immediate);
}

void jumpInfo () {
    printf("FIRST ");
    dumpJump(true);
    printf("SECOND ");
    dumpJump(false);
    printf("sizeof (jmp_buf) = %d bytes\n", sizeof (jmp_buf));
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);

    wait_ms(500);
    jumpInfo();
    wait_ms(100);
}

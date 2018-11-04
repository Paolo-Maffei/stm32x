// Load and convert a RIM< loader binary file to compact 8+4 bit data format
// -jcw, 2018-11-05

#include <stdio.h>
#include <stdlib.h>

typedef uint16_t Word;

#define MEMSIZE 4096
uint8_t store [MEMSIZE+MEMSIZE/2];

static Word mask12(Word w) { return w & 07777; }

struct Mem12 {
    class MemRef {
        Word addr;
    public:
        MemRef (int a) : addr (a) {}

        operator Word () const {
            uint8_t u = store[MEMSIZE+addr/2];
            u = addr & 1 ? u >> 4 : u & 0x0F;
            return store[addr] + (u << 8);
        }

        void operator= (Word value) {
            uint8_t u = store[MEMSIZE+addr/2];
            if (addr & 1)
                u = (u & 0x0F) | ((value & 0xF00) >> 4);
            else
                u = (u & 0xF0) | ((value & 0xF00) >> 8);
            store[MEMSIZE+addr/2] = u;
            store[addr] = value;
        }
    };

    MemRef operator[] (int addr) const {
        return addr;
    }
} mem;

int loader (FILE* fp) {
    Word addr = 0;
    printf("LOAD");
    Word b;
    while (!feof(fp)) {
        if (fgetc(fp) == 0200) // skip until run-in found
            break;
    }
    while (!feof(fp)) {
        // read next word
        b = fgetc(fp);
        if (b & 0200) // skip run-in
            continue;
        b = (b << 6) | fgetc(fp);

        // look for run-out, to ignore word before it as being a checksum
        int c = fgetc(fp);
        if (c & 0200)
            break;
        ungetc(c, fp);

        // process one word
        if (b & 010000) {
            if (addr != 0)
                printf("-%04o", addr - 1);
            addr = mask12(b);
            printf(" %04o", addr);
        } else
            mem[addr++] = b;
    }
    printf("-%04o CHECK %04o\n", mask12(addr - 1), mask12(b));
    return addr;
}

int main (int argc, const char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s binrimfile outfile\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == 0) {
        perror(argv[1]);
        return 2;
    }
    if (loader(fp) == 0)
        return 3;
    fclose(fp);

    FILE* ofp = fopen(argv[2], "w");
    if (ofp == 0) {
        perror(argv[2]);
        return 4;
    }

    if (fwrite(store, 1, sizeof store, ofp) <= 0) {
        perror(argv[2]);
        return 5;
    }

    fclose(ofp);
}

// PDP-8 emulator, ported from Posix to STM32F103 with JeeH
// -jcw, 2016-08-29

typedef uint16_t Word;

#define MEMSIZE 4096
extern uint8_t store [MEMSIZE+MEMSIZE/2];

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

static Word incr12 (Word w) { return (w+1) & 07777; }
static Word mask13 (Word w) { return w & 017777; }

static Word opAddr (int ir, Word pc) {
    Word a = ir & 0177;
    if (ir & 0200)
        a |= (pc - 1) & 07600;
    if (ir & 0400) {
        if ((a & 07770) == 010)
            mem[a] = mem[a] + 1;
        a = mem[a];
    }
    return a;
}

void run () {
    Word pc = 0200;
    Word sr = 0;

    Word ac = 0, mq = 0;
    int iena = 0;
    for (;;) {
        iena >>= 1; // delayed interrupt enabling, relies on sign extension

        static short counter; // HACK: every 1024 ops, we fake an interrupt
        if ((iena & 1) && (++counter & 0x03FF) == 0) {
            mem[0] = pc;
            pc = 1;
            iena = 0;
        }

        int ir = mem[pc];
        pc = incr12(pc);
        switch ((ir >> 9) & 07) {

            case 0: // AND
                ac &= mem[opAddr(ir, pc)] | 010000;
                break;

            case 1: // TAD
                ac = mask13(ac + mem[opAddr(ir, pc)]);
                break;

            case 2: { // ISZ
                Word t = opAddr(ir, pc);
                mem[t] = mem[t] + 1;
                if (mem[t] == 0)
                    pc = incr12(pc);
                break;
            }

            case 3: // DCA
                mem[opAddr(ir, pc)] = ac;
                ac &= 010000;
                break;

            case 4: { // JMS
                Word t = opAddr(ir, pc);
                mem[t] = pc;
                pc = incr12(t);
                break;
            }

            case 5: // JMP
                pc = opAddr(ir, pc);
                break;

            case 6: // IOT
                switch ((ir >> 3) & 077) {
                    case 00:
                        switch (ir) {
                            case 06001: iena = ~1; break; // delays one cycle
                            case 06002: iena = 0; break;
                        }
                        break;
                    case 03: // keyboard
                        if ((ir & 01) && rxReady()) // skip if ready
                            pc = incr12(pc);
                        if (ir & 04) // read byte
                            ac = (ac & 010000) | rxReceive();
                        break;
                    case 04: // teleprinter
                        if ((ir & 01) && txReady()) // skip if ready
                            pc = incr12(pc);
                        if (ir & 04) // send byte
                            txSend(ac & 0177); // strip off parity
                        if (ir & 02) // clear flag
                            ac &= 010000;
                        break;
                }
                break;

            case 7: // OPR
                if ((ir & 0400) == 0) { // group 1
                    if (ir & 0200) // CLA
                        ac &= 010000;
                    if (ir & 0100) // CLL
                        ac &= 07777;
                    if (ir & 040) // CMA
                        ac ^= 07777;
                    if (ir & 020) // CML
                        ac ^= 010000;
                    if (ir & 01) // IAC
                        ac = mask13(ac + 1);
                    switch (ir & 016) {
                        case 012: // RTR
                            ac = mask13((ac >> 1) | (ac << 12)); // fall through
                        case 010: // RAR
                            ac = mask13((ac >> 1) | (ac << 12));
                            break;
                        case 06: // RTL
                            ac = mask13((ac >> 12) | (ac << 1)); // fall through
                        case 04: // RAL
                            ac = mask13((ac >> 12) | (ac << 1));
                            break;
                        case 02: // BSW
                            ac = (ac & 010000) | ((ac >> 6) & 077)
                                                | ((ac << 6) & 07700);
                            break;
                    }
                } else if ((ir & 01) == 0) { // group 2
                    // SMA, SPA, SZA, SNA, SNL, SZL
                    int s = ((ir & 0100) && (ac & 04000)) ||
                            ((ir & 040) && (ac & 07777) == 0) ||
                            ((ir & 020) && (ac & 010000) != 0) ? 0 : 010;
                    if (s == (ir & 010))
                        pc = incr12(pc);
                    if (ir & 0200) // CLA
                        ac &= 010000;
                    if (ir & 04) // OSR
                        ac |= sr;
                    if (ir & 02) // HLT
                        return;
                } else { // group 3
                    Word t = mq;
                    if (ir & 0200) // CLA
                        ac &= 010000;
                    if (ir & 020) { // MQL
                        mq = ac & 07777;
                        ac &= 010000;
                    }
                    if (ir & 0100)
                        ac |= t;
                }
                break;
        }
    }
}

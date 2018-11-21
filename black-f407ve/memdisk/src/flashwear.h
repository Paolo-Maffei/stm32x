// Wear-leveling wrapper around the internal Flash driver.

class FlashWear {
    static constexpr bool DEBUG = false;
    static constexpr int SECLEN = 128;
    static constexpr int NUM_MODS = 500;
    static constexpr int SEC_PER_SEG = 1024;
    static constexpr int NUM_SEGS = 3;

    typedef struct {
        uint16_t map [NUM_MODS];
        uint8_t flags [NUM_MODS];
        uint8_t segs [16];
        uint8_t spare [20];
        uint8_t sectors [NUM_MODS][SECLEN];
    } ModPage;

    typedef uint8_t Segment [SEC_PER_SEG][SECLEN];

    static ModPage const& mods;  // used to collect all changes
    static Segment const* segs;  // actual data storage segments
    static uint16_t fill;        // next unused entry in map

    static void reFlashChanges () {
        if (DEBUG)
            printf("reFlashChanges\n");

        for (;;) {}
    }

public:
    static int init (bool erase =false) {
        if (DEBUG)
            printf("FlashWear %d, ModPage %d, Segment %d\n",
                    sizeof (FlashWear), sizeof (ModPage), sizeof (Segment));
        if (erase || mods.segs[0] > 2) {
            printf("initialising internal flash\n");
            Flash::erasePage(&mods);
            for (int i = 0; i < NUM_SEGS-1; ++i)
                Flash::write8(mods.segs + i, i+1);
        }
        for (fill = NUM_MODS; mods.map[fill-1] == 0xFFFF; --fill)
            if (fill == 0)
                break;
        if (DEBUG) {
            printf("fill %d, segs:", fill);
            for (int i = 0; i < NUM_SEGS-1; ++i)
                printf(" %d", mods.segs[i]);
            printf("\n");
        }
        return (NUM_SEGS-1) * SEC_PER_SEG;
    }

    static void readSector (int pos, void* buf) {
        for (int i = fill; --i >= 0; )
            if (mods.map[i] == pos) {
                if (DEBUG)
                    printf("readSector %d mod %d\n", pos, i);
                memcpy(buf, mods.sectors[i], SECLEN);
                return; // return modified sector
            }
        // no changed version found, return the original sector
        int segment = mods.segs[pos/SEC_PER_SEG];
        if (DEBUG)
            printf("readSector %d d seg %d @ %d\n",
                    pos, segment, pos % SEC_PER_SEG);
        memcpy(buf, segs[segment][pos%SEC_PER_SEG], SECLEN);
    }

    static void writeSector (int pos, void const* buf) {
        if (fill >= NUM_MODS)
            reFlashChanges();
        int n = fill++;
        if (DEBUG)
            printf("writeSector %d mod %d\n", pos, n);
        Flash::write16(mods.map + n, pos);
        Flash::write32buf(mods.sectors[n], (uint32_t const*) buf, SECLEN/4);
    }
};

FlashWear::ModPage const& FlashWear::mods =
    *(FlashWear::ModPage const*) 0x00010000;  // @ 64K
FlashWear::Segment const* FlashWear::segs =
    (FlashWear::Segment const*) 0x00020000;  // @ 128K
uint16_t FlashWear::fill = 0;

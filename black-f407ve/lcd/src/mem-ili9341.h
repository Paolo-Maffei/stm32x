// Driver for an ILI9341-based 320x240 LCD TFT display, using 16b-mode FSMC
// see https://jeelabs.org/ref/ILI9341.pdf

template< uint32_t ADDR >
struct ILI9341 {
    constexpr static int width = 240;
    constexpr static int height = 320;

    static void init () {
        static uint8_t const config [] = {
            // cmd, count, data bytes ...
#if 1
           0x3A, 1, 0x55, // pxiel format 16b
           0x36, 1, 0xB8, // orientation, bits 7..4 = MY MX MV ML
           0x11, 0,       // sleep off
#elif 0
            0xB1, 2, 0x00, 0x18,                     // FRMCTR1
            0xB6, 3, 0x08, 0x82, 0x27,               // DFUNCTR
            0xF2, 1, 0x00,
            0x26, 1, 0x01,                           // GAMMASET
            0xE0, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                      0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,  // GMCTRP1
            0xE1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                      0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,  // GMCTRN1
#elif 0
            0xEF, 3, 0x03, 0x80, 0x02,               // ??
            0xCF, 3, 0x00, 0xC1, 0x30,               // power control B
            0xED, 4, 0x64, 0x03, 0x12, 0x81,         // power on sequence ctrl
            0xE8, 3, 0x85, 0x00, 0x78,               // driver timing control A
            0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,   // power control A
            0xF7, 1, 0x20,                           // pump ration control
            0xEA, 2, 0x00, 0x00,                     // driver timing control B
            0xC0, 1, 0x23,                           // PWCTR1
            0xC1, 1, 0x10,                           // PWCTR2
            0xC5, 2, 0x3e, 0x28,                     // VMCTR1
            0xC7, 1, 0x86,                           // VMCTR2
            0x36, 1, 0x68,      // memory access ctrl: column order, BGR filter
            0x37, 2, 0, 0,                           // vertical scroll start
            0x3A, 1, 0x55,                           // format: 16 bits/pixel
            0xB1, 2, 0x00, 0x18,                     // FRMCTR1
            0xB6, 3, 0x08, 0x82, 0x27,               // DFUNCTR
            0xF2, 1, 0x00,
            0x26, 1, 0x01,                           // GAMMASET
            0xE0, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                      0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,  // GMCTRP1
            0xE1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                      0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,  // GMCTRN1
            0x11, 0,                                 // SLPOUT
#elif 0
            0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
            0xCF, 3, 0x00, 0xA2, 0xF0,
            0xE8, 3, 0x84, 0x11, 0x7A,
            0xEA, 2, 0x66, 0x00,
            0xED, 4, 0x55, 0x01, 0x23, 0x01,
            0xF7, 1, 0x20,
            0xC0, 1, 0x1C,
            0xC1, 1, 0x13,
            0xC5, 2, 0x23, 0x3F,
            0xC7, 1, 0xA5,
            0xB1, 2, 0x00, 0x17,
            0x3A, 1, 0x55,
            0xB6, 4, 0x0A, 0xA2, 0x27, 0x00,
            0x36, 1, 0x08,
            0xF2, 1, 0x02,
            0x26, 1, 0x01,
            0xE0, 15, 0x0F, 0x14, 0x13, 0x0C, 0x0E, 0x05, 0x45, 0x85,
                      0x36, 0x09, 0x14, 0x05, 0x09, 0x03, 0x00,
            0xE1, 15, 0x00, 0x24, 0x26, 0x03, 0x0F, 0x04, 0x3F, 0x14,
                      0x52, 0x04, 0x10, 0x0E, 0x38, 0x39, 0x0F,
            0x2A, 4, 0x00, 0x00, 0x00, 0xEF,
            0x2B, 4, 0x00, 0x00, 0x01, 0x3F,
            0x11, 0,
#else
            0xCF, 3, 0x00, 0x83, 0x30,
            0xED, 4, 0x64, 0x03, 0x12, 0x81,
            0xE8, 3, 0x85, 0x01, 0x79,
            0xCB, 5, 0x39, 0X2C, 0x00, 0x34, 0x02,
            0xF7, 1, 0x20,
            0xEA, 2, 0x00, 0x00,
            0xC0, 1, 0x26,
            0xC1, 1, 0x11,
            0xC5, 2, 0x35, 0x3E,
            0xC7, 1, 0xBE,
            0xB1, 2, 0x00, 0x1B,
            0xB6, 4, 0x0A, 0x82, 0x27, 0x00,
            0xB7, 1, 0x07,
            0x3A, 1, 0x55,
            0x11, 0,
#endif
        };

#if 1
        for (uint8_t const* p = config; p < config + sizeof config; ++p) {
            if (*p == 0xFF)
                wait_ms(*++p);
            else {
                cmd(*p);
                int n = *++p;
                while (--n >= 0)
                    out16(*++p);
            }
        }
#endif

        wait_ms(120);
        cmd(0x29);      // DISPON
    }

    static void cmd (int v) {
        MMIO16(ADDR-2) = v;
    }

    static void out16 (int v) {
        MMIO16(ADDR) = v;
    }

    static void pixel (int x, int y, uint16_t rgb) {
        cmd(0x2A);
        out16(y>>8);
        out16(y);
        out16(yEnd>>8);
        out16(yEnd);

        cmd(0x2B);
        out16(x>>8);
        out16(x);
        out16(xEnd>>8);
        out16(xEnd);

        cmd(0x2C);
        out16(rgb);
    }

    static void pixels (int x, int y, uint16_t const* rgb, int len) {
        pixel(x, y, *rgb);

        for (int i = 1; i < len; ++i)
            out16(rgb[i]);
    }

    static void bounds (int xend =width-1, int yend =height-1) {
        xEnd = xend;
        yEnd = yend;
    }

    static void fill (int x, int y, int w, int h, uint16_t rgb) {
        bounds(x+w-1, y+h-1);
        pixel(x, y, rgb);

        int n = w * h;
        while (--n > 0)
            out16(rgb);
    }

    static void clear () {
        fill(0, 0, width, height, 0);
    }

    static void vscroll (int vscroll =0) {
        cmd(0x37);
        out16(vscroll>>8);
        out16(vscroll);
    }

    static uint16_t xEnd, yEnd;
};

template< uint32_t ADDR>
uint16_t ILI9341<ADDR>::xEnd = width-1;

template< uint32_t ADDR>
uint16_t ILI9341<ADDR>::yEnd = height-1;

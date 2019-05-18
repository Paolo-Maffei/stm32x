// these are ads from www.buy-display.com, the OLED manufacturer

const uint8_t image1 []= {/*--ER-OLED0.28-.bmp  --*/
    /*--  256*64  --*/
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x00,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,
    0x01,0x00,0x00,0x00,0x00,0x10,0x08,0x40,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x00,0x01,0xE0,0x00,0x00,0x00,0x00,0x20,0xF8,0x08,0x04,0x04,0x00,0x00,0x00,
    0x01,0x00,0x01,0x00,0x03,0x10,0x08,0x40,0x01,0x00,0x0D,0xF0,0x01,0x00,0x07,0xF1,
    0x80,0x00,0x01,0xE0,0x00,0x00,0x00,0x00,0x13,0x08,0x08,0x84,0x04,0x00,0x03,0xE0,
    0x01,0x70,0x00,0x80,0x1C,0x10,0x08,0x40,0x01,0xFC,0x35,0x10,0x00,0x80,0x00,0x11,
    0x80,0x00,0x03,0xE4,0x00,0x00,0x00,0x00,0x04,0xA0,0x08,0xA4,0x07,0x3C,0x0C,0x20,
    0x1F,0x80,0x00,0x00,0x04,0x50,0x08,0x78,0x3E,0x00,0x29,0xD0,0x04,0x40,0x03,0x11,
    0x80,0x00,0x07,0xEF,0xC0,0x00,0x00,0x00,0x41,0x10,0x08,0xA4,0x3D,0x44,0x08,0x20,
    0x02,0x00,0x00,0xFC,0x07,0x10,0x0E,0xC0,0x02,0x00,0x29,0x20,0x04,0x40,0x1C,0x11,
    0x80,0x00,0x0F,0xEF,0xFE,0x00,0x00,0x00,0x22,0x80,0x0C,0xA4,0x05,0x74,0x08,0x20,
    0x02,0x80,0x3F,0x80,0x1C,0x50,0x38,0x40,0x03,0xE0,0x25,0xE0,0x08,0x20,0x00,0x11,
    0x80,0x00,0x0F,0xE7,0xFF,0xE0,0x00,0x00,0x00,0xF8,0x38,0xA4,0x05,0x44,0x0F,0xA0,
    0x04,0x80,0x01,0x00,0x66,0x10,0x08,0x70,0x06,0x20,0x3D,0x08,0x09,0x18,0x07,0x11,
    0x80,0x00,0x07,0xE7,0x3F,0xFE,0x00,0x00,0x17,0x80,0x08,0xA4,0x09,0x44,0x08,0x20,
    0x09,0xF0,0x01,0xE0,0x0D,0x3E,0x0D,0x90,0x0B,0xA0,0x21,0x90,0x11,0x0E,0x19,0x11,
    0x80,0x00,0x07,0xE7,0x07,0xFF,0xF0,0x00,0x11,0xC0,0x0E,0xA4,0x09,0x78,0x08,0x20,
    0x0E,0x80,0x02,0x20,0x14,0xD0,0x18,0xA0,0x12,0x20,0x21,0x60,0x22,0x00,0x11,0x11,
    0x80,0x00,0x07,0xE7,0x00,0xFF,0xFE,0x00,0x22,0xA0,0x19,0x24,0x11,0x02,0x08,0x20,
    0x00,0x90,0x04,0x20,0x24,0x10,0x68,0x40,0x22,0x20,0x21,0x20,0x04,0x40,0x1F,0x11,
    0x80,0x00,0x07,0xF7,0x00,0x1F,0xFE,0x00,0x24,0x98,0x61,0x24,0x21,0x02,0x0F,0xE0,
    0x08,0x88,0x08,0x20,0x04,0x10,0x08,0xA0,0x43,0xA0,0x21,0x5E,0x09,0xE0,0x10,0x11,
    0x80,0x00,0x03,0xF7,0x00,0x03,0xFE,0x00,0x28,0x8E,0x02,0x04,0x41,0x02,0x00,0x20,
    0x08,0x84,0x31,0x40,0x04,0x10,0x0B,0x18,0x02,0x20,0x21,0x80,0x0E,0x20,0x00,0x11,
    0x80,0x00,0x03,0xF7,0x00,0x00,0x7E,0x00,0x00,0x80,0x04,0x04,0x00,0xFC,0x00,0x00,
    0x11,0x80,0x00,0x80,0x04,0x10,0x18,0x0E,0x04,0x60,0x21,0x00,0x00,0x00,0x00,0x71,
    0x80,0x00,0x03,0xF3,0x00,0x00,0x1C,0x00,0x00,0x80,0x00,0x04,0x00,0x00,0x00,0x00,
    0x00,0x80,0x00,0x00,0x00,0x10,0x00,0x00,0x04,0x20,0x00,0x00,0x00,0x00,0x00,0x21,
    0x80,0x00,0x01,0xF3,0x00,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0xC0,0x01,0xF3,0x80,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0xF8,0x01,0xF3,0x80,0x00,0x3C,0x00,0x0F,0x30,0x00,0x01,0xFB,0x00,0x00,0x03,
    0xE0,0x00,0x27,0xCC,0x03,0x00,0x00,0x3F,0x00,0x03,0x00,0x00,0x18,0x00,0x00,0x01,
    0x80,0x7F,0x00,0xF3,0x80,0x00,0x3C,0x00,0x19,0xB0,0x00,0x00,0x1B,0x00,0x00,0x03,
    0x00,0x00,0x66,0x60,0x00,0x00,0x00,0x0C,0x00,0x03,0x00,0x00,0x18,0x00,0x00,0x01,
    0x80,0x7F,0xC0,0xF3,0x80,0x00,0x38,0x00,0x18,0x3E,0x3C,0xF8,0x33,0xE3,0xCF,0x83,
    0x07,0x1E,0xF6,0x6C,0xF3,0x7C,0x7C,0x0C,0x3C,0x73,0xE7,0xC7,0x99,0xE3,0xF8,0xC1,
    0x80,0x3F,0xF8,0xF3,0x80,0x00,0x38,0x00,0x1E,0x33,0x66,0xCC,0x63,0x36,0x6C,0xC3,
    0xE9,0xB3,0x66,0x6D,0x9B,0x66,0xCC,0x0C,0x66,0xDB,0x36,0x6C,0xDB,0x36,0x6D,0x81,
    0x80,0x3F,0xFE,0xF9,0x80,0x00,0x38,0x00,0x07,0xB3,0x7E,0xCC,0x63,0x37,0xEC,0xC3,
    0x07,0xBC,0x67,0xCD,0xE3,0x66,0xCC,0x0C,0x7E,0xC3,0x36,0x6C,0xDB,0x36,0x6D,0x81,
    0x80,0x1D,0xFF,0x79,0x80,0x00,0x78,0x00,0x01,0xB3,0x60,0xCC,0xC3,0x36,0x0C,0xC3,
    0x0D,0x8F,0x66,0xCC,0x7B,0x66,0xCC,0x0C,0x60,0xC3,0x36,0x6C,0xDB,0x36,0x6D,0x81,
    0x80,0x1C,0x3F,0x79,0x80,0x00,0x70,0x00,0x19,0xB3,0x66,0xCD,0x83,0x36,0x6C,0xC3,
    0x0D,0xB3,0x66,0x6D,0x9B,0x66,0xCC,0x0C,0x66,0xDB,0x36,0x6C,0xDB,0x36,0x67,0x01,
    0x80,0x1C,0x07,0x79,0x80,0x00,0x70,0x00,0x0F,0x33,0x3C,0xCD,0xFB,0x33,0xCC,0xC3,
    0xE7,0x9E,0x36,0x3C,0xF3,0x66,0x7C,0x0C,0x3C,0x73,0x36,0x67,0x99,0xE3,0xE7,0x01,
    0x80,0x0E,0x00,0x39,0x80,0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x8C,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x66,0x01,
    0x80,0x0E,0x00,0x39,0xC0,0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xDC,0x01,
    0x80,0x06,0x00,0x39,0xC0,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x07,0x00,0x18,0xC0,0x00,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x03,0x00,0x18,0xC0,0x00,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x03,0x00,0x1C,0xC0,0x00,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x01,0x80,0x1C,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x01,0x80,0x0C,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x1F,0xE7,0xF8,0x00,
    0xF0,0x60,0x3F,0xCF,0xC1,0xC1,0xC7,0x07,0x87,0x00,0x0C,0x00,0x00,0x00,0x00,0x01,
    0x80,0x00,0x80,0x0C,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x1F,0xE7,0xFC,0x03,
    0xFC,0x60,0x3F,0xCF,0xF1,0xC1,0xCF,0x8F,0xCF,0x80,0x1C,0x00,0x00,0x00,0x00,0x01,
    0x80,0x00,0x80,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x06,0x0C,0x03,
    0x0C,0x60,0x30,0x0C,0x31,0xE3,0xDD,0xD8,0xD8,0xC0,0x3C,0x00,0x00,0x00,0x00,0x01,
    0x80,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x06,0x0C,0x06,
    0x06,0x60,0x30,0x0C,0x19,0xE3,0xD8,0xD8,0xD8,0xC0,0x6C,0x07,0x03,0x83,0x30,0x01,
    0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xE6,0x0C,0x06,
    0x06,0x60,0x3F,0xCC,0x19,0xA2,0xD8,0xC0,0xCF,0x80,0x4C,0x0D,0x86,0xC3,0x30,0x01,
    0x87,0xE0,0x00,0x02,0x3F,0x0C,0x00,0xC0,0x00,0x00,0x00,0x00,0x1F,0xE7,0xF8,0x06,
    0x06,0x60,0x3F,0xCC,0x19,0xB6,0xD8,0xC1,0x8F,0x80,0x0C,0x01,0x86,0xC3,0x30,0x01,
    0x86,0x00,0x00,0x06,0x31,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x07,0xF0,0xF6,
    0x06,0x60,0x30,0x0C,0x19,0xB6,0xD8,0xC3,0x18,0xCF,0x0C,0x01,0x83,0x84,0x40,0x01,
    0x86,0x03,0xC3,0xCF,0x31,0x8C,0x78,0xCD,0x86,0xC0,0x00,0x00,0x18,0x06,0x38,0xF6,
    0x06,0x60,0x30,0x0C,0x19,0x9C,0xD8,0xC6,0x18,0xCF,0x0C,0x03,0x06,0xC0,0x00,0x01,
    0x86,0x04,0x66,0x66,0x31,0x8C,0xCC,0xCE,0xCD,0xC0,0x00,0x00,0x18,0x06,0x1C,0x03,
    0x0C,0x60,0x30,0x0C,0x31,0x9C,0xDD,0xCC,0x18,0xC0,0x0C,0x06,0x06,0xC0,0x00,0x01,
    0x87,0xE1,0xE7,0x06,0x3F,0x0C,0xE0,0xCC,0xCC,0xC0,0x00,0x00,0x1F,0xE6,0x0C,0x03,
    0xFC,0x7F,0x3F,0xCF,0xF1,0x9C,0xCF,0x9F,0xCF,0x80,0x0C,0x0C,0x36,0xC0,0x00,0x01,
    0x86,0x03,0x63,0xC6,0x33,0x0C,0x78,0xCC,0xCC,0xC0,0x00,0x00,0x1F,0xE6,0x0E,0x00,
    0xF0,0x7F,0x3F,0xCF,0xC1,0x88,0xC7,0x1F,0xC7,0x00,0x0C,0x0F,0xB3,0x80,0x00,0x01,
    0x86,0x06,0x60,0xE6,0x31,0x8C,0x1C,0xCC,0xCC,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x86,0x06,0x66,0x66,0x31,0x8C,0xCC,0xCC,0xCD,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x87,0xE3,0xE3,0xC3,0x30,0xCC,0x78,0xCC,0xC6,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xC7,0x9C,0x47,0x04,0x3C,
    0x01,0x00,0x04,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x14,0x04,0x5E,0x00,0x10,0x11,
    0xA0,0x00,0x00,0x28,0x00,0x20,0x00,0x00,0x00,0x00,0x02,0x24,0x22,0xE8,0x8C,0x22,
    0x01,0x00,0x0C,0x44,0x44,0x00,0x00,0x00,0x00,0x00,0x14,0x04,0x61,0x00,0x00,0x11,
    0xA0,0x00,0x00,0x20,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x28,0x20,0x48,0x14,0x21,
    0x3B,0x9C,0x14,0x40,0x82,0xA7,0x22,0x05,0x8E,0x53,0x94,0xE4,0xA0,0x39,0x53,0x91,
    0xBC,0x8A,0x21,0xE8,0xEF,0x27,0x22,0x18,0xE7,0x60,0x00,0x2F,0x3C,0xAF,0x14,0x21,
    0x45,0x22,0x04,0x78,0x80,0xC8,0xA2,0x06,0x51,0x64,0x55,0x14,0x98,0x45,0x94,0x51,
    0xA2,0x89,0x42,0x29,0x08,0xA0,0x94,0x25,0x14,0x90,0x00,0x40,0xA2,0x08,0xA4,0x21,
    0x45,0x18,0x04,0x44,0x8E,0x87,0x94,0x04,0x4F,0x43,0xD5,0xF4,0x86,0x7D,0x13,0xD1,
    0xA2,0x89,0x5A,0x28,0xC8,0xA3,0x94,0x21,0x14,0x90,0x00,0x80,0xA2,0x08,0xBE,0x21,
    0x45,0x04,0x04,0x44,0x82,0x88,0x94,0x04,0x51,0x44,0x55,0x04,0x81,0x41,0x14,0x51,
    0xA2,0x89,0x42,0x28,0x28,0xA4,0x94,0x25,0x14,0x90,0x01,0x08,0xA2,0x08,0x84,0x22,
    0x45,0x22,0x04,0x44,0x44,0x89,0x88,0x06,0x53,0x44,0xD5,0x15,0x21,0x45,0x14,0xD1,
    0xBC,0x78,0x81,0xE9,0xCF,0x23,0x88,0x98,0xE4,0x90,0x03,0xE7,0x1C,0x07,0x04,0x3C,
    0x39,0x9C,0x04,0x38,0x38,0x86,0x88,0x05,0x8D,0x43,0x54,0xE5,0x1E,0x39,0x13,0x51,
    0x80,0x00,0x80,0x00,0x08,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x03,0x00,0x00,0x08,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

const uint8_t image2 []= {
    0xFC,0x00,0x00,0x20,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,
    0x80,0x00,0x00,0xFE,0x00,0x00,0x00,0x0C,0x00,0x0C,0x07,0x00,0x00,0x00,0x00,0x00,
    0x00,0x7B,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xE0,0x00,0x01,
    0x80,0x00,0x39,0xBE,0x1E,0x00,0x00,0x1C,0x00,0x3E,0x0F,0xFF,0x80,0x00,0x00,0x00,
    0x01,0xFF,0x7E,0x00,0x00,0x00,0x0F,0x82,0x78,0x00,0x00,0x00,0x07,0xF0,0x00,0x01,
    0x80,0x01,0xFF,0x07,0x3F,0xF8,0x00,0x3C,0x00,0x7F,0x3E,0x73,0x80,0x00,0x00,0x00,
    0x03,0xFC,0x1F,0x00,0x00,0x00,0x1F,0xCF,0xD8,0x00,0x00,0x00,0x1F,0xF8,0x00,0x01,
    0x80,0x07,0x82,0x03,0x3F,0xF8,0x00,0x3C,0x00,0xFF,0xE0,0x03,0x00,0x00,0x00,0x00,
    0x07,0xFC,0x01,0x80,0x00,0x00,0x3F,0xD8,0x30,0x00,0x00,0x00,0x1F,0xF8,0x00,0x01,
    0x80,0x03,0x00,0x1F,0xFF,0xF8,0x00,0x3E,0x01,0xFF,0xC0,0x01,0x80,0x00,0x00,0x00,
    0x07,0xFC,0x00,0xC0,0x00,0x00,0x3F,0xE0,0x0C,0x00,0x00,0x00,0x3F,0xF8,0x00,0x01,
    0x00,0x03,0x00,0x7E,0x7F,0xF8,0x00,0x7E,0x01,0xFF,0xE0,0x01,0xC3,0xC0,0x00,0x00,
    0x07,0xFC,0x00,0x80,0x00,0x00,0x7F,0xE0,0x06,0x00,0x00,0x00,0x3F,0xF8,0x10,0x01,
    0x00,0x03,0x7F,0xFF,0x7F,0xFC,0x00,0x7E,0x03,0xFF,0xC0,0x01,0xEF,0xE0,0x00,0x00,
    0x07,0xEF,0xFB,0x80,0x00,0x00,0x7F,0xF8,0x04,0x00,0x00,0x00,0x3F,0xF8,0xF8,0x00,
    0x00,0x03,0xF7,0xFF,0xFF,0xFE,0x00,0xEE,0x03,0xFF,0xE0,0x01,0xFF,0xE0,0x00,0x00,
    0x07,0xFF,0xFF,0xC0,0x00,0x00,0x7F,0xCF,0xFE,0x00,0x00,0x00,0x3F,0xF1,0xDC,0x00,
    0x00,0x07,0xFE,0x7F,0x3F,0xFE,0x00,0xB2,0x03,0xFF,0xF8,0x03,0x1F,0xF0,0x00,0x00,
    0x07,0xFF,0xE1,0xB0,0x00,0x00,0x3F,0x83,0xFE,0x00,0x00,0x00,0x3F,0xF3,0x86,0x00,
    0x07,0xCF,0xFC,0x1E,0x1F,0xFF,0x01,0xFA,0x01,0xFF,0x1C,0x7F,0xDF,0xF0,0x00,0x00,
    0x0C,0xFB,0xFE,0xD8,0x00,0x00,0x3F,0x03,0xFD,0x00,0x00,0x00,0x1F,0x1F,0x03,0x80,
    0x07,0xFF,0xF8,0x07,0xCF,0xFF,0x83,0xCF,0x01,0xFF,0xEF,0xF8,0xEF,0xF0,0x00,0x00,
    0x18,0x03,0xFF,0xCD,0xE0,0x00,0x60,0x03,0xFD,0x00,0x00,0x00,0x18,0x1F,0xC1,0x90,
    0x1F,0xFB,0xE0,0x1F,0xF7,0xFF,0x0F,0xEF,0x00,0xEF,0xF7,0xFF,0xEF,0xF0,0x00,0x00,
    0x1F,0x81,0xFF,0x07,0xF0,0x00,0xC0,0x03,0xFF,0x00,0x00,0x00,0x30,0x1F,0x60,0xF8,
    0x1F,0xF8,0x38,0x1F,0xFF,0xFE,0x0F,0xFF,0x00,0x4F,0xFC,0xFF,0xE7,0xE0,0x00,0x00,
    0x3F,0x80,0xFE,0x07,0xF8,0x01,0x80,0x01,0xFE,0x80,0x00,0x00,0x60,0x1F,0xB0,0x78,
    0x1F,0xF0,0xFE,0x39,0xFF,0xFE,0x00,0x03,0x00,0x4F,0xFC,0xFF,0xE3,0xE0,0x00,0x00,
    0x70,0x80,0xF8,0x07,0xF8,0x01,0x00,0x00,0xFC,0x80,0x00,0x00,0xC0,0x1F,0xD8,0x7C,
    0x1F,0xF1,0xFF,0x30,0x7F,0xFC,0x00,0x01,0x80,0x83,0xF8,0x7F,0xE3,0xC0,0x00,0x01,
    0xE3,0x03,0xF8,0x07,0xF8,0x02,0x00,0x00,0x20,0x80,0x00,0x01,0x80,0xDF,0xEC,0x7C,
    0x1F,0xE3,0xF3,0x30,0x7F,0xFC,0x00,0x01,0x81,0x83,0xE0,0x3F,0xC2,0x00,0x00,0x03,
    0x67,0x06,0x3C,0x03,0xF8,0x06,0x00,0x00,0x0E,0xC0,0x00,0x03,0x03,0xFF,0xFC,0xFC,
    0x1F,0xE3,0xC1,0x26,0x3F,0xF8,0x00,0x00,0x81,0x0F,0xC0,0x06,0x02,0x00,0x00,0x06,
    0x66,0x06,0x3C,0x03,0xF8,0x04,0x00,0x00,0x1F,0x40,0x00,0x06,0x07,0xFF,0xFD,0xFC,
    0x07,0xE7,0xC1,0x2B,0x3F,0xF8,0x00,0x00,0xC3,0x3F,0xE0,0x38,0x03,0x00,0x00,0x0C,
    0x6E,0x0E,0x3C,0x03,0xF8,0x0C,0x00,0x00,0x3F,0x40,0x00,0x04,0x0F,0xFF,0xEF,0xFC,
    0x03,0xE7,0x9E,0x08,0x3F,0xF8,0x00,0x00,0xFF,0x7F,0xE0,0xFE,0x03,0x00,0x00,0x18,
    0x30,0x1E,0x3C,0x03,0xF0,0x08,0x00,0x00,0x39,0xC0,0x00,0x0C,0x1F,0xFF,0xF7,0xF8,
    0x00,0x67,0x92,0x00,0x7F,0xF8,0x00,0x00,0xC6,0xFC,0xE0,0xFF,0x83,0x00,0x00,0x10,
    0x0E,0x1C,0x3C,0x03,0xF0,0x10,0x00,0x00,0x70,0x60,0x00,0x18,0x3F,0x9C,0x07,0xB0,
    0x00,0x67,0x90,0x01,0xFD,0xF8,0x00,0x01,0xE7,0xFB,0xE1,0xFF,0xC3,0x00,0x00,0x30,
    0x0F,0x18,0x7C,0x03,0xC0,0x10,0x00,0x00,0x73,0x20,0x00,0x30,0x3F,0x0C,0x07,0xF0,
    0x00,0x67,0x82,0x01,0xF9,0xF8,0x00,0x03,0x7D,0xFE,0xC1,0xCF,0xC3,0x00,0x00,0x32,
    0x00,0x18,0xFC,0x06,0x00,0x30,0x00,0x00,0x73,0x20,0x00,0x20,0x3E,0x04,0x03,0xF0,
    0x00,0x67,0xC6,0x70,0x01,0xF8,0x00,0x02,0x3D,0xFC,0xC1,0xEF,0xC2,0x00,0x00,0x33,
    0x00,0x0F,0xF8,0x06,0x00,0x20,0x00,0x00,0x33,0x30,0x00,0x60,0x3E,0x1C,0x03,0xE0,
    0x00,0x67,0xFC,0xF0,0x01,0xF8,0x00,0x02,0x3D,0xF9,0x01,0x2F,0xE3,0x00,0x00,0x31,
    0x08,0x07,0xF0,0x06,0x00,0x20,0x00,0x00,0x3B,0x18,0x00,0x40,0x3E,0x38,0x07,0xC0,
    0x00,0x67,0xF8,0x23,0x01,0xF8,0x00,0x03,0x1D,0xFF,0x00,0x0F,0xC3,0x00,0x00,0x30,
    0xFC,0x01,0xC0,0x04,0x00,0x60,0x00,0x00,0x3C,0x18,0x00,0x40,0x3E,0x38,0x0F,0x80,
    0x00,0x63,0xF0,0x33,0x01,0xF8,0x00,0x01,0x8C,0xFF,0xC0,0xFF,0xC3,0x00,0x04,0x10,
    0x64,0x00,0x00,0x04,0x00,0xE0,0x00,0x00,0x0C,0x08,0x00,0x60,0x1E,0x30,0x1F,0x80,
    0x00,0x60,0x01,0xFE,0x01,0xF8,0x00,0x01,0xDC,0x7D,0xC0,0xFF,0xC3,0x00,0x0E,0x18,
    0x06,0x00,0x00,0x0C,0x00,0xC0,0x00,0x00,0x00,0x08,0x00,0x60,0x0F,0x80,0x33,0x00,
    0x00,0x60,0x01,0xC0,0x01,0xF8,0x00,0x00,0xFC,0x00,0x80,0x3F,0x83,0x00,0x1F,0x08,
    0x03,0x00,0x00,0x0C,0x00,0xC0,0x00,0x00,0x00,0x18,0x00,0x20,0x33,0x80,0x73,0x00,
    0x00,0x60,0x00,0x00,0x01,0xF8,0x00,0x00,0x1E,0x00,0x00,0x0E,0x06,0x00,0x3F,0x0C,
    0x01,0x80,0x18,0x08,0x00,0xC0,0x00,0x00,0x00,0x50,0x00,0x30,0x18,0x00,0x63,0x00,
    0x00,0x60,0x00,0x00,0x03,0xF8,0x00,0x00,0x0F,0x00,0x00,0x00,0x0E,0x00,0x7F,0xE6,
    0x00,0xE0,0x70,0x08,0x00,0x60,0x00,0x00,0x00,0xD0,0x00,0x10,0x08,0x02,0x67,0x00,
    0x00,0x60,0x00,0x00,0x03,0xF8,0x00,0x00,0x05,0x87,0xF0,0x00,0x0C,0x00,0xFF,0xFA,
    0x00,0x3F,0xC0,0x18,0x00,0x20,0x00,0x00,0x00,0xB0,0x00,0x78,0x04,0x03,0x0E,0x00,
    0x00,0x60,0x00,0x00,0x07,0xF8,0x00,0x00,0x06,0xC3,0xE0,0x00,0x18,0x00,0xFF,0xFF,
    0x00,0x00,0x00,0x10,0x00,0x30,0x00,0x00,0x01,0x60,0x01,0xFC,0x06,0x03,0x7E,0x00,
    0x00,0x30,0x00,0x00,0x0F,0xF0,0x00,0x00,0x06,0x60,0x00,0x00,0x70,0x00,0xFC,0xFF,
    0xC0,0x00,0x00,0x30,0x00,0x10,0x00,0x00,0x00,0xC0,0x01,0xFC,0x03,0x00,0x7E,0x00,
    0x00,0x38,0x00,0x00,0x3F,0xF0,0x00,0x00,0x03,0x38,0x00,0x00,0xE0,0x00,0x7F,0x7F,
    0xF0,0x00,0x00,0x60,0x00,0x0C,0x00,0x00,0x03,0x80,0x07,0xFE,0x00,0xF8,0x3A,0x00,
    0x00,0x1C,0x00,0x00,0x67,0xF0,0x00,0x00,0x03,0x0E,0x00,0x03,0x80,0x00,0x7B,0x7F,
    0xF8,0x00,0x01,0xC0,0x00,0x06,0x00,0x00,0x06,0x00,0x0F,0xFF,0x80,0x3C,0x02,0x00,
    0x00,0x0F,0x80,0x01,0x81,0xF0,0x00,0x00,0x01,0x8F,0xF8,0xFF,0x80,0x00,0x73,0x7F,
    0xFF,0x00,0x0F,0x80,0x00,0x01,0x83,0xC0,0x1C,0x00,0x1F,0xFF,0x80,0x07,0xC4,0x00,
    0x00,0x01,0xFF,0xF8,0x00,0x70,0x00,0x00,0x01,0x9E,0x3F,0xFF,0xC0,0x00,0xF3,0x7F,
    0xFF,0xFD,0xFC,0x00,0x00,0x00,0xF7,0xE0,0x70,0x00,0x1F,0xFF,0x00,0x03,0x8C,0x00,
    0x00,0x00,0x1F,0xE0,0x00,0x60,0x00,0x00,0x00,0xDC,0x00,0x77,0xC0,0x00,0xE2,0xFF,
    0xFF,0xFF,0xE0,0x00,0x00,0x00,0x7F,0xE3,0xC0,0x00,0x1F,0xFE,0x00,0x00,0x18,0x00,
    0x00,0x00,0x0F,0x80,0x00,0x20,0x00,0x00,0x00,0xD8,0x00,0x63,0xC0,0x01,0xF6,0xFF,
    0xFF,0xFF,0xE0,0x00,0x00,0x00,0xFF,0xF2,0xC0,0x00,0x1F,0xFE,0x06,0x00,0xF0,0x00,
    0x00,0x00,0x1F,0x80,0x00,0x20,0x00,0x00,0x00,0x70,0x00,0x61,0xC0,0x01,0xDD,0xFF,
    0xFF,0xFF,0xE0,0x00,0x00,0x01,0x5F,0xC0,0x40,0x00,0x3F,0xFC,0x03,0xFF,0x80,0x00,
    0x00,0x00,0x3F,0x00,0x00,0x30,0x00,0x00,0x00,0x70,0x00,0x60,0xC0,0x07,0xE3,0xCF,
    0xFF,0xFF,0xE0,0x00,0x00,0x03,0xDF,0xC0,0x40,0x00,0x3F,0xD0,0x03,0x80,0x00,0x00,
    0x00,0x00,0x3F,0x00,0x00,0x30,0x00,0x00,0x00,0x20,0x00,0x70,0x40,0x07,0xFF,0x93,
    0xFF,0xFF,0xE0,0x00,0x00,0x03,0xBF,0x80,0x60,0x00,0x3B,0x80,0x03,0x80,0x00,0x00,
    0x00,0x00,0x7F,0x00,0x00,0x30,0x00,0x00,0x00,0x60,0x00,0x18,0x60,0x07,0xFF,0x03,
    0xFF,0xFF,0xE0,0x00,0x00,0x03,0xFF,0x80,0x30,0x00,0x2B,0x00,0x07,0x80,0x00,0x00,
    0x00,0x00,0x7F,0x80,0x00,0x10,0x00,0x00,0x00,0x60,0x00,0x08,0x30,0x01,0xFF,0x73,
    0xFF,0xFF,0xC0,0x00,0x00,0x03,0xFF,0x00,0x3E,0x00,0x36,0x00,0x07,0xC0,0x00,0x00,
    0x00,0x00,0x7F,0x80,0x00,0x10,0x00,0x00,0x00,0x60,0x00,0x0C,0x30,0x00,0xFF,0x1B,
    0xFF,0xFF,0xC0,0x00,0x00,0x07,0xFF,0x00,0x3F,0x00,0x1C,0x00,0x07,0xE0,0x00,0x00,
    0x00,0x00,0x7F,0x00,0x00,0x10,0x00,0x00,0x00,0x40,0x00,0x06,0x18,0x00,0x7F,0x8B,
    0xF3,0xFF,0xC0,0x00,0x00,0x07,0xFE,0x00,0x3F,0x00,0x1C,0x00,0x07,0xF0,0x00,0x00,
    0x00,0x00,0x3F,0x00,0x00,0x10,0x00,0x00,0x00,0x40,0x00,0x04,0x10,0x00,0x1F,0xC7,
    0xF1,0xFF,0xC0,0x00,0x00,0x07,0xFE,0x00,0x3F,0x00,0x2C,0x00,0x0F,0xF8,0x00,0x00,
    0x00,0x00,0x3F,0x00,0x00,0x10,0x00,0x00,0x00,0x40,0x00,0x04,0x10,0x00,0x03,0xFF,
    0xE0,0xFF,0x80,0x00,0x00,0x07,0xFC,0x00,0x3F,0x00,0xC8,0x00,0x0F,0xFC,0x00,0x00,
    0x00,0x00,0x1F,0x00,0x00,0x18,0x00,0x00,0x00,0x40,0x00,0x07,0x30,0x00,0x00,0xFF,
    0xE0,0x7F,0x80,0x00,0x00,0x07,0xFC,0x00,0x3E,0x00,0xC8,0x00,0x0B,0xF8,0x00,0x00,
    0x00,0x00,0x0E,0x00,0x00,0x18,0x00,0x00,0x00,0x40,0x00,0x03,0xE0,0x00,0x00,0xFF,
    0xC0,0x1F,0x80,0x00,0x00,0x07,0xFC,0x00,0x3C,0x00,0xC8,0x00,0x1B,0xF8,0x00,0x00,
    0x00,0x00,0x04,0x00,0x00,0x18,0x00,0x00,0x00,0x40,0x00,0x03,0xC0,0x00,0x00,0xC7,
    0x80,0x0F,0x00,0x00,0x00,0x07,0xF8,0x00,0x20,0x00,0xC8,0x00,0x1B,0xF8,0x00,0x00,
    0x00,0x00,0x0C,0x00,0x00,0x18,0x00,0x00,0x00,0x40,0x00,0x03,0x80,0x00,0x00,0x40,
    0x00,0x08,0x00,0x00,0x00,0x03,0xF8,0x00,0x20,0x00,0xCC,0x00,0x13,0xF0,0x00,0x00,
    0x00,0x00,0x0C,0x00,0x00,0x18,0x00,0x00,0x00,0x40,0x00,0x03,0x00,0x00,0x00,0x40,
    0x00,0x18,0x00,0x00,0x00,0x01,0xC0,0x00,0x60,0x00,0xCC,0x01,0x30,0x00,0x00,0x00,
    0x00,0x00,0x0C,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x03,0x60,0x00,
    0x30,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0x60,0x00,0xE4,0x01,0x60,0x00,0x00,0x00,
    0x00,0x00,0x0C,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x03,0x00,0x00,
    0x30,0x00,0x00,0x00,0x00,0x00,0xB0,0x00,0x60,0x00,0x3C,0x00,0xC0,0x00,0x00,0x00,
    0x00,0x00,0x08,0x00,0x00,0x10,0x64,0xD9,0x36,0x4C,0x3E,0x67,0x8C,0x1F,0x67,0x9F,
    0x33,0xB1,0x87,0x1E,0x7F,0xC1,0x90,0x00,0x30,0x00,0x06,0x00,0x80,0x00,0x00,0x00,
    0x00,0x00,0x08,0x06,0x00,0x10,0x6E,0xDB,0xB6,0xEC,0x33,0x66,0xD8,0x33,0x6C,0xD9,
    0xB4,0xDB,0x0D,0xB3,0x66,0x61,0x18,0x00,0x10,0x00,0x02,0x00,0x80,0x00,0x00,0x01,
    0x80,0x00,0x08,0x03,0xE0,0x30,0x6E,0xDB,0xB6,0xEC,0x33,0x66,0xD8,0x33,0x6F,0x19,
    0xB3,0xDB,0x0C,0x33,0x66,0x61,0x0C,0x00,0x10,0x00,0x02,0x00,0x80,0x00,0x00,0x01,
    0x80,0x00,0x08,0x02,0x40,0x20,0x3B,0x8E,0xE3,0xB8,0x33,0x66,0xDB,0xB3,0x63,0xD9,
    0xB6,0xDB,0x0C,0x33,0x66,0x61,0x87,0x00,0x10,0x00,0x02,0x01,0x80,0x00,0x00,0x01,
    0x80,0x00,0x0C,0x06,0x40,0x20,0x3B,0x8E,0xE3,0xB9,0xB3,0x66,0x70,0x33,0x6C,0xD9,
    0xB6,0xCE,0x6D,0xB3,0x66,0x60,0xC3,0xE0,0x20,0x00,0x03,0x03,0x00,0x00,0x00,0x01,
    0x80,0x00,0x06,0x04,0x60,0xF8,0x31,0x8C,0x63,0x19,0xBE,0x3E,0x70,0x1F,0x67,0x9F,
    0x33,0xCE,0x67,0x1E,0x66,0x60,0x61,0xE0,0x60,0x00,0x03,0x8E,0x00,0x00,0x00,0x01,
    0x80,0x00,0x1F,0xFF,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x18,
    0x00,0x0C,0x00,0x00,0x00,0x01,0x3F,0x31,0xC0,0x00,0x01,0xFC,0x00,0x00,0x00,0x01,
    0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xC0,0x00,0x00,0x18,
    0x00,0x38,0x00,0x00,0x00,0x00,0x1E,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,
};

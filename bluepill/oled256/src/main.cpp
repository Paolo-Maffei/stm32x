// 256x64 OLED tryout, using i8080 8-bit parallel access mode
// see https://www.buydisplay.com/default/blue-2-8-inch-oled-display-module-256x64-graphic-breakout-board

#include <jee.h>
#include "font8x16.h"
#include "images.h"

UartBufDev< PinA<9>, PinA<10> > console;

int printf (const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinB<0> RS;
PinB<1> CS1;
PinB<10> RW_WR;
PinB<11> E_RD;
PinC<13> RST;
Port<'A'> DATA_BUS;

uint8_t contrast = 0x80;

void oledInit ();
void oledClear ();
void oledContrast (uint8_t mod);
void drawPattern (uint8_t value1,uint8_t value2);
void drawImage (const uint8_t pic[]);
void drawChar (uint16_t x, uint16_t y, uint8_t ascii);
void drawString (uint16_t x, uint16_t y, const char *pStr);
void greyTest ();
void bw2grey (uint8_t temp);

static void wrData (uint8_t dat) {
    RS=1;
    CS1=0;
    RW_WR=0;
    MMIO8(DATA_BUS.odr) = dat;
    RW_WR=1;
    CS1=1;
}

static void wrCmd (uint8_t cmd) {
    RS=0;
    CS1=0;
    RW_WR=0;
    MMIO8(DATA_BUS.odr) = cmd;
    RW_WR=1;
    CS1=1;
}

static const uint8_t gMap [] = { 0x00, 0x0F, 0xF0, 0xFF };

//turns 1 byte B/W data to 4 byte grey data  
void bw2grey (uint8_t temp) {
    for (int i = 0; i < 4; ++i) {
        wrData(gMap[temp>>6]);
        temp <<= 2;
    }
}

static void gotoXY (uint8_t x, uint8_t y) {
    wrCmd(0x75);/*SET SECOND PRE-CHARGE PERIOD*/ 
    wrData(y&0x3F);
    wrData(0x3F);
    wrCmd(0x15); /*SET SECOND PRE-CHARGE PERIOD*/  
    wrData(0x1C + (x&0x3F)); 
    wrData(0x5B);
    wrCmd(0x5c);
}

void oledContrast (uint8_t mod) {
    wrCmd(0xC1);
    wrData(mod);
}

void oledInit () {
    RST = 1;
    wait_ms(2);
    RST = 0;
    wait_ms(2);
    RST = 1;
    wait_ms(2);

    wrCmd(0xFD); //Set Command Lock
    wrCmd(0xFD); /*SET COMMAND LOCK*/ 
    wrData(0x12); /* UNLOCK */ 
    wrCmd(0xAE); /*DISPLAY OFF*/ 
    wrCmd(0xB3);/*DISPLAYDIVIDE CLOCKRADIO/OSCILLATAR FREQUANCY*/ 
    wrData(0x91); wrCmd(0xCA); /*multiplex ratio*/ 
    wrData(0x3F); /*duty = 1/64*/ 
    wrCmd(0xA2); /*set offset*/ 
    wrData(0x00);
    wrCmd(0xA1); /*start line*/ 
    wrData(0x00); 
    wrCmd(0xA0); /*set remap*/
    wrData(0x14); 
    wrData(0x11);
    /*wrCmd(0xB5); //GPIO wrCmd(0x00); */
    wrCmd(0xAB); /*funtion selection*/ 
    wrData(0x01); /* selection external vdd */ 
    wrCmd(0xB4); /* */ 
    wrData(0xA0);
    wrData(0xFD); 
    wrCmd(0xC1); /*set contrast current */ 
    wrData(contrast); 
    wrCmd(0xC7); /*master contrast current control*/ 
    wrData(0x0f); 
    /*wrCmd(0xB9); GREY TABLE*/ 
    wrCmd(0xB1); /*SET PHASE LENGTH*/
    wrData(0xE2); 
    wrCmd(0xD1); /**/
    wrData(0x82); 
    wrData(0x20); 
    wrCmd(0xBB); /*SET PRE-CHANGE VOLTAGE*/ 
    wrData(0x1F);
    wrCmd(0xB6); /*SET SECOND PRE-CHARGE PERIOD*/
    wrData(0x08); 
    wrCmd(0xBE); /* SET VCOMH */ 
    wrData(0x07); 
    wrCmd(0xA6); /*normal display*/ 
    oledClear();
    wrCmd(0xAF); /*display ON*/
}

void oledClear () {
    wrCmd(0x15); 
    wrData(0x00); 
    wrData(0x77); 
    wrCmd(0x75); 
    wrData(0x00); 
    wrData(0x7f); 
    wrCmd(0x5C); 

    for (int y=0;y<128;y++)
        for (int x=0;x<120;x++)
            wrData(0x00); 
}

void drawPattern (uint8_t value1, uint8_t value2) {
    gotoXY(0, 0);
    for (int i=0;i<32;i++) {  	
        for (int k=0;k<32;k++)
            bw2grey(value1);
        for (int k=0;k<32;k++)
            bw2grey(value2);
    }
}

void drawChar (uint16_t x, uint16_t y, uint8_t ascii) {
    uint16_t off = (ascii - 32)*16;

    for (int i=0;i<16;i++) {
        gotoXY(x, y+i);
        bw2grey (font8x16[off+i]);             
    }
}

void drawString (uint16_t x, uint16_t y, const char *pStr) {
    while (*pStr != 0) {
        drawChar(x, y, *pStr++);
        x += 2;
    }
}

void drawImage (const uint8_t pic[]) {
    gotoXY(0, 0); 		
    for (int i=0;i<64;i++)
        for (int j=0;j<32;j++)
            bw2grey(pic[i*32+j]);
}

void greyTest () {
    gotoXY(0, 0);
    for (int m=0;m<32;m++) { 
        int j=0;
        for (int k=0;k<16;k++)   {
            for (int i=0;i<8;i++)
                wrData(j);
            j+=0x11;
        }
    }

    for (int m=0;m<32;m++) { 
        int j=255;
        for (int k=0;k<16;k++)   {
            for (int i=0;i<8;i++)
                wrData(j);
            j-=0x11;
        }
    }
}

int main () {	
    console.init();
    console.baud(115200, fullSpeedClock());

    RS.mode(Pinmode::out);    RS = 1;
    RW_WR.mode(Pinmode::out); RW_WR = 1;
    E_RD.mode(Pinmode::out);  E_RD = 1;
    CS1.mode(Pinmode::out);   CS1 = 1;
    RST.mode(Pinmode::out);   RST = 1;

    DATA_BUS.modeMap(0b0000000011111111, Pinmode::out);

    while (1) {
        printf("%d\n", ticks);

        oledInit();

        wrCmd(0xA5);//--all display on
        wait_ms(500);
        wrCmd(0xA4);//--all Display off
        wait_ms(500);

        wrCmd(0xA6);//--set normal display

        uint32_t t = ticks;
        for (int i = 0; i < 100; ++i)
            drawImage(image1);
        t = ticks - t;
        printf("t %d.%02d ms\n", t/100, t%100);

        wait_ms(1000);
        wrCmd(0xA7);//--set Inverse Display	
        wait_ms(1000);
        wrCmd(0xA6);//--set normal display
        drawImage(image2);
        wait_ms(1000);
        wrCmd(0xA7);//--set Inverse Display	
        wait_ms(1000);

        wrCmd(0xA6);//--set normal display

        drawPattern(0x00,0x00); //clear display
        drawString(0, 0, "**** 01234567 ****");
        drawString(0, 16, "EASTRISING ");
        drawString(0, 32, "WWW.BUY-DISPLAY.COM");
        drawString(0, 48, "2013.04.22");
        wait_ms(1000);

        greyTest(); 
        wait_ms(1000);

        drawPattern(0x55,0xAA);
        wait_ms(500);
        drawPattern(0xAA,0x55);
        wait_ms(500);
        drawPattern(0x55,0x55);
        wait_ms(500);
        drawPattern(0xAA,0xAA);
        wait_ms(500);
        drawPattern(0xFF,0x00);
        wait_ms(500);
        drawPattern(0x00,0xFF);
        wait_ms(500);

        drawPattern(0x00,0x00); //clear display	  
        wait_ms(1000);
    }
}

// 256x64 OLED tryout, using i8080 8-bit parallel access mode
// see https://www.buydisplay.com/default/blue-2-8-inch-oled-display-module-256x64-graphic-breakout-board

#include <jee.h>
#include "font8x16.h"
#include "images.h"

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinB<0> RS;
PinB<1> CS1;
PinB<10> RW_WR;
PinB<11> E_RD;
PinC<13> RST;
Port<'A'> DATA_BUS;

uint8_t Contrast_level=0x80;

void Initial ();
void Clear_ram ();
void Set_Contrast_Control_Register (uint8_t mod);
void Display_Chess (uint8_t value1,uint8_t value2);
void Display_Picture (const uint8_t pic[]);
void DrawSingleAscii (uint16_t x, uint16_t y, uint8_t ascii);
void DrawString (uint16_t x, uint16_t y, const char *pStr);
void Gray_test (void);
void Data_processing (uint8_t temp);

static void Write_Data(uint8_t dat) {
    RS=1;
    CS1=0;
    RW_WR=0;
    MMIO8(DATA_BUS.odr) = dat;
    RW_WR=1;
    CS1=1;
}

static void Write_Instruction(uint8_t cmd) {
    RS=0;
    CS1=0;
    RW_WR=0;
    MMIO8(DATA_BUS.odr) = cmd;
    RW_WR=1;
    CS1=1;
}

static const uint8_t gMap [] = { 0x00, 0x0F, 0xF0, 0xFF };

//turns 1 byte B/W data to 4 byte gray data  
void Data_processing(uint8_t temp) {
    for (int i = 0; i < 4; ++i) {
        Write_Data(gMap[temp>>6]);
        temp <<= 2;
    }
}

// Set row address 0~32
static void Set_Row_Address(uint8_t add) {
    Write_Instruction(0x75);/*SET SECOND PRE-CHARGE PERIOD*/ 
    add=0x3F&add;
    Write_Data(add);
    Write_Data(0x3F);
}

// Set row address 0~64  for Gray mode)
static void Set_Column_Address(uint8_t add) {
    add=0x3F&add;
    Write_Instruction(0x15); /*SET SECOND PRE-CHARGE PERIOD*/  
    Write_Data(0x1C+add); 
    Write_Data(0x5B);
}

// Set Contrast
void Set_Contrast_Control_Register(uint8_t mod) {
    Write_Instruction(0xC1);
    Write_Data(mod);
}

void Initial() {
    RST = 1;
    wait_ms(2);
    RST = 0;
    wait_ms(2);
    RST = 1;
    wait_ms(2);

    Write_Instruction(0xFD); //Set Command Lock
    Write_Instruction(0xFD); /*SET COMMAND LOCK*/ 
    Write_Data(0x12); /* UNLOCK */ 
    Write_Instruction(0xAE); /*DISPLAY OFF*/ 
    Write_Instruction(0xB3);/*DISPLAYDIVIDE CLOCKRADIO/OSCILLATAR FREQUANCY*/ 
    Write_Data(0x91); Write_Instruction(0xCA); /*multiplex ratio*/ 
    Write_Data(0x3F); /*duty = 1/64*/ 
    Write_Instruction(0xA2); /*set offset*/ 
    Write_Data(0x00);
    Write_Instruction(0xA1); /*start line*/ 
    Write_Data(0x00); 
    Write_Instruction(0xA0); /*set remap*/
    Write_Data(0x14); 
    Write_Data(0x11);
    /*Write_Instruction(0xB5); //GPIO Write_Instruction(0x00); */
    Write_Instruction(0xAB); /*funtion selection*/ 
    Write_Data(0x01); /* selection external vdd */ 
    Write_Instruction(0xB4); /* */ 
    Write_Data(0xA0);
    Write_Data(0xFD); 
    Write_Instruction(0xC1); /*set contrast current */ 
    Write_Data(Contrast_level); 
    Write_Instruction(0xC7); /*master contrast current control*/ 
    Write_Data(0x0f); 
    /*Write_Instruction(0xB9); GRAY TABLE*/ 
    Write_Instruction(0xB1); /*SET PHASE LENGTH*/
    Write_Data(0xE2); 
    Write_Instruction(0xD1); /**/
    Write_Data(0x82); 
    Write_Data(0x20); 
    Write_Instruction(0xBB); /*SET PRE-CHANGE VOLTAGE*/ 
    Write_Data(0x1F);
    Write_Instruction(0xB6); /*SET SECOND PRE-CHARGE PERIOD*/
    Write_Data(0x08); 
    Write_Instruction(0xBE); /* SET VCOMH */ 
    Write_Data(0x07); 
    Write_Instruction(0xA6); /*normal display*/ 
    Clear_ram();
    Write_Instruction(0xAF); /*display ON*/
}

void Clear_ram() {
    Write_Instruction(0x15); 
    Write_Data(0x00); 
    Write_Data(0x77); 
    Write_Instruction(0x75); 
    Write_Data(0x00); 
    Write_Data(0x7f); 
    Write_Instruction(0x5C); 

    for (int y=0;y<128;y++)
        for (int x=0;x<120;x++)
            Write_Data(0x00); 
}

void Display_Chess (uint8_t value1, uint8_t value2) {
    Set_Row_Address(0);
    Set_Column_Address(0);		
    Write_Instruction(0x5c);
    for (int i=0;i<32;i++) {  	
        for (int k=0;k<32;k++)
            Data_processing(value1);
        for (int k=0;k<32;k++)
            Data_processing(value2);
    }
}

void DrawSingleAscii(uint16_t x, uint16_t y, uint8_t ascii) {
    uint8_t str;
    uint16_t OffSet;

    OffSet = (ascii - 32)*16;

    for (int i=0;i<16;i++) {
        Set_Row_Address(y+i);
        Set_Column_Address(x);
        Write_Instruction(0x5c);
        str = *(font8x16 + OffSet + i);  
        Data_processing (str);             
    }
}

void DrawString(uint16_t x, uint16_t y, const char *pStr) {
    while (*pStr != 0) {
        DrawSingleAscii(x, y, *pStr++);
        x += 2;
    }
}

void Display_Picture(const uint8_t pic[]) {
    Set_Row_Address(0); 		
    Set_Column_Address(0);
    Write_Instruction(0x5c);

    for (int i=0;i<64;i++)
        for (int j=0;j<32;j++)
            Data_processing(pic[i*32+j]);
}

void Gray_test(void) {
    Set_Row_Address(0);
    Set_Column_Address(0);
    Write_Instruction(0x5c);

    int j=0;
    for (int m=0;m<32;m++) { 
        for (int k=0;k<16;k++)   {
            for (int i=0;i<8;i++)
                Write_Data(j);
            j+=0x11;
        }
        j=0;
    }

    j=255;
    for (int m=0;m<32;m++) { 
        for (int k=0;k<16;k++)   {
            for (int i=0;i<8;i++)
                Write_Data(j);
            j-=0x11;
        }
        j=255;
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

        Initial();

        Write_Instruction(0xA5);//--all display on
        wait_ms(500);
        Write_Instruction(0xA4);//--all Display off
        wait_ms(500);

        Write_Instruction(0xA6);//--set normal display

        uint32_t t = ticks;
        for (int i = 0; i < 100; ++i)
            Display_Picture(image1);
        t = ticks - t;
        printf("t %d.%02d ms\n", t/100, t%100);

        wait_ms(1000);
        Write_Instruction(0xA7);//--set Inverse Display	
        wait_ms(1000);
        Write_Instruction(0xA6);//--set normal display
        Display_Picture(image2);
        wait_ms(1000);
        Write_Instruction(0xA7);//--set Inverse Display	
        wait_ms(1000);

        Write_Instruction(0xA6);//--set normal display

        Display_Chess(0x00,0x00); //clear display
        DrawString(0, 0, "**** 01234567 ****");
        DrawString(0, 16, "EASTRISING ");
        DrawString(0, 32, "WWW.BUY-DISPLAY.COM");
        DrawString(0, 48, "2013.04.22");
        wait_ms(1000);

        Gray_test(); 
        wait_ms(1000);

        Display_Chess(0x55,0xAA);
        wait_ms(500);
        Display_Chess(0xAA,0x55);
        wait_ms(500);
        Display_Chess(0x55,0x55);
        wait_ms(500);
        Display_Chess(0xAA,0xAA);
        wait_ms(500);
        Display_Chess(0xFF,0x00);
        wait_ms(500);
        Display_Chess(0x00,0xFF);
        wait_ms(500);

        Display_Chess(0x00,0x00); //clear display	  
        wait_ms(1000);
    }
}

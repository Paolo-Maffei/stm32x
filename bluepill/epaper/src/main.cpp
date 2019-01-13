#include <jee.h>

PinC<13> led;

PinA<0> din;
PinA<1> clk;
PinA<2> cs;
PinA<3> dc;
PinA<4> rst;
PinA<5> busy;

SpiGpio< decltype(din), NoPin, decltype(clk), decltype(cs) > spi;

struct Epaper {
    static constexpr int width = 200;
    static constexpr int height = 200;

    struct {
        void operator= (int b) {
            dc = 0;
            spi.enable();
            spi.transfer(b);
            spi.disable();
        }
    } cmd;

    struct {
        void operator= (int b) {
            dc = 1;
            spi.enable();
            spi.transfer(b);
            spi.disable();
        }
    } data;

    void reset () {
        rst = 1;
        wait_ms(200);
        rst = 0;
        wait_ms(200);
        rst = 1;
        wait_ms(200);
    }

    void init (bool partial =false) {
        dc.mode(Pinmode::out);
        rst.mode(Pinmode::out); rst = 1;
        busy.mode(Pinmode::in_pulldown);

        reset();

        cmd = 0x01; // DRIVER_OUTPUT_CONTROL
        data = height-1; // height lo
        data = (height-1)>>8; // height hi
        data = 0; // GD = 0; SM = 0; TB = 0;
        cmd = 0x0C; // BOOSTER_SOFT_START_CONTROL
        data = 0xD7;
        data = 0xD6;
        data = 0x9D;
        cmd = 0x2C; // WRITE_VCOM_REGISTER
        data = 0xA8; // VCOM 7C
        cmd = 0x3A; // SET_DUMMY_LINE_PERIOD
        data = 0x1A; // 4 dummy lines per gate
        cmd = 0x3B; // SET_GATE_TIME
        data = 0x08; // 2us per line
        cmd = 0x11; // DATA_ENTRY_MODE_SETTING
        data = 0x03;

        static const uint8_t lutFull [] = {
            0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
            0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
            0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
            0x35, 0x51, 0x51, 0x19, 0x01, 0x00
        };

        static const uint8_t lutPartial [] = {
            0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        cmd = 0x32; // WRITE_LUT_REGISTER
        for (int i = 0; i < sizeof lutPartial; ++i)
            data = (partial ? lutPartial : lutFull)[i];
    }

    void waitIdle () {
        while (busy) {}
    }

    void turnOn () {
        cmd = 0x22; // DISPLAY_UPDATE_CONTROL_2
        data = 0xC4;
        cmd = 0x20; // MASTER_ACTIVATION
        cmd = 0xFF; // TERMINATE_FRAME_READ_WRITE
        waitIdle();
    }

    void cursor (int x, int y) {
        cmd = 0x4E; // SET_RAM_X_ADDRESS_COUNTER
        data = x>>3;

        cmd = 0x4F; // SET_RAM_Y_ADDRESS_COUNTER
        data = y;
        data = y>>8;
    }

    void window (int xs, int ys, int xe, int ye) {
        cmd = 0x44; // SET_RAM_X_ADDRESS_START_END_POSITION
        data = xs>>3;
        data = xe>>3;

        cmd = 0x45; // SET_RAM_Y_ADDRESS_START_END_POSITION
        data = ys;
        data = ys>>8;
        data = ye;
        data = ye>>8;
    }

    void display (const uint8_t* buf, int inc) {
        window(0, 0, width, height);

        int w = (width+7) >> 3;
        for (int j = 0; j < height; ++j) {
            cursor(0, j);
            cmd = 0x24; // WRITE_RAM
            for (int i = 0; i < w; ++i) {
                data = *buf;
                buf += inc;
            }
        }
        turnOn();
    }

    void sleep () {
        cmd = 0x10; // DEEP_SLEEP_MODE
        data = 0x01;
    }
} epaper;

int main() {
    enableSysTick();
    led.mode(Pinmode::out);

    spi.init();
    epaper.init();

    epaper.display((const uint8_t*) "\xFF", 0);

    wait_ms(3000);
    epaper.display((const uint8_t*) "\x7F", 0);

    //wait_ms(3000);
    //epaper.display((const uint8_t*) "\x00", 0);

    epaper.sleep();

    while (true) {
        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(400);
    }
}

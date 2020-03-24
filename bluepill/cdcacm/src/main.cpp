#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

// avoid including the libopencm3 headers, there are some comflicts with JeeH
extern "C" {
typedef struct _usbd_device usbd_device;
extern uint16_t usbd_ep_write_packet(usbd_device*,uint8_t,const void*,uint16_t);
extern uint16_t usbd_ep_read_packet(usbd_device*,uint8_t,void*,uint16_t);
extern void usbd_poll(usbd_device *usbd_dev);

usbd_device* setupUsb (); // in src/usb.c
void cdcacm_data_rx_cb (usbd_device*, uint8_t);
}

struct XUsbDev {
    RingBuffer<100> recv;
    uint8_t xmitBuf [64];
    uint8_t xmitFill = 0;
    usbd_device* dev = 0;

    void init () {
        dev = setupUsb();
    }

    bool writable () {
        poll();
        return xmitFill < sizeof xmitBuf;
    }

    void putc (int c) {
        while (!writable()) {}
        xmitBuf[xmitFill++] = c;
    }

    bool readable () {
        poll();
        return recv.avail() > 0;
    }

    int getc () {
        while (!readable()) {}
        return recv.get();
    }

    void poll () {
        usbd_poll(dev);

	// if there's data to send, try doing so
	if (xmitFill > 0) {
            int n = usbd_ep_write_packet(dev, 0x82, xmitBuf, xmitFill);
            if (n > 0) // TODO make sure n == xmitFill!
                xmitFill = 0;
	}
    }

    void callback () {
        // there is room if less than 30 of 100 slots are used
        if (recv.avail() < 30) {
            uint8_t tmpBuf [64];
            int n = usbd_ep_read_packet(dev, 0x01, tmpBuf, sizeof tmpBuf);
                for (int i = 0; i < n; ++i)
                    recv.put(tmpBuf[i]);
        }
    }
};

XUsbDev usb;

void cdcacm_data_rx_cb (usbd_device* dev, uint8_t) {
    usb.callback();
}

int main () {
    // avoid "unused" warnings
    (void) enableClkAt8MHz;
    (void) powerDown;

    console.init();
    console.baud(115200, fullSpeedClock());

    // pulse PA12 low to force re-enumeration
    PinA<12> usbPin;
    usbPin.mode(Pinmode::out);
    wait_ms(2);
    usbPin.mode(Pinmode::in_float);

    usb.init();
    while (true) {
        if (console.readable() && usb.writable())
            usb.putc(console.getc());
        if (usb.readable() && console.writable())
            console.putc(usb.getc());
    }
}

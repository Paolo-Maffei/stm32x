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

void cdcacm_data_rx_cb (usbd_device* dev, uint8_t) {
    char buf[64];
    int len = usbd_ep_read_packet(dev, 0x01, buf, sizeof buf);

    // this approach is flawed: data is lost when it can't all be sent at once
    if (len)
        usbd_ep_write_packet(dev, 0x82, buf, len);
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

    usbd_device* dev = setupUsb();
    while (1)
        usbd_poll(dev);
}

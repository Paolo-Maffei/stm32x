#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>

extern "C" usbd_device* setupUsb ();

extern "C" void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t) {
    char buf[64];
    int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, sizeof buf);

    // this approach is flawed: data is lost when it can't all be sent at once
    if (len)
        usbd_ep_write_packet(usbd_dev, 0x82, buf, len);
}

int main (void) {
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    // pulse PA12 low to force re-enumeration
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
    for (int i = 0; i < 10000000; i++) asm ("");
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, GPIO12);

    usbd_device* usbd_dev = setupUsb();

    while (1)
        usbd_poll(usbd_dev);
}

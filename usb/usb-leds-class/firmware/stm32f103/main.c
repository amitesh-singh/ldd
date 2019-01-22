/*
 * Copyright (C) 2017  Amitesh Singh <singh.amitesh@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>

#include <stdio.h>
#include <stdlib.h>
#include <libopencm3/usb/usbd.h>

#define LED_ON 1
#define LED_OFF 0

static const char *usb_strings[] = {
     "ami",
     "usbLed",
     "POOJA",
};

static usbd_device *usbd_dev;

const struct usb_device_descriptor dev_descr = {
     //The size of this descriptor in bytes.
     .bLength = USB_DT_DEVICE_SIZE,
     // A value of 1 indicates that this is a device descriptor
     .bDescriptorType = USB_DT_DEVICE,
     // This device supports USB 2.0
     .bcdUSB = 0x0200,
     // When cereating a multi-function device with more than one interface per
     // logical function (as we are doing with the CDC interfaces below to create
     // a virtual serial device) one  must use Interface Association Descriptors
     // and the next three values must be set to the exact values specified. The
     // values have assigned meanings, which are mentioned in the comments, but
     // since they must be used when using IADs that makes their given
     // definitions meaningless. See
     // http://www.usb.org/developers/docs/InterfaceAssociationDescriptor_ecn.pdf
     // and http://www.usb.org/developers/whitepapers/iadclasscode_r10.pdf
     .bDeviceClass = 0xFF, // custom class 
     // common class
     .bDeviceSubClass = 0,
     // interface Association
     .bDeviceProtocol = 0,
     // Packet size for endpoint zero in bytes.
     .bMaxPacketSize0 = 64,
     // The id of the vendor (VID) who makes this device. This must be a VID
     // assigned by the USB-IF. The VID/PID combo must be unique to a product.
     // For now, we will use a VID reserved for prototypes and an arbitrary PID.
     .idVendor =  0x16C0,
     // Product ID within the Vendor ID space. The current PID is arbitrary since
     // we're using the prototype VID.

     .idProduct = 0x03e8,
     // Version number for the device. Set to 1.0.0 for now.
     .bcdDevice = 0x0200,
     // The index of the string in the string table that represents the name of
     // the manufacturer of this device.
     .iManufacturer = 1,
     // The index of the string in the string table that represents the name of
     // the product.
     .iProduct = 2,
     // The index of the string in the string table that represents the serial
     // number of this item in string form. Zero means there isn't one.
     .iSerialNumber = 3,
     // The number of possible configurations this device has. This is one for
     // most devices.
     .bNumConfigurations = 1,
};

const struct usb_endpoint_descriptor endpoint = {
     // The size of the endpoint descriptor in bytes: 7.
     .bLength = USB_DT_ENDPOINT_SIZE,
     // A value of 5 indicates that this describes an endpoint.
     .bDescriptorType = USB_DT_ENDPOINT,
     // Bit 7 indicates direction: 0 for OUT (to device) 1 for IN (to host).
     // Bits 6-4 must be set to 0.
     // Bits 3-0 indicate the endpoint number (zero is not allowed).
     // Here we define the IN side of endpoint 1.
     .bEndpointAddress = 0x81,
     // Bit 7-2 are only used in Isochronous mode, otherwise they should be
     // 0.
     // Bit 1-0: Indicates the mode of this endpoint.
     // 00: Control
     // 01: Isochronous
     // 10: Bulk
     // 11: Interrupt
     // Here we're using interrupt.
     .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
     // Maximum packet size.
     .wMaxPacketSize = 64,
     // The frequency, in number of frames, that we're going to be sending
     // data. Here we're saying we're going to send data every millisecond.
     .bInterval = 0x20,
};

const struct usb_interface_descriptor iface = {
     // The size of an interface descriptor: 9
     .bLength = USB_DT_INTERFACE_SIZE,
     // A value of 4 specifies that this describes and interface.
     .bDescriptorType = USB_DT_INTERFACE,
     // The number for this interface. Starts counting from 0.
     .bInterfaceNumber = 0,
     // The number for this alternate setting for this interface.
     .bAlternateSetting = 0,
     // The number of endpoints in this interface.
     .bNumEndpoints = 1,
     // The interface class for this interface is DATA, indicated by 10.
     .bInterfaceClass = 0,
     // There are no subclasses defined for the data class so it must be zero.
     .bInterfaceSubClass = 0, /* boot */
     // We are not using any class specific protocols for data so this is set to
     // zero
     .bInterfaceProtocol = 0, /* keyboard */
     // A string representing this interface. Zero means not provided.
     .iInterface = 0,
     // A pointer to the array of endpoints in this interface.
     .endpoint = &endpoint,

     .extra = NULL,
     .extralen = 0,
};

const struct usb_interface ifaces[] = {{
     .num_altsetting = 1,
        .altsetting = &iface,
}};

const struct usb_config_descriptor config = {
     // The length of this header in bytes, 9.
     .bLength = USB_DT_CONFIGURATION_SIZE,
     // A value of 2 indicates that this is a configuration descriptor.
     .bDescriptorType = USB_DT_CONFIGURATION,
     // This should hold the total size of the configuration descriptor including
     // all sub interfaces. This is automatically filled in by the usb stack in
     // libopencm3.
     .wTotalLength = 0,
     // The number of interfaces in this configuration.
     .bNumInterfaces = 1,
     // The index of this configuration. Starts counting from 1.
     .bConfigurationValue = 1,
     // A string index describing this configration. Zero means not provided.
     .iConfiguration = 0,
     // Bit flags:
     // 7: Must be set to 1.
     // 6: This device is self powered.
     // 5: This device supports remote wakeup.
     // 4-0: Must be set to 0.
     // TODO: Add remote wakeup.
     .bmAttributes = 0x80,  // 0b1010000
     // The maximum amount of current that this device will draw in 2mA units.
     // This indicates 100mA.
     .bMaxPower = 0x32,

     // A pointer to an array of interfaces.
     .interface = ifaces,
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

static enum usbd_request_return_codes control_request(usbd_device *dev, struct usb_setup_data *req,
                           uint8_t **buf, uint16_t *len,
                           void (**complete)(usbd_device *,
                                             struct usb_setup_data *))
{
   (void)complete;
   (void)dev;

   if ((req->bmRequestType & 0x7F) != USB_REQ_TYPE_VENDOR)
     return 0;

   (*len) = 1;
   (*buf)[0] = 1; //success

   if (req->bRequest == LED_ON)
     {
        gpio_set(GPIOC, GPIO13);
     }
   else if (req->bRequest == LED_OFF)
     {
        gpio_clear(GPIOC, GPIO13);
     }
   else
     {
        (*buf)[0] = -1; // FAILURE
     }

   return 1;
}

static void set_config(usbd_device *dev, uint16_t wValue)
{
   (void)wValue;
   //(void)dev;

   //we are not going to use this 0x81, EP0 is good enough
   //usbd_ep_setup(dev, 0x81, USB_ENDPOINT_ATTR_INTERRUPT, 9, NULL);

   usbd_register_control_callback(dev, 
                                  USB_REQ_TYPE_VENDOR,//USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
                                  USB_REQ_TYPE_TYPE, /// | USB_REQ_TYPE_RECIPIENT,
                                  control_request);
}

static void my_delay_1( void )
{
   for (unsigned i = 0; i < 800000; i++)
     {
        __asm__("nop");
     }
}

int main( void )
{
   //set STM32 to 72 MHz
   rcc_clock_setup_in_hse_8mhz_out_72mhz();
   rcc_periph_clock_enable(RCC_GPIOC);
   gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
                 GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
   /*
    * This is a somewhat common cheap hack to trigger device re-enumeration
    * on startup.  Assuming a fixed external pullup on D+, (For USB-FS)
    * setting the pin to output, and driving it explicitly low effectively
    * "removes" the pullup.  The subsequent USB init will "take over" the
    * pin, and it will appear as a proper pullup to the host.
    * The magic delay is somewhat arbitrary, no guarantees on USBIF
    * compliance here, but "it works" in most places.
    */
   rcc_periph_clock_enable(RCC_GPIOA);
   gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
                 GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
   gpio_clear(GPIOA, GPIO12);
   my_delay_1();

   // USB initialization
   usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &config,
                        usb_strings, 3, usbd_control_buffer,
                        sizeof(usbd_control_buffer));
   usbd_register_set_config_callback(usbd_dev, set_config);

   //This is alternative to calling usbd_poll in mainloop. quite handy sometimes
   nvic_enable_irq(NVIC_USB_LP_CAN_RX0_IRQ);
   nvic_enable_irq(NVIC_USB_WAKEUP_IRQ);

   while( 1 )
     {
        // do nothing
     }
}

__attribute__((used)) void usb_wakeup_isr(void)
{
   usbd_poll(usbd_dev);
}

__attribute__((used)) void usb_lp_can_rx0_isr(void)
{
   usbd_poll(usbd_dev);
}

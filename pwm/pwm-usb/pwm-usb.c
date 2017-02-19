//pwm over usb skeleton example

#include <linux/init.h>// macros used to markup functions e.g. __init, __exit
#include <linux/module.h>// Core header for loading LKMs into the kernel
#include <linux/kernel.h>// Contains types, macros, functions for the kernel
#include <linux/device.h>// Header to support the kernel Driver Model

#include <linux/usb.h> //for usb stuffs
#include <linux/slab.h> //for

#include <linux/workqueue.h> //for work_struct
#include <linux/pwm.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh <singh.amitesh@gmail.com>");
MODULE_DESCRIPTION("led blink driver for Xboard");
MODULE_VERSION("0.1");

#define VUSB_VENDOR_ID 0x16c0
#define VUSB_PRODUCT_ID 0x03e8
static struct usb_device_id xboard_pwm_usb_table[] = {
       { USB_DEVICE(VUSB_VENDOR_ID, VUSB_PRODUCT_ID) },
       {},
};

MODULE_DEVICE_TABLE(usb, xboard_pwm_usb_table);

struct pwm_usb {
     struct usb_device *udev;
     struct usb_endpoint_descriptor *endpoint;
     struct pwm_chip chip;
};

static int xboard_pwm_config(struct pwm_chip *chip, struct pwm_device *pwm,
                             int duty_ns, int period_ns)
{
   printk(KERN_INFO "xboard_pwm_config");

   return 0;
}

static int xboard_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwm)
{
   printk(KERN_INFO "pwm: %p", pwm);
   printk(KERN_INFO "xboard_pwm_enable");
   if (pwm->label)
     printk(KERN_INFO "label: %s", pwm->label);
   return 0;
}

static void xboard_pwm_disable(struct pwm_chip *chip, struct pwm_device *pwm)
{
   printk(KERN_INFO "xboard_pwm_disable");
}

static const struct pwm_ops xboard_pwm_ops = {
     //int (*request)(struct pwm_chip *chip, struct pwm_device *pwm);
     // optional hook for requesting a PWM

     //@free: optional hook for freeing a PWM
     // void (*free)(struct pwm_chip *chip, struct pwm_device *pwm);
     //pwm_config, configure duty cycles and period length for this PWM
     .config = xboard_pwm_config,
     //pwm_enable - enable pwm output toggling
     .enable = xboard_pwm_enable,
     //pwm_disable - disable pwm output toggling
     .disable = xboard_pwm_disable,
     .owner = THIS_MODULE,
};

static int
xboard_pwm_usb_probe(struct usb_interface *interface,
                     const struct usb_device_id *id)
{
   struct usb_device *udev = interface_to_usbdev(interface);
   struct usb_host_interface *iface_desc;
   struct usb_endpoint_descriptor *endpoint;
   struct pwm_usb *data;
   int i;

   printk(KERN_INFO "manufacturer: %s", udev->manufacturer);
   printk(KERN_INFO "product: %s", udev->product);

   iface_desc = interface->cur_altsetting;
   printk(KERN_INFO "vusb led %d probed: (%04X:%04X)",
          iface_desc->desc.bInterfaceNumber, id->idVendor, id->idProduct);
   printk(KERN_INFO "bNumEndpoints: %d", iface_desc->desc.bNumEndpoints);

   data = kzalloc(sizeof(struct pwm_usb), GFP_KERNEL);
   if (data == NULL)
     {
        //handle error
     }

   for (i = 0; i < iface_desc->desc.bNumEndpoints; i++)
     {
        endpoint = &iface_desc->endpoint[i].desc;

        printk(KERN_INFO "EP[%d]->bEndpointAddress: 0x%02X\n",
               i, endpoint->bEndpointAddress);
        printk(KERN_INFO "EP[%d]->bmAttributes: 0x%02X\n",
               i, endpoint->bmAttributes);
        printk(KERN_INFO "EP[%d]->wMaxPacketSize: 0x%04X (%d)\n",
               i, endpoint->wMaxPacketSize, endpoint->wMaxPacketSize);
        printk(KERN_INFO "EP[%d]->bInterval: %d ms\n", i, endpoint->bInterval);
        data->endpoint = endpoint;
     }

   //increase ref count, make sure u call usb_put_dev() in disconnect()
   data->udev = usb_get_dev(udev);

   data->chip.dev = &udev->dev;
   data->chip.ops = &xboard_pwm_ops;
   data->chip.base = -1;
   data->chip.npwm = 1;

   int err;

   err = pwmchip_add(&data->chip);

   if (err < 0) return -EINVAL;

   usb_set_intfdata(interface, data);

   return 0;
}

static void
xboard_pwm_usb_disconnect(struct usb_interface *interface)
{
   struct pwm_usb *data;
   int err;

   data = usb_get_intfdata(interface);
   err = pwmchip_remove(&data->chip);
   if (err < 0) return;

}

static struct usb_driver xboard_pwm_usb_driver = {
     .name = "xboard-pwm-led",
     .id_table = xboard_pwm_usb_table,
     .probe = xboard_pwm_usb_probe,
     .disconnect = xboard_pwm_usb_disconnect,
};

module_usb_driver(xboard_pwm_usb_driver);

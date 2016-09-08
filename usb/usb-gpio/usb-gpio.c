// Author: (C) 2016 Amitesh Singh

#include <linux/init.h>// macros used to markup functions e.g. __init, __exit
#include <linux/module.h>// Core header for loading LKMs into the kernel
#include <linux/kernel.h>// Contains types, macros, functions for the kernel
#include <linux/device.h>// Header to support the kernel Driver Model

#include <linux/usb.h> //for usb stuffs
#include <linux/slab.h> //for kzmalloc and kfree

#include <linux/workqueue.h> //for work_struct
#include <linux/gpio.h> //for led

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("usb gpio (output only) example"); //sysfs
MODULE_VERSION("0.1"); 

//one structure for each connected device
struct my_usb {
     struct usb_device *udev;
     struct work_struct work;
     struct gpio_chip chip; //this is our GPIO chip
};

#define MY_USB_VENDOR_ID 0x16c0
#define MY_USB_PRODUCT_ID 0x03e8
static struct usb_device_id my_usb_table[] = {
       { USB_DEVICE(MY_USB_VENDOR_ID, MY_USB_PRODUCT_ID) },
       {},
};

MODULE_DEVICE_TABLE(usb, my_usb_table);

static uint8_t gpio_val = 0;

static void
_gpio_work_job(struct work_struct *work)
{
   struct my_usb *sd = container_of(work, struct my_usb, work);

   printk(KERN_ALERT "Modifying port i/o: %d", gpio_val);
   usb_control_msg(sd->udev,
                   usb_sndctrlpipe(sd->udev, 0),
                   gpio_val, USB_TYPE_VENDOR | USB_DIR_OUT,
                   0, 0,
                   NULL, 0,
                   1000);
}

//this is called when you do echo 1 > value
static void
_gpioa_set(struct gpio_chip *chip,
           unsigned offset, int value)
{
   struct my_usb *data = container_of(chip, struct my_usb,
                                      chip);
   printk(KERN_INFO "GPIO SET INFO for pin: %d", offset);

   if (offset == 0)
     {
        gpio_val = value;
        schedule_work(&data->work);
     }
}

// this is called when you read 'value'
// cat value (/sys/class/gpio/gpio{X}/value)
static int
_gpioa_get(struct gpio_chip *chip,
           unsigned offset)
{
   //struct my_usb *data = container_of(chip, struct my_usb,
   //                                  chip);
   int ret = -1;
   printk(KERN_INFO "GPIO GET INFO: %d", offset);

   if (offset == 0)
     ret = gpio_val;

   return ret;
}

//We are enabling output only pin
static int
_direction_output(struct gpio_chip *chip,
                  unsigned offset, int value)
{
   printk("Setting pin to OUTPUT");

   return 0;
}

//called when a usb device is connected to PC
static int
my_usb_probe(struct usb_interface *interface,
             const struct usb_device_id *id)
{
   struct usb_device *udev = interface_to_usbdev(interface);
   struct usb_host_interface *iface_desc;
   struct usb_endpoint_descriptor *endpoint;
   struct my_usb *data;
   int i;

   printk(KERN_INFO "manufacturer: %s", udev->manufacturer);
   printk(KERN_INFO "product: %s", udev->product);

   iface_desc = interface->cur_altsetting;
   printk(KERN_INFO "vusb led %d probed: (%04X:%04X)",
          iface_desc->desc.bInterfaceNumber, id->idVendor, id->idProduct);
   printk(KERN_INFO "bNumEndpoints: %d", iface_desc->desc.bNumEndpoints);

   for (i = 0; i < iface_desc->desc.bNumEndpoints; i++)
     {
        endpoint = &iface_desc->endpoint[i].desc;

        printk(KERN_INFO "ED[%d]->bEndpointAddress: 0x%02X\n",
               i, endpoint->bEndpointAddress);
        printk(KERN_INFO "ED[%d]->bmAttributes: 0x%02X\n",
               i, endpoint->bmAttributes);
        printk(KERN_INFO "ED[%d]->wMaxPacketSize: 0x%04X (%d)\n",
               i, endpoint->wMaxPacketSize, endpoint->wMaxPacketSize);
     }

   data = kzalloc(sizeof(struct my_usb), GFP_KERNEL);
   if (data == NULL)
     {
        //handle error
     }

   //increase ref count, make sure u call usb_put_dev() in disconnect()
   data->udev = usb_get_dev(udev);

   /// gpio_chip struct info is inside KERNEL/include/linux/gpio/driver.h
   data->chip.label = "vusb-gpio"; //name for diagnostics
   data->chip.dev = &data->udev->dev; // optional device providing the GPIOs
   data->chip.owner = THIS_MODULE; // helps prevent removal of modules exporting active GPIOs, so this is required for proper cleanup
   data->chip.base = -1; // identifies the first GPIO number handled by this chip; 
   // or, if negative during registration, requests dynamic ID allocation.
   // i was getting 435 on -1.. nice. Although, it is deprecated to provide static/fixed base value. 

   data->chip.ngpio = 1; // the number of GPIOs handled by this controller; the last GPIO
   data->chip.can_sleep = true; // 
   /*
      flag must be set iff get()/set() methods sleep, as they
    * must while accessing GPIO expander chips over I2C or SPI. This
    * implies that if the chip supports IRQs, these IRQs need to be threaded
    * as the chip access may sleep when e.g. reading out the IRQ status
    * registers.
    */
   data->chip.set = _gpioa_set;
   data->chip.get = _gpioa_get;
   //TODO  implement it later in firmware
   // data->chip.direction_input = _direction_input;
   data->chip.direction_output = _direction_output;

   if (gpiochip_add(&data->chip) < 0)
     {
        printk(KERN_ALERT "Failed to add gpio chip");
     }
   else
     {
        printk(KERN_INFO "Able to add gpiochip: %s", data->chip.label);
     }

   usb_set_intfdata(interface, data);

   printk(KERN_INFO "usb device is connected");

   INIT_WORK(&data->work, _gpio_work_job);

   //swith off the led
   usb_control_msg(data->udev,
                   usb_sndctrlpipe(data->udev, 0),
                   0, USB_TYPE_VENDOR | USB_DIR_OUT,
                   0, 0,
                   NULL, 0,
                   1000);

   return 0;
}

//called when unplugging a USB device
static void
my_usb_disconnect(struct usb_interface *interface)
{
   struct my_usb *data;

   data = usb_get_intfdata(interface);

   cancel_work_sync(&data->work);
   gpiochip_remove(&data->chip);

   usb_set_intfdata(interface, NULL);

   //deref the count
   usb_put_dev(data->udev);

   kfree(data); //deallocate, allocated by kzmalloc()

   printk(KERN_INFO "usb device is disconnected");
}

static struct usb_driver my_usb_driver = {
     .name = "my first usb driver",
     .id_table = my_usb_table,
     .probe = my_usb_probe,
     .disconnect = my_usb_disconnect,
};


//we could use module_usb_driver(my_usb_driver); instead of 
// init and exit functions
//called on module loading
static int __init
_usb_init(void)
{
   int result;
   printk(KERN_INFO "usb driver is loaded");

   result = usb_register(&my_usb_driver);
   if (result)
     {
        printk(KERN_ALERT "device registeration failed!!");
     }
   else
     {
        printk(KERN_INFO "device registered");
     }

   return result;
}

//called on module unloading
static void __exit
_usb_exit(void)
{
   printk(KERN_INFO "usb driver is unloaded");
   usb_deregister(&my_usb_driver);
}

module_init(_usb_init);
module_exit(_usb_exit);

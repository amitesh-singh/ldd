#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model

#include <linux/usb.h> //for usb stuffs
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("create a dev to provide access to usb device");
MODULE_VERSION("0.1"); // --> module version of kernel driver

#define MY_USB_VENDOR_ID 0x16c0
#define MY_USB_PRODUCT_ID 0x03e8

struct my_usb {
     struct usb_device *udev;
     struct usb_class_driver udc;
     unsigned char int_in_buf[8];
     unsigned char int_out_buf[8];
     struct file_operations fops;
};

static struct my_usb *g_sd; //global ptr to data
static struct usb_device *g_udev; //global ptr to data

static struct usb_device_id my_usb_table[] = {
       { USB_DEVICE(MY_USB_VENDOR_ID, MY_USB_PRODUCT_ID) },
       {},
};

MODULE_DEVICE_TABLE(usb, my_usb_table);


static int
_device_open(struct inode *i, struct file *f)
{
   return 0;
}

static int
_device_close(struct inode *i, struct file *f)
{
   return 0;
}

static ssize_t
_device_read(struct file *f, char __user *buf, size_t cnt, loff_t *off)
{
   int retval;
   int read_count;

   ///better to populate these values from endpoints in _probe()
   retval = usb_interrupt_msg(g_sd->udev,
                              usb_rcvintpipe(g_sd->udev, 0x81), // pipe to read from interrupt endpoint IN 1
                              g_sd->int_in_buf, //input buffer to read
                              0x08, // max packet size
                              &read_count, //how much we read
                              100); // usb timeout poll
   if (retval)
     printk(KERN_ALERT "Failed to recv interrupt data");
   if (copy_to_user(buf, g_sd->int_in_buf, read_count < cnt ? read_count : cnt))
     {
        printk(KERN_ALERT "Failed at copy to user");
     }

   printk(KERN_ALERT "read data: %s",
          g_sd->int_in_buf);

   return read_count < cnt ? read_count : cnt;
}

static ssize_t
_device_write(struct file *f, const char __user *buf, size_t cnt, loff_t *off)
{
   int retval;
   int write_count;



   if (copy_from_user(g_sd->int_out_buf, buf, cnt <= 8 ? cnt : 8))
     {
        printk(KERN_ALERT "Failed at copy from user");
     }
   //g_sd->int_out_buf[0] = buf[0];
   printk(KERN_ALERT "write value: %s",
          g_sd->int_out_buf);
   int val;

   val = simple_strtoul(buf, NULL, 10);
   /*
   retval = usb_interrupt_msg(g_udev,
                              usb_sndintpipe(g_udev, 0x02), // pipe to read from interrupt endpoint IN 1
                              val, //input buffer to read
                              4, // max packet size
                              &write_count, //how much we read
                              1000); // usb timeout poll
   if (retval)
     {
        printk(KERN_ALERT "Failed to send interrupt");
     }
                              */
   usb_control_msg(g_sd->udev,
                   usb_sndctrlpipe(g_sd->udev, 0),
                   val, USB_TYPE_VENDOR | USB_DIR_OUT,
                   0, 0,
                   NULL, 0,
                   1000);


   return write_count;
}

//called when a usb device is connected to PC
static int
my_usb_probe(struct usb_interface *interface,
             const struct usb_device_id *id)
{
   int retval;
   struct my_usb *sd;
   struct usb_device *udev = interface_to_usbdev(interface);

   printk(KERN_INFO "usb device is connected");

   sd = kzalloc(sizeof(struct my_usb), GFP_KERNEL);
   if (sd == NULL)
     {
        // handler error
     }

   g_udev = udev;

   sd->udev = usb_get_dev(udev);

   sd->fops.open = _device_open;
   sd->fops.release = _device_close;
   sd->fops.read = _device_read;
   sd->fops.write = _device_write;

   sd->udc.name = "led%d";
   sd->udc.fops = &sd->fops;

   usb_set_intfdata(interface, sd);

   g_sd = sd;

   retval = usb_register_dev(interface, &sd->udc);
   if (retval < 0)
     {
        printk(KERN_ALERT "unable to register dev");
     }
   else
     printk(KERN_INFO "minor: %d", interface->minor);

   //dev_set_drvdata(interface->usb_dev, sd); ?
   return 0;
}

//called when unplugging a USB device
static void
my_usb_disconnect(struct usb_interface *interface)
{
   struct my_usb *sd = usb_get_intfdata(interface);
   printk(KERN_INFO "usb device is disconnected");
   usb_deregister_dev(interface, &sd->udc);
   usb_put_dev(sd->udev);

   kfree(sd);
}

static struct usb_driver my_usb_driver = {
     .name = "usbfile",
     .id_table = my_usb_table,
     .probe = my_usb_probe,
     .disconnect = my_usb_disconnect,
};

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
   usb_deregister(&my_usb_driver);
   printk(KERN_INFO "usb driver is unloaded");
}

module_init(_usb_init);
module_exit(_usb_exit);

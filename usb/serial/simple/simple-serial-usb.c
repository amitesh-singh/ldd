#include <linux/kernel.h>
#include <linux/tty.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>
#include <linux/uaccess.h>
#include <linux/init.h>

static const struct usb_device_id id_table[] = {
       {USB_DEVICE(0x16c0,
                   0x03e8)},
       {},
};

MODULE_DEVICE_TABLE(usb, id_table);

//this is called when i did sudo minicom
//although on connecting the v-usb device based devices, 
// it does disconnect sometimes - it happens because of the disconnect we do initially in vusb firmware :)
static int
_serial_open(struct tty_struct *tty,
             struct usb_serial_port *port)
{
   int retval;
   printk("device open\n");
   
   //send some usb signal to avr for config? etcs?
   //
   // ..
   // ..
   
   retval = usb_serial_generic_open(tty, port);

   return retval;
}

//this is called when i closed `sudo minicom`
static void
_serial_close(struct usb_serial_port *port)
{
   usb_serial_generic_close(port);
   printk("device close\n");
}

static struct usb_serial_driver my_serial_device = {
     .driver = {
          .owner = THIS_MODULE,
          .name = "my_serial",
     },
     .id_table = id_table,
     .num_ports = 1,
     .open = _serial_open,
     .close = _serial_close
};

static struct usb_serial_driver * const serial_drivers[] = {
     &my_serial_device, NULL
};

module_usb_serial_driver(serial_drivers, id_table);

MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("simple serial driver");
MODULE_LICENSE("GPL");

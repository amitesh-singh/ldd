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
   
   /*
   retval = usb_serial_generic_open(tty, port);

   return retval;
*/
   return 0;
}

//this is called when i closed `sudo minicom`
static void
_serial_close(struct usb_serial_port *port)
{
   usb_serial_generic_close(port);
   printk("device close\n");
}

static void
_port_probe(struct usb_serial_port *port)
{
}

/*
int  (*write)(struct tty_struct *tty, struct usb_serial_port *port,
                       const unsigned char *buf, int count);
                       */

//this is called whenever you write something on tty terminal opened via minicom
static int
_serial_write(struct tty_struct *tty, struct usb_serial_port *port,
              const unsigned char *buf, int count)
{
   static uint8_t value = 0;

   value ^= 0x1;

   printk(KERN_INFO "serial write= %s",
          buf);
   //send message
   usb_control_msg(port->serial->dev,
                   usb_sndctrlpipe(port->serial->dev, 0),
                   value, USB_TYPE_VENDOR | USB_DIR_OUT,
                   0, 0,
                   NULL, 0,
                   1000);

   return count;
}

static void
_read_int_callback(struct urb *urb)
{
   printk(KERN_ALERT "got interrupt data from device");
}

static void
_read_bulk_callback(struct urb *urb)
{
   printk(KERN_ALERT "got bulk to read");
}

static void
_write_int_callback(struct urb *urb)
{
   printk (KERN_ALERT "write int ..");
}

static void
_write_bulk_callback(struct urb *urb)
{
   printk(KERN_ALERT "write bulk..");
}

static struct usb_serial_driver my_serial_device = {
     .driver = {
          .owner = THIS_MODULE,
          .name = "my_serial",
     },
     .id_table = id_table,
     .num_ports = 1,
     .open = _serial_open,
     .close = _serial_close,
     //.port_probe = _port_probe,
     .write = _serial_write,
     .read_int_callback = _read_int_callback,
     .write_int_callback = _write_int_callback,
     .read_bulk_callback = _read_bulk_callback,
     .write_bulk_callback = _write_bulk_callback,
};

static struct usb_serial_driver * const serial_drivers[] = {
     &my_serial_device, NULL
};

module_usb_serial_driver(serial_drivers, id_table);

MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("simple serial driver");
MODULE_LICENSE("GPL");

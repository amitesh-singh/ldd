#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>

#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/device.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("example on how  to read interrupt in from device at HOST side");
MODULE_VERSION("0.1"); //

//one structure for each connected device
struct my_usb {
     struct usb_device *udev;
     //our single end point (interrupt 0)
     // only 8 bytes of data can be transferred on low end device
     struct usb_endpoint_descriptor *int_out_endpoint;
     //this gonna hold the data which we send to device
     uint8_t *int_out_buf;
     struct urb *int_out_urb;

     struct usb_endpoint_descriptor *int_in_endpoint;
     uint8_t *int_in_buf;
     struct urb *int_in_urb;
};

#define MY_USB_VENDOR_ID 0x16c0
#define MY_USB_PRODUCT_ID 0x03e8
static struct usb_device_id my_usb_table[] = {
       { USB_DEVICE(MY_USB_VENDOR_ID, MY_USB_PRODUCT_ID) },
       {},
};

MODULE_DEVICE_TABLE(usb, my_usb_table);

static void
int_cb(struct urb *urb)
{
   struct my_usb *sd = urb->context;
   printk(KERN_ALERT "urb interrupt is called");
   //This will show "A" sent from device on interrupt end point only
   //TODO: use endpoint3 also
   printk(KERN_ALERT "received data: %s", sd->int_in_buf);
}

static void
out_cb(struct urb *urb)
{
   struct my_usb *sd = urb->context;

   printk(KERN_ALERT "sent data: %s", sd->int_out_buf);
}

static unsigned long old_value;

static ssize_t
get_value(struct device *dev,
          struct device_attribute *attr,
          char *buf)
{
   return sprintf(buf, "%ld", old_value);
}

static ssize_t
set_value(struct device *dev,
          struct device_attribute *attr,
          const char *buf, size_t count)
{
   struct my_usb *sd = dev_get_drvdata(dev);
   long new_value = 0;
   int ret;

   if (kstrtol(buf, 10, &new_value))
     {
        return -EINVAL;
     }
   old_value = new_value;

   //set up our IN endpoint before sending OUT endpoint data 
   ret = usb_submit_urb(sd->int_in_urb, GFP_KERNEL);
   if (ret)
     {
        printk(KERN_ALERT "Failed to submit out urb");
     }

   sd->int_out_buf[0] = new_value;

   ret = usb_submit_urb(sd->int_out_urb, GFP_KERNEL);
   if (ret)
     {
        printk(KERN_ALERT "Failed to submit out urb");
     }

   return count;
}

static DEVICE_ATTR(value, 0660, get_value, set_value);

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

   data = kzalloc(sizeof(struct my_usb), GFP_KERNEL);
   if (data == NULL)
     {
        //handle error
     }

   for (i = 0; i < iface_desc->desc.bNumEndpoints; i++)
     {
        endpoint = &iface_desc->endpoint[i].desc;

        printk(KERN_INFO "ED[%d]->bEndpointAddress: 0x%02X\n",
               i, endpoint->bEndpointAddress);
        printk(KERN_INFO "ED[%d]->bmAttributes: 0x%02X\n",
               i, endpoint->bmAttributes);
        printk(KERN_INFO "ED[%d]->wMaxPacketSize: 0x%04X (%d)\n",
               i, endpoint->wMaxPacketSize, endpoint->wMaxPacketSize);
        if (endpoint->bmAttributes == 0x03)
          {
             //OUT interrupt end point
             if (endpoint->bEndpointAddress == 0x02)
               {
                  data->int_out_endpoint = endpoint;
               }
             //IN - interrupt end point
             else if (endpoint->bEndpointAddress == 0x81)
               {
                  data->int_in_endpoint = endpoint;
               }
          }
     }

   //increase ref count, make sure u call usb_put_dev() in disconnect()
   data->udev = usb_get_dev(udev);

   // allocate our urb for interrupt in 
   data->int_out_urb = usb_alloc_urb(0, GFP_KERNEL);
   data->int_in_urb = usb_alloc_urb(0, GFP_KERNEL);

   //allocate the interrupt buffer to be used
   data->int_out_buf = kmalloc(le16_to_cpu(data->int_out_endpoint->wMaxPacketSize), GFP_KERNEL);
   data->int_in_buf = kmalloc(le16_to_cpu(data->int_in_endpoint->wMaxPacketSize), GFP_KERNEL);

   data->int_out_buf[0] = 1;
   usb_fill_int_urb(data->int_out_urb, udev,
                    usb_sndintpipe(udev, data->int_out_endpoint->bEndpointAddress),
                    data->int_out_buf,
                    le16_to_cpu(data->int_out_endpoint->wMaxPacketSize),
                    out_cb, // this callback is called when we are done sending/recieving urb
                    data,
                    (data->int_out_endpoint->bInterval));

   usb_fill_int_urb(data->int_in_urb, udev,
                    usb_rcvintpipe(udev, data->int_in_endpoint->bEndpointAddress),
                    data->int_in_buf,
                    le16_to_cpu(data->int_in_endpoint->wMaxPacketSize),
                    int_cb,
                    data,
                    data->int_in_endpoint->bInterval);

   usb_set_intfdata(interface, data);

   printk(KERN_INFO "usb device is connected");

   //its better to use udev->dev though
   //location of 'value' file will be changed a bit
   i = device_create_file(&interface->dev, &dev_attr_value);
   if (i < 0)
     {
        printk(KERN_ALERT "Failed to create device attrib value");
     }

   //uncomment below line in case you use udev->dev
   // for creating sysfs file.
   //dev_set_drvdata(&udev->dev, data);

   return 0;
}

//called when unplugging a USB device
static void
my_usb_disconnect(struct usb_interface *interface)
{
   struct my_usb *data;

   data = usb_get_intfdata(interface);

   //off the led on disconnection
   usb_control_msg(interface_to_usbdev(interface),
                   usb_sndctrlpipe(interface_to_usbdev(interface), 0),
                   0, USB_TYPE_VENDOR | USB_DIR_OUT,
                   0, 0,
                   NULL, 0,
                   1000);
   usb_set_intfdata(interface, NULL);

   usb_kill_urb(data->int_out_urb);
   usb_kill_urb(data->int_in_urb);
   usb_free_urb(data->int_out_urb);
   usb_free_urb(data->int_in_urb);
   kfree(data->int_out_buf);
   kfree(data->int_in_buf);

   //its better to use udev->dev though
   //location of 'value' file will be changed a bit
   device_remove_file(&interface->dev, &dev_attr_value);

   //deref the count
   usb_put_dev(data->udev);

   kfree(data); //deallocate, allocated by kzmalloc()

   printk(KERN_INFO "usb device is disconnected");
}

static struct usb_driver my_usb_driver = {
     .name = "urb_read_write",
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

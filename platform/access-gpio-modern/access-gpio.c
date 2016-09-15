#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
//#include <linux/gpio/consumer.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("Access GPIO created by platform-gpio module");
MODULE_VERSION("0.1");

struct gpio_desc *desc;
static long gpio_no = 432;
static unsigned irq_no;

static irqreturn_t
_irq_handler(unsigned irq, void *dev_id)
{
   printk(KERN_ALERT "irq handler or ISR is called");
   return IRQ_HANDLED;
}

static int __init
_gpio_access_init(void)
{
   int status;
   int result;

   printk(KERN_INFO "GPIO access init");

   // get gpio_desc *desc from gpio no.
   desc = gpio_to_desc(gpio_no);

   if (!desc)
     {
        printk(KERN_ALERT "invalid gpio: %ld", gpio_no);
        return -EINVAL; // Invalid argument
     }
   //request for GPIO
   status = gpio_request(gpio_no, "sysfs"); //-- old way
   if (status < 0)
     {
        printk(KERN_ALERT "Failed to get gpio");
        return -EINVAL;
     }

   //if (gpio_export(gpio_no, false) < 0) -- old way
   //export a GPIO through sysfs
   status = gpiod_export(desc, true); //this makes gpio<N> to be visible in sysfs/

   if (status < 0)
     {
        printk(KERN_ALERT "failed to export gpio pin: %ld", gpio_no);
        return -EINVAL;
     }

   //gpio_direction_output(gpio_no, true); // old way 
   gpiod_direction_output(desc, false); // gpio initial value would be LOW

   // gpiod_to_irq(gpio_no) -- old way
   irq_no = gpiod_to_irq(desc);
   printk(KERN_INFO "gpio is mapped to IRQ: %d", irq_no);

   result = request_irq(irq_no,
                        (irq_handler_t)_irq_handler,
                        IRQF_TRIGGER_HIGH,
                        //IRQF_TRIGGER_MASK,
                        "interrput-handler",
                        NULL);
   if (result < 0)
     {
        printk(KERN_ALERT "Failed to aquire irq = %d", irq_no);
     }
   printk(KERN_ALERT "interrupt add result: %d",
          result);

   return 0;
}

static void __exit
_gpio_access_exit(void)
{
   printk(KERN_INFO "GPIO access exit");

   free_irq(irq_no, NULL);
   gpiod_unexport(desc);
   gpio_free(gpio_no);
}

module_init(_gpio_access_init);
module_exit(_gpio_access_exit);

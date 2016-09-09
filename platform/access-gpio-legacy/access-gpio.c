#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("Access GPIO created by platform-gpio module");
MODULE_VERSION("0.1");

static long gpio_no = 435;
static unsigned irq_no;

static irq_handler_t
_irq_handler(unsigned irq, void *dev_id,
             struct pt_regs *regs)
{
   printk(KERN_ALERT "irq handler or ISR is called");
   return (irq_handler_t)IRQ_HANDLED;
}

static int __init
_gpio_access_init(void)
{
   printk(KERN_INFO "GPIO access init");

   //request for GPIO
   gpio_request(gpio_no, "sysfs");

   //export a GPIO through sysfs
   if (gpio_export(gpio_no, false) < 0)
     {
        printk(KERN_ALERT "failed to export gpio pin: %ld", gpio_no);
        return -EINVAL;
     }

   gpio_direction_output(gpio_no, true);

   irq_no = gpio_to_irq(gpio_no);
   printk(KERN_INFO "gpio is mapped to IRQ: %d", irq_no);

   int result;

   result = request_irq(1,
                        (irq_handler_t)_irq_handler,
                        IRQF_TRIGGER_MASK,
                        "interrput-handler",
                        NULL);
   printk(KERN_ALERT "interrupt add result: %d",
          result);

   return 0;
}

static void __exit
_gpio_access_exit(void)
{
   printk(KERN_INFO "GPIO access exit");

   free_irq(irq_no, NULL);
   gpio_unexport(gpio_no);
   gpio_free(gpio_no);
}

module_init(_gpio_access_init);
module_exit(_gpio_access_exit);

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("custom soc chip driver to create fake GPIOs");
MODULE_VERSION("0.1");

struct my_device_platform_data
{
   struct gpio_chip chip;
   int gpio_pin_val; // only 1 pin we have, for simplicity
   struct irq_chip *irqchip;

   void (*gpio_create)(struct my_device_platform_data *, struct platform_device *pdev);
   void (*gpio_remove)(struct my_device_platform_data *, struct platform_device *pdev);
};

static int
_gpio_get(struct gpio_chip *chip,
          unsigned offset)
{
   struct my_device_platform_data *sd =
      container_of(chip,
                   struct my_device_platform_data,
                   chip);

   printk(KERN_INFO "gpio_get(): %d", sd->gpio_pin_val);

   return sd->gpio_pin_val;
}

static void
_gpio_set(struct gpio_chip *chip,
          unsigned offset, int value)
{
   struct my_device_platform_data *sd =
      container_of(chip,
                   struct my_device_platform_data,
                   chip);
   sd->gpio_pin_val = value;
   printk(KERN_INFO "gpio_set(): %d", value);
}

static int
_direction_output(struct gpio_chip *chip,
                  unsigned offset, int value)
{
   printk(KERN_INFO "Setting pin to OUTPUT");

   return 0;
}

static int
_to_irq(struct gpio_chip *chip,
        unsigned offset)
{
   printk("to_irq():");

   return 101;
}

static void
_irq_mask(struct irq_data *d)
{
   printk("irq mask");
}

static void
_irq_unmask(struct irq_data *d)
{
   printk("irq unmask");
}

static int
_irq_set_type(struct irq_data *d, unsigned type)
{
   printk("irq set type: %d, type\n", type);

   return 0;
}

static int
_irq_request_resources(struct irq_data *d)
{
   return 0;
}

static struct irq_chip my_gpio_irq_chip = {
     .name = "irq-gpio",
     .irq_mask = _irq_mask,
     .irq_unmask = _irq_unmask,
     .irq_set_type = _irq_set_type,
     .irq_request_resources = _irq_request_resources,
};

static irqreturn_t
_irq_handler(int irq, void *dev_id)
{
   printk(KERN_ALERT "........................ handled irq interrupt");
   return IRQ_HANDLED;
}


static void
_gpio_create(struct my_device_platform_data *sd, struct platform_device *pdev)
{
   printk(KERN_INFO "GPIO created");
   sd->chip.dev = &pdev->dev;
   sd->chip.label = "plat-gpio";
   sd->chip.owner = THIS_MODULE;
   sd->chip.base = -1;
   sd->chip.ngpio = 4;
   sd->chip.can_sleep = false;
   sd->chip.set = _gpio_set;
   sd->chip.get = _gpio_get;
   sd->chip.direction_output = _direction_output;
   //sd->chip.to_irq = _to_irq;

   sd->irqchip = &my_gpio_irq_chip;

   if (gpiochip_add(&sd->chip) < 0)
     {
        printk(KERN_ALERT "Failed to add gpio chip");
     }
   else
     printk (KERN_INFO "able to add gpiochip: %s",
             sd->chip.label);

   int ret;
      /*
   ret = devm_request_threaded_irq(&pdev->dev, 36,
                                   NULL, _irq_handler, IRQF_TRIGGER_HIGH,
                                   dev_name(&pdev->dev), &sd->chip);
                                   */

   ret = gpiochip_irqchip_add(&sd->chip,
                              sd->irqchip,
                              0,
                              handle_simple_irq,
                                  IRQ_TYPE_NONE);
                              //IRQ_TYPE_LEVEL_HIGH);
   if (ret)
     {
        printk(KERN_NOTICE "Failed to add irqchip");
     }

   gpiochip_set_chained_irqchip(&sd->chip,
                                sd->irqchip,
                                36,
                                NULL);


   ret = request_irq(36,
                     _irq_handler,
                     IRQF_TRIGGER_HIGH,
                     dev_name(&pdev->dev),
                     NULL);
   if (ret)
     {
        printk("Failed to request IRQ:\n");
     }

}

static void
_gpio_remove(struct my_device_platform_data *sd, struct platform_device *pdev)
{
   printk(KERN_INFO "GPIO is removed");
   free_irq(36, 0);
   gpiochip_remove(&sd->chip);
}

static struct my_device_platform_data my_device_data = {
     .gpio_create = _gpio_create,
     .gpio_remove = _gpio_remove,
};

static void
_device_release(struct device *pdev)
{
   //fixes kernel warnings:
   /*  +0.000010] WARNING: CPU: 3 PID: 11891 at /build/lin
       ux-a2WvEb/linux-4.4.0/drivers/base/core.c:251 device_r
       elease+0x89/0x90() [  +0.000010] Device 'platform-gpio-device' does not have a release() function, it is broken and must be fixed.
    */

   printk(KERN_ALERT "Device is released");
}

static struct platform_device my_device = {
     .name = "platform-gpio-device",
     .id = -1, //let kernel decide 
     .dev.platform_data = &my_device_data,
     .dev.release = _device_release,
};

static int
_sample_platform_driver_probe(struct platform_device *pdev)
{
   struct my_device_platform_data *data;

   printk(KERN_INFO "platfrom device connected/probed");
   ///struct my_driver_data *mdd;

   data = dev_get_platdata(&pdev->dev);

   if (data->gpio_create) data->gpio_create(data, pdev);

   return 0;
}

static int
_sample_platform_driver_remove(struct platform_device *pdev)
{
   struct my_device_platform_data *data;

   data = dev_get_platdata(&pdev->dev);

   if (data->gpio_remove) data->gpio_remove(data, pdev);

   printk(KERN_INFO "platfrom device removed");
   return 0;
}

static struct platform_driver sample_platform_driver = {
     .probe = _sample_platform_driver_probe,
     .remove = _sample_platform_driver_remove,
     .driver = {
          .name = "platform-gpio-device", //platform_device will also use same name
     },
};

static int __init
_platform_driver_init(void)
{
   printk(KERN_INFO "platform driver init");
   platform_driver_register(&sample_platform_driver);
   platform_device_register(&my_device);

   return 0;
}

static void __exit
_platform_driver_exit(void)
{
   printk(KERN_INFO "platform driver exit");
   platform_driver_unregister(&sample_platform_driver);
   platform_device_unregister(&my_device);
}

module_init(_platform_driver_init);
module_exit(_platform_driver_exit);

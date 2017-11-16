#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bhargav Shah");

#define MAX_SUPPORTED_CHAR     0xFF

long old_value = 0;

/*
GPIO 9 (Pin 21 on RPI header)  <-----> pin 14 of shift reg
GPIO 25 (Pin 22 on RPI header)  <-----> pin 11 of shift re
GPIO 11 (Pin 23 on RPI header)  <-----> pin 12 of shift reg
GPIO 8 (Pin 24 on RPI header)  <-----> pin 10 of shift reg
*/

/* GPIO pins to control shift reg */
static const int dataPin = 9;
static const int clkPin = 25;
static const int latchPin = 11;
static const int sclkBarPin = 8;

void serial_clk(void)
{
	udelay(1);
    gpio_set_value(clkPin, 1);
	udelay(1);
	gpio_set_value(clkPin, 0);
}

void latch_clk(void)
{
	udelay(1);
    gpio_set_value(latchPin, 1);
	udelay(1);
	gpio_set_value(latchPin, 0);
}

static ssize_t get_value_callback(struct device *dev,
                                  struct device_attribute *attr,
                                  char *buf)
{
	return 	sprintf(buf, "%ld\n", old_value);
}

static ssize_t set_value_callback(struct device* dev,
								   struct device_attribute* attr,
								   const char* buf,
								   size_t count)
{
	long new_value = 0;
	int ct;
	unsigned char mask = 0x80;

	if (kstrtol(buf, 10, &new_value) < 0)
		return -EINVAL;

	/* mask the value to 8 bit because we have 8 bit shift register */
	new_value = new_value & MAX_SUPPORTED_CHAR;
	printk(KERN_ERR "Test: New value is : %ld\n\n", new_value);

	/* add bits in shift register */
	for(ct=0; ct < 8; ct++) {

		if(new_value & mask){
			gpio_set_value(dataPin, 1);
		} else {
			gpio_set_value(dataPin, 0);
		}

		serial_clk();
		new_value = new_value << 1;
	}

	latch_clk();
	old_value = new_value;

	return count;
}

static DEVICE_ATTR(value, 0660, get_value_callback, set_value_callback);

static struct class *s_pDeviceClass;
static struct device *s_pDeviceObject;

static int __init ShiftReg_init(void)
{
	int result = 0;

	printk("%s: Inserting shift register module\n", __func__);

	/* request GPIO */
	result = gpio_request(dataPin, "shift-reg-data pin");
    if (result) {
      printk(KERN_WARNING "unable to request GPIO_PG%d\n", dataPin);
      return result;
    }

	result = gpio_request(latchPin, "shift-reg-latch pin");
    if (result) {
      printk(KERN_WARNING "unable to request GPIO_PG%d\n", latchPin);
      return result;
    }

	result = gpio_request(clkPin, "shift-reg-clk pin");
    if (result) {
      printk(KERN_WARNING "unable to request GPIO_PG%d\n", clkPin);
      return result;
    }

	result = gpio_request(sclkBarPin, "shift-reg-sclkBar pin");
    if (result) {
      printk(KERN_WARNING "unable to request GPIO_PG%d\n", sclkBarPin);
      return result;
    }

	/* Configure all pins to output pin */
	if(gpio_direction_output(dataPin, 0))
		printk(KERN_WARNING "Error in setting data pin dirrection\n");

	if(gpio_direction_output(latchPin, 0))
		printk(KERN_WARNING "Error in setting latch pin dirrection\n");

	if(gpio_direction_output(clkPin, 0))
		printk(KERN_WARNING "Error in setting clk pin dirrection\n");

	if(gpio_direction_output(sclkBarPin, 1))
		printk(KERN_WARNING "Error in setting sclkBar dirrection\n");

    /* create sysfs interface */
	s_pDeviceClass = class_create(THIS_MODULE, "shift_reg");
	BUG_ON(IS_ERR(s_pDeviceClass));

	s_pDeviceObject = device_create(s_pDeviceClass, NULL, 0, NULL, "shift_reg");
	BUG_ON(IS_ERR(s_pDeviceObject));

	result = device_create_file(s_pDeviceObject, &dev_attr_value);
	BUG_ON(result < 0);

	return 0;
}

static void __exit ShiftReg_exit(void)
{
	printk("%s: Removing shift register module\n", __func__);

	/* clear sysfs node */
	device_remove_file(s_pDeviceObject, &dev_attr_value);
	device_destroy(s_pDeviceClass, 0);
	class_destroy(s_pDeviceClass);

	/* free GPIO */
	gpio_free(dataPin);
	gpio_free(clkPin);
	gpio_free(latchPin);
	gpio_free(sclkBarPin);
}

module_init(ShiftReg_init);
module_exit(ShiftReg_exit);

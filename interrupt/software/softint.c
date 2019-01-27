#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model

#include <linux/interrupt.h> // for interrupts related functions 
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");

//11
#define IRQ_NO 2

static struct delayed_work mywork;

static irqreturn_t irq_handler(int rq, void *dev_id)
{
    printk("irq_handler is called.");
    return IRQ_HANDLED;
}

static void delay_work_function(struct work_struct *w)
{
    //0x20 + 0x10 + irqno
    asm("int $50");
    printk("delayed work function called.");
}

static int __init
_irq_init(void)
{
    printk("going to register interrupt 11");
    if (request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "meh", (void *)irq_handler))
    {
        printk(KERN_ALERT "Failed to register 11 interrupt");
        return -EIO;
    }
    INIT_DELAYED_WORK(&mywork, delay_work_function);

    schedule_delayed_work(&mywork, 4 * HZ);
    //asm("int $0x3B");
    return 0;
}

//called on module unloading
static void __exit
_irq_exit(void)
{
    free_irq(IRQ_NO, (void *)irq_handler);
    cancel_delayed_work(&mywork);
}

module_init(_irq_init);
module_exit(_irq_exit);

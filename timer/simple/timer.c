#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/timer.h> // for timer and timer related functions

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("A simple timer example");
MODULE_VERSION("0.1");

//jiffies is the current time.
static struct timer_list timer_1;

static void
timer_timeout(struct timer_list *timer)
{
   printk(KERN_ALERT "timer is called");
   //keep calling timer_1 on every 100 ms
   //timer_1.expires = jiffies + msecs_to_jiffies(100);
   //add_timer(&timer_1);
   mod_timer(&timer_1, jiffies + msecs_to_jiffies(100));
}

static int __init
_timer_init(void)
{
   //for first time use
   // or timer_setup
   timer_setup_on_stack(&timer_1, timer_timeout, 0);

   mod_timer(&timer_1, jiffies + msecs_to_jiffies(100));
   printk(KERN_INFO "started timer..");

   return 0;
}

static void
_timer_exit(void)
{
   destroy_timer_on_stack(&timer_1);
   del_timer(&timer_1);
   printk(KERN_INFO "exit timer..");
}

module_init(_timer_init);
module_exit(_timer_exit);


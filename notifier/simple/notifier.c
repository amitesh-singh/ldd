#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/notifier.h> //for notifier related APIs

static long counter = 0;

static BLOCKING_NOTIFIER_HEAD(my_nh);
static int
my_notifier_call(struct notifier_block *b,unsigned long event,
                 void *data)
{
   long *val = (long *)data;

   *val += 100;

   pr_info("Current data: %ld, event: %ld\n", *val, event);

   return NOTIFY_OK;
}

static struct notifier_block notifier_blck = {
   .notifier_call = my_notifier_call,
   .priority = 0,
};

static int __init
_custom_notify_init(void)
{
   int ret;

   if (blocking_notifier_chain_register(&my_nh, &notifier_blck))
   {
      printk(KERN_ALERT "Failed to register notifier chain");
      return -1;
   }

   ret = blocking_notifier_call_chain(&my_nh, 1000, &counter);
   pr_info("ret value from chain: %d\n", ret);

   pr_info("custom notifier module init\n");

   return 0;
}

static void __exit
_custom_notify_exit(void)
{
   blocking_notifier_chain_unregister(&my_nh, &notifier_blck);
   pr_info("custom notifier module exit\n");
}

module_init(_custom_notify_init);
module_exit(_custom_notify_exit);

MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("simple notifier usb driver");
MODULE_LICENSE("GPL");

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("A Hello Ami module");

static int __init hello_init(void)
{
   //printk(KERN_INFO " Hello ami!\n");
   //pr_info is defined as printk(KERN_INFO..), pr_alert() also can be used instead of
   // printk(KERN_ALERT ..);
   pr_info(" Hello Ami!");
   return 0;
}

static void __exit hello_exit(void)
{
   //printk(KERN_INFO "Bye, ami\n");
   pr_info(" Bye Ami!");
}

module_init(hello_init);
module_exit(hello_exit);

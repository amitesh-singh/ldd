#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("A Hello Ami module");

static int hello_init(void) {
  printk(KERN_INFO " Hello ami!\n");
  return 0;
}

static void hello_exit(void) {
  printk(KERN_INFO "Bye, ami\n");
}

module_init(hello_init);
module_exit(hello_exit);

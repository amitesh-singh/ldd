#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("A Hello Ami module");
MODULE_VERSION("0.1"); // --> module version of kernel

static char *name = "ami";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");
//The above statement is useful  to pass the parameter while doing
// insmod of driver.
//usage:
// #insmod hello-1.ko name="Mymodule"
// check dmesg or /var/log/kern.log

//The __init macro means that for a built-in driver (not a LKM) the function is only used at initialization
// time and that it can be discarded and its memory freed up after that point.
// @returns 0 if successful
static int __init hello_init(void) {
  printk(KERN_INFO "%s: Hello ami!\n", name);
  return 0;
}

//  The __exit macro notifies that if this
//  code is used for a built-in driver (not a LKM) that this function is not required.
static void __exit hello_exit(void) {
  printk(KERN_INFO "%s: Bye, ami\n", name);
}

module_init(hello_init);
module_exit(hello_exit);

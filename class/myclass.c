#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>

/*
 *
 *
 * In the Linux kernel, the class_create function is used to create a new class under /sys/class. It allocates and initializes a struct class object and creates the corresponding class directory in the sysfs filesystem.
 */
struct class *myclass;

static int __init
myclass_init(void)
{
	myclass = class_create(THIS_MODULE, "myclass");
	if (!myclass)
		return -ENODEV;

	pr_info("check /sys/class/myclass");
	return 0;
}

static void __exit
myclass_exit(void)
{
	class_destroy(myclass);
}

MODULE_LICENSE("GPL");

module_init(myclass_init);
module_exit(myclass_exit);

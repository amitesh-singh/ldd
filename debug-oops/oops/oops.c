#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

/*
 * objdump -S oops.o 
 * check EXTRA_CFLAGS += -g to enable debug info
 *
 *
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("oops creator");

static int __init hello_init(void)
{
   int *a = NULL;
   *a = 3;
   return 0;
}

static void __exit hello_exit(void)
{
   pr_info(" Bye Ami!");
}

module_init(hello_init);
module_exit(hello_exit);

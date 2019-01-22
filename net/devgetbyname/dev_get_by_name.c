#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/etherdevice.h>

static int __init _init_module(void)
{
   struct net_device *dev = dev_get_by_name(&init_net, "lo");
   if (dev)
     printk("Dev: name: %s, ifindex: %d\n", dev->name,
            dev->ifindex);

   return 0;
}

static void  __exit _cleanup (void)
{
}

module_init(_init_module);
module_exit(_cleanup);
MODULE_LICENSE("GPL");

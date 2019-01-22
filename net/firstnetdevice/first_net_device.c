#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/etherdevice.h>

static int __init _init_module(void)
{
   struct net_device *dev = first_net_device(&init_net);
   while (dev)
     {
        printk("Dev: name: %s, ifindex: %d\n", dev->name,
               dev->ifindex);
        if (netif_running(dev))
          {
             printk("Device is up\n");
          }
        else
          printk("Device is down\n");

        dev = next_net_device(dev);
     }

   return 0;
}

static void  __exit _cleanup (void)
{
}

module_init(_init_module);
module_exit(_cleanup);
MODULE_LICENSE("GPL");

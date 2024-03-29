#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/etherdevice.h>

struct net_device *esp_net_device;

static int _open(struct net_device *dev)
{
   printk("open called\n");

   //start up the transmission queue.
   netif_start_queue(dev);
   return 0;
}

static int _release(struct net_device *dev)
{
   printk("release called\n");
   //shutdown the transmission queue
   netif_stop_queue(dev);
   return 0;
}

/* Note this method is only needed on some; without it
   module will fail upon removal or use. At any rate there is a memory
   leak whenever you try to send a packet through in any case*/

static int _xmit(struct sk_buff *skb, struct net_device *dev)
{
   int i =0;
   printk("xmit function called...\n");

   for (; i < skb->len; ++i)
     {
        if ((i & 0xf) == 0)
          printk("\n");
        printk("%02x ", skb->data[i]);
     }
   printk("\n");

   dev_kfree_skb(skb);
   ++dev->stats.tx_packets;

   return 0;
}

static int _init(struct net_device *dev)
{
   printk("device initialized\n");
   return 0;
};

const struct net_device_ops my_netdev_ops = {
     .ndo_init = _init,
     .ndo_open = _open,
     .ndo_stop = _release,
     .ndo_start_xmit = _xmit,
};

static void esp_setup(struct net_device *dev)
{
   int i = 0;

   //fill the mac address.
   for (;i < ETH_ALEN; ++i)
     dev->dev_addr[i] = (char) i;

   //assign ethernet protocol values to interface protocol specific fields of net_dev structure
   ether_setup(dev);

   dev->netdev_ops = &my_netdev_ops;
}

static int __init _init_module(void)
{
   int result;

   esp_net_device = alloc_netdev(0, "esp%d", NET_NAME_UNKNOWN, esp_setup);

   if((result = register_netdev(esp_net_device)))
     {
        printk(KERN_ALERT "esp: Error %d initalizing card ...", result);
        free_netdev(esp_net_device);
        return result;
     }
   printk("interface %s is created\n", dev_name(&esp_net_device->dev));

   return 0;
}

static void  __exit _cleanup (void)
{
   printk ("esp - cleanup\n");
   unregister_netdev (esp_net_device);
   free_netdev(esp_net_device);
}

module_init(_init_module);
module_exit(_cleanup);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("netdriver skeleton");

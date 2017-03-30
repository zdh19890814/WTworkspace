/*
 * ed_device.c embeded software project(1) device file.
 *
 * Copyright (C) 2003 Li Suke,Software School of Peking University.
 * 
 * The data flow of the devices is:
 *
 *          neted
 *     _______|___________
 *     |                  |
 *     |                  |
 *   ed_rec            ed_tx
 *   (recieve)          (transmit)
 *  
 * neted: pseodu network device
 * ed_rec: character device
 * ed_tx:  character device
 * You can modify and distribute this source code freely.   
 * 
 */

#ifndef __KERNEL__
#define __KERNEL__
#endif
#ifndef MODULE
#define MODULE
#endif

#include <linux/config.h>
#include <linux/module.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#ifdef LINUX_24
#include <linux/mm.h> 
#else
#include <linux/malloc.h>
#endif

#include <linux/string.h>
#include <linux/errno.h>  
#include <linux/types.h>  

#include <linux/in.h>
#include <linux/netdevice.h>  
#include <linux/etherdevice.h> 
#include <linux/ip.h>          
#include <linux/skbuff.h>
#include <linux/ioctl.h>
#ifdef LINUX_20
#include <linux/if_ether.h>
#endif

#include <asm/uaccess.h>

#include "ed_device.h"
#include "ed_ioctl.h"

MODULE_AUTHOR("Li Suke");

/* We must define the htons() function here, for the kernel has no
 * this API if you do not make source code dep
 */ 
#define htons(x) ((x>>8) | (x<<8))

char ed_names[16];
struct ed_device ed[2];
#ifdef LINUX_24
struct net_device ednet_dev;
#else
struct device ednet_dev;
#endif

#ifdef LINUX_24
static int timeout = ED_TIMEOUT;
#endif

#ifdef LINUX_24
void ednet_rx(struct net_device *dev,int len,unsigned char *buf);
#else
void ednet_rx(struct device *dev,int len,unsigned char *buf);
#endif

#ifdef LINUX_24
void ednet_tx_timeout (struct net_device *dev);
#else
void ednet_tx_timeout (struct device *dev);
#endif
      
/* Initialize the ed_rec and ed_tx device,the two devices
   are allocate the initial buffer to store the incoming and
   outgoing data. If the TCP/IP handshake need change the
   MTU,we must reallocte the buffer using the new MTU value.
 */
static int device_init(){
    
    int i;
    int err;
    err = -ENOBUFS;
    strcpy(ed[ED_REC_DEVICE].name, ED_REC_DEVICE_NAME);
    strcpy(ed[ED_TX_DEVICE].name, ED_TX_DEVICE_NAME);
    for (i = 0 ;i < 2; i++ )
    {   
    	ed[i].buffer_size = BUFFER_SIZE;     	
        ed[i].buffer = kmalloc(ed[i].buffer_size + 4 , GFP_KERNEL);
        ed[i].magic = ED_MAGIC;
        ed[i].mtu = ED_MTU;
        ed[i].busy = 0;
        #ifdef LINUX_24
        init_waitqueue_head(&ed[i].rwait);
        #endif
        
	if (ed[i].buffer == NULL)
	    goto err_exit;	    	
	spin_lock_init(&ed[i].lock);     
    }  
    err = 0;
    return err;
    
err_exit:
    printk("There is no enongh memory for buffer allocation. \n");
    return err;		
}
static int ed_realloc(int new_mtu){
    int err;
    int i;
    err = -ENOBUFS;
    char *local_buffer[2];
    int size;
    for (i=0;i<2;i++){
        local_buffer[i] = kmalloc(new_mtu + 4,GFP_KERNEL);
        #ifdef LINUX_20
        if(new_mtu >= ed[i].buffer_size)
            size = new_mtu;
        else 
            size = ed[i].buffer_size;
        #else
        size = min(new_mtu,ed[i].buffer_size);
        #endif

        memcpy(local_buffer[i],ed[i].buffer,size);
        kfree(ed[i].buffer);

        ed[i].buffer = kmalloc(new_mtu + 4,GFP_KERNEL);
        if( ed[i].buffer < 0){
            printk("Can not realloc the buffer from kernel when change mtu.\n");
            return err;
        }
           
    }
    return 0;
	
}
/* Open the two character devices,and let the ed_device's private pointer 
 * point to the file struct */

static int device_open(struct inode *inode,struct file *file)
{
    int Device_Major;
    struct ed_device *edp;
    Device_Major = inode->i_rdev >> 8;
    
    #ifdef _DEBUG
    printk("Get the Device Major Number is %d\n",Device_Major);
    #endif
    if (Device_Major == MAJOR_NUM_REC )
    {
        file->private_data = &ed[ED_REC_DEVICE];
        ed[ED_REC_DEVICE].file = file;
    }    
    else    
    if (Device_Major == MAJOR_NUM_TX){
        file->private_data = &ed[ED_TX_DEVICE];
        ed[ED_TX_DEVICE].file = file;
    }    
    else
        return -NODEV;
    edp = (struct ed_device *)file->private_data;
                    
    if(edp->busy != 0){
       printk("The device is open!\n");	
       return -EBUSY;
    }
    
    edp->busy++;
 
    return 0;
       
}

/* release the devices */
int device_release(struct inode *inode,struct file *file)
{
	
    struct ed_device *edp;
    edp = (struct ed_device *)file->private_data;
    edp->busy = 0;
    
    return 0;	
}

/* read data from ed_tx device */
ssize_t device_read(struct file *file,char *buffer,size_t length, loff_t *offset)
{
    #ifdef _DEBUG
    int i;
    #endif
    struct ed_device *edp;
    edp = (struct ed_device *)file->private_data;
#ifdef LINUX_24
    DECLARE_WAITQUEUE(wait,current);
    add_wait_queue(&edp->rwait,&wait);
    for(;;){
        
        set_current_state(TASK_INTERRUPTIBLE);
        if ( file->f_flags & O_NONBLOCK)
            break;
        if ( edp->tx_len > 0)
            break;
        
        if ( signal_pending(current))
            break;
        schedule();
        
    }
    set_current_state(TASK_RUNNING);
    remove_wait_queue(&edp->rwait,&wait);
#endif
    
    spin_lock(&edp->lock);
    
    if(edp->tx_len == 0) {
         spin_unlock(&edp->lock);
         return 0;

        
    }else
    {
   
        copy_to_user(buffer,edp->buffer,edp->tx_len);
        memset(edp->buffer,0,edp->buffer_size); 
        
        #ifdef _DEBUG
        printk("\n read data from ed_tx \n");
        for(i=0;i<edp->tx_len;i++)
            printk(" %02x",edp->buffer[i]&0xff);
        printk("\n");
        #endif
        
        length = edp->tx_len;
        edp->tx_len = 0;
    }
    spin_unlock(&edp->lock);
    return length;
    
}
/* This function is called by ednet device to write the network data 
 * into the ed_tx character device.
 */
ssize_t kernel_write(const char *buffer,size_t length,int buffer_size)
{
          
    if(length > buffer_size )
        length = buffer_size;
    memset(ed[ED_TX_DEVICE].buffer,0,buffer_size);
    memcpy(ed[ED_TX_DEVICE].buffer,buffer,buffer_size);
    ed[ED_TX_DEVICE].tx_len = length;
    #ifdef LINUX_24
    wake_up_interruptible(&ed[ED_TX_DEVICE].rwait);	
    #endif
    
    return length;
}

/* Device write is called by server program, to put the user space
 * network data into ed_rec device.
 */
ssize_t device_write(struct file *file,const char *buffer, size_t length,loff_t *offset)
{
    #ifdef _DEBUG
    int i;
    #endif
    struct ed_device *edp;
    edp = (struct ed_device *)file->private_data;
    
    spin_lock(&ed[ED_REC_DEVICE].lock);
    if(length > edp->buffer_size)
        length =  edp->buffer_size;
        
    copy_from_user( ed[ED_REC_DEVICE].buffer,buffer, length);
    ednet_rx(&ednet_dev,length,ed[ED_REC_DEVICE].buffer);
    
    #ifdef _DEBUG
    printk("\nNetwork Device Recieve buffer:\n");
    for(i =0;i< length;i++)
       printk(" %02x",ed[ED_REC_DEVICE].buffer[i]&0xff);
    printk("\n");
    #endif
    spin_unlock(&ed[ED_REC_DEVICE].lock);     
    return length;   
}        
int device_ioctl(struct inode *inode,
                 struct file *file,
                 unsigned int ioctl_num,
                 unsigned long ioctl_param){
    struct ed_device *edp;
    edp = (struct ed_device *)file->private_data;
    switch(ioctl_num)
    {
        case IOCTL_SET_BUSY:
           edp->busy = ioctl_param;
           break;
      
       
    }
    return 0;

}

/* 
 * All the ednet_* functions are for the ednet pseudo network device ednet.
 * ednet_open and ednet_release are the two functions which open and release
 * the device.
 */
 
#ifdef LINUX_20
int ednet_open(struct device *dev)
#else
int ednet_open(struct net_device *dev)
#endif
{
    MOD_INC_USE_COUNT;
    
    /* Assign the hardware pseudo network hardware address,
     * the MAC address's first octet is 00,for the MAC is
     * used for local net,not for the Internet.
     */
    memcpy(dev->dev_addr, "\0ED000", ETH_ALEN);
   
#ifndef LINUX_24                
    dev->start = 1;
    dev->tbusy = 0;             
#endif
             
#ifdef LINUX_24             
    netif_start_queue(dev);
#endif
    return 0;
}
#ifdef LINUX_24
int ednet_release(struct net_device *dev)
#else
int ednet_release(struct device *dev)
#endif
{
    
#ifdef LINUX_24    
    netif_stop_queue(dev); 
#endif

#ifndef LINUX_24                
    dev->start = 0;
    dev->tbusy = 1;             
#endif                          
    MOD_DEC_USE_COUNT;
    
    return 0;
}

/*
 * Used by ifconfig,the io base addr and IRQ can be modified
 * when the net device is not running.
 */
#ifdef LINUX_24
int ednet_config(struct net_device *dev, struct ifmap *map)
#else
int ednet_config(struct device *dev, struct ifmap *map)
#endif
{
    if (dev->flags & IFF_UP) 
        return -EBUSY;

    /* change the io_base addr */
    if (map->base_addr != dev->base_addr) {
        printk(KERN_WARNING "ednet: Can't change I/O address\n");
        return -EOPNOTSUPP;
    }

    /* can change the irq */
    if (map->irq != dev->irq) {
        dev->irq = map->irq;
        
    }

    
    return 0;
}

/*
 * ednet_rx,recieves a network packet and put the packet into TCP/IP up
 * layer,netif_rx() is the kernel API to do such thing. The recieving
 * procedure must alloc the sk_buff structure to store the data,
 * and the sk_buff will be freed in the up layer.
 */
#ifdef LINUX_24
void ednet_rx(struct net_device *dev, int len, unsigned char *buf)
#else
void ednet_rx(struct device *dev ,int len, unsigned char *buf)
#endif
{
    struct sk_buff *skb;
    struct ednet_priv *priv = (struct ednet_priv *) dev->priv;

    skb = dev_alloc_skb(len+2);
    if (!skb) {
        printk("ednet_rx can not allocate more memory to store the packet. drop the packet\n");
        priv->stats.rx_dropped++;
        return;
    }
    skb_reserve(skb, 2);
    memcpy(skb_put(skb, len), buf, len);

    skb->dev = dev;
    skb->protocol = eth_type_trans(skb, dev);
    /* We need not check the checksum */
    skb->ip_summed = CHECKSUM_UNNECESSARY; 
    priv->stats.rx_packets++;
#ifndef LINUX_20                        
    priv->stats.rx_bytes += len;
#endif                                  
    netif_rx(skb);
    return;
}
    
        

/*
 * pseudo network hareware transmit,it just put the data into the 
 * ed_tx device.
 */
#ifdef LINUX_24
void ednet_hw_tx(char *buf, int len, struct net_device *dev)
#else
void ednet_hw_tx(char *buf, int len, struct device *dev)
#endif
{
   
    struct ednet_priv *priv;
   
    /* check the ip packet length,it must more then 34 octets */
    if (len < sizeof(struct ethhdr) + sizeof(struct iphdr)) {
        printk("Bad packet! It's size is less then 34!\n");
        
        return;
    }
    /* now push the data into ed_tx device */  
    ed[ED_TX_DEVICE].kernel_write(buf,len,ed[ED_TX_DEVICE].buffer_size); 
    
    /* record the transmitted packet status */
    priv = (struct ednet_priv *) dev->priv;
    priv->stats.tx_packets++;
    priv->stats.rx_bytes += len;
    
    /* remember to free the sk_buffer allocated in upper layer. */
    dev_kfree_skb(priv->skb);
 
   
}

/*
 * Transmit the packet,called by the kernel when there is an
 * application wants to transmit a packet.
 */
#ifdef LINUX_24
int ednet_tx(struct sk_buff *skb, struct net_device *dev)
#else
int ednet_tx(struct sk_buff *skb, struct device *dev)
#endif
{
    int len;
    char *data;
    struct ednet_priv *priv = (struct ednet_priv *) dev->priv;
    
    if( ed[ED_TX_DEVICE].busy ==1){
     
        return -EBUSY;
    }
#ifndef LINUX_24
    if (dev->tbusy || skb == NULL) {
  
        ednet_tx_timeout (dev);
        if (skb == NULL)
            return 0;
    }
#endif

    len = skb->len < ETH_ZLEN ? ETH_ZLEN : skb->len;
    data = skb->data;
    /* stamp the time stamp */
    dev->trans_start = jiffies;

    /* remember the skb and free it in ednet_hw_tx */
    priv->skb = skb;
    
    /* pseudo transmit the packet,hehe */
    ednet_hw_tx(data, len, dev);

    return 0; 
}

/*
 * Deal with a transmit timeout.
 */
#ifdef LINUX_24
void ednet_tx_timeout (struct net_device *dev)
#else
void ednet_tx_timeout (struct device *dev)
#endif
{
    struct ednet_priv *priv = (struct ednet_priv *) dev->priv;
    priv->stats.tx_errors++;
#ifdef LINUX_24
    netif_wake_queue(dev);
#endif   
    return;
}



/*
 * When we need some ioctls.
 */
#ifdef LINUX_24
int ednet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
#else
int ednet_ioctl(struct device *dev, struct ifreq *rq, int cmd)
#endif
{
 
    return 0;
}

/*
 * ifconfig to get the packet transmitting status.
 */
#ifdef LINUX_24
struct net_device_stats *ednet_stats(struct net_device *dev)
#else
struct enet_statistics *ednet_stats(struct device *dev)
#endif
{
    struct ednet_priv *priv = (struct ednet_priv *) dev->priv;
    return &priv->stats;
}

/*
 * TCP/IP handshake will call this function, if it need.
 */
#ifdef LINUX_24
int ednet_change_mtu(struct net_device *dev, int new_mtu)
#else
int ednet_change_mtu(struct device *dev, int new_mtu)
#endif
{
    int err;
    unsigned long flags;
    spinlock_t *lock = &((struct ednet_priv *) dev->priv)->lock;
    
    /* en, the mtu CANNOT LESS THEN 68 OR MORE THEN 1500. */
    if (new_mtu < 68)
        return -EINVAL;
   
    
    spin_lock_irqsave(lock, flags);
    dev->mtu = new_mtu;
    /* realloc the new buffer */
    
    err = ed_realloc(new_mtu);
    spin_unlock_irqrestore(lock, flags);
    return err; 
}
#ifdef LINUX_24
int ednet_header(struct sk_buff *skb,
                 struct net_device *dev,
                 unsigned short type,
                 void *daddr,
                 void *saddr,
                 unsigned int len)
#else
int ednet_header(struct sk_buff *skb,
                 struct device *dev,
                 unsigned short type,
                 void *daddr,
                 void *saddr,
                 unsigned int len)
#endif
{
    struct ethhdr *eth = (struct ethhdr *)skb_push(skb,ETH_HLEN);
    eth->h_proto = htons(type);
    memcpy(eth->h_source,saddr? saddr : dev->dev_addr,dev->addr_len);
    memcpy(eth->h_dest,   daddr? daddr : dev->dev_addr, dev->addr_len);
    return (dev->hard_header_len);

}

int ednet_rebuild_header(struct sk_buff *skb)
{
    struct ethhdr *eth = (struct ethhdr *)skb_push(skb,ETH_HLEN);
    
#ifdef LINUX_24
    struct net_device *dev = skb->dev;
#else 
    struct device *dev = skb->dev;
#endif

    memcpy(eth->h_source, dev->dev_addr ,dev->addr_len);
    memcpy(eth->h_dest,   dev->dev_addr , dev->addr_len);
    return 0;

}


#ifdef LINUX_24
int ednet_init(struct net_device *dev)
#else
int ednet_init(struct device *dev)
#endif
{

   
    ether_setup(dev); 
    dev->open            = ednet_open;
    dev->stop            = ednet_release;
    dev->set_config      = ednet_config;
    dev->hard_start_xmit = ednet_tx;
    dev->do_ioctl        = ednet_ioctl;
    dev->get_stats       = ednet_stats;
    dev->change_mtu      = ednet_change_mtu;  
#ifdef LINUX_24
    dev->hard_header     = ednet_header;
#endif
    dev->rebuild_header  = ednet_rebuild_header;
#ifdef LINUX_24
    dev->tx_timeout     = ednet_tx_timeout;
    dev->watchdog_timeo = timeout;
#endif
    /* We do not need the ARP protocol. */
    dev->flags           |= IFF_NOARP;
#ifndef LINUX_20                        
    dev->hard_header_cache = NULL;      
#endif 

#ifdef LINUX_24                                 
    SET_MODULE_OWNER(dev);
#endif

    dev->priv = kmalloc(sizeof(struct ednet_priv), GFP_KERNEL);
    if (dev->priv == NULL)
        return -ENOMEM;
    memset(dev->priv, 0, sizeof(struct ednet_priv));
    spin_lock_init(& ((struct ednet_priv *) dev->priv)->lock);
    return 0;
}

struct file_operations ed_ops ={
#ifdef LINUX_24
    NULL,
#endif
    NULL,
    device_read,
    device_write,
    NULL,
    NULL,
    device_ioctl,
    NULL,
    device_open,
    NULL,
    device_release,
    	
};

/* initialize the character devices */
int eddev_module_init(void)
{
    int err;
    int i;
    if((err=device_init()) != 0)
    {
        printk("Init device error:");
        return err;
    }
    err = register_chrdev(MAJOR_NUM_REC,ed[ED_REC_DEVICE].name,&ed_ops);
    if( err != 0)
        printk("Install the buffer rec device %s fail", ED_REC_DEVICE_NAME);
    for(i=0; i<2;i++)
        ed[i].kernel_write = kernel_write;  
    
    err = register_chrdev(MAJOR_NUM_TX,ed[ED_TX_DEVICE].name,&ed_ops);
    if( err != 0)
        printk("Install the buffer tx device %s fail",ED_TX_DEVICE_NAME);		
    return err;    
    

}
/* clean up the character devices */
void eddev_module_cleanup(void)
{
    int err;
    int i;
    for (i = 0 ;i < 2; i++){
        kfree(ed[i].buffer);   
    }    

    err = unregister_chrdev(MAJOR_NUM_REC,ed[ED_REC_DEVICE].name);
    if(err != 0)
        printk("UnInstall the buffer recieve device %s fail",ED_REC_DEVICE_NAME);
    err = unregister_chrdev(MAJOR_NUM_TX,ed[ED_TX_DEVICE].name);
    if(err != 0)
        printk("UnInstall the buffer recieve device %s fail",ED_TX_DEVICE_NAME);
        	
}


int ednet_module_init(void)
{

    int err;
    strcpy(ednet_dev.name, "ed0");
    ednet_dev.init = ednet_init;
    if ( (err = register_netdev(&ednet_dev)) )
            printk("ednet: error %i registering pseudo network device \"%s\"\n",
                   err, ednet_dev.name);
        
    return err;
}

void ednet_module_cleanup(void)
{
   
    kfree(ednet_dev.priv);
    unregister_netdev(&ednet_dev);
    return;
}

/* called by the kernel to setup the module*/
int init_module(void)
{
    int err;
    err = eddev_module_init();
    if(err < 0)
        return err;
    err = ednet_module_init();
    if(err < 0)
        return err;
    return err;	
	
}
/* cleanup the module */
void cleanup_module()
{
    eddev_module_cleanup();
    ednet_module_cleanup();	
	
}
#ifdef LINUX_24
MODULE_LICENSE("GPL");
#endif


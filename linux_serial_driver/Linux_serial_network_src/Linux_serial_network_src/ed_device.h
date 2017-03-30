/* this is the header of the ed_device*/

#ifndef _ED_DEVICE_H
#define _ED_DEVICE_H

#define ED_REC_DEVICE      0
#define ED_TX_DEVICE       1
/* define the charactor device name */
#define ED_REC_DEVICE_NAME  "ed_rec"
#define ED_TX_DEVICE_NAME   "ed_tx"

#define ED_MTU              192
#define ED_MAGIC	     0x999
#define BUFFER_SIZE	     2048

#include <linux/config.h>

struct ed_device{
	int magic;
	char name[8]; 	
	int busy;
	unsigned char *buffer;
        #ifdef LINUX_24
	
        wait_queue_head_t rwait;
     
        #endif
	int mtu;
	spinlock_t lock;

	int tx_len;
        int rx_len;
        int buffer_size;

	struct file *file;
        ssize_t (*kernel_write)(const char *buffer,size_t length,int buffer_size);
};

/* this is the private data struct of ednet */
struct ednet_priv {
#ifdef LINUX_24
    struct net_device_stats stats;
#else
    struct enet_statistics stats;
#endif
    struct sk_buff *skb;
    spinlock_t lock;
};

#define ED_TIMEOUT 5
/* These are the flags in the statusword */


#endif

#include <linux/init.h>  
#include <linux/cdev.h>
#include <linux/module.h>  
#include <linux/version.h>  
#include <linux/netfilter_ipv4.h>  
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kthread.h>

#include "dscore.h"

#define DS_MODULE_NAME   "DS"
#define DS_DEVICE_COUNT        ( 1 )

static struct class *drv_class;
static dev_t sDevice;
static struct cdev sCharDevice;
static unsigned int sMajor = 0;
static struct task_struct *devinfo_update_task;

static int ds_open( struct inode* node, struct file* filp  )
{      
	return ds_real_open(&filp->private_data);
}      

static int ds_release( struct inode* node, struct file* filp  )
{      
	return ds_real_release(&filp->private_data);;
}      

int DtDevUpdateThread(void *pContext)
{
	while (1)
	{
		set_current_state(TASK_UNINTERRUPTIBLE);
		if(kthread_should_stop()) break;
		schedule_timeout(HZ*1+100);
		
		ds_update_dev();
	}

	return 0;
}

#if HAVE_UNLOCKED_IOCTL
static long ds_unlockedioctl( struct file* filp, unsigned int cmd, unsigned long arg  )
#else
static int ds_ioctl( struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg )
#endif
{
	return ds_real_ioctl(cmd, arg, &(filp->private_data));
}

static int ds_mmap(struct file *filp, struct vm_area_struct *vma)
{

	return ds_real_mmap(filp->private_data, vma);
}

static struct file_operations sFileOperations = {
	.open            = ds_open,
	.release         = ds_release,
#if HAVE_UNLOCKED_IOCTL
	.unlocked_ioctl  = ds_unlockedioctl,
#else
	.ioctl           = ds_ioctl,
#endif
	.mmap            = ds_mmap
};

unsigned int ds_hookfn( const struct nf_hook_ops* aOperations,  
		struct sk_buff *skb,  
		const struct net_device *in,  
		const struct net_device *out,  
		int (*okfn)(struct sk_buff *))  
{
	if (ds_filter_proc((void *)skb))
		return NF_DROP;
	else
		return NF_ACCEPT;
} 

static struct nf_hook_ops nfho = {  
	.hook     = (nf_hookfn *)ds_hookfn,  
	.pf       = PF_INET,  
	.hooknum  = NF_INET_PRE_ROUTING,  
	.priority = NF_IP_PRI_FIRST,  
};  

static int __init dsnf_init(void)  
{
	int err;
#if LINUX_VERSION_CODE < KERNEL_VERSION( 3, 13, 0 )
	return -1000;
#endif
	err = ds_init();	
	if (err != 0)
		return err;
	
	devinfo_update_task = kthread_create(DtDevUpdateThread, NULL, "devinfo_task");
	if(IS_ERR(devinfo_update_task)){
		printk("Unable to start kernel thread update info. \n");
		err = PTR_ERR(devinfo_update_task);
		devinfo_update_task = NULL;
		return err;
	}


	if( sMajor )
	{
		sDevice = MKDEV( sMajor, 0  );
		if( register_chrdev_region( sDevice, DS_DEVICE_COUNT, DS_MODULE_NAME  ) < 0  ) 
		{          
			sMajor = MAJOR( sDevice  );
			printk(" module is cannot get the major requested. New major is %d\n", sMajor);
			goto failed_ds_init;
			
		}          
	}              
	else           
	{              
		alloc_chrdev_region( &sDevice, 0, DS_DEVICE_COUNT, DS_MODULE_NAME  );
		sMajor = MAJOR( sDevice  );
	} 

	cdev_init( &sCharDevice, &sFileOperations  );
	sCharDevice.owner = THIS_MODULE;
	sCharDevice.ops = &sFileOperations;
	if( unlikely( cdev_add( &sCharDevice, sDevice, DS_DEVICE_COUNT  ) < 0  )  ) 
	{              
		printk("module is unable to create a char device node\n");
		goto failed_cdev_init;
	} 

	drv_class = class_create(THIS_MODULE, "dsfilter");
	device_create(drv_class, NULL, MKDEV(sMajor, 0), NULL, "dsfilter");

#if LINUX_VERSION_CODE >= KERNEL_VERSION( 4, 13, 0 )
	if (nf_register_net_hook(&init_net, &nfho))
#else
	if (nf_register_hook(&nfho))
#endif    
	{
		printk("nf_register_hook() failed\n");  
		goto failed_nf_hook;
    	}  

	wake_up_process(devinfo_update_task);

	return 0;  

failed_nf_hook:
	device_destroy(drv_class, MKDEV(sMajor, 0));
	class_destroy(drv_class);

failed_cdev_init:
	cdev_del(&sCharDevice);
	unregister_chrdev_region (sDevice, 1);
	
failed_ds_init:
	ds_uninit();
	return -1;
}  

static void __exit dsnf_exit(void)  
{
	printk ("remove ds net filter %ld\n", jiffies);

	if(devinfo_update_task){
		kthread_stop(devinfo_update_task);
		devinfo_update_task = NULL;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION( 4, 13, 0 )
	nf_unregister_net_hook( &init_net, &nfho );
#else
	nf_unregister_hook( &nfho );
#endif

	device_destroy(drv_class, MKDEV(sMajor, 0));
	class_destroy(drv_class);

	cdev_del(&sCharDevice);
	unregister_chrdev_region (sDevice, 1);

	ds_uninit();
}  

module_init(dsnf_init);  
module_exit(dsnf_exit);  
MODULE_AUTHOR("do3think");  
MODULE_LICENSE("GPL"); 

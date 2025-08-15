#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <asm/page.h>
#include <linux/mm.h>
#include <linux/kthread.h>

#include "control.h"

struct dt_device
{
    struct pci_dev* pci_dev;
    struct cdev cdev;
    dev_t devno;
}my_device;

struct class *dt_class;

static struct pci_device_id dt_pcie_tbl[] = {
    {0x10ee ,0x8888, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x1901, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x1902, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x1903, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x1920, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x1921, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x1922, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x1923, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0400, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0800, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0120, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0140, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0160, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0180, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0220, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0240, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0260, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0280, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0320, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0340, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0360, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0380, PCI_ANY_ID, PCI_ANY_ID, },
    {0x10ee ,0x0242, PCI_ANY_ID, PCI_ANY_ID, },
    {0, },
};

MODULE_DEVICE_TABLE (pci, dt_pcie_tbl);

static int dtpcie_probe (struct pci_dev *pdev,
                         const struct pci_device_id *ent)
{
	return dtpcie_real_probe((void **)&pdev);
}

static void dtpcie_remove (struct pci_dev *pdev)
{
	return dtpcie_real_remove((void **)&pdev);
}

static struct pci_driver dt_pcie_driver = {
    .name       = "DT2",
    .id_table   = dt_pcie_tbl,
    .probe      = dtpcie_probe,
    .remove     = dtpcie_remove,
};

static int dt_open(struct inode *inode, struct file *file)
{
	return dt_real_open((void **)&inode, (void **)&file);
}

static int dt_close(struct inode *inode, struct file *filp)
{
	return dt_real_close((void **)&filp);
}

static int dt_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return dt_real_mmap((void **)&filp, (void **)&vma);
}

static long dt_unlocked_ioctl(struct file *filp, uint32_t cmd, unsigned long arg)
{
	return dt_real_ioctl((void **)&filp, cmd, arg);
}

static struct file_operations dt_fops = {
    .owner   		=  THIS_MODULE,
    .open   		=  dt_open,
    .mmap           =  dt_mmap,
    .release 		=  dt_close,
    .unlocked_ioctl =  dt_unlocked_ioctl,
};

static int __init dtpcie_init_module (void)
{
    int ret;
    int i;

    printk("%s \n", __func__);

	itemsize = sizeof(spinlock_t);
	headsize = sizeof(wait_queue_head_t);
	mutexsize = sizeof(struct mutex);
	dmaaddrsize = sizeof(dma_addr_t);

	pMutex = kmalloc(mutexsize, GFP_KERNEL);
	if(!pMutex){
		printk("kmalloc pMutex failed!\n");
		return -1;
	}

	mutex_init((struct mutex*)pMutex);

	LogTextInit();
  	DtUninitAllPcieDevice();

    ret = pci_register_driver(&dt_pcie_driver);
    if (ret < 0) {
        printk("failed: pci_register_driver\n");
        return ret;
    }

    ret=alloc_chrdev_region(&my_device.devno,0,dt_get_device_num(),"dt");
    if (ret < 0) {
        printk("failed: register_chrdev_region\n");
        return ret;
    }

    cdev_init(&my_device.cdev, &dt_fops);
    ret = cdev_add(&my_device.cdev, my_device.devno, dt_get_device_num());
    if (ret < 0) {
        printk("faield: cdev_add\n");
        return ret;
    }
#ifdef VER_HIGH
	dt_class = class_create("dt_class");
#else
	dt_class = class_create(THIS_MODULE, "dt_class");
#endif
    
    for ( i = 0; i < dt_get_device_num(); i++)
    { 
        //device_create(dt_class, NULL, MKDEV(MAJOR(my_device.devno), i), NULL, dt_pcie_devname[i]);
        device_create(dt_class, NULL, MKDEV(MAJOR(my_device.devno), i), NULL, dt_get_dev_name(i));
    }

    return 0;
}

static void __exit dtpcie_cleanup_module (void)
{
    int i;

    printk("%s \n", __func__);

    for ( i = 0; i < dt_get_device_num(); i++)
        device_destroy(dt_class, MKDEV(MAJOR(my_device.devno), i));

    class_destroy(dt_class);

    cdev_del(&(my_device.cdev));
    unregister_chrdev_region(my_device.devno,dt_get_device_num());
    pci_unregister_driver(&dt_pcie_driver);

	kfree(pMutex);
}

module_init(dtpcie_init_module);
module_exit(dtpcie_cleanup_module);

MODULE_AUTHOR ("dothinkey <dothinkey@dothinkey.com>");
MODULE_DESCRIPTION ("dothinkey pcie card driver");
MODULE_LICENSE("Dual BSD/GPL");

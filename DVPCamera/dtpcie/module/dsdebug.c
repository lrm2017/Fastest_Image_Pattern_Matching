
/*net*/
#include <linux/ip.h>
#include <linux/inet.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include <linux/version.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/rtc.h>


#include "dsdebug.h"


/*定义这个结构体的原因是工作队列的回调函数需要用到dtpcie_info，那么就要通过回调函数的参数来找dtpcie_info。
*但是control.c是直接编译进动态库的，不能include内核相关的头文件，那么在control.c就不能定义struct work_struct变量。
*如果在dtpcie_info里面用void*的方式定义struct work_struct，通过struct work_struct变量找不到dtpcie_info。那么就只能
*通过这个结构体，通过struct work_struct变量找到dtpcie_info。
*/
typedef struct my_dtpcie_s{
	void 			* pcie_info;
	struct work_struct grab_work;
}my_dtpcie;

my_dtpcie * g_my_pcie[8];

bool dt_is_udp(const void *);
bool dt_is_tcp(const void *);
unsigned char *get_src_mac(const void *);
unsigned char *get_dst_mac(const void *);
unsigned short get_src_port(const void *);
unsigned short get_dst_port(const void *);
unsigned int get_data_len(const void *);
void *rebuild_buf_data(void *);

void *dt_memcpy(void *, const void *, size_t);
int dt_memcmp(const void *, const void *, size_t);
char *dt_strcpy(char *, const char *);
void *dt_memmove(void *, const void *, size_t);
size_t dt_strlen(const char *);
void *dt_memset(void *, int, __kernel_size_t);
void *dt_kmalloc(size_t);
void dt_kfree(void *);

void *dt_vmalloc(unsigned long);
void dt_vfree(const void *);

size_t dt_get_spinlock_size(void);
void dt_lock_spinlock_irqsave(void *, unsigned long *);
void dt_unlock_spinlock_irqrestore(void *, unsigned long);
size_t dt_get_waitqueue_size(void);
void dt_spin_lock_init(void *);
void dt_init_waitqueue_head(void *);

void dt_wake_up_interruptible(void *);
long dt_wait_event_interruptible_timeout(void *, unsigned int *, unsigned int);

long dt_copy_from_user(void *to, const void *from, unsigned long n);
long dt_copy_to_user(void *to, const void *from, unsigned long n);

int dt_remap(void *, void *, void **);

int dt_print(const char *fmt, ...);
void ShowMem(void *p, unsigned int len);


char * dt_strstr(const char *str1, const char *str2)
{
    char *cp = (char *)str1;
    char *s1, *s2;

    if (!*str2)
        return((char *)str1);

    while (*cp)
    {
        s1 = cp;
        s2 = (char *)str2;

        while (*s2 && !(*s1 - *s2))
            s1++, s2++;

        if (!*s2)
            return(cp);

        cp++;
    }

    return NULL;
}

size_t dt_get_struct_file_size(void)
{
	return sizeof(struct file*);
}

void * dt_filp_open(const char *name, int flags, unsigned short mode)
{
	return (void *)filp_open(name, flags, mode);
}

int dt_filp_close(void *filp, void* id)
{
	return filp_close((struct file*)filp, id);
}

#if 0

void dt_set_fs(unsigned long inputFs)
{
	mm_segment_t fs;
	fs.seg = inputFs;

	set_fs(fs);
}

unsigned long dt_get_fs(void)
{
	mm_segment_t fs = get_fs();

	return fs.seg;
}


void dt_set_kernel_ds(void)
{
	set_fs(KERNEL_DS);
}

ssize_t dt_vfs_read(void * file, char *buf, size_t count, long long *pos)
{
	return vfs_read((struct file *)file, buf, count, pos);
}

ssize_t dt_vfs_write(void * file, const char *buf, size_t count, long long *pos)
{
	return vfs_write((struct file *)file, buf, count, pos);
}
#else

ssize_t dt_kernel_read(void * file, char *buf, size_t count, long long *pos)
{
	return kernel_read((struct file *)file, buf, count, pos);
	//return kernel_read((struct file *)file, *pos, buf, count);
}

ssize_t dt_kernel_write(void * file, const char *buf, size_t count, long long *pos)
{
	return kernel_write((struct file *)file, buf, count, pos);
	//return kernel_write((struct file *)file, buf, count, *pos);
}

#endif

//void dt_rtc_time_to_tm(long long time, void **p)
//{
//	struct rtc_time *tm = *p;
//	rtc_time_to_tm(time, tm);
//}

int dt_get_date_buf(char *buf)
{
    unsigned long cur_usec;
    struct rtc_time tm;
    ktime_t kt;

    kt = ktime_get_real();
    tm = rtc_ktime_to_tm(kt);

    cur_usec = (unsigned long)ktime_to_us(kt);
    cur_usec = cur_usec%1000000;

    snprintf(buf, sizeof("[%04d-%02d-%02d %02d:%02d:%02d.%06lu] "), "[%04d-%02d-%02d %02d:%02d:%02d.%06lu] ",
	tm.tm_year+1900,
        tm.tm_mon+1,
        tm.tm_mday,
        tm.tm_hour+8,
        tm.tm_min,
        tm.tm_sec,
        cur_usec
	);

    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION( 5, 0, 0 )
long long dt_get_cur_time(void)
{
	struct timeval tv;

	do_gettimeofday(&tv);
	return tv.tv_usec + tv.tv_sec * 1000000;
}
#else
long long dt_get_cur_time(void)
{
	struct timespec64 tv64;

	ktime_get_real_ts64(&tv64);
	return tv64.tv_nsec / 1000 + tv64.tv_sec * 1000000;
}
#endif

void dt_set_current_task_uninterruptible(void)
{
	set_current_state(TASK_UNINTERRUPTIBLE);
}

bool dt_kthread_should_stop(void)
{
	return kthread_should_stop();
}

void dt_schedule_timeout_add_1hz(signed long timeout)
{
	schedule_timeout(HZ*1+ timeout);
}

int dt_sprintf(char *str, const char *fmt, ...)
{
	va_list args;
	char dsdbg_buf[512];

	va_start(args, fmt);
	vsnprintf(dsdbg_buf, sizeof(dsdbg_buf), fmt, args);
	va_end(args);

	//printk("dt_sprintf %s\n", dsdbg_buf);

	return sprintf(str, dsdbg_buf);
}

void *dt_memcpy(void *dest, const void *src, size_t count)
{
	return memcpy(dest, src, count);
}

int dt_memcmp(const void *cs, const void *ct, size_t count)
{
	return memcmp(cs, ct, count);
}

char *dt_strcpy(char *dest, const char *src)
{
	return strcpy(dest, src);
}

void *dt_memmove(void *dest, const void *src, size_t count)
{
	return memmove(dest, src, count);
}

size_t dt_strlen(const char *s)
{
	return strlen(s);
}

void *dt_memset(void *s, int c, __kernel_size_t count)
{
	return memset(s, c, count);
}

bool dt_unlikely(int rc)
{
	return unlikely(rc);
}

bool DT_IS_ERR(void *p)
{
	return IS_ERR(p);
}

bool DT_PTR_ERR(void *p)
{
	return PTR_ERR(p);
}

void dt_pcie_set_private_data(void **pfile, void *pritvate_info)
{
	struct file *file = (struct file *)(*pfile);

	file->private_data = pritvate_info;
}

void dt_pcie_get_private_data(void **pfile, void **pritvate_info)
{
	struct file *file = (struct file *)(*pfile);

	*pritvate_info = file->private_data;
}

void dt_pci_set_drvdata(void **pdata, void *pcie_info)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

	pci_set_drvdata(pdev, pcie_info);
}

void dt_pci_get_drvdata(void **pdata, void **pcie_info)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

	*pcie_info = pci_get_drvdata (pdev);
}

int dt_pci_enable_device(void **pdata)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

	return pci_enable_device (pdev);
}

void dt_pci_disable_device(void **pdata)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

	pci_disable_device (pdev);
}

unsigned long dt_pci_resource_start(void **pdata, unsigned int idx)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

	return pci_resource_start(pdev, idx);
}

unsigned long dt_pci_resource_end(void **pdata, unsigned int idx)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

	return pci_resource_end (pdev, idx);
}

unsigned long dt_pci_resource_flags(void **pdata, unsigned int idx)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

	return pci_resource_flags (pdev, idx);
}

unsigned long dt_pci_resource_len(void **pdata, unsigned int idx)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

	return pci_resource_len (pdev, idx);
}

int dt_pci_request_regions(void **pdata, const char *name)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

	return pci_request_regions (pdev, name);
}

void dt_pci_release_regions(void **pdata)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

	pci_release_regions (pdev);
}

void * dt_ioremap(unsigned long mmio_start,unsigned long mmio_len)
{
	return ioremap(mmio_start, mmio_len);
}

void dt_iounmap(void *io_addr)
{
	iounmap(io_addr);
}

void dt_pci_set_master(void **pdata)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

	pci_set_master(pdev);
}

unsigned long dt_get_irqf_shared_flags(void)
{
	return IRQF_SHARED;
}

int dt_request_irq(void **pdata, pcie_interrupt fp, unsigned long flags, const char *name, void *dev)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);
	int rc;

    rc = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_MSI);

    pdev->irq = pci_irq_vector(pdev, 0);
	return request_irq(pdev->irq, fp, flags, name, dev);
}

int dt_pci_set_dma_mask(void **pdata, uint32_t mask)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

 #ifdef VER_HIGH

	return 0;
 #else
        return pci_set_dma_mask(pdev, DMA_BIT_MASK(mask));
 #endif

}

void dt_pci_set_consistent_dma_mask(void **pdata, uint32_t mask)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);


#ifdef VER_HIGH 
        
#else    
       	pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(mask));
#endif
	
}

void dt_free_irq(void **pdata, void *dev)
{
	struct pci_dev *pdev = (struct pci_dev *)(*pdata);

	free_irq (pdev->irq, dev);
	pci_free_irq_vectors(pdev);
}

unsigned long dt_get_vma_size(void **pvma)
{
	struct vm_area_struct * vma = (struct vm_area_struct *)(*pvma);
	unsigned long size = vma->vm_end - vma->vm_start;

	return size;
}

void dt_get_vma_start(void **pvma, void **pStart)
{
	struct vm_area_struct * vma = (struct vm_area_struct *)(*pvma);
	*pStart = (void*)vma->vm_start;
}

size_t dt_get_dma_addr_size(void)
{
	return sizeof(dma_addr_t);
}

#if 0
int dt_remap_pfn_range(void **pvma, uint32_t dma_buffer_size, u64 mdl, int i)
{
	int ret = 0;
	struct vm_area_struct * vma = (struct vm_area_struct *)(*pvma);
	//dma_addr_t mdl = (dma_addr_t)(*pMdl);
//	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	ret = remap_pfn_range(vma, vma->vm_start + i*dma_buffer_size, 
					((dma_addr_t)mdl) >> PAGE_SHIFT, dma_buffer_size, vma->vm_page_prot);
	if (ret)
	{
		printk("%s: remap_pfn_range failed at [0x%lx  0x%lx]\n",
			__func__, vma->vm_start+i*dma_buffer_size, vma->vm_end);
		return ret;
	}
	else
	{
		printk("%s: map 0x%llx to 0x%lx, size: 0x%x\n", __func__, mdl,
			vma->vm_start+i*dma_buffer_size, dma_buffer_size);
	}

	return ret;
}
#else
int dt_remap_pfn_range(void* pdata,void*sysaddr,void **pvma, uint32_t dma_buffer_size, u64 mdl, int i)
{
	int ret = 0;
	unsigned long long page;
	struct vm_area_struct * vma = (struct vm_area_struct *)(*pvma);
	
	if(sysaddr == NULL)
	  return -1;
	  
	page = virt_to_phys(sysaddr);

	//printk("%s: sysaddr 0x%p  mdl 0x%llx\n", __func__,sysaddr, mdl);


	ret = remap_pfn_range(vma, vma->vm_start + i*dma_buffer_size, 
					page >> PAGE_SHIFT, dma_buffer_size, vma->vm_page_prot);

 	if (ret)
 	{
 		printk("%s: remap_pfn_range failed at [0x%lx  0x%lx]\n",
 			__func__, vma->vm_start+i*dma_buffer_size, vma->vm_end);
 		return ret;
 	}
 	else
 	{
 		//printk("%s: map pageaddr 0x%llx to 0x%lx, size: 0x%x\n", __func__, page,
 		//	vma->vm_start+i*dma_buffer_size, dma_buffer_size);
 	}

	return ret;
}
#endif


void * dt_dma_alloc_coherent(void *pdata, uint32_t dma_buffer_size, u64 *pMdl)
{
	struct pci_dev *pdev = (struct pci_dev *)pdata;

	//return dma_alloc_coherent(pdev == NULL ? NULL : &(pdev->dev), dma_buffer_size, (dma_addr_t *)pMdl, GFP_KERNEL);

#if 1
//	void * sysaddr = kmalloc(dma_buffer_size,GFP_KERNEL | __GFP_NORETRY );
	void * sysaddr = kmalloc(dma_buffer_size,GFP_KERNEL);
	if(sysaddr == NULL){
	  printk("malloc buffer failed\n ");
	   return NULL;
	}
	*pMdl = dma_map_single(&(pdev->dev),sysaddr,dma_buffer_size,DMA_FROM_DEVICE);
	//*pMdl = dma_map_single(&(pdev->dev),sysaddr,dma_buffer_size,DMA_BIDIRECTIONAL);
	if(dma_mapping_error(&(pdev->dev),*pMdl)){
 		printk("dma_mapping_error failed\n ");
		//dma_unmap_single(&(pdev->dev),(dma_addr_t) *pMdl,dma_buffer_size,DMA_FROM_DEVICE);
		kfree(sysaddr);
		return NULL;
	}
	return sysaddr;
#endif

}

void dt_pci_free_consistent(void *pdata, uint32_t dma_buffer_size, void *sysAddr, u64 mdl)
{
	struct pci_dev *pdev = (struct pci_dev *)pdata;
	//dma_addr_t mdl = (dma_addr_t)(*pMdl);
//#ifdef VER_HIGH
//	dma_free_coherent(pdev == NULL ? NULL : &(pdev->dev), dma_buffer_size,sysAddr,(dma_addr_t *)mdl);
//#else
//	pci_free_consistent(pdev, dma_buffer_size, sysAddr, (dma_addr_t) mdl);
//#endif

	dma_unmap_single(&(pdev->dev),(dma_addr_t) mdl,dma_buffer_size,DMA_FROM_DEVICE);
	//dma_unmap_single(&(pdev->dev),(dma_addr_t) mdl,dma_buffer_size,DMA_BIDIRECTIONAL);
	kfree(sysAddr);
	
	


}

int dt_get_pcie_idx(void **pinode)
{
	struct inode *inode = (struct inode *)(*pinode);

	printk("driver: dt_open %x\n", inode->i_rdev); /* i_rdev i_cdev->dev 这2个值一样 可以判断出来打开的是哪个设备 以此来区分不同的采集卡 */

	return inode->i_rdev & 0x7;
}

size_t dt_get_waitqueue_size(void)
{
	return sizeof(wait_queue_head_t);
}

void dt_init_waitqueue_head(void *pwq)
{
	init_waitqueue_head((wait_queue_head_t *)pwq);
}

void dt_wake_up_interruptible(void *pwq)
{
	wake_up_interruptible((wait_queue_head_t *)pwq);
}

long dt_wait_event_interruptible_timeout(void *pwq, unsigned int *flag, unsigned int timeout)
{
	long ret;
	ret = wait_event_interruptible_timeout(*(wait_queue_head_t *)(pwq), (*flag != 0), timeout);
	return ret;
}

size_t dt_get_spinlock_size(void)
{
	return sizeof(spinlock_t);
}

void dt_lock_spinlock(void *plock)
{
	spin_lock(plock);
}

void dt_unlock_spinlock(void *plock)
{
	spin_unlock(plock);
}

void dt_lock_spinlock_irqsave(void *plock, unsigned long *Irql)
{
	unsigned long flag;
	spin_lock_irqsave(plock, flag);
	*Irql = flag;
}

void dt_unlock_spinlock_irqrestore(void *plock, unsigned long flag)
{
	spin_unlock_irqrestore(plock, flag);
}

void dt_spin_lock_init(void *plock)
{
	spin_lock_init((spinlock_t *)(plock));
}

unsigned int dt_get_wq_unbound_flag(void)
{
	return WQ_UNBOUND;
}

unsigned int dt_get_wq_mem_reclaim_flag(void)
{
	return WQ_MEM_RECLAIM;
}

unsigned int dt_get_wq_sysfs_flag(void)
{
	return WQ_SYSFS;
}

void* dt_get_work_struct(uint32_t dev_id)
{
	return (void *)(&g_my_pcie[dev_id]->grab_work);
}

void* dt_alloc_workqueue(const char*name, unsigned int flags, unsigned int max_active)
{
	return (void*)(alloc_workqueue(name, flags, max_active));
}

void DT_INIT_WORK(uint32_t idx, grab_image fp)
{
	work_func_t func = (work_func_t)fp;

	//printk("%s: %u\n", __func__, idx);

	INIT_WORK(&g_my_pcie[idx]->grab_work, func);
}

void dt_flush_work(uint32_t idx)
{
	//printk("%s: %u\n", __func__, idx);
	flush_work(&g_my_pcie[idx]->grab_work);
}

void dt_queue_work(void *pWq,  uint32_t idx)
{
	//printk("%s: idx:%u pcie_info:%x %x work: %x\n", __func__, idx, &(g_my_pcie[idx]->pcie_info),
	//	g_my_pcie[idx]->pcie_info, &g_my_pcie[idx]->grab_work);
	queue_work((struct workqueue_struct *)pWq, &g_my_pcie[idx]->grab_work);
}

void dt_destroy_workqueue(void * pWq)
{
	destroy_workqueue((struct workqueue_struct *)pWq);
}

//size_t ds_get_workqueue_struct_size(void)
//{
//	return sizeof(struct workqueue_struct);
//}

//size_t ds_get_work_struct_size(void)
//{
//	return sizeof(ds_work_struct_t);
//}

size_t dt_get_mutex_size(void)
{
	return sizeof(struct mutex);
}

void dt_mutex_init(void *pMutex)
{
	mutex_init((struct mutex*)pMutex);
}

void dt_mutex_lock(void *pMutex)
{
	mutex_lock((struct mutex*)pMutex);
}

void dt_mutex_unlock(void *pMutex)
{
	mutex_unlock((struct mutex*)pMutex);
}

void *dt_kthread_create(dev_update fp, void *pdate, const char * name)
{
	return (void *)kthread_create(fp, pdate, name);
}

void dt_kthread_stop(void * pTaskStruct)
{
	kthread_stop((struct task_struct *)pTaskStruct);
}

int dt_wake_up_process(void * pTaskStruct)
{
	return wake_up_process((struct task_struct *)pTaskStruct);
}

int dt_init_my_pcie(void *pcie_info, uint32_t idx)
{
	my_dtpcie * pdev =   dt_kmalloc(sizeof(my_dtpcie));
	if(!pdev)
		return -1;

	pdev->pcie_info = pcie_info;

	g_my_pcie[idx] = pdev;

	return 0;
}

void dt_uninit_my_pcie(uint32_t idx)
{
	dt_kfree(g_my_pcie[idx]);
}

void *dt_kmalloc(size_t size)
{
	return kmalloc(size, GFP_KERNEL);
}

void dt_kfree(void *pbuf)
{
	kfree(pbuf);
}

void *dt_vmalloc(unsigned long size)
{
	return vmalloc_user(size);
}

void dt_vfree(const void *addr)
{
	vfree(addr);
}

void ShowMem(void *p, unsigned int len)
{
	uint32_t i, j, k, l;
	char buf[512];
	char buf1[16];

	k = len / 16;
	l = len % 16;

	printk("len=%d\n", len);

	for (i = 0; i < k; i++)
	{
		buf[0] = 0;
		for (j = 0; j < 16; j++)
		{
			sprintf(buf1, "%02x ", ((uint8_t *)p)[i * 16 + j]);
			strcat(buf, buf1);
		}
		printk("%s\n", buf);
	}

	buf[0] = 0;
	for (j = 0; j < l; j++)
	{
		sprintf(buf1, "%02x ", ((uint8_t *)p)[i * 16 + j]);
		strcat(buf, buf1);
	}
	if (l)
		printk("%s\n", buf);
}

int dt_print_buf( char *outBuffer, const char *fmt, ...)
{
	va_list args;
	char dsdbg_buf[512];

	va_start(args, fmt);
	vsnprintf(dsdbg_buf, sizeof(dsdbg_buf), fmt, args);
	va_end(args);

	memcpy(outBuffer, dsdbg_buf, sizeof(dsdbg_buf));
	//printk("%s\n", outBuffer);

	return 0;
}

int dt_print( const char *fmt, ...)
{
	va_list args;
	char dsdbg_buf[640];

	va_start(args, fmt);
	vsnprintf(dsdbg_buf, sizeof(dsdbg_buf), fmt, args);
	va_end(args);

	return printk("%s", dsdbg_buf);
}

int dt_pcie_readl(void *iomem, uint32_t reg)
{
	uint32_t value = readl(iomem + reg*4);

    return value;
}

void dt_pcie_writel(void *iomem, uint32_t reg, uint32_t val)
{
	return writel(val, iomem + reg*4);
}

bool dt_is_udp(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff *)(pbuf);
	struct iphdr *ipHeader = ip_hdr(skb);
	return (ipHeader->protocol == IPPROTO_UDP);
}

bool dt_is_tcp(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff *)(pbuf);
	struct iphdr *ipHeader = ip_hdr(skb);
	return (ipHeader->protocol == IPPROTO_TCP);
}

unsigned char *get_src_mac(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff *)(pbuf);
	return skb_mac_header(skb);
}

unsigned char *get_dst_mac(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff *)(pbuf);
	return (skb_mac_header(skb) + 6);
}

unsigned short get_src_port(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff *)(pbuf);
	struct udphdr *udpHeader = udp_hdr(skb);
	return be16_to_cpu(udpHeader->source);
}

unsigned short get_dst_port(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff *)(pbuf);
	struct udphdr *udpHeader = udp_hdr(skb);
	return be16_to_cpu(udpHeader->dest);
}

unsigned int get_data_len(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff *)(pbuf);
	return skb->len;
}

void *rebuild_buf_data(void *pbuf)
{
	struct sk_buff *skb = (struct sk_buff *)(pbuf);
	skb_linearize(skb);
	return skb->data;
}

long dt_copy_from_user(void *to, const void *from, unsigned long n)
{
	return copy_from_user(to, (const void __user *)from, n);
}

long dt_copy_to_user(void *to, const void *from, unsigned long n)
{
	return copy_to_user((void __user *)to, from, n);
}

int dt_remap(void *pvma, void *kaddr, void **uaddr)
{
	int ret = 0;
	struct vm_area_struct *vma = (struct vm_area_struct *)(pvma);

#ifdef VER_HIGH

#else
        vma->vm_flags |= VM_IO;
        vma->vm_flags |= VM_LOCKED;

#endif

	ret = remap_vmalloc_range(vma, kaddr, vma->vm_pgoff);
	if (ret == 0)
	{
		*uaddr = (void *)vma->vm_start;
	}
	else
	{
		*uaddr = NULL;
	}
	return ret;
}

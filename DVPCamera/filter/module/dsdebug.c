
/*net*/
#include <linux/ip.h>  
#include <linux/inet.h>  
#include <linux/tcp.h>
#include <linux/udp.h>



#include <linux/vmalloc.h>
#include <linux/slab.h>

#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/string.h>

bool   ds_is_udp(const void *);
bool   ds_is_tcp(const void *);
unsigned char *get_src_mac(const void *);
unsigned char *get_dst_mac(const void *);
unsigned short get_src_port(const void *);
unsigned short get_dst_port(const void *);
unsigned int   get_data_len(const void *);
void*   rebuild_buf_data(void *);

void * dsmemcpy(void *, const void *, size_t );
int    dsmemcmp(const void *, const void *, size_t );
char * dsstrcpy(char *, const char *);
void * dsmemmove(void *, const void *, size_t );
size_t dsstrlen(const char *);
void * dsmemset(void *, int, __kernel_size_t );
void * dskmalloc(size_t );
void   dskfree(void *);

void * dsvmalloc(unsigned long );
void   dsvfree(const void *);

size_t ds_get_spinlock_size(void);
void   ds_lock_spinlock(void *, unsigned long *);
void   ds_unlock_spinlock(void*, unsigned long);
size_t ds_get_waitqueue_size(void);
void   ds_spin_lock_init(void *); 
void   ds_init_waitqueue_head(void *); 

void   ds_wake_up(void *);
long   ds_wait_event(void *, unsigned int *, unsigned int);

long    ds_copy_from_user(void *to, const void * from, unsigned long n);
long    ds_copy_to_user(void *to, const void *from, unsigned long n);

int    ds_remap(void *, void *, void **);

int dsprint(const char *fmt, ...);
void ShowMem(void* p,unsigned int len);

void *dsmemcpy(void *dest, const void *src, size_t count)
{
	return memcpy(dest, src, count);
}

int dsmemcmp(const void *cs, const void *ct, size_t count)
{
	return memcmp(cs, ct, count);
}

char *dsstrcpy(char *dest, const char *src)
{
	return strcpy(dest, src);
}

void   *dsmemmove(void *dest, const void *src, size_t count)
{
	return memmove(dest, src, count);
}

size_t dsstrlen(const char *s)
{
	return strlen(s);
}

void * dsmemset(void *s, int c, __kernel_size_t count)
{
	return memset(s, c, count);
}

size_t ds_get_waitqueue_size(void)
{
	return sizeof (wait_queue_head_t);
}

void   ds_init_waitqueue_head(void *pwq)
{
	init_waitqueue_head((wait_queue_head_t *)pwq);
}

void   ds_wake_up(void *pwq)
{
	wake_up_interruptible((wait_queue_head_t *)pwq);
}

long   ds_wait_event(void *pwq, unsigned int *flag, unsigned int timeout)
{
	long ret;
	ret=wait_event_interruptible_timeout(*(wait_queue_head_t*)(pwq), (*flag != 0), timeout);
	return ret;
}

size_t ds_get_spinlock_size(void)
{
	return sizeof (spinlock_t);
}

void ds_lock_spinlock(void *plock, unsigned long *Irql)
{
	unsigned long flag;
	spin_lock_irqsave (plock, flag);
	*Irql = flag;
}

void ds_unlock_spinlock(void* plock, unsigned long flag)
{
	spin_unlock_irqrestore(plock, flag);
}

void ds_spin_lock_init(void *plock)
{
	spin_lock_init((spinlock_t *)(plock));
}

void * dskmalloc(size_t size)
{
	return kmalloc(size, GFP_KERNEL);
}

void   dskfree(void *pbuf)
{
	kfree(pbuf);
}

void * dsvmalloc(unsigned long size)
{
	return vmalloc_user(size); 
}

void   dsvfree(const void *addr)
{
	vfree(addr);
}

void ShowMem(void* p,unsigned int len)
{
	uint32_t i, j, k, l;
	char buf[512];
	char buf1[16];
	
	k = len / 16;
	l = len % 16;
	
	printk("len=%d\n", len);

	for( i=0; i<k; i++)
	{
		buf[0] = 0;
		for ( j = 0; j < 16; j++)
		{
			sprintf (buf1, "%02x ", ((uint8_t*)p)[i*16+j]);
			strcat (buf, buf1);
		}
		printk ("%s\n", buf);
	}

	buf[0] = 0;
	for (j = 0; j < l; j++)
	{
		sprintf (buf1, "%02x ", ((uint8_t*)p)[i*16+j]);
		strcat (buf, buf1);
	}
	if (l)
		printk ("%s\n", buf);
}

int dsprint(const char *fmt, ...)
{
	va_list args;
	char dsdbg_buf[512];

	va_start(args, fmt);
	vsnprintf(dsdbg_buf, sizeof(dsdbg_buf), fmt, args);
	va_end(args);

	return   printk ("%s", dsdbg_buf);
}

bool   ds_is_udp(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff*)(pbuf);
	struct iphdr    *ipHeader = ip_hdr(skb);
	return (ipHeader->protocol == IPPROTO_UDP);
}

bool   ds_is_tcp(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff*)(pbuf);
	struct iphdr    *ipHeader = ip_hdr(skb);
	return (ipHeader->protocol == IPPROTO_TCP);
}

unsigned char *get_src_mac(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff*)(pbuf);
	return skb_mac_header(skb);
}

unsigned char *get_dst_mac(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff*)(pbuf);
	return (skb_mac_header(skb)+6);
}

unsigned short get_src_port(const void *pbuf)
{
	const struct sk_buff *skb  = (const struct sk_buff*)(pbuf);
	struct udphdr   *udpHeader = udp_hdr(skb); 
	return be16_to_cpu(udpHeader->source);
}

unsigned short get_dst_port(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff*)(pbuf);
	struct udphdr   *udpHeader = udp_hdr(skb); 
	return be16_to_cpu(udpHeader->dest);
}

unsigned int   get_data_len(const void *pbuf)
{
	const struct sk_buff *skb = (const struct sk_buff*)(pbuf);
	return skb->len;
}

void *rebuild_buf_data(void *pbuf)
{
	struct sk_buff *skb = (struct sk_buff*)(pbuf);
	skb_linearize(skb);
	return skb->data;
}

long    ds_copy_from_user(void *to, const void * from, unsigned long n)
{
	return copy_from_user (to, (const void __user *)from, n);
}

long    ds_copy_to_user(void *to, const void *from, unsigned long n)
{
	return copy_to_user ((void __user * )to, from, n);
}

int    ds_remap(void *pvma, void *kaddr, void **uaddr)
{
	int ret = 0;
	struct vm_area_struct *vma = (struct vm_area_struct*)(pvma);

	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_LOCKED;

	ret = remap_vmalloc_range(vma, kaddr, vma->vm_pgoff );
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

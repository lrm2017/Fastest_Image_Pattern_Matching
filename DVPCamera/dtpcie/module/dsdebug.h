#ifndef __DSDEBUG_H_
#define __DSDEBUG_H_

#include <linux/irqreturn.h>


/* 调试打印等级(Debug Level) */
/* 调试打印错误信息 */
#define DL_ERROR	(1<<0)

/* 调试打印警告信息 */
#define DL_WARN		(1<<1)

/* 调试打印提示信息 */
#define DL_INFO		(1<<2)

/* 调试打印过程信息 */
#define DL_PROC		(1<<4)


/* 报表，这些信息将收集到特定的界面中 */
#define DL_REPORT	(1<<28)

/* 强制方式，不管是否使能过，强制记录到文件 */
#define DL_FORCE    (1<<29)

/* 直接DUMP文本信息，不做格式化处理 */
#define DL_DUMP		(1<<30)

/* 立即通知到用户，例如：给出弹框提示 */
#define DL_MSG		(1<<31)


/* 调试信息分类(Debug Part) */
/* 默认或未分类的调试信息*/
#define DP_DEFAULT	(0)

///* 调试设备版本(Version)相关*/
#define DP_ENUM     (1)

/* 调试控制相关 */
#define DP_CTRL		(2)

/* 调试通讯(USB/Ethernet)相关 */
#define DP_LINK		(3)

/* 调试I2C总线相关 */
#define DP_I2C		(4)

/* 调试SPI总线相关 */
#define DP_SPI		(5)

/* 调试EEPROM相关 */
#define DP_E2P		(6)

/* 调试系统相关 */
#define DP_SYS		(7)

/* 调试采集相关 */
#define DP_GRAB		(8)
#define DP_STREAM   DP_GRAB

/* 调试GPIO */
#define DP_GPIO		(9)

/* 调试FPGA */
#define DP_FPGA		(10)

/* 调试MCU */
#define DP_MCU		(11)

/* 调试ROM */
#define DP_ROM		(12)

/* 调试电源相关 */
#define DP_POWER	(13)

/* 调试API相关 */
#define DP_API		(14)

/* 调试ISP相关 */
#define DP_IMGP		(15)

/* 调试Sensor相关 */
#define DP_SEN		(16)

/* 调试Trigger相关 */
#define DP_TRIG		(17)

/* 设备端发出的调试信息 */
#define DP_RMT      (18)

/* 帧信息相关 */
#define DP_FRAME	(19)

/* 参数相关 */
#define DP_PARAM    (20)

/* 初始化相关 */
#define DP_INIT     (21)

#define DP_IOCTL	(22)

/* 用户应用系统调试相关 */
#define DP_APP      (31)


typedef irqreturn_t (*pcie_interrupt) (int ,  void *);
typedef void (*grab_image) (void *);
typedef int (*dev_update) (void *);

extern long long dt_get_cur_time(void);
extern void dt_set_current_task_uninterruptible(void);
extern bool dt_kthread_should_stop(void);
extern void dt_schedule_timeout_add_1hz(signed long timeout);
//extern unsigned int dt_real_get_device_num(void);

extern bool dt_is_udp(const void *);
extern bool dt_is_tcp(const void *);
//extern void TraceEvents (uint32_t TraceEventsLevel, uint32_t TraceEventsFlag, char *DebugMessage, ...);

extern unsigned char *get_src_mac(const void *);
extern unsigned char *get_dst_mac(const void *);
extern unsigned short get_src_port(const void *);
extern unsigned short get_dst_port(const void *);
extern unsigned int get_data_len(const void *);
extern void *rebuild_buf_data(void *);

extern int dt_sprintf(char *str, const char *fmt, ...);
extern void *dt_memcpy(void *, const void *, size_t);
extern int dt_memcmp(const void *, const void *, size_t);
extern char *dt_strcpy(char *, const char *);
extern void *dt_memmove(void *, const void *, size_t);
extern size_t dt_strlen(const char *);
extern void *dt_memset(void *, int, __kernel_size_t);
extern void *dt_kmalloc(size_t);
extern void dt_kfree(void *);
extern char * dt_strstr(const char *str1, const char *str2);

extern bool dt_unlikely(int rc);
extern bool DT_IS_ERR(void *p);
extern bool DT_PTR_ERR(void *p);

extern void *dt_vmalloc(unsigned long);
extern void dt_vfree(const void *);

extern int dt_pcie_readl(void *iomem, uint32_t reg);
extern void dt_pcie_writel(void *iomem, uint32_t reg, uint32_t val);

extern int dt_get_pcie_idx(void **pinode);
extern void dt_pcie_set_private_data(void **pfile, void *pritvate_info);
extern void dt_pcie_get_private_data(void **pfile, void **pritvate_info);

extern void dt_pci_set_drvdata(void **pdata, void *pcie_info);
extern void dt_pci_get_drvdata(void **pdata, void **pcie_info);
extern int dt_pci_enable_device(void **pdata);
extern void dt_pci_disable_device(void **pdata);

extern unsigned long dt_pci_resource_start(void **pdata, unsigned int idx);
extern unsigned long dt_pci_resource_end(void **pdata, unsigned int idx);
extern unsigned long dt_pci_resource_flags(void **pdata, unsigned int idx);
extern unsigned long dt_pci_resource_len(void **pdata, unsigned int idx);

extern int dt_pci_request_regions(void **pdata, const char *name);
extern void dt_pci_release_regions(void **pdata);
extern void * dt_ioremap(unsigned long mmio_start,unsigned long mmio_len);
extern void dt_iounmap(void *io_addr);

extern void dt_pci_set_master(void **pdata);

extern unsigned long dt_get_irqf_shared_flags(void);
extern int dt_request_irq(void **pdata, pcie_interrupt fp, unsigned long flags, const char *name, void *dev);
extern void dt_free_irq(void **pdata, void *dev);

extern int dt_pci_set_dma_mask(void **pdata, uint32_t mask);
extern void dt_pci_set_consistent_dma_mask(void **pdata, uint32_t mask);

extern size_t dt_get_dma_addr_size(void);
extern unsigned long dt_get_vma_size(void **pvma);
extern void dt_get_vma_start(void **pvma, void **pStart);
//extern int dt_remap_pfn_range(void **pvma, uint32_t dma_buffer_size, u64 mdl, int i);
extern int dt_remap_pfn_range(void* pdata,void*sysaddr,void **pvma, uint32_t dma_buffer_size, u64 mdl, int i);
extern void * dt_dma_alloc_coherent(void *pdata, uint32_t dma_buffer_size, u64 *pMdl);
extern void dt_pci_free_consistent(void *pdata, uint32_t dma_buffer_size, void *sysAddr,  u64 mdl);

extern size_t dt_get_spinlock_size(void);
extern void dt_spin_lock_init(void *);
extern void dt_lock_spinlock(void *plock);
extern void dt_unlock_spinlock(void *plock);
extern void dt_lock_spinlock_irqsave(void *, unsigned long *);
extern void dt_unlock_spinlock_irqrestore(void *, unsigned long);

extern size_t dt_get_mutex_size(void);
extern void dt_mutex_init(void *pMutex);
extern void dt_mutex_lock(void *pMutex);
extern void dt_mutex_unlock(void *pMutex);

extern unsigned int dt_get_wq_unbound_flag(void);
extern unsigned int dt_get_wq_mem_reclaim_flag(void);
extern unsigned int dt_get_wq_sysfs_flag(void);

extern size_t dt_get_waitqueue_size(void);
extern void dt_init_waitqueue_head(void *);
extern void* dt_alloc_workqueue(const char*name, unsigned int flags, unsigned int max_active);
extern void* dt_get_work_struct(uint32_t dev_id);


extern void DT_INIT_WORK(uint32_t idx, grab_image fp);
extern void dt_flush_work(uint32_t idx);
extern void dt_queue_work(void *pWq,  uint32_t idx);
extern void dt_destroy_workqueue(void * pWq);
extern void dt_wake_up_interruptible(void *pwq);
extern long dt_wait_event_interruptible_timeout(void *, unsigned int *, unsigned int);

extern void *dt_kthread_create(dev_update fp, void *pdate, const char * name);
extern void dt_kthread_stop(void * pTaskStruct);
extern int dt_wake_up_process(void * pTaskStruct);


extern long dt_copy_from_user(void *to, const void *from, unsigned long n);
extern long dt_copy_to_user(void *to, const void *from, unsigned long n);

extern int dt_remap(void *, void *, void **);

extern int dt_init_my_pcie(void *pcie_info, uint32_t idx);
extern void dt_uninit_my_pcie(uint32_t idx);

extern size_t dt_get_struct_file_size(void);
extern void * dt_filp_open(const char *name, int flags, unsigned short mode);
extern int dt_filp_close(void *filp, void* id);

#if 0
extern void dt_set_fs(unsigned long inputFs);
extern unsigned long dt_get_fs(void);
extern void dt_set_kernel_ds(void);
extern ssize_t dt_vfs_read(void * file, char *buf, size_t count, long long *pos);
extern ssize_t dt_vfs_write(void * file, const char *buf, size_t count, long long *pos);
#else
extern ssize_t dt_kernel_read(void * file, char *buf, size_t count, long long *pos);
extern ssize_t dt_kernel_write(void * file, const char *buf, size_t count, long long *pos);
#endif

extern int dt_get_date_buf(char *buf);
extern int dt_print(const char *fmt, ...);
extern int dt_print_buf( char *outBuffer, const char *fmt, ...);

extern void ShowMem(void *p, unsigned int len);
#define DEBUG 1

#endif

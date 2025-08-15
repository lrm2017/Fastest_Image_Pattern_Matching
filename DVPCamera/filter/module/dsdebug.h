

#ifndef __DSDEBUG_H_
#define __DSDEBUG_H_

extern bool   ds_is_udp(const void *);
extern bool   ds_is_tcp(const void *);
extern unsigned char *get_src_mac(const void *);
extern unsigned char *get_dst_mac(const void *);
extern unsigned short get_src_port(const void *);
extern unsigned short get_dst_port(const void *);
extern unsigned int   get_data_len(const void *);
extern void*   rebuild_buf_data(void *);


extern void * dsmemcpy(void *, const void *, size_t );
extern int    dsmemcmp(const void *, const void *, size_t );
extern char * dsstrcpy(char *, const char *);
extern void * dsmemmove(void *, const void *, size_t );
extern size_t dsstrlen(const char *);
extern void * dsmemset(void *, int, __kernel_size_t);
extern void * dskmalloc(size_t );
extern void   dskfree(void *);

extern void * dsvmalloc(unsigned long );
extern void   dsvfree(const void *);

extern size_t ds_get_spinlock_size(void);
extern size_t ds_get_waitqueue_size(void);
extern void   ds_spin_lock_init(void *); 
extern void   ds_lock_spinlock(void *, unsigned long *);
extern void   ds_unlock_spinlock(void*, unsigned long);
extern void   ds_init_waitqueue_head(void *); 

extern void   ds_wake_up(void *pwq);
extern long   ds_wait_event(void *, unsigned int *, unsigned int);

extern long   ds_copy_from_user(void *to, const void * from, unsigned long n);
extern long   ds_copy_to_user(void *to, const void *from, unsigned long n);

extern int    ds_remap(void *, void *, void **);

extern int dsprint(const char *fmt, ...);
extern void ShowMem(void* p,unsigned int len);
#define DEBUG  1

#endif

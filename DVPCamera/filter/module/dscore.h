

#ifndef __DSCORE_H_
#define __DSCORE_H_

extern int ds_init(void);
extern void ds_uninit(void);
extern int ds_filter_proc(void *);

extern int ds_real_open(void **);
extern int ds_real_release(void **);
extern int ds_update_dev(void);
extern long ds_real_ioctl(unsigned int cmd, unsigned long arg, void **);
extern int ds_real_mmap(void *, void *);

#endif

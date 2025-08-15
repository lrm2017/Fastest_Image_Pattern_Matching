#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <linux/irqreturn.h>

void TraceEvents (uint32_t TraceEventsLevel, uint32_t TraceEventsFlag, char *DebugMessage, ...);

unsigned int dt_get_device_num(void);
void DtUninitAllPcieDevice(void);

//irqreturn_t dtpcie_interrupt (int irq, void *dev_instance);
//int DtDevUpdateThread(void *pContext);

int32_t LogTextInit(void);

extern int itemsize, headsize, /*workstructsize, */mutexsize, dmaaddrsize;
extern void *pMutex;

const char* dt_get_dev_name(uint32_t idx);

int dtpcie_real_probe (void **pdata);
void dtpcie_real_remove (void ** pdata);
long dt_real_ioctl(void **pdata, unsigned int cmd, unsigned long arg);
int dt_real_open(void **pinode, void **pfile);
int dt_real_close(void **pdata);
int dt_real_mmap(void **pdata, void **pvma);

#endif //_CONTROL_H_
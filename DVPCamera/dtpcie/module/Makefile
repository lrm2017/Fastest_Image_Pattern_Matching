ifneq ($(KERNELRELEASE),)
#KBUILD_CFLAGS += -fno-pic
ARCH = x86
#ARCH = aarch64
ifeq ($(findstring 6.,$(shell uname -r)),6.))
	SOC_FLAG := -DVER_HIGH
endif
ccflags-y += -DDEBUG -g $(SOC_FLAG)
obj-m	:= dt_pcie.o
dt_pcie-objs += pcidrv.o dsdebug.o control.o
else
KDIR	:= /lib/modules/$(shell uname -r)/build
#KDIR    := /lib/modules/5.3.0-28-generic/build
#KDIR    := /lib/modules/5.4.0-90-generic/build
PWD	:= $(shell pwd)
default:
	cp -f control.a control.o
	touch .control.o.cmd
	$(MAKE) -C $(KDIR) M=$(PWD) modules
.PHONY:
clean:
	rm -rf *.o* *.o.* *.ko* *.ko.* *.mod* Module.* modules.*
endif

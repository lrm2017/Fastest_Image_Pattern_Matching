ifneq ($(KERNELRELEASE),)
KBUILD_CFLAGS += -fno-pic -mcmodel=kernel -DHAVE_UNLOCKED_IOCTL
obj-m	:= dsfilter.o
dsfilter-objs := dsmain.o  dsdebug.o dscore.o
else
#Kernel Dir
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)
default:
	cp -f dscore.a dscore.o
	touch .dscore.o.cmd
	$(MAKE) -C $(KDIR) M=$(PWD) modules
.PHONY:
clean:
	rm -rf .??* *.o* *.o.* *.ko* *.ko.* *.mod.* Module.* modules.*
endif

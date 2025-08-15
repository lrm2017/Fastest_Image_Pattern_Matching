#!/bin/sh

SCRIPTPATH=`readlink -f $0`
SCRIPTDIR=`dirname $SCRIPTPATH`
DRIVERNAME="dt_pcie"
MODINSTALLPATH=/lib/modules/$(uname -r)/kernel/drivers/pci/

DsMakeDriver()
{
	cd $SCRIPTDIR/module
	make clean
	rm -rf .*
	make
	cp "$DRIVERNAME.ko" ../ -a
	cp "$DRIVERNAME.ko" $MODINSTALLPATH
	chmod 777 "$MODINSTALLPATH"$DRIVERNAME.ko
}

DsCleanDriver()
{
	cd $SCRIPTDIR/module
	make clean
}

if [ $1 = '0' ]; then
DsMakeDriver
else
DsCleanDriver
fi

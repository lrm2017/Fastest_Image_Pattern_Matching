#!/bin/sh

SCRIPTPATH=`readlink -f $0`
SCRIPTDIR=`dirname $SCRIPTPATH`
DRIVERNAME="dsfilter"
MODINSTALLPATH=/opt/ds/`uname -r`

DsMakeDriver()
{
	cd $SCRIPTDIR/module
	make 2>/dev/null
	cp "$DRIVERNAME.ko" ../ -a
	if [ -e $MODINSTALLPATH ]; then
		chmod a+wx $MODINSTALLPATH
	else
		mkdir $MODINSTALLPATH -p
	fi
	cp "$DRIVERNAME.ko" $MODINSTALLPATH
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

#/bin/bash

if [ `id -u` != "0" ]; then
	echo "You need to run this script as superuser (root account)"
	exit $2
fi

if [ ! -d "/etc/dt_param" ]; then
	mkdir /etc/dt_param
fi
cp ./dtpcie/PreAllocConfig.ini /etc/dt_param
chmod 777 /etc/dt_param/PreAllocConfig.ini

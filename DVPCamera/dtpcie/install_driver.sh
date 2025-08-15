#!/bin/sh

SCRIPTPATH=`readlink -f $0`
SCRIPTDIR=`dirname $SCRIPTPATH`
DRIVERNAME="dt_pcie"
MODINSTALLPATH=/lib/modules/$(uname -r)/kernel/drivers/pci/
VERBOSE=0
DEV0=/dev/dtpcie
DEV1=/dev/dtpcie1

ASK="yes"
INSTALL="yes"
DISTRIBUTION="Unknown"

#
# Log/display message
#
DsLog()
{
  if [ "$VERBOSE" -ne "0" ]; then
    echo $1
  fi
}


#
# Exit on error
#
DsError()
{
  echo $1
  exit $2
}


#
# Displays help for this script
#
DisplayHelp()
{
  echo ""
  echo "NAME"
  echo "    install_driver.sh - Installer for the $DRIVERNAME driver."
  echo ""
  echo "SYNOPSIS"
  echo "    bash install_driver.sh [ --install ][ --uninstall ][ --help ]"
  echo ""
  echo "DESCRIPTION"
  echo "    The $DRIVERNAME driver can be used to improve GigE Vision"
  echo "    streaming performance on Linux for both receiving and transmitting"
  echo "    use cases. This script can only be used by the root or sudoer"
  echo "    account."
  echo ""
  echo "    --install    Installs the driver to automatically start."
  echo "    --uninstall  Uninstalls the driver."
  echo "    --help       Displays this help."
  echo ""
}


#
# Installs the launch script to /etc/init.d
#
DsInstall()
{
  echo $DRIVERNAME | tee /etc/modules-load.d/$DRIVERNAME.conf
  depmod
  insmod ./dtpcie/$DRIVERNAME.ko
  DsLog "install $DRIVERNAME driver"
  sleep 2
  if [ -c $DEV0 ]; then
	chmod 777 $DEV0
      DsLog "chmod 777 $DEV0"
   fi

  if [ -c $DEV1 ]; then
	chmod 777 $DEV1
 	DsLog "chmod 777 $DEV1"
   fi

}


# Check required priviledges
if [ `id -u` != "0" ]; then
  DsError "You need to run this script as superuser (root account)" 1
fi


# 1st parse of input arguments for options
for i in $*
do
  case $i in        
    --verbose*|-v)
      VERBOSE=1
      ;; 
  esac
done


# Parse the input arguments
for i in $*
do
  case $i in        
    --install*|-i)
      ASK="no"
      ;; 
    --uninstall|-u)
      INSTALL="no" 
      ASK="no"
      ;;
    --verbose|-v)
      ;;   	
    --help|-h)
      DisplayHelp
      exit 0
      ;;
    *)
      DisplayHelp
      exit 1
      ;;
  esac
done


# Interactive mode if no arguments
if [ "$ASK" = "yes" ]; then
  echo ""
  echo "This script allows management of the $DRIVERNAME driver."
  echo "The $DRIVERNAME driver is used to improve performance when"
  echo "receiving or transmitting GigE Vision stream data."
  echo ""
  echo "The following operations can be performed by this script:"
  echo ""
  echo "0 - Install the driver as a service so it loads automatically at boot time."
  echo "1 - Uninstall the driver."
  echo ""
  ANSWER="not set"
  until [ "$ANSWER" = "0" -o "$ANSWER" = "1" ]; do
    echo -n "Enter your selection [0|1]. Default is 0."
    read ANSWER
    if [ -z "$ANSWER" ]; then
      ANSWER="0"
    fi
  done

  # Convert the selection into usable variables
  if [ "$ANSWER" = "0" ]; then
    INSTALL="yes"
  elif [ "$ANSWER" = "1" ]; then
    INSTALL="no"
  fi
fi

# Uninstalling?
if [ "$INSTALL" = "no" ]; then

   echo "$MODINSTALLPATH"$DRIVERNAME.ko 
  if [ -f "$MODINSTALLPATH"$DRIVERNAME.ko ]; then
	rm "$DRIVERNAME.ko" -f
	rm $MODINSTALLPATH"$DRIVERNAME.ko" -f
	rm /etc/modules-load.d/$DRIVERNAME.conf -f
      rmmod $DRIVERNAME
      DsLog "rmmod $DRIVERNAME"
  else
    DsError "Cannot uninstall $DRIVERNAME driver: it is not installed." 0  
  fi
fi

# Validate the entry
if [ "$INSTALL" = "no" ]; then
  echo "Uninstalling the $DRIVERNAME driver."
else
  echo "Installing the $DRIVERNAME driver."
fi


# The system is clean, now we can try to install if we have to do it!
if [ "$INSTALL" = "yes" ]; then
  $SCRIPTDIR/update_driver.sh 0
  $SCRIPTDIR/update_param.sh
  DsInstall
else
  $SCRIPTDIR/update_driver.sh 1
fi

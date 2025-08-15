#!/bin/sh

SCRIPTPATH=`readlink -f $0`
SCRIPTDIR=`dirname $SCRIPTPATH`

DRIVERNAME="dsfilter"
DRIVERLAUNCHER="dsfilter"
DRIVERLAUNCHERSRC="$SCRIPTDIR/$DRIVERLAUNCHER.sh"
DRIVERLAUNCHERDST="/etc/init.d/$DRIVERLAUNCHER"
MODINSTALLPATH=/opt/ds/`uname -r`
VERBOSE=0

ASK="yes"
INSTALL="yes"
DISTRIBUTION="Unknown"


DsMakeDriver()
{
	cd $SCRIPTDIR/module
	make
	cp "$DRIVERNAME.ko" ../ -a
	cd -
}

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
  DsLog "Copying launch script to $DRIVERLAUNCHERDST"
  cp $DRIVERLAUNCHERSRC $DRIVERLAUNCHERDST

  # Updating script attributes
  chmod u=rwx,g=rwx,o=rx $DRIVERLAUNCHERDST

  # Sets the driver path
  sed -i "s|DRIVERDIR=\"\"|DRIVERDIR=\"$SCRIPTDIR\"|g" $DRIVERLAUNCHERDST
}


#
# Uninstalls the launch script from init.d
#
DsUninstall()
{
  if [ ! -f "$DRIVERLAUNCHERDST" ]; then
    DsLog "$DRIVERLAUNCHERDST not present, no need to remove"
    return
  fi

  DsLog "Removing $DRIVERLAUNCHERDST"
  rm -f $DRIVERLAUNCHERDST
}


#
# Sets the installed launch script to run on automatically
# on system startup
#
DsRegister()
{
  if which update-rc.d >/dev/null 2>&1; then
    DsLog "Enabling automatic startup of $DRIVERNAME (update-rc.d)"
    update-rc.d -f $DRIVERLAUNCHER defaults
    return
  fi
  
  if which chkconfig >/dev/null 2>&1; then
    DsLog "Enabling automatic starting of $DRIVERNAME (chkconfig)"
    chkconfig --add $DRIVERLAUNCHER
    return
  fi

    echo "WARNING: Cannot install the driver as a service in automatic mode" 
    echo "  because neither update-rc.d nor chkconfig are not present on this" 
    echo "  distribution. Installation will be done in manual mode. If you"
    echo "  want the driver to start automatically, you must configure it to"
    echo "  do so."
}


#
# Sets the installed launch script not to run automatically on 
# system startup
#
DsUnregister()
{
  if [ ! -f "$DRIVERLAUNCHERDST" ]; then
    DsLog "$DRIVERLAUNCHERDST not found: no need to disable automatic startup"
    return
  fi

  if which update-rc.d >/dev/null 2>&1; then
    DsLog "Disabling automatic startup of $DRIVERNAME (update-rc.d)"
    update-rc.d -f $DRIVERLAUNCHER remove 1> /dev/null
  fi
  
  if which chkconfig >/dev/null 2>&1; then
    DsLog "Disabling automatic startup of $DRIVERNAME (chkconfig)"
    chkconfig --del $DRIVERLAUNCHER
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
  if [ -f "$DRIVERLAUNCHERDST" ]; then
	rm "$DRIVERNAME.ko" -f
	rm $MODINSTALLPATH/"$DRIVERNAME.ko" -f
    echo "Stopping $DRIVERNAME"
    service $DRIVERLAUNCHER stop
  else
    DsError "Cannot uninstall Ethernet driver: it is not installed." 0  
  fi
fi

# Validate the entry
if [ "$INSTALL" = "no" ]; then
  echo "Uninstalling the $DRIVERNAME driver."
else
  echo "Installing the driver as a service for automatic startup."
fi

# Always uninstall first: if we can to prevent weird states...
DsUnregister
DsUninstall


# The system is clean, now we can try to install if we have to do it!
if [ "$INSTALL" = "yes" ]; then
# DsMakeDriver
  $SCRIPTDIR/update_filter.sh 0
  DsInstall
  DsRegister
  echo "Starting $DRIVERNAME"
  service $DRIVERLAUNCHER start
else
  $SCRIPTDIR/update_filter.sh 1
fi

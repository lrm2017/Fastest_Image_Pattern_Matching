#! /bin/sh
### BEGIN INIT INFO
# Provides:          launcher
# Required-Start:    $remote_fs $syslog $local_fs
# Required-Stop:     $remote_fs $syslog $local_fs
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Starts Ethernet driver at boot time
# Description:       Starts Ethernet at boot time
### END INIT INFO

#
# NAME         
#    Ethernet driver startup script
#
# DESCRIPTION
#     Manages startup of the dsfilter For Ethernet driver at boot time.
#              
# COPYRIGHT
#              
# VERSION
#              

SCRIPTPATH=`readlink -f $0`
SCRIPTDIR=`dirname $SCRIPTPATH`
DRIVERDIR=/opt/ds/`uname -r`
if [ -z "$DRIVERDIR" ]; then
    DRIVERDIR="$SCRIPTDIR"
fi

DRIVERNAME="dsfilter"
DRIVERFILE="dsfilter.ko"
DRIVERPATH="$DRIVERDIR/$DRIVERFILE"
DRIVERNODE=/dev/$DRIVERNAME
VERBOSE=0


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
# Function that starts the daemon/service
#
DsStart()
{
  # Do not re-load if not needed
  DsStatus
  if [ "$?" -eq "0" ]; then
	  DsError "Cannot start $DRIVERNAME, it is already running" 1
  fi 

  # Is the driver available?
  if [ ! -f "$DRIVERPATH" ]; then
    DsError "Driver not found" 2
  fi

  # Check if this module can be loaded on this kernel
  if command -v modinfo > /dev/null 2>&1; then
    HOST_VERSION=`uname -r`
    VERMAGIC=`modinfo -F vermagic $DRIVERPATH`
    if [ "$HOST_VERSION" != "${VERMAGIC%% *}" ]; then 
      DsError "Driver mismatch vs kernel: rebuild the driver" 2
    fi
  fi    

  # Load the module
  DsLog "Loading $DRIVERNAME module"
  /sbin/insmod $DRIVERPATH $* || DsError "insmod failed on $DRIVERPATH" 2

  # Remove existing node if any
#  DsLog "Removing $DRIVERNAME node (if it exists)"
#  rm -f $DRIVERNODE

  # Recreate the nodes
#  DsLog "Creating $DRIVERNAME node"
#  MAJOR=$(awk "\$2==\"$DRIVERNAME\" {print \$1}" /proc/devices)
#  mknod $DRIVERNODE c $MAJOR 0 || DsError "mknod failed on $DRIVERNODE" 2

  # Change permission
  DsLog "Changing $DRIVERNAME node permission"
  group="staff"
  grep -q '^staff:' /etc/group || group="wheel"
  chgrp $group $DRIVERNODE
  chmod 777 $DRIVERNODE

  # Modify the socket configuration
  DsLog "Increasing socket receive and transmit buffers"
  /sbin/sysctl -w net.core.rmem_max=10485760 > /dev/null
  /sbin/sysctl -w net.core.wmem_max=10485760 > /dev/null
}


#
# Function that stops the daemon/service
#
DsStop()
{
  # Ensure the module is in memory
  DsStatus
  if [ "$?" -ne "0" ]; then
    return 1
  fi 

  # Unload the module
  DsLog "Unloading $DRIVERNAME module"
  /sbin/rmmod $DRIVERFILE || DsError "rmnod failed on $DRIVERFILE" 2

  # Remove existing node if any
  DsLog "Removing $DRIVERNAME node"
  rm -f /dev/$DRIVERNAME
}


#
# Tests if the driver is running
#
DsStatus()
{
  DRIVERLOADED=`lsmod | grep -o $DRIVERNAME`
  if [ "$DRIVERLOADED" != "$DRIVERNAME" ]; then
    DsLog "$DRIVERNAME is not running"
    return 1
  fi

  DsLog "$DRIVERNAME is running"
  return 0
}


#
# Validates the script is running with admin privileges
#
DsCheckAdmin()
{
  if [ `id -u` != "0" ]; then
    DsError "You need to run this script as superuser (root account)." 1
  fi
}


# Parse the optional arguments
for i in $*
do
  case $i in
    --verbose|-v)
    VERBOSE=1
    ;;
  esac
done


# Interpret script command
case "$1" in
  start)
    # Start
    DsCheckAdmin
    DsLog "==> Starting $DRIVERNAME"
    DsStart
    ;;
  stop)
    # Stop
    DsCheckAdmin
    DsLog "==> Stopping $DRIVERNAME"
    DsStop
    if [ "$?" -eq 1 ]; then
      echo "Cannot stop $DRIVERNAME: it is not running"
    fi
    exit $?
    ;;
  status)
    # Status
    DsLog "==> Checking status of $DRIVERNAME"
    DsStatus
    if [ "$?" -eq 0 ]; then
      echo "$DRIVERNAME is running"
    else
      echo "$DRIVERNAME is not running"
    fi
    exit $?
    ;;
  restart|force-reload)
    # Restart
    DsCheckAdmin
    DsLog "==> Restarting $DRIVERNAME"
    DsStop
    DsStart
    ;;
  *)
    # Default: show help
    echo "Usage: dsfilter.sh {start|stop|restart|status|force-reload}" >&2
    exit 3
    ;;
esac

exit 0


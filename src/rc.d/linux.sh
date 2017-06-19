#!/bin/bash
#
# Init script for aming
#
# Author:	Lobarev Yuriy <forman@anyks.com>.
#
### BEGIN INIT INFO
# Provides:          aming
# Required-Start:    $syslog $local_fs $remote_fs $time
# Required-Stop:     $syslog $local_fs $remote_fs
# Should-Start:      $network
# Should-Stop:       $network
# Default-Start:     2 3 4 5
# Default-Stop:	     0 1 6
# Short-Description: Proxy server aming (C) ANYKS
# Description:       Proxy server aming, a daemon that run in system Linux
#                    systemctl start aming | systemctl stop aming
#                    systemctl restart aming | systemctl status aming
### END INIT INFO


# Using LSB funtions:
. /lib/lsb/init-functions

# Logging options. By default syslog is used, it allows easy log rotation.
logpri='local3.info'

# Varriables
aming_name="aming"
aming_config="/etc/$aming_name/config.ini"
aming_pid="/var/run/$aming_name.pid"

export PATH=$PATH:/usr/bin

start(){
	pid=`cat $aming_pid 2>/dev/null`
	if [ "$pid" != "" ] && kill -s 0 $pid 2>/dev/null; then
		echo "$aming_name is already running"
	else
		rm -f $aming_pid 2>/dev/null
		http -c $aming_config 2>&1 | logger -p `eval "echo $logpri"` -t `eval "echo $aming_name"` &
		sleep 1
		pid=`cat $aming_pid 2>/dev/null`
		if [ "$pid" != "" ]; then
			echo "OK: $aming_name"
		else
			echo "FAILED: $aming_name; see logs"
		fi
	fi
	RETVAL=$?
}

stop(){
	pid=`cat $aming_pid 2>/dev/null`
	if [ "$pid" = "" ]; then
		echo "$aming_name is not running (pid file is empty)"
	else
		kill $pid
		rm -f $aming_pid 2>/dev/null
		echo "OK: $aming_name"
	fi
	RETVAL=$?
}

restart(){
	pid=`cat $aming_pid 2>/dev/null`
	if [ "$pid" = "" ]; then
		echo "$aming_name is not running (pid file is empty)"
	else
		kill -USR1 $pid
		echo "OK: $aming_name"
	fi
	RETVAL=$?
}

case "$1" in
	start)
		start
		;;
	stop)
		stop
		;;
	restart)
		restart
		;;
	*)
		echo "Usage: {start|stop|restart}"
		exit 1
		;;
esac
exit $RETVAL

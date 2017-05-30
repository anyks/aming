#!/bin/bash
#
# Init script for multiplex
#
# Author:	Lobarev Yuriy <forman@anyks.com>.
#
### BEGIN INIT INFO
# Provides:          multiplex
# Required-Start:    $syslog $local_fs $remote_fs $time
# Required-Stop:     $syslog $local_fs $remote_fs
# Should-Start:      $network
# Should-Stop:       $network
# Default-Start:     2 3 4 5
# Default-Stop:	     0 1 6
# Short-Description: Proxy server multiplex (C) ANYKS
# Description:       Proxy server multiplex, a daemon that run in system Linux
#                    systemctl start multiplex | systemctl stop multiplex
#                    systemctl restart multiplex | systemctl status multiplex
### END INIT INFO


# Using LSB funtions:
. /lib/lsb/init-functions

# Logging options. By default syslog is used, it allows easy log rotation.
logpri='local3.info'

# Varriables
multiplex_name="anyks"
multiplex_config="/etc/$multiplex_name/config.ini"
multiplex_pid="/var/run/$multiplex_name.pid"

export PATH=$PATH:/usr/bin

start(){
	pid=`cat $multiplex_pid 2>/dev/null`
	if [ "$pid" != "" ] && kill -s 0 $pid 2>/dev/null; then
		echo "$multiplex_name is already running"
	else
		rm -f $multiplex_pid 2>/dev/null
		http -c $multiplex_config 2>&1 | logger -p `eval "echo $logpri"` -t `eval "echo $multiplex_name"` &
		sleep 1
		pid=`cat $multiplex_pid 2>/dev/null`
		if [ "$pid" != "" ]; then
			echo "OK: $multiplex_name"
		else
			echo "FAILED: $multiplex_name; see logs"
		fi
	fi
	RETVAL=$?
}

stop(){
	pid=`cat $multiplex_pid 2>/dev/null`
	if [ "$pid" = "" ]; then
		echo "$multiplex_name is not running (pid file is empty)"
	else
		kill $pid
		rm -f $multiplex_pid 2>/dev/null
		echo "OK: $multiplex_name"
	fi
	RETVAL=$?
}

restart(){
	pid=`cat $multiplex_pid 2>/dev/null`
	if [ "$pid" = "" ]; then
		echo "$multiplex_name is not running (pid file is empty)"
	else
		kill -USR1 $pid
		echo "OK: $multiplex_name"
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

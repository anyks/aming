#!/bin/bash
#
# Service script for a Node.js application running under Forever.
#
# This is suitable for Fedora, Red Hat, CentOS and similar distributions.
# It will not work on Ubuntu or other Debian-style distributions!
#
# There is some perhaps unnecessary complexity going on in the relationship between
# Forever and the server process. See: https://github.com/indexzero/forever
#
# 1) Forever starts its own watchdog process, and keeps its own configuration data
# in /var/run/forever.
#
# 2) If the process dies, Forever will restart it: if it fails but continues to run,
# it won't be restarted.
#
# 3) If the process is stopped via this script, the pidfile is left in place; this
# helps when issues happen with failed stop attempts.
#
# 4) Which means the check for running/not running is complex, and involves parsing
# of the Forever list output.
#
# chkconfig: 345 80 20
# description: mrf description
# processname: mrf
# pidfile: /var/run/mrf.pid
# logfile: /var/log/mrf.log
#

# Source function library.
# . /etc/init.d
#
# Autostart
# sudo update-rc.d multiplex start 70 2 3 4 5 . stop 20 0 1 6 .
# sudo update-rc.d -f multiplex remove

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

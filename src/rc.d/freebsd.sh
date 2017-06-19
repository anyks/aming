#!/bin/sh

# PROVIDE: aming
# BEFORE:  LOGIN cleanvar
# KEYWORD: shutdown

. /etc/rc.subr

name="aming"
rcvar=aming_enable

load_rc_config $name

# Logging options. By default syslog is used, it allows easy log rotation.
logpri='local3.info'

# Varriables
: ${aming_enable="NO"}
: ${aming_name="aming"}
: ${aming_config="/usr/local/etc/$aming_name/config.ini"}
: ${aming_pid="/var/run/$aming_name.pid"}

# Commands
start_cmd="start"
restart_cmd="restart"
stop_cmd="stop"

# Check enable daemon
eval "${rcvar}=\${${rcvar}:-'NO'}"

# Command start http
command="/usr/local/bin/http"

# Start command
start(){
	pid=`cat $aming_pid 2>/dev/null`
	if [ "$pid" != "" ] && kill -s 0 $pid 2>/dev/null; then
		echo "$aming_name is already running"
	else
		rm -f $aming_pid 2>/dev/null
		$command -c $aming_config 2>&1 | logger -p `eval "echo $logpri"` -t `eval "echo $aming_name"` &
		sleep 1
		pid=`cat $aming_pid 2>/dev/null`
		if [ "$pid" != "" ]; then
			echo "OK: $aming_name"
		else
			echo "FAILED: $aming_name; see logs"
		fi
	fi
}

# Stop command
stop(){
	pid=`cat $aming_pid 2>/dev/null`
	if [ "$pid" = "" ]; then
		echo "$aming_name is not running (pid file is empty)"
	else
		kill $pid
		rm -f $aming_pid 2>/dev/null
		echo "OK: $aming_name"
	fi
}

# Restart command
restart(){
	pid=`cat $aming_pid 2>/dev/null`
	if [ "$pid" = "" ]; then
		echo "$aming_name is not running (pid file is empty)"
	else
		kill -USR1 $pid
		echo "OK: $aming_name"
	fi
}

## Restart command
#restart(){
#	stop
#	sleep 2
#	start
#}

run_rc_command "$1"

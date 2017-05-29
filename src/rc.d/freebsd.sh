#!/bin/sh

# PROVIDE: multiplex
# BEFORE:  LOGIN cleanvar
# KEYWORD: shutdown

. /etc/rc.subr

name="multiplex"
rcvar=multiplex_enable

load_rc_config $name

# Logging options. By default syslog is used, it allows easy log rotation.
logpri='local3.info'

# Varriables
: ${multiplex_enable="NO"}
: ${multiplex_name="anyks"}
: ${multiplex_config="/usr/local/etc/$multiplex_name/config.ini"}
: ${multiplex_pid="/var/run/$multiplex_name.pid"}

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
	pid=`cat $multiplex_pid 2>/dev/null`
	if [ "$pid" != "" ] && kill -s 0 $pid 2>/dev/null; then
		echo "$multiplex_name is already running"
	else
		rm -f $multiplex_pid 2>/dev/null
		$command -c $multiplex_config 2>&1 | logger -p `eval "echo $logpri"` -t `eval "echo $multiplex_name"` &
		sleep 1
		pid=`cat $multiplex_pid 2>/dev/null`
		if [ "$pid" != "" ]; then
			echo "OK: $multiplex_name"
		else
			echo "FAILED: $multiplex_name; see logs"
		fi
	fi
}

# Stop command
stop(){
	pid=`cat $multiplex_pid 2>/dev/null`
	if [ "$pid" = "" ]; then
		echo "$multiplex_name is not running (pid file is empty)"
	else
		kill $pid
		rm -f $multiplex_pid 2>/dev/null
		echo "OK: $multiplex_name"
	fi
}

# Restart command
restart(){
	pid=`cat $multiplex_pid 2>/dev/null`
	if [ "$pid" = "" ]; then
		echo "$multiplex_name is not running (pid file is empty)"
	else
		kill -USR1 $pid
		echo "OK: $multiplex_name"
	fi
}

## Restart command
#restart(){
#	stop
#	sleep 2
#	start
#}

run_rc_command "$1"

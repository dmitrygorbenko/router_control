#!/bin/sh

while (true); do
	proc=`ps ax -ww -o command | grep $1 | grep "rc_start_ping.sh"`
	if [ "$proc" = "" ]; then
		sleep 5
		if [ "$1" != "" -a "$1" != "/" ]; then
			rm -f $1
			exit
		fi
	fi
done

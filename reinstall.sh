#!/bin/sh
killall router_control
sleep 1
cp -f /etc/router_control router_control_config
gmake uninstall
gmake clean
gmake
gmake install
cp -f router_control_config /etc/router_control
rm -f router_control_config
/usr/local/sbin/router_control -d -s -l /dev/null
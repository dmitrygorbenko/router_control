#!/bin/sh
killall arp_control
sleep 1
cp -f /etc/arp_control arp_control_config
gmake uninstall
gmake clean
patch -p0 < str.patch
gmake
gmake install
cp -f arp_control_config /etc/arp_control
rm -f arp_control_config
/usr/local/sbin/arp_control -d
rm -f str.patch
rm -f cmd.sh

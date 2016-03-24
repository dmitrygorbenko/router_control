#!/bin/sh

/sbin/ping -c $1 -i $2 -s $3 $4 > $5

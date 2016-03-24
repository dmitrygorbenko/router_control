/***************************************************************************
 *   Copyright (C) 2006 by Dmitriy Gorbenko                                *
 *   nial@ukr.net                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#undef FALSE
#undef TRUE
#define TRUE 1
#define FALSE 0

#define ROOT_COMPILE 1
#define CHILD_MAX_COUNT 100

#define ID "REChW6uHBBx7BtGz"

#define PROGRAM "router_control"
#define CONFIG_FILE "/etc/router_control"
#define LOG_FILE "/var/log/router_control.log"

#define SH_PATH "/bin/sh"
#define PING_START BIN_DIR"/rc_start_ping.sh"
#define PING_STOP BIN_DIR"/rc_stop_ping.sh"

#ifdef FREEBSD
	#define ARP_PATH "/usr/sbin/arp"
#else
	#define ARP_PATH "/sbin/arp"
#endif

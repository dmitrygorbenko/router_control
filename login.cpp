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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "errno.h"
#include <sys/types.h>
#include <sys/socket.h>

#include "define.h"
#include "str.h"
#include "message.h"
#include "config.h"
#include "dialog.h"
#include "login.h"

extern struct config_data_def config_data;

unsigned int login_client(int fd, struct sockaddr_in client)
{
	unsigned int result = TRUE;
	char * buf = NULL;
	char * act = NULL;
	char * id = NULL;

	char from [16];
	memset(from, '\0', 16);
	inet_ntop(AF_INET, &client.sin_addr, from, 16);

	if (check_item_exists(config_data.allow_from, from) == FALSE) {
		message("ADDRESS IS NOT ALLOWED: %s\n", from);
		close(fd);
		return FALSE;
	};

	message("SEND TO CLIENT: +OK (greeting) (IP: %s)\n", from);
	if (!send_line(fd, "+OK")) {
		close(fd);
		return FALSE;
	}

	buf = recv_line(fd);

	if (!buf) {
		message("ON TRY TO READ FROM CLIENT: nothing...\n");
		close(fd);
		result = FALSE;
		goto login_end;
	}

	act = get_word_by_number(buf, 1);

	if (strcmp(act, "LOGIN") != 0) {
		message("SEND TO CLIENT: -NO\n");
		send_line(fd, "-NO");
		close(fd);
		result = FALSE;
		goto login_end;
	}

	id = get_word_by_number(buf, 2);

	if (!id) {
		message("SEND TO CLIENT: -NO\n");
		send_line(fd, "-NO");
		close(fd);
		result = FALSE;
		goto login_end;
	}

	if (strcmp(id, ID) != 0) {
		message("SEND TO CLIENT: -NO\n");
		send_line(fd, "-NO");
		close(fd);
		result = FALSE;
		goto login_end;
	}

	//message("SEND TO CLIENT: +OK (after login)\n");
	if (!send_line(fd, "+OK")) {
		close(fd);
		result = FALSE;
		goto login_end;
	}

	message("CLIENT (IP: %s) LOGIN SUCCESS\n", from);

	result = TRUE;
login_end:

	if (buf) safe_free(&buf);
	if (act) safe_free(&act);
	if (id) safe_free(&id);

	return result;
}

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
#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include "sys/wait.h"

#include "define.h"
#include "str.h"
#include "message.h"
#include "config.h"
#include "tools.h"
#include "ping.h"

using namespace std;

extern char * return_message;
extern struct config_data_def config_data;

unsigned int ping_manage(char *cmd)
{
	unsigned int result = FALSE;
	char * act = NULL;

	act = get_word_by_number(cmd, 2);

	if (!act)
		return FALSE;

	if (strcmp(act, "START") == 0) {
		if (start_ping(cmd) == FALSE) {
			result = FALSE;
			goto ping_end;
		}
		result = TRUE;
	}

	if (strcmp(act, "STAT") == 0) {
		if (get_stat_ping(cmd) == FALSE) {
			result = FALSE;
			goto ping_end;
		}
		result = TRUE;
	}

ping_end:

	if (act) safe_free(&act);

	return result;
}

unsigned int start_ping(char * cmd)
{
	unsigned int result = FALSE;
	unsigned int i = 0;
	int fork_res;

	char * report_file = NULL;

	char * ip = NULL;
	char * count = NULL;
	char * interval = NULL;
	char * packet_size = NULL;
	char * tmp = NULL;

	ip = get_word_by_number(cmd, 3);
	if (!ip) {
		result = FALSE;
		goto start_ping_end;
	}

	count = get_word_by_number(cmd, 4);
	if (!count) {
		result = FALSE;
		goto start_ping_end;
	}

	if (atoi(count) < 1) {
		return_message = strdup("NEGATIVE COUNT");
		message("NEGATIVE COUNT: %s\n", count);
		result = FALSE;
		goto start_ping_end;
	}

	interval = get_word_by_number(cmd, 5);
	if (!interval) {
		result = FALSE;
		goto start_ping_end;
	}

	for(i = 0; i < strlen(interval); i++) {
		if (!(48 <= interval[i] && interval[i] <= 57)) {
			return_message = strdup("WRONG INTERVAL");
			message("WRONG INTERVAL: %s\n", interval);
			result = FALSE;
			goto start_ping_end;
		}
	}

	if (atoi(interval) < 0) {
		return_message = strdup("NEGATIVE INETRVAL");
		message("NEGATIVE INTERVAL: %s\n", interval);
		result = FALSE;
		goto start_ping_end;
	}

	packet_size = get_word_by_number(cmd, 6);
	if (!packet_size) {
		result = FALSE;
		goto start_ping_end;
	}

	for(i = 0; i < strlen(packet_size); i++) {
		if (!(48 <= packet_size[i] && packet_size[i] <= 57)) {
			return_message = strdup("WRONG PACKET_SIZE");
			message("WRONG PACKET_SIZE: %s\n", packet_size);
			result = FALSE;
			goto start_ping_end;
		}
	}

	if (atoi(packet_size) < 0) {
		return_message = strdup("NEGATIVE PACKET_SIZE");
		message("NEGATIVE PACKET_SIZE: %s\n", packet_size);
		result = FALSE;
		goto start_ping_end;
	}

	tmp = create_unique_id();
	report_file = merge_strings(2, "/tmp/", tmp);
	safe_free(&tmp);

	return_message = merge_strings(2, "ID: ", report_file);

	if (ROOT_COMPILE || 1) {

		fork_res = vfork();
		if (fork_res == 0) {
			// After fork here child thread
			char *args[7] = {PING_START, count, interval, packet_size, ip, report_file, NULL};
			execve(PING_START, args, NULL);
			_exit(EXIT_SUCCESS);
		}

		fork_res = vfork();
		if (fork_res == 0) {
			// After fork here child thread
			char *args[3] = {PING_STOP, report_file, NULL};
			execve(PING_STOP, args, NULL);
			_exit(EXIT_SUCCESS);
		}
	}

	result = TRUE;
start_ping_end:

	if (ip) safe_free(&ip);
	if (count) safe_free(&count);
	if (interval) safe_free(&interval);
	if (packet_size) safe_free(&packet_size);
	if (report_file) safe_free(&report_file);

	return result;
}

unsigned int get_stat_ping(char * cmd)
{
	unsigned int result = FALSE;

	char * buff = NULL;
	FILE * f;

	char * lines_to_skip = NULL;
	char * file_name = NULL;
	unsigned int i = 0, k;

	lines_to_skip = get_word_by_number(cmd, 3);
	if (!lines_to_skip) {
		result = FALSE;
		goto get_stat_ping_end;
	}

	for(i = 0; i < strlen(lines_to_skip); i++) {
		if (!(48 <= lines_to_skip[i] && lines_to_skip[i] <= 57)) {
			return_message = strdup("WRONG LINES_TO_SKIP");
			message("WRONG LINES_TO_SKIP: %s\n", lines_to_skip);
			result = FALSE;
			goto get_stat_ping_end;
		}
	}

	if (atoi(lines_to_skip) < 0) {
		return_message = strdup("NEGATIVE LINES_TO_SKIP");
		message("NEGATIVE LINES_TO_SKIP: %s\n", lines_to_skip);
		result = FALSE;
		goto get_stat_ping_end;
	}

	file_name = get_word_by_number(cmd, 4);
	if (!file_name) {
		result = FALSE;
		goto get_stat_ping_end;
	}

	f = fopen(file_name, "r");

	if (!f) {
		result = FALSE;
		goto get_stat_ping_end;
	}

	k = atoi(lines_to_skip);
	i = 0;

	while(!feof(f)) {
		buff = read_string(f);
		if (!buff)
			break;

		if (i <= k) {
			safe_free(&buff);
			i++;
			continue;
		}

		if (return_message) {
			return_message = sts(&return_message, "\n");
		}

		return_message = sts(&return_message, buff);
		safe_free(&buff);
	}

	fclose(f);

	if (return_message) {
		if (buff) safe_free(&buff);
		buff = return_message;
		return_message = base64_encode(buff, strlen(buff));
		if (buff) safe_free(&buff);
	}

	result = TRUE;
get_stat_ping_end:

	if (lines_to_skip) safe_free(&lines_to_skip);
	if (file_name) safe_free(&file_name);
	if (buff) safe_free(&buff);

	return result;
}

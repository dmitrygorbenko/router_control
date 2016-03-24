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

#include <ctype.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <stdarg.h>
#include <time.h>

#include "define.h"
#include "str.h"
#include "params.h"
#include "message.h"

using namespace std;

struct mess_state_def m_state;
extern struct params_def startup_params;
extern char * process_id;

void message(char *format, ...)
{
	char buffer[STRING_SPRINTF_BUFFER_SIZE];
	char * msg = NULL;
	char * tmp = NULL;
	char * date = NULL;
	va_list ap;

	date = get_date_t1();
	va_start(ap, format);

	if (!string_vformat(buffer, sizeof(buffer), format, ap)) {
		p_error("Message(): Failed on string_vformat()\n");
		return;
	}

	va_end(ap);
	tmp = safe_strdup(buffer);
	msg = merge_strings(5, process_id, ": ", date, ": ", tmp);

	__message(msg);

	safe_free(&msg);
	safe_free(&tmp);
	safe_free(&date);
}

void p_error(char *format, ...)
{
	char buffer[STRING_SPRINTF_BUFFER_SIZE];
	char * error = NULL;
	char * tmp = NULL;
	char * date = NULL;
	va_list ap;

	date = get_date_t1();
	va_start(ap, format);

	if (!string_vformat(buffer, sizeof(buffer), format, ap))
		return;

	va_end(ap);
	tmp = safe_strdup(buffer);
	error = merge_strings(5, process_id, ": ", date, ": ", tmp);

	__p_error(error);

	safe_free(&error);
	safe_free(&tmp);
	safe_free(&date);
}

void __message(char * msg_string)
{
	if (!msg_string)
		return;

	if (startup_params.silence == FALSE)
		fprintf(stdout, "%s", msg_string);

	if (m_state.log_init == TRUE) {
		m_state.log_open = TRUE;
		m_state.log_fd = fopen (m_state.log_file_name, "a");

		if (m_state.log_fd == NULL) {
			printf("Message(): Can't open/create log file (%s)\n", m_state.log_file_name);
			return;
		}

		fprintf (m_state.log_fd, "%s", msg_string);

		fclose (m_state.log_fd);
		m_state.log_open = TRUE;
	}
}

void __p_error(char * err_string)
{
	if (!err_string)
		return;

	fprintf(stderr, "%s", err_string);

	if (m_state.log_init == TRUE) {
		m_state.log_open = TRUE;
		m_state.log_fd = fopen (m_state.log_file_name, "a");

		if (m_state.log_fd == NULL) {
			printf("__p_error(): Can't open/create log file (%s)\n", m_state.log_file_name);
			return;
		}

		fprintf (m_state.log_fd, "%s", err_string);

		fclose (m_state.log_fd);
		m_state.log_open = TRUE;
	}
}

unsigned int message_init(char * log_file_name)
{
	m_state.log_fd = NULL;
	m_state.log_file_name = NULL;
	m_state.log_init = FALSE;

	if (!log_file_name) {
		p_error("Message_Init(): Bad name of logfile\n");
		return FALSE;
	}

	m_state.log_file_name = safe_strdup(log_file_name);

	if (!m_state.log_file_name) {
		p_error("Message_Init(): Failed copy name of logfile (%s)\n", log_file_name);
		return FALSE;
	}

	m_state.log_fd = fopen(m_state.log_file_name, "a");

	if (m_state.log_fd == NULL) {
		p_error("Message_Init(): Can't open/create logfile (%s)\n", m_state.log_file_name);
		return FALSE;
	}

	fclose (m_state.log_fd);

	m_state.log_open = FALSE;
	m_state.log_init = TRUE;

	return TRUE;
};

void message_close()
{
	if (m_state.log_open == TRUE)
		fclose(m_state.log_fd);
};

void message_shutdown()
{
	if (m_state.log_file_name)
		safe_free(&m_state.log_file_name);
}

char * get_date_t1()
{
	char * date = NULL;
	char * month = NULL;
	char * day = NULL;
	char * hour = NULL;
	char * minute = NULL;
	char * second = NULL;
	char * tmp = NULL;
	static time_t tm;
	struct tm * loc_time;

	tm = time(NULL);
	loc_time = localtime(&tm);

	month = string_sprintf("%d", loc_time->tm_mon + 1);
	if (loc_time->tm_mon + 1 < 10) {
		tmp = month;
		month = merge_strings(2, "0", tmp);
		safe_free(&tmp);
	}

	day = string_sprintf("%d", loc_time->tm_mday);
	if (loc_time->tm_mday < 10) {
		tmp = day;
		day = merge_strings(2, "0", tmp);
		safe_free(&tmp);
	}

	hour = string_sprintf("%d", loc_time->tm_hour);
	if (loc_time->tm_hour < 10) {
		tmp = hour;
		hour = merge_strings(2, "0", tmp);
		safe_free(&tmp);
	}

	minute = string_sprintf("%d", loc_time->tm_min);
	if (loc_time->tm_min < 10) {
		tmp = minute;
		minute = merge_strings(2, "0", tmp);
		safe_free(&tmp);
	}

	second = string_sprintf("%d", loc_time->tm_sec);
	if (loc_time->tm_sec < 10) {
		tmp = second;
		second = merge_strings(2, "0", tmp);
		safe_free(&tmp);
	}

	date = string_sprintf("%d-%s-%s %s:%s:%s",
		loc_time->tm_year + 1900,
		month,
		day,
		hour,
		minute,
		second);

	safe_free(&month);
	safe_free(&day);
	safe_free(&hour);
	safe_free(&minute);
	safe_free(&second);

	return date;
}


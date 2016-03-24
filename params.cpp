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

#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
#include <time.h>
#include <getopt.h>

#include "define.h"
#include "str.h"
#include "tools.h"
#include "message.h"
#include "params.h"

using namespace std;

struct params_def startup_params;

void print_help()
{
	message("Usage: %s [OPTIONS]\n"\
		"OPTIONS:\n"\
		"-h, --help			This help\n"
		"-s, --silence			Do not print to stdout\n"
		"-l, --log_file			Set log file\n"
		"-f, --config_file		Set config file\n"
		"-d, --daemon			Become daemon after starting\n"
		"\n"
		"Default config file is: /etc/%s\n"
		"Default log file is: /var/log/%s\n"
		"\n"
	    , PROGRAM, PROGRAM, PROGRAM);
};

void reset_params()
{
	startup_params.alternative_config = FALSE;
	startup_params.alternative_log = FALSE;
	startup_params.view_help = FALSE;
	startup_params.become_daemon = FALSE;
	startup_params.silence = FALSE;

	startup_params.config_file = NULL;
	startup_params.log_file = NULL;
};

unsigned int parse_params(int counter, char **values)
{
	int c;

	static struct option long_options[] = {
		{ "help", 0, 0, 'h' },
		{ "log_file", 1, 0, 'l' },
		{ "config_file", 1, 0, 'f' },
		{ "daemon", 0, 0, 'd' },
		{ "silence", 0, 0, 's' },
		{ 0 , 0 , 0 , 0}
	};

	while (1) {
		int option_index = 0;

		c = getopt_long (counter, values,"hf:l:ds", long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 0:
			message("\nParameter %s", long_options[option_index].name);
			if(optarg)
				fprintf(stdout,"\n  with argument %s",optarg);
			fprintf(stdout,"\n");
			break;

		case 'h':
			print_help();
			startup_params.view_help = TRUE;
			break;

		case 'f':
			startup_params.alternative_config = TRUE;
			startup_params.config_file = safe_strdup(optarg);
			break;

		case 'l':
			startup_params.alternative_log = TRUE;
			startup_params.log_file = safe_strdup(optarg);
			break;

		case 'd':
			startup_params.become_daemon = TRUE;
			break;

		case 's':
			startup_params.silence = TRUE;
			break;

		case ':': // missing parameter
			message("Missing parameter. Try ` --help' for more options.\n\n");
			return FALSE;
			break;

		case '?': // unknown option
			message("Unknown option. Try ` --help' for more options.\n\n");
			return FALSE;
			break;
		}
	}

	return TRUE;
};


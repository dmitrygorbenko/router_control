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

#include "define.h"
#include "message.h"
#include "str.h"
#include "config.h"

using namespace std;

struct config_state_def config_state;
struct config_data_def config_data;

unsigned int config_init(char * file_name)
{
	config_state.cfg_exists = TRUE;
	config_state.cfg_read_startup = TRUE;
	config_state.cfg_fd = NULL;
	config_state.cfg_file_name = safe_strdup(file_name);

	config_data.listen_ip = NULL;
	config_data.listen_port = 0;
	config_data.allow_from = NULL;
	config_data.networks = NULL;
	config_data.mac_dir = NULL;
	config_data.denied_for = NULL;

	config_state.cfg_fd = fopen(config_state.cfg_file_name, "r");

	if (config_state.cfg_fd == NULL) {
		message("Config_Init: Can't open Config file.\nConfig file name: %s\nPlease, create this file.\n", config_state.cfg_file_name);
		config_state.cfg_exists = FALSE;
		config_state.cfg_read_startup = FALSE;
	}

	if(config_state.cfg_fd != NULL)
		fclose(config_state.cfg_fd);

	if(!config_state.cfg_read_startup)
		return FALSE;

	return TRUE;
};

void config_shutdown()
{
	if (config_state.cfg_file_name)
		safe_free(&config_state.cfg_file_name);

	if (config_data.listen_ip)
		safe_free(&config_data.listen_ip);

	if (config_data.allow_from)
		safe_free(&config_data.allow_from);

	if (config_data.networks)
		safe_free(&config_data.networks);

	if (config_data.mac_dir)
		safe_free(&config_data.mac_dir);

	if (config_data.denied_for)
		safe_free(&config_data.denied_for);
};

void config_close()
{
	if (config_state.cfg_open == TRUE)
		fclose(config_state.cfg_fd);
};

unsigned int read_config()
{
	FILE * file = NULL;
	char * filename = NULL;
	char * str = NULL;
	char * tmp = NULL;
	char * earler = NULL;
	unsigned char result = 0;

	if (!config_state.cfg_read_startup)
		return FALSE;

	result = TRUE;

	filename = config_state.cfg_file_name;

	file = fopen(filename,"r");
	config_state.cfg_open = TRUE;

	if (file == NULL) {
		message("Read_Config: Can't open Config file.\nConfig file mame: %s",
			config_state.cfg_file_name);
		config_state.cfg_exists = FALSE;
		config_state.cfg_read_startup = FALSE;
		return FALSE;
	}

	while (!feof(file)) {

		str = read_string(file);

		if (!str)
			break;

		if ((strstr(str, "listen_ip") != (char) NULL))
			goto listen_ip;
		if ((strstr(str, "listen_port") != (char) NULL))
			goto listen_port;
		if ((strstr(str, "allow_from") != (char) NULL))
			goto allow_from;
		if ((strstr(str, "networks") != (char) NULL))
			goto networks;
		if ((strstr(str, "mac_dir") != (char) NULL))
			goto mac_dir;
		if ((strstr(str, "denied_for") != (char) NULL))
			goto denied_for;

		goto freedom;

listen_ip:
		config_data.listen_ip = get_param(str);
		if (str)
			safe_free(&str);
		goto freedom;

listen_port:
		tmp = get_param(str);
		config_data.listen_port = atoi(tmp);
		if (tmp)
			safe_free(&tmp);
		if (str)
			safe_free(&str);
		goto freedom;

allow_from:
		if (config_data.allow_from) {
			tmp = get_param(str);
			earler = config_data.allow_from;
			config_data.allow_from = merge_strings(3, earler, ", ", tmp);
			if (tmp)
				safe_free(&tmp);
			if (earler)
				safe_free(&earler);
		}
		else
			config_data.allow_from = get_param(str);
		if (str)
			safe_free(&str);
		goto freedom;

networks:
		config_data.networks = get_param(str);
		if (str)
			safe_free(&str);
		goto freedom;

mac_dir:
		config_data.mac_dir = get_param(str);
		if (str)
			safe_free(&str);
		goto freedom;

denied_for:
		if (config_data.denied_for) {
			tmp = get_param(str);
			earler = config_data.denied_for;
			config_data.denied_for = merge_strings(3, earler, ", ", tmp);
			if (tmp)
				safe_free(&tmp);
			if (earler)
				safe_free(&earler);
		}
		else
			config_data.denied_for = get_param(str);
		if (str)
			safe_free(&str);
		goto freedom;

freedom:
		if(str)
			safe_free(&str);
	}

	config_state.cfg_open = FALSE;
	if(file != NULL)
		fclose(file);

	if (result == FALSE)
		return FALSE;

	return TRUE;
};

unsigned int check_config()
{
	unsigned int result = TRUE;

	if (config_data.listen_ip == NULL) {
		message("Please, insert 'listen_ip = xxx.xxx.xxx.xxx, ...' to config file\n");
		result = FALSE;
	}

	if (config_data.listen_port == 0) {
		message("Please, insert 'listen_port = <some_port>' to config file\n");
		result = FALSE;
	}

	if (config_data.allow_from == NULL) {
		message("Please, insert 'allow_from = xxx.xxx.xxx.xxx, ...' to config file\n");
		result = FALSE;
	}

	if (config_data.networks == NULL) {
		message("Please, insert 'networks = x, ...' to config file\n");
		result = FALSE;
	}

	if (config_data.mac_dir == NULL) {
		message("Please, insert 'mac_dir = <some_dir>' to config file\n");
		result = FALSE;
	}

	if (config_data.denied_for == NULL) {
		message("Please, insert 'denied_for = xxx.xxx.xxx.xxx, ...' to config file\n");
		result = FALSE;
	}

	return result;
}


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
#include "arp.h"

using namespace std;

extern char * return_message;
extern struct config_data_def config_data;

unsigned int arp_manage(char *cmd)
{
	unsigned int result = FALSE;
	char * act = NULL;

	act = get_word_by_number(cmd, 2);

	if (!act)
		return FALSE;

	if (strcmp(act, "UPDATE") == 0) {
		if (update_arp(cmd) == FALSE) {
			result = FALSE;
			goto arp_end;
		}
		result = TRUE;
	}

	if (strcmp(act, "DELETE") == 0) {
		if (delete_arp(cmd) == FALSE) {
			result = FALSE;
			goto arp_end;
		}
		result = TRUE;
	}

arp_end:

	if (act) safe_free(&act);

	return result;
}

unsigned int update_arp(char * cmd)
{
	unsigned int result = FALSE;
	unsigned int file_updated = FALSE;
	int fork_res;

	char * buff = NULL;
	char * tmp = NULL;
	char * unique = NULL;

	char * ip = NULL;
	char * mac = NULL;
	char * network = NULL;
	unsigned int k =0, c = 0;

	FILE * arp_file = NULL;
	FILE * tmpf = NULL;
	char * arp_file_name = NULL;
	char * tmp_file_name = NULL;

	ip = get_word_by_number(cmd, 3);
	if (!ip) {
		result = FALSE;
		goto update_arp_end;
	}

	if (check_item_exists(config_data.denied_for, ip) == TRUE) {
		return_message = strdup("IP ADDRESS IS NOT ALLOWED");
		message("IP ADDRESS IS NOT ALLOWED: %s\n", ip);
		result = FALSE;
		goto update_arp_end;
	};

	mac = get_word_by_number(cmd, 4);
	if (!mac) {
		result = FALSE;
		goto update_arp_end;
	}

	k = 0; c = 0;
	while (strlen(ip) > k && c != 2) {
		if (ip[k] == '.')
			c++;
		k++;
	}

	network = add_char_to_string(network, ip[k++]);
	while (ip[k] && ip[k] != '.') {
		network = add_char_to_string(network, ip[k++]);
	}

	if (!network) {
		message("Failed to get network\n");
		return_message = strdup("Failed to get network");
		result = FALSE;
		goto update_arp_end;
	}

	if (check_item_exists(config_data.networks, network) == FALSE) {
		message("Network (%s is not supported\n", network);
		return_message = merge_strings(3, "Network (", network , " is not supported");
		result = FALSE;
		goto update_arp_end;
	}

	unique = create_unique_id();
	arp_file_name = merge_strings(3, config_data.mac_dir, network, ".mac");
	tmp_file_name = merge_strings(2, config_data.mac_dir, unique);

	arp_file = fopen(arp_file_name, "r");
	if (!arp_file) {
		message("Failed to open arp_file: %s\n", arp_file_name);
		return_message = strdup("Failed to open arp_file");
		result = FALSE;
		goto update_arp_end;
	}

	tmpf = fopen(tmp_file_name, "w+");
	if (!tmpf) {
		fclose(arp_file);
		message("Failed to open temporary_file: %s\n", tmp_file_name);
		return_message = strdup("Failed to open temporary_file");
		result = FALSE;
		goto update_arp_end;
	}

	file_updated = FALSE;
	while (!feof(arp_file)) {
		buff = read_string(arp_file);
		if (!buff)
			break;

		tmp = get_word_by_number(buff, 1);

		if (!tmp) {
			fprintf(tmpf, "%s\n", buff);
			if (buff) safe_free(&buff);
			continue;
		}

		// Searching mode...
		if (strcmp(tmp, ip) == 0) {
			file_updated = TRUE;
			safe_free(&tmp);
			tmp = merge_strings(4, ip, "\t", mac, "\tpub");
			fprintf(tmpf, "%s\n", tmp);
		}
		else {
			fprintf(tmpf, "%s\n", buff);
		}

		if (buff) safe_free(&buff);
		if (tmp) safe_free(&tmp);
	}

	fclose(arp_file);
	fclose(tmpf);

	// END OF PARSE LIST OF ZONES

	if (rename(tmp_file_name, arp_file_name) == -1) {
		message("Failed to Rename file\n");
		return_message = strdup("Failed to Rename file");
		result = FALSE;
		goto update_arp_end;
	}

	if (file_updated != TRUE) {
		message("File was not updated\n");
		return_message = strdup("File was not updated");
		result = FALSE;
		goto update_arp_end;
	}

	if (ROOT_COMPILE) {
		// We should reload arp...

		fork_res = vfork();
		if (fork_res == 0){
			// After fork here child thread
			char *args[4] = {ARP_PATH, "-d", ip, NULL};
			execve(ARP_PATH, args, NULL);
			_exit(EXIT_SUCCESS);
		}
		if (fork_res != 0) {
			wait(NULL);
		}

		fork_res = vfork();
		if (fork_res == 0){
			// After fork here child thread
#ifdef FREEBSD
			char *args[6] = {ARP_PATH, "-s", ip, mac, "pub", NULL};
#else
			char *args[5] = {ARP_PATH, "-s", ip, mac, NULL};
#endif
			execve(ARP_PATH, args, NULL);
			_exit(EXIT_SUCCESS);
		}
		if (fork_res != 0) {
			wait(NULL);
		}
	}

	result = TRUE;
update_arp_end:

	if (ip) safe_free(&ip);
	if (mac) safe_free(&mac);
	if (network) safe_free(&network);
	if (arp_file_name) safe_free(&arp_file_name);
	if (tmp_file_name) safe_free(&tmp_file_name);
	if (buff) safe_free(&buff);
	if (tmp) safe_free(&tmp);
	if (unique) safe_free(&unique);

	return result;
}

unsigned int delete_arp(char * cmd)
{
	unsigned int result = FALSE;
	int fork_res;

	char * ip = NULL;
	char * network = NULL;
	unsigned int k =0, c = 0;

	ip = get_word_by_number(cmd, 3);
	if (!ip) {
		result = FALSE;
		goto delete_arp_end;
	}

	if (check_item_exists(config_data.denied_for, ip) == TRUE) {
		return_message = strdup("IP ADDRESS IS NOT ALLOWED");
		message("IP ADDRESS IS NOT ALLOWED: %s\n", ip);
		result = FALSE;
		goto delete_arp_end;
	};

	k = 0; c = 0;
	while (strlen(ip) > k && c != 2) {
		if (ip[k] == '.')
			c++;
		k++;
	}

	network = add_char_to_string(network, ip[k++]);
	while (ip[k] && ip[k] != '.') {
		network = add_char_to_string(network, ip[k++]);
	}

	if (!network) {
		message("Failed to get network\n");
		return_message = strdup("Failed to get network");
		result = FALSE;
		goto delete_arp_end;
	}

	if (check_item_exists(config_data.networks, network) == FALSE) {
		message("Network (%s is not supported\n", network);
		return_message = merge_strings(3, "Network (", network , " is not supported");
		result = FALSE;
		goto delete_arp_end;
	}

	if (ROOT_COMPILE) {
		// We should reload arp...

		fork_res = vfork();
		if (fork_res == 0){
			// After fork here child thread
			char *args[4] = {ARP_PATH, "-d", ip, NULL};
			execve(ARP_PATH, args, NULL);
			_exit(EXIT_SUCCESS);
		}
		if (fork_res != 0) {
			wait(NULL);
		}
	}

	result = TRUE;
delete_arp_end:

	if (ip) safe_free(&ip);
	if (network) safe_free(&network);

	return result;
}

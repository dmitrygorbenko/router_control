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

#include <ctype.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <csignal>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "sys/wait.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include "define.h"
#include "dialog.h"
#include "config.h"
#include "message.h"
#include "params.h"
#include "str.h"
#include "tools.h"
#include "main.h"

#include "ping.h"

using namespace std;

extern struct params_def startup_params;
extern struct mess_state_def m_state;
extern struct config_data_def config_data;
extern struct mess_state_def m_state;
extern unsigned int broken_pipe;
struct { uid_t real; uid_t effective; } uid_startup;

unsigned int child_count = 0;
char * process_id;

void Signal_TERM(int sig)
{
	if (sig == SIGINT)
		p_error("Cought SIGINT\n");
	else if (sig == SIGHUP)
		p_error("Cought SIGHUP\n");
	else if (sig == SIGTERM)
		p_error("Cought SIGTERM\n");
	else if (sig == SIGSEGV)
		p_error("Cought SIGSEGV\n");

	config_shutdown();
	message_shutdown();

	exit(0);
};

void Signal_PIPE(int sig)
{
        if (sig == SIGPIPE) {
                broken_pipe = TRUE;
        }

	return;
};

void Signal_CHILD(int sig)
{
	if (sig == SIGCHLD) {
		wait(NULL);
		child_count--;
	}
	return;
};

void Set_Signal_Handler(void)
{
	signal(SIGINT,   Signal_TERM);
	signal(SIGHUP,   Signal_TERM);
	signal(SIGTERM,  Signal_TERM);
	signal(SIGSEGV,  Signal_TERM);
	signal(SIGPIPE,  Signal_PIPE);
	signal(SIGCHLD,  Signal_CHILD);
};

unsigned int become_root()
{
	uid_t real;
	uid_t effective;

	real = getuid();
	effective = geteuid();

	uid_startup.real = real;
	uid_startup.effective = effective;

	if (real == 0) {
		return TRUE;
	}

	if (effective != 0) {
		p_error("You are not root or file haven't '+s' flag or file owner is not 'root'\n");
		return FALSE;
	}

	// Becoming root...

	if (setreuid(effective, effective) == -1) {
		p_error("I need root privileges to do my work.\n");
		return FALSE;
	}

	return TRUE;
};

void shutdown()
{
	config_shutdown();
	message_shutdown();

	exit(0);
}

int main(int argc, char *argv[])
{
	unsigned int result = FALSE;
	int listenfd = 0;
	int clientfd = 0;
	int rr = 0;
	int x = 1, n = 0;
	int fork_res;
	char ** IPs, *IP, ** copy, *tmp;

	struct sockaddr_in listaddr;
	struct sockaddr_in cliaddr;
	socklen_t clilen;

	Set_Signal_Handler();

// *********************************************
// Startup Init

	m_state.log_init = FALSE;
	process_id = strdup("main");
	reset_params();

// startup params
	if (parse_params(argc, argv) != TRUE) {
		p_error("Failed on parse_params()\n");
		exit(0);
	}
	if (startup_params.view_help == TRUE)
		exit(0);

// become root
	if (ROOT_COMPILE) {
		if (become_root() != TRUE) {
			p_error("Can't become root\n");
			exit(0);
		}
	}

// log subsystem
	if (startup_params.alternative_log == TRUE)
		result = message_init(startup_params.log_file);
	else
		result = message_init(LOG_FILE);
	if (result == FALSE) {
		p_error("Failed on message_init()\n");
		exit(0);
	}

// config file
	if (startup_params.alternative_config == TRUE)
		result = config_init(startup_params.config_file);
	else
		result = config_init(CONFIG_FILE);

	if (result == FALSE) {
		p_error("Failed on config_init()\n");
		shutdown();
	}

	result = read_config();
	if (result == FALSE) {
		p_error("Failed on read_config()\n");
		shutdown();
	}

	if (check_config() == FALSE) {
		p_error("Failed on check_config()\n");
		shutdown();
	}

// *********************************************
// Network Init

	IPs = split(config_data.listen_ip, ',');

	n = 0; copy = IPs;
	while(*IPs) {
		n++;
		*IPs++;
	}

	IPs = copy;

	if (n > 1) {
		while(*IPs) {
			fork_res = fork();

			if (fork_res == 0){
				tmp = int_to_string(n);
				process_id = sts(&process_id, tmp);
				safe_free(&tmp);
				goto new_child;
			}

			*IPs++;

			if (--n == 1)
				break;
		}
	}

new_child:
	IP = *IPs;

	if (!IP) {
		p_error("Bad IP address: null\n");
		shutdown();
	}

	if (strlen(IP) == 0) {
		p_error("Bad IP address: zero length\n");
		shutdown();
	}

	listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenfd < 0) {
		p_error("Failed on socket()\n");
		shutdown();
	}

	rr = setsockopt (listenfd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof (x));

	if (rr == -1) {
		p_error("listenfd reuseaddr failed\n");
		shutdown();
	}

	memset((void *) &listaddr, '\0', (size_t) sizeof(listaddr));

	listaddr.sin_family = AF_INET;
	result = get_s_addr(IP, &(listaddr.sin_addr));
	if (result == FALSE) {
		p_error("Failed on get_s_addr() (IP: %s)\n", IP);
		shutdown();
	}

	listaddr.sin_port = htons(config_data.listen_port);

	if (bind(listenfd, (struct sockaddr *) &listaddr,  sizeof(listaddr)) < 0) {
		p_error("Failed on bind() (IP: %s, port: %d)\n", config_data.listen_ip, config_data.listen_port);
		shutdown();
	}

	if (listen(listenfd, 256) < 0) {
		p_error("Failed on listen()\n");
		shutdown();
	}

// *********************************************
// Some isues

	if (startup_params.become_daemon == TRUE) {
		if (daemon(0, 1) == -1) {
			p_error("Failed on daemon()\n");
			shutdown();
		}
	}

// *********************************************
// Some isues

	message("Server started...\n");

// *********************************************
// Main loop

	for (;;) {
		memset((void *) &cliaddr, '\0', sizeof(cliaddr));

		clilen = (socklen_t) sizeof(cliaddr);

		clientfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

		if (clientfd != 0 && child_count < CHILD_MAX_COUNT) {
			fork_res = fork();

			if (fork_res == 0){
				safe_free(&process_id);
				process_id = create_unique_id();
				begin_dialog(clientfd, cliaddr);
				safe_free(&process_id);
				exit(EXIT_SUCCESS);
			}
			if (fork_res != 0) {
				child_count++;
				close(clientfd);
			}
		}
		else {
			if (clientfd != 0)
				close(clientfd);
		}
		close(clientfd);
	}

// *********************************************
// Shutdown

	config_shutdown();
	message_shutdown();
	safe_free(&process_id);

	exit(0);
	/* silence compiler warnings */
	return EXIT_SUCCESS;
	(void) argc;
	(void) argv;
};

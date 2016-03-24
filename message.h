/***************************************************************************
 *   Copyright (C) 2003, 2004, 2005                                              *
 *   by Dmitriy Gorbenko. "XAI" University, Kharkov, Ukraine               *
 *   e-mail: nial@ukr.net                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

struct mess_state_def {
	unsigned int 	log_open;
	unsigned int 	log_init;
	FILE *		log_fd;
	char * 		log_file_name;
};

char * get_date_t1();
void message(char *format, ...);
void p_error(char *format, ...);
void __message(char * msg_string);
void __p_error(char * err_string);
unsigned int message_init(char * log_file_name);
void message_close();
void message_shutdown();

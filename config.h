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

struct config_state_def {
	unsigned int	cfg_exists;
	unsigned int	cfg_read_startup;
	unsigned int	cfg_open;
	FILE *	cfg_fd;
	char *	cfg_file_name;

};

struct config_data_def {
	unsigned int	listen_port;
	char *	listen_ip;
	char *	allow_from;
	char *	networks;
	char *	mac_dir;
	char *	denied_for;
};

void print_config();
void config_shutdown();
void config_close();
unsigned int config_init(char * file_name);
unsigned int read_config();
unsigned int check_config();


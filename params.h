/***************************************************************************
 *   Copyright (C) 2005 by Dmitriy Gorbenko                                *
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

struct params_def {
	unsigned int view_help;
	unsigned int alternative_config;
	unsigned int alternative_log;
	unsigned int become_daemon;
	unsigned int silence;

	char * config_file;
	char * log_file;
};

void print_help();
void reset_params();
unsigned int parse_params(int counter, char **values);

/***************************************************************************
 *   Copyright (C) 2003, 2004, 2005, 2006                                  *
 *   by Dmitriy Gorbenko. "XAI" University, Kharkov, Ukraine               *
 *   e-mail: nial@ukr.net                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#include <cstring>
#endif

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>

#include "param.h"
#include "html.h"
#include "define.h"
#include "message.h"
#include "str.h"
#include "sql.h"
#include "common.h"
#include "conf.h"

using namespace std;

char * get_time()
{
	static time_t t;
	struct tm * struct_time;
	char * res_time = NULL;

	t = time(0);
	struct_time = localtime(&t);
	res_time = strdup(asctime(struct_time));
	free(struct_time);

	return res_time;
};

char * get_price_for_domain(char * z_name, unsigned int lease_time)
{
	char * price = NULL;
	char * query = NULL;
	unsigned int result;
	unsigned int price_d = 0;

	if (!z_name)
		return NULL;
	if (lease_time < 1 || lease_time > 2)
		return NULL;

	query = strdup("SELECT price FROM domain_cfg WHERE d_zone = '");
	query = sts (&query, z_name);
	query = sts (&query, "'");
	result = sql_make_query(query);
	safe_free(&query);

	if (result == SQL_RESULT_ERROR)
		return NULL;

	if (result == SQL_RESULT_EMPTY)
		return NULL;

	price_d = atoi(sql_get_value(0, 0));

	price = int_to_string(price_d * lease_time);

	return price;
};

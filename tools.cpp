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

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/times.h>
#include <time.h>
#include <netdb.h>

#include "define.h"
#include "str.h"
#include "message.h"
#include "tools.h"

using namespace std;

extern int h_errno;

char * create_unique_id()
{
	time_t tm = 0;
	struct tms *tmsbuf = NULL;
	clock_t ticks;
	char base62_chars[63] = {"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};
	char yield1[7] = {"------"};
	char yield2[7] = {"------"};
	char yield3[3] = {"--"};
	unsigned int value = 0;
	char * result = NULL;
	signed int i;

	tm = time(NULL);
	tmsbuf = (struct tms *) malloc(sizeof(struct tms));
	ticks = times(tmsbuf);

	value = tm;
	for (i = 5; i >= 0; i--) {
		yield1[i] = base62_chars[value % 62];
		value = value / 62;
	}

	value = getpid();
	for (i = 5; i >= 0; i--) {
		yield2[i] = base62_chars[value % 62];
		value = value / 62;
	}

	value = ticks;
	for (i = 1; i >= 0; i--) {
		yield3[i] = base62_chars[value % 62];
		value = value / 62;
	}

	result = strdup(yield1);
	result = sts(&result, "-");
	result = sts(&result, yield2);
	result = sts(&result, "-");
	result = sts(&result, yield3);

	free(tmsbuf);

	return result;
}

static char dec64table[] = {
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, /*  0-15 */
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, /* 16-31 */
	255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63, /* 32-47 */
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,255,255,255, /* 48-63 */
	255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, /* 64-79 */
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255, /* 80-95 */
	255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /* 96-111 */
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255  /* 112-127*/
};

static char *enc64table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_decode(char *code, char **ptr) {
	register int x, y;
	char *result = (char *)malloc(3*(strlen(code)/4) + 1);

	*ptr = result;

	/* Each cycle of the loop handles a quantum of 4 input bytes. For the last
	quantum this may decode to 1, 2, or 3 output bytes. */

	while ((x = (*code++)) != 0) {
		if (x > 127 || (x = dec64table[x]) == 255)
			return -1;
		if ((y = (*code++)) == 0 || (y = dec64table[y]) == 255)
			return -1;

		*result++ = (x << 2) | (y >> 4);

		if ((x = (*code++)) == '=') {
			if (*code++ != '=' || *code != 0)
				return -1;
		}
		else {
			if (x > 127 || (x = dec64table[x]) == 255)
				return -1;
			*result++ = (y << 4) | (x >> 2);
			if ((y = (*code++)) == '=') {
				if (*code != 0)
					return -1;
			}
			else {
				if (y > 127 || (y = dec64table[y]) == 255)
					return -1;
				*result++ = (x << 6) | y;
			}
		}
	}

	*result = 0;
	return result - *ptr;
}

char * base64_encode(char *clear, int len)
{
	char *code = (char *)malloc(4*((len+2)/3) + 1);
	char *p = code;

	while (len-- > 0) {
		register int x, y;

		x = *clear++;
		*p++ = enc64table[(x >> 2) & 63];

		if (len-- <= 0) {
			*p++ = enc64table[(x << 4) & 63];
			*p++ = '=';
			*p++ = '=';
			break;
		}

		y = *clear++;
		*p++ = enc64table[((x << 4) | ((y >> 4) & 15)) & 63];

		if (len-- <= 0) {
			*p++ = enc64table[(y << 2) & 63];
			*p++ = '=';
			break;
		}

		x = *clear++;
		*p++ = enc64table[((y << 2) | ((x >> 6) & 3)) & 63];

		*p++ = enc64table[x & 63];
	}

	*p = 0;

	return code;
}

unsigned int get_s_addr(char * name, in_addr *result)
{
	int res = 0;
	struct hostent * host_ip;

	res = inet_aton(name, result);

	if (!res) {
		host_ip = gethostbyname (name);

		if (!host_ip) {
			if (h_errno == HOST_NOT_FOUND)
				p_error("get_s_addr(): gethostbyname(): HOST_NOT_FOUND (%s)\n", name);

			return FALSE;
		}

		result->s_addr = (in_addr_t)host_ip->h_addr;
	}

	return TRUE;
}

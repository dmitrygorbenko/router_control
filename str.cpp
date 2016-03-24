/***************************************************************************
 *   Copyright (C) 2003, 2004, 2005                                        *
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
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <stdarg.h>
#include "ctype.h"

#include "define.h"
#include "str.h"

using namespace std;

char * safe_strdup(char * str)
{
	char * result = NULL;
	unsigned int len = 0;

	if (!str)
		return NULL;
	else
		len = strlen(str);

	result = (char*) calloc(len + 1, sizeof(char));

	if (!result)
		return NULL;

	strncat(result, str, len);

	result[len] = '\0';

	return result;
};

#ifdef FREEBSD
char * strndup (char * old, unsigned int size)
{
	char * result = NULL;

	result = strdup(old);

	if (strlen(result) > size)
	result[size] = 0;

        return result;
}
#endif

void safe_free(char ** str)
{
	if (!(*str))
		return;

	free(*str);
	*str = NULL;
};

char * read_string_3 (FILE * file)
{
	unsigned int i = 0;
	char * str = NULL;
	char buf[MAX_READ_COUNT+1];

	do {
		if (i == 0)
			str = (char*)calloc(MAX_READ_COUNT+1,sizeof(char));
		else
			str = (char *)realloc(str,strlen(str) + MAX_READ_COUNT + i);

		memset(buf,0,MAX_READ_COUNT+1);
		fgets(buf,MAX_READ_COUNT+1,file);

		strcat(str,buf);

		if (buf[strlen(buf)-1] == '\n') {
			str[strlen(str)-1] = '\0';
			break;
		}

		i++;
	} while (!feof(file));

	return str;
};

char * read_string_2(FILE * file)
{
	char * str = NULL;

	do {
		if (str)
			safe_free(&str);
		str = read_string_3(file);
	}
	while ( (str[0] == '#' || (strstr(str, "#") != (char) NULL)) && !feof(file) );

	if (!str)
		return NULL;

	return str;
};

char * read_string(FILE * file)
{
	char * str = NULL;
	char * buf = NULL;
	unsigned int result = TRUE;

	do {
		buf = NULL;
		str = read_string_2(file);
		buf = get_first_word(str);

		if (buf == NULL) {
			safe_free(&str);
			result = FALSE;
		}
		else {
			safe_free(&buf);
			result = TRUE;
		}
	}
	while (result == FALSE && !feof(file));

	return str;
};

unsigned int is_white_space(char c) {
	if (c == ' ' || c == '\t' || c == '\n')
		return TRUE;

	return FALSE;
}

unsigned int is_not_white_space(char c) {
	if (is_white_space(c))
		return FALSE;

	return TRUE;
}

char * cut_quotes(char * str)
{
	unsigned int i = 0;
	unsigned int len = 0;
	unsigned int len1 = 0;
	unsigned int len2 = 0;
	unsigned int find_quote = 0;
	char * buf = NULL;
	char * temp = NULL;
	char * result = NULL;

	if (!str)
		return NULL;

	len = strlen(str);

	for (i=0; i<len && !find_quote; i++)
		if (str[i] == '\"')
			find_quote = 1;

	if (find_quote == 0) {
		result = safe_strdup(str);
		return result;
	}

	if (i >= (len-1))
		return NULL;

	buf = strchr(str,'\"');
	buf++;
	buf = safe_strdup(buf);

	len1 = strlen(buf);

	temp = strrchr(buf,'\"');
	if (temp == NULL)
		return NULL;
	len2 = strlen(temp);

	result = (char *)strndup(buf, len1-len2);

	if (buf)
		safe_free(&buf);

	return result;
};

char * get_first_word(char *str)
{
	unsigned int i = 0;
	unsigned int len = 0;
	char * result = NULL;
	char * copy = NULL, * back_copy = NULL;

	/* is string correct ? */
	if(!str)
		return NULL;

	/* we should work with copy */
	copy = safe_strdup(str);
	if (!copy)
		return NULL;

	back_copy = copy;
	len = strlen(copy); /* if we don't want receive SIGSEGV */

	/* skip white spaces */
	while ((*copy == ' ' || *copy == '\t') && i < len)
		{ copy++; i++; }

	/* where are only white spaces -> return */
	if (i >= len) { // >= is really need
		copy = back_copy;
		safe_free(&copy);
		return NULL;
	}

	len = strlen(copy);  /* if we don't want receive SIGSEGV */
	i = 0;
	while ((copy[i] != ' ' && copy[i] != '\t') && i < len)
		i++;

	result = (char *)strndup(copy,i);
	if (!result) {
		copy = back_copy;
		safe_free(&copy);
		return NULL;
	}

	/* restore copy for free memory */
	copy = back_copy;
	safe_free(&copy);

	return result;
};

char * del_first_word(char *str)
{
	unsigned int i = 0, len = 0;
	char * buf = NULL;
	char * copy = NULL;

	if(!str)
		return NULL;

	/* save str - we need it for free(str) bellow */
	copy = str;

	len = strlen(str);

	/* skipping white spaces at begining */
	while ((*str == ' ' || *str == '\t') && i < len)
		{ str++; i++; }

	if(i == len) /* where are only spaces in str - return NULL */
	/* and keep `str` is same */
		return NULL;

	/* one step to word */
	/* `( )word' */
	str++; i++;
	/* ` (w)ord' */

	/* move to the end of word */
	while (*str != ' ' && *str != '\t' && i < len)
		{ str++; i++; }

	/* check, if we have only white spaces at end*/
	while ((*str == ' ' || *str == '\t') && i < len)
		{ str++; i++; }

	/*
	 * (i == len) means where are nothing after word - return NULL
	 * but we do not need to do something, because buf already == NULL,
	 * and next if(...) == FALSE, because i==len .
	 */

	/* It's all right */
	if (strlen(str) != 0 && i < len)
		buf = safe_strdup(str);

	/* recover str for free */
	str = copy;
	safe_free(&str);

	if (!buf)
		return NULL;

	return buf;
};

char smart_get_byte(char * str, unsigned int pos)
{
	unsigned int len = 0;
	char byte;

	if(!str)
		return 0;

	len = strlen(str);
	if (pos >= len)
		return 0;

	byte = str[pos];

	return byte;
};

char * add_char_to_string(char * str, char byte)
{
	char * result = NULL;
	unsigned int len = 0;

	if (!str)
		len = 0;
	else
		len = strlen(str);

	result = (char*) calloc(len+2, sizeof(char));

	if (!result)
		return NULL;

	if (len != 0)
		strcat(result,str);

	result[len] = byte;
	result[len+1] = '\0';

	if (str)
		safe_free(&str);

	return result;
};

unsigned int how_much_words(char * str)
{
	unsigned int count = 0;
	unsigned int i = 0;
	unsigned int len = 0;
	unsigned int above_word = FALSE;

	if (!str)
		return 0;

	len = strlen(str);

	while (*str) {
		if (i >= len) {
			if (above_word)
				count++;
			break;
		}

		/* s(k)ip white spaces */
		if (above_word == TRUE) {
			/* skip( )white spaces */
			if (is_white_space(*str)) {
				count++;
				above_word = FALSE;
			}
		}
		/*( )skip white spaces */
		else {
			/* (s)kip white spaces */
			if (is_not_white_space(*str)) {
				above_word = TRUE;
			}
		}

		str++;
		i++;
	}

	if (above_word)
		count++;

	return count;
}

unsigned int get_word_position(char * str, char * example)
{
	unsigned int count = 0;
	unsigned int position = 0;
	char * copy = NULL;
	char * temp = NULL;

	if (!str)
		return 0;

	if (!example)
		return 0;

	copy = safe_strdup(str);

	while(copy) {
		temp = get_first_word(copy);
		if (strcmp(temp, example) == 0)
			position = count;
		copy = (char *)del_first_word(copy);
		count++;
		if (temp)
			safe_free(&temp);
	}

	return position;
}

char * get_and_del_first_word(char **str)
{
	char * result = NULL;

	if (!str)
		return NULL;

	if (!(*str))
		return NULL;

	result = get_first_word(*str);
	*str = del_first_word(*str);

	return result;
}

char * get_word_by_number(char *str, unsigned int k)
{
	unsigned int i = 0;
	unsigned int len = 0;
	unsigned int above_word = FALSE;
	unsigned int change_flag = FALSE;
	unsigned int skipped = 0;
	char * result = NULL;
	char * copy = NULL;
	char c = 0;

	if (!str)
		return NULL;

	if (how_much_words(str) < k)
		return NULL;

	if (k < 1)
		return NULL;

	len = strlen(str);

	// now, move to our word...
	// only if we do not need first word
	while (*str) {
		if (i >= len) {
			break;
		}

		/* s(k)ip white spaces */
		if (above_word == TRUE) {
			/* skip( )white spaces */
			if (is_white_space(*str)) {
				skipped++;
				above_word = FALSE;
			}
		}
		/*( )skip white spaces */
		else {
			/* (s)kip white spaces */
			if (is_not_white_space(*str)) {
				above_word = TRUE;
			}
		}

		// if we need first word and first char is space,
		// we should prevent situatinon, then first char
		// of first word become absent
		if (above_word != TRUE || i != 0)
			str++;

		if (skipped == (k - 1))
			break;

		i++;
	}

	// skip white spaces before word we need
	while (*str && is_white_space(*str))
		str++;

	copy = str;

	// move to the next word after one we need
	while (*str && is_not_white_space(*str))
		str++;

	if (*str) {
		c = *str;
		*str = '\0';
		change_flag = TRUE;
	}

	result = strdup(copy);

	if (change_flag == TRUE) {
		*str = c;
	}

	return result;
}

char * sts(char ** str, char* add)
{
	char * result = NULL;
	unsigned int len = 0;
	unsigned int len_add = 0;
	unsigned int i = 0;

	if (!str)
		len = 0;
	else {
		if (!(*str))
			len = 0;
		else
			len = strlen((*str));
	}

	if (!add)
		len_add = 0;
	else
		len_add = strlen(add);

	result = (char*) calloc(len + len_add + 1, sizeof(char));

	if (!result)
		return NULL;

	if (len != 0) {
		strcat(result, (*str));
		i = 0;
		while (i < len_add)
			result[len + i++] = add[i];
	}
	else {
		if (len_add != 0)
			strcat(result, add);
	}

	result[len + len_add] = '\0';

	if (str)
		if ((*str))
			safe_free(str);

	return result;
};

char * int_to_string(int i)
{
	char * result = NULL;
	char tmp = 0;
	unsigned int len = 0;
	unsigned int k = 0;
	unsigned int sign = FALSE;

	if (i < 0) {
		result = add_char_to_string(result, '-');
		i = -i;
		sign = TRUE;
	}

	if (i == 0) {
		result = add_char_to_string(result, '0');
		return result;
	}

	while (i) {
		result = add_char_to_string(result, (i % 10) + 48);
		i = i / 10;
	}

	if (sign == TRUE)
		result++;

	len = strlen(result);

	for(k=0; k<(len/2); k++) {
		tmp = result[k];
		result[k] = result[len-k-1];
		result[len-k-1] = tmp;
	}

	if (sign == TRUE)
		result--;

	return result;
};

char * get_param(char * str)
{
	char * buf = NULL;
	char * temp = NULL;

	if (!str)
		return NULL;

	buf = strstr(str,"=");
	buf = safe_strdup(++buf);

	if (how_much_words(buf) == 0) {
		if (buf)
			safe_free(&buf);
		return NULL;
	}

	temp = cut_quotes(buf);

	if (!temp) {
		if (buf)
			safe_free(&buf);
		return NULL;
	}

	if (buf)
		safe_free(&buf);

	buf = temp;

	temp = trim(buf);

	if (buf)
		safe_free(&buf);

	return temp;
};

char * trim(char * str)
{
	unsigned int i = 0;
	unsigned int k = 0;
	unsigned int len = 0;
	char * result = NULL;
	char * copy = NULL, * back_copy = NULL;

	/* is string correct ? */
	if(!str)
		return NULL;

	/* we should work with copy */
	copy = safe_strdup(str);
	if (!copy)
		return NULL;

	back_copy = copy;
	len = strlen(copy); /* if we don't want receive SIGSEGV */

	/* skip white spaces at the beginning*/
	while ((*copy == ' ' || *copy == '\t') && i < len)
		{ copy++; i++; }

	/* where are only white spaces -> return */
	if (i >= len) { // >= is really need
		copy = back_copy;
		safe_free(&copy);
		return NULL;
	}

	i = strlen(copy) - 1;

	/* skip white spaces at the end */
	while ((copy[i] == ' ' || copy[i] == '\t') && i > 0)
		{ i--; k++; }

	result = (char *)strndup(copy, strlen(copy) - k);

	/* restore copy for free memory */
	copy = back_copy;
	safe_free(&copy);

	return result;
};

// From Exim
unsigned int string_vformat(char *buffer, int buflen, char *format, va_list ap)
{
	enum { L_NORMAL, L_SHORT, L_LONG, L_LONGLONG, L_LONGDOUBLE };

	unsigned int yield = TRUE;
	int width, precision;
	char *fp = format;             /* Deliberately not unsigned */
	char *p = buffer;
	char *last = buffer + buflen - 1;

	/* Scan the format and handle the insertions */

	while (*fp != 0) {
		int length = L_NORMAL;
		int *nptr;
		int slen;
		char *null = "NULL";         /* ) These variables */
		char *item_start, *s;        /* ) are deliberately */
		char newformat[16];          /* ) not unsigned */

		/* Non-% characters just get copied verbatim */

		if (*fp != '%') {
			if (p >= last) {
				yield = FALSE;
				break;
			}
			*p++ = (char)*fp++;
			continue;
		}

		/* Deal with % characters. Pick off the width and precision, for checking
		strings, skipping over the flag and modifier characters. */

		item_start = fp;
		width = precision = -1;

		if (strchr("-+ #0", *(++fp)) != NULL) {
			if (*fp == '#')
				null = "";
			fp++;
		}

		if (isdigit((char)*fp)) {
			width = *fp++ - '0';
			while (isdigit((char)*fp))
				width = width * 10 + *fp++ - '0';
		}
		else if (*fp == '*') {
			width = va_arg(ap, int);
			fp++;
		}

		if (*fp == '.') {
			if (*(++fp) == '*') {
				precision = va_arg(ap, int);
				fp++;
			}
			else {
				precision = 0;
				while (isdigit((char)*fp))
					precision = precision*10 + *fp++ - '0';
			}
		}

		/* Skip over 'h', 'L', 'l', and 'll', remembering the item length */

		if (*fp == 'h') {
			fp++;
			length = L_SHORT;
		}
		else if (*fp == 'L') {
			fp++;
			length = L_LONGDOUBLE;
		}
		else if (*fp == 'l') {
			if (fp[1] == 'l') {
				fp += 2;
				length = L_LONGLONG;
			}
			else {
				fp++;
				length = L_LONG;
			}
		}

		/* Handle each specific format type. */

		switch (*fp++) {
			case 'n':
				nptr = va_arg(ap, int *);
				*nptr = p - buffer;
				break;

			case 'd':
			case 'o':
			case 'u':
			case 'x':
			case 'X':
				if (p >= last - ((length > L_LONG)? 24 : 12)) {
					yield = FALSE;
					goto END_FORMAT;
				}
				strncpy(newformat, item_start, fp - item_start);
				newformat[fp - item_start] = 0;

				/* Short int is promoted to int when passing through ..., so we must use
				int for va_arg(). */

				switch(length) {
					case L_SHORT:
					case L_NORMAL:   sprintf( p, newformat, va_arg(ap, int)); break;
					case L_LONG:     sprintf( p, newformat, va_arg(ap, long int)); break;
					case L_LONGLONG: sprintf( p, newformat, va_arg(ap, long long int)); break;
				}
				while (*p)
					p++;
				break;

			case 'p':
				if (p >= last - 24) {
					yield = FALSE;
					goto END_FORMAT;
				}
				strncpy(newformat, item_start, fp - item_start);
				newformat[fp - item_start] = 0;
				sprintf( p, newformat, va_arg(ap, void *));
				while (*p)
					p++;
				break;

			/* %f format is inherently insecure if the numbers that it may be
			handed are unknown (e.g. 1e300). However, in Exim, %f is used for
			printing load averages, and these are actually stored as integers
			(load average * 1000) so the size of the numbers is constrained.
			It is also used for formatting sending rates, where the simplicity
			of the format prevents overflow. */

			case 'f':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
				if (precision < 0)
					precision = 6;
				if (p >= last - precision - 8) {
					yield = FALSE;
					goto END_FORMAT;
				}
				strncpy(newformat, item_start, fp - item_start);
				newformat[fp-item_start] = 0;
				if (length == L_LONGDOUBLE)
					sprintf( p, newformat, va_arg(ap, long double));
				else
					sprintf( p, newformat, va_arg(ap, double));
				while (*p)
					p++;
				break;

			/* String types */

			case '%':
				if (p >= last) {
					yield = FALSE;
					goto END_FORMAT;
				}
				*p++ = '%';
				break;

			case 'c':
				if (p >= last) {
					yield = FALSE;
					goto END_FORMAT;
				}
				*p++ = va_arg(ap, int);
				break;

			case 's':
			case 'S':                   /* Forces *lower* case */
				s = va_arg(ap, char *);

//				INSERT_STRING:              /* Come to from %D above */
				if (s == NULL)
					s = null;
				slen = strlen(s);

				/* If the width is specified, check that there is a precision
				set; if not, set it to the width to prevent overruns of long
				strings. */

				if (width >= 0) {
					if (precision < 0)
						precision = width;
				}

				/* If a width is not specified and the precision is specified, set
				the width to the precision, or the string length if shorted. */

				else if (precision >= 0) {
					width = (precision < slen)? precision : slen;
				}

				/* If neither are specified, set them both to the string length. */


				else
					width = precision = slen;

				/* Check string space, and add the string to the buffer if ok. If
				not OK, add part of the string (debugging uses this to show as
				much as possible). */

				if (p >= last - width) {
					yield = FALSE;
					width = precision = last - p - 1;
				}
				sprintf( p, "%*.*s", width, precision, s);
				if (fp[-1] == 'S')
					while (*p) {
						*p = tolower(*p);
						p++;
					}
				else
					while (*p)
						p++;
				if (!yield)
					goto END_FORMAT;
				break;

			/* Some things are never used in Exim; also catches junk. */

			default:
				strncpy(newformat, item_start, fp - item_start);
				newformat[fp-item_start] = 0;
				break;
		}
	}

	/* Ensure string is complete; return TRUE if got to the end of the format */

	END_FORMAT:

	*p = 0;
	return yield;
}

// From Exim
char * string_sprintf(char *format, ...)
{
	va_list ap;
	char buffer[STRING_SPRINTF_BUFFER_SIZE];
	va_start(ap, format);

	if (!string_vformat(buffer, sizeof(buffer), format, ap))
  		return NULL;

	va_end(ap);

	return safe_strdup(buffer);
}

char * merge_strings(unsigned int count, ...)
{
	va_list ap;
	char *result = NULL;

	va_start(ap, count);

	for(;count > 0; count--)
		result = sts(&result, va_arg(ap, char *));

	va_end(ap);

	return result;
}

unsigned int check_item_exists(char * params, char * item)
{
	unsigned int result = 0;
	unsigned int not_found = TRUE;
	unsigned int inside_quote = FALSE;
	unsigned int len = 0, i = 0;
	char * temp = NULL;

	if (!params || !item)
		return FALSE;

	if (strlen(params) == 0 || strlen(item) == 0)
		return FALSE;

	len = strlen(params);

	while (not_found) {

		for(; i < len; i++) {

			if (params[i] == '\'' || params[i] == '"') {
				if (inside_quote == TRUE)
					inside_quote = FALSE;
				else
					inside_quote = TRUE;
				continue;
			}

			if (params[i] == ',' && inside_quote == FALSE)
				break;

			if (params[i] == ' ' && inside_quote == FALSE)
				continue;

			temp = add_char_to_string(temp, params[i]);
		}
		i++;

		if (strcmp(temp, item) == 0) {
			not_found = FALSE;
			result = TRUE;
		}

		if (i >= len) {
			not_found = FALSE;
		}

		if (temp)
			safe_free(&temp);
	}

	return result;
}

char** split(char* params, char delim)
{
	int n = 1;
	char *it, *beg = NULL, *tmp;
	char **result;

	/* Check argument */
	if( params == NULL || *params == 0 )
		return NULL;

	/* Counting the number of delims in the string. */
	for (it = params; *it != 0; ++it )
		if (*it == delim) {
			++n;
		}

	/* Allocate result. */
	result = (char**)malloc( (n + 1) * sizeof( char* ) );
	result[n] = NULL;

	/* Fill result */
	for (it = params, n=0; *it != 0; ++it)
	{
		if (beg == NULL)
			beg = it;

		if (*it == delim) {
			result[n] = strncpy((char*)calloc(it - beg + 1, sizeof(char)), beg, it - beg);
			tmp = result[n];
			result[n] = trim(tmp);
			safe_free(&tmp);
			beg = NULL;
			n++;
		}
	}

	if (beg == NULL)
		beg = it;

	result[n] = strncpy((char*)calloc(it - beg + 1, sizeof(char)), beg, it - beg);
	tmp = result[n];
	result[n] = trim(tmp);
	safe_free(&tmp);

	return result;
}

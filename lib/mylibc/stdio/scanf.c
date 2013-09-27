/*
 *  linux/lib/vsprintf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  BingYao,Luo 2013-09-27
 *  - Use _gets in the scanf to print char input by
 *    user. Also support delete and backspace key.
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define USHRT_MAX       ((uint16_t)(~0U))
#define SHRT_MAX        ((int16_t)(USHRT_MAX >> 1))
#define SHRT_MIN        ((int16_t)(-SHRT_MAX - 1))

static int skip_atoi(const char **s)
{
	int i = 0;

	while (isdigit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

/**
 * skip_spaces - Removes leading whitespace from @str.
 * @str: The string to be stripped.
 *
 * Returns a pointer to the first non-whitespace character in @str.
 */
char *skip_spaces(const char *str)
{
	while (isspace(*str))
		++str;
	return (char *)str;
}

/**
 * vsscanf - Unformat a buffer into a list of arguments
 * @buf:	input buffer
 * @fmt:	format of buffer
 * @args:	arguments
 */
int vsscanf(const char *buf, const char *fmt, va_list args)
{
	const char *str = buf;
	char *next;
	char digit;
	int num = 0;
	uint8_t qualifier;
	uint8_t base;
	int16_t field_width;
	int is_sign;

	while (*fmt && *str) {
		/* skip any white space in format */
		/* white space in format matchs any amount of
		 * white space, including none, in the input.
		 */
		if (isspace(*fmt)) {
			fmt = skip_spaces(++fmt);
			str = skip_spaces(str);
		}

		/* anything that is not a conversion must match exactly */
		if (*fmt != '%' && *fmt) {
			if (*fmt++ != *str++)
				break;
			continue;
		}

		if (!*fmt)
			break;
		++fmt;

		/* skip this conversion.
		 * advance both strings to next white space
		 */
		if (*fmt == '*') {
			while (!isspace(*fmt) && *fmt != '%' && *fmt)
				fmt++;
			while (!isspace(*str) && *str)
				str++;
			continue;
		}

		/* get field width */
		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);

		/* get conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || tolower(*fmt) == 'l' ||
		    tolower(*fmt) == 'z') {
			qualifier = *fmt++;
			if (qualifier == *fmt) {
				if (qualifier == 'h') {
					qualifier = 'H';
					fmt++;
				} else if (qualifier == 'l') {
					qualifier = 'L';
					fmt++;
				}
			}
		}

		if (!*fmt || !*str)
			break;

		base = 10;
		is_sign = 0;

		switch (*fmt++) {
		case 'c':
		{
			char *s = (char *)va_arg(args, char*);
			if (field_width == -1)
				field_width = 1;
			do {
				*s++ = *str++;
			} while (--field_width > 0 && *str);
			num++;
		}
		continue;
		case 's':
		{
			char *s = (char *)va_arg(args, char *);
			if (field_width == -1)
				field_width = SHRT_MAX;
			/* first, skip leading white space in buffer */
			str = skip_spaces(str);

			/* now copy until next white space */
			while (*str && !isspace(*str) && field_width--)
				*s++ = *str++;
			*s = '\0';
			num++;
		}
		continue;
		case 'n':
			/* return number of characters read so far */
		{
			int *i = (int *)va_arg(args, int*);
			*i = str - buf;
		}
		continue;
		case 'o':
			base = 8;
			break;
		case 'x':
		case 'X':
			base = 16;
			break;
		case 'i':
			base = 0;
		case 'd':
			is_sign = 1;
		case 'u':
			break;
		case '%':
			/* looking for '%' in str */
			if (*str++ != '%')
				return num;
			continue;
		default:
			/* invalid format; stop here */
			return num;
		}

		/* have some sort of integer conversion.
		 * first, skip white space in buffer.
		 */
		str = skip_spaces(str);

		digit = *str;
		if (is_sign && digit == '-')
			digit = *(str + 1);

		if (!digit
		    || (base == 16 && !isxdigit(digit))
		    || (base == 10 && !isdigit(digit))
		    || (base == 8 && (!isdigit(digit) || digit > '7'))
		    || (base == 0 && !isdigit(digit)))
			break;

		switch (qualifier) {
		case 'H':	/* that's 'hh' in format */
			if (is_sign) {
				signed char *s = (signed char *)va_arg(args, signed char *);
				*s = (signed char)strtol(str, &next, base);
			} else {
				unsigned char *s = (unsigned char *)va_arg(args, unsigned char *);
				*s = (unsigned char)strtoul(str, &next, base);
			}
			break;
		case 'h':
			if (is_sign) {
				short *s = (short *)va_arg(args, short *);
				*s = (short)strtol(str, &next, base);
			} else {
				unsigned short *s = (unsigned short *)va_arg(args, unsigned short *);
				*s = (unsigned short)strtoul(str, &next, base);
			}
			break;
		case 'l':
			if (is_sign) {
				long *l = (long *)va_arg(args, long *);
				*l = strtol(str, &next, base);
			} else {
				unsigned long *l = (unsigned long *)va_arg(args, unsigned long *);
				*l = strtoul(str, &next, base);
			}
			break;
#if 0
		case 'L':
			if (is_sign) {
				long long *l = (long long *)va_arg(args, long long *);
				*l = simple_strtoll(str, &next, base);
			} else {
				unsigned long long *l = (unsigned long long *)va_arg(args, unsigned long long *);
				*l = simple_strtoull(str, &next, base);
			}
			break;
#endif
		case 'Z':
		case 'z':
		{
			size_t *s = (size_t *)va_arg(args, size_t *);
			*s = (size_t)strtoul(str, &next, base);
		}
		break;
		default:
			if (is_sign) {
				int *i = (int *)va_arg(args, int *);
				*i = (int)strtol(str, &next, base);
			} else {
				unsigned int *i = (unsigned int *)va_arg(args, unsigned int*);
				*i = (unsigned int)strtoul(str, &next, base);
			}
			break;
		}
		num++;

		if (!next)
			break;
		str = next;
	}

	/*
	 * Now we've come all the way through so either the input string or the
	 * format ended. In the former case, there can be a %n at the current
	 * position in the format that needs to be filled.
	 */
	if (*fmt == '%' && *(fmt + 1) == 'n') {
		int *p = (int *)va_arg(args, int *);
		*p = str - buf;
	}

	return num;
}

/**
 * sscanf - Unformat a buffer into a list of arguments
 * @buf:	input buffer
 * @fmt:	formatting of buffer
 * @...:	resulting arguments
 */
int sscanf(const char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsscanf(buf, fmt, args);
	va_end(args);

	return i;
}

#define BACKSP_KEY 0x08
#define DELETE_KEY 0x7F
char *_gets(char *s)
{
	int c, i = 0;

	do {
		c = getchar();
		if (c == DELETE_KEY || c == BACKSP_KEY) {
			if(i) {
				s[i] = '\0';
				i--;
				puts("\b \b");
			}
		} else if (c != -1 && c != '\r' && c != '\n') {
			s[i++] = (char)c;
			putchar(c);
		}
	} while(c && c != -1 && c != '\r' && c != '\n');

	s[i] = 0;

	return s;
}

/**
 * scanf - Unformat STDIN into a list of arguments
 * @fmt:	formatting of buffer
 * @...:	resulting arguments
 */
int scanf(const char *fmt, ...)
{
	char line[256];
	va_list args;
	int i = 0;

	do {
		_gets(line);

		if (strlen(line) <= 0)
			continue;
		va_start(args, fmt);
		i = vsscanf(line, fmt, args);
		va_end(args);
		break;
	} while (!line || !i);

	return i;
}

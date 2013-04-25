#ifndef _LINUX_CTYPE_H
#define _LINUX_CTYPE_H

/*
 * NOTE! This ctype does not handle EOF like the standard C
 * library is required to.
 */

#define isspace(c)      ((c) == ' ')
#define islower(c)      ((c) >= 'a' && (c) <= 'z')
#define isupper(c)      ((c) >= 'A' && (c) <= 'Z')
#define isascii(c)      (((unsigned char)(c))<=0x7f)
#define toascii(c)      (((unsigned char)(c))&0x7f)
#define tolower(c)      (isupper(c) ? ((char)(c) - (char)('A' - 'a')) : (c))
#define toupper(c)      (islower(c) ? ((char)(c) - (char)('a' - 'A')) : (c))
#define isdigit(c)      ((c) >= '0' && (c) <= '9')
#define isxdigit(c)     (isdigit(c) || (tolower(c) >= 'a' && tolower(c) <= 'f'))
#define isalpha(c)      (islower(c) || isupper(c))
#define isalnum(c)      (isalpha(c) || isdigit(c))
#define isprint(c)      (((c) >= 0x20) && ((c) < 0x7f))
#define ispunct(c)      (isprint(c) && !isalnum(c))

/*
 * Rather than doubling the size of the _ctype lookup table to hold a 'blank'
 * flag, just check for space or tab.
 */
#define isblank(c)	(c == ' ' || c == '\t')

#define isascii(c) (((unsigned char)(c))<=0x7f)
#define toascii(c) (((unsigned char)(c))&0x7f)

#endif

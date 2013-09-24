#ifndef _LINUX_STRING_H_
#define _LINUX_STRING_H_

#include <stddef.h>
#include <ctype.h>

char * strtok(char *,const char *);
char * strcpy(char *,const char *);
char * strncpy(char *,const char *, size_t);
size_t strlcpy(char *, const char *, size_t);
char * strcat(char *, const char *);
char * strncat(char *, const char *, size_t);
size_t strlcat(char *, const char *, size_t);
int strcmp(const char *,const char *);
int strncmp(const char *,const char *,size_t);
int strnicmp(const char *, const char *, size_t);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);
char * strchr(const char *,int);
char * strnchr(const char *, size_t, int);
char * strrchr(const char *,int);
char * strim(char *);

static inline char *strstrip(char *str)
{
	return strim(str);
}

char * strstr(const char *, const char *);
char * strnstr(const char *, const char *, size_t);
size_t strlen(const char *);
size_t strnlen(const char *,size_t);
char * strpbrk(const char *,const char *);
char * strsep(char **,const char *);
size_t strspn(const char *,const char *);
size_t strcspn(const char *,const char *);

void * memset(void *,int,size_t);
void * memcpy(void *,const void *,size_t);
void * memmove(void *,const void *,size_t);
void * memscan(void *,int,size_t);
int memcmp(const void *,const void *,size_t);
void * memchr(const void *,int,size_t);

/**
 * strstarts - does @str start with @prefix?
 * @str: string to examine
 * @prefix: prefix to look for.
 */
static inline int strstarts(const char *str, const char *prefix)
{
	return strncmp(str, prefix, strlen(prefix)) == 0;
}

#endif /* _LINUX_STRING_H_ */

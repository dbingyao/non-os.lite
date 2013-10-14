#include <stdlib.h>
#include <string.h>

/**
 * strnchr - Find a character in a length limited string
 * @s: The string to be searched
 * @count: The number of characters to be searched
 * @c: The character to search for
 */
char *strnchr(const char *s, size_t count, int c)
{
	for (; count-- && *s != '\0'; ++s) {
		if (*s == (char)c)
			return (char *)s;
	}

	return NULL;
}

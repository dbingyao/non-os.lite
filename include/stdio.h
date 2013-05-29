/*
 * Copyright (c) 2010, Dante Su (dantesu@faraday-tech.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the <organization>.
 * 4. Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>

#ifndef NULL
#define	NULL	0
#endif

/* End of file character.
   Some things throughout the library rely on this being -1.  */
#ifndef EOF
#define EOF 	(-1)
#endif

#ifndef BUFSIZ
#define	BUFSIZ			1024
#endif

#ifndef FOPEN_MAX
#define	FOPEN_MAX		4
#endif

#ifndef FILENAME_MAX
#define	FILENAME_MAX	1024
#endif

#ifndef SEEK_SET
#define	SEEK_SET		0	/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR		1	/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define	SEEK_END		2	/* set file offset to EOF plus offset */
#endif


#ifdef CONFIG_VFS_FATFS

#include <ff.h>

typedef FIL FILE;

void   ffeconv(FRESULT rs);	/* FatFS error code convert */
FILE  *fopen(const char *filename, const char *mode);
int    fclose(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);
int    fflush(FILE *stream);
int    fseek(FILE *stream, long int offset, int whence);
char   *fgets(char *s, int n, FILE *stream);
#define fputc(c, fs)				f_putc(c, fs)
#define fputs(s, fs)				f_puts(s, fs)
#define fprintf(fs, fmt, args...)	f_printf(fs, fmt, ## args)
#define feof(fs)					f_eof(fs)
#define ferror(fs)					f_error(fs)
#define ftell(fs)					(long int)f_tell(fs)

#endif	/* #ifdef CONFIG_VFS_FATFS */

/* Read/Write characters from/to stdin. */

#define getchar()	ftuart_getc()
#define putchar(c)	ftuart_putc(c)
#define gets(s) 	ftuart_gets(s)
#define puts(s) 	ftuart_puts(s)
#define kbhit() 	ftuart_kbhit()

/*
 * printf(...)
 */
int simple_sprintf (char * buf, const char *fmt, ...);
int simple_snprintf(char *buf, size_t sz, const char *fmt, ...);
int simple_printf  (const char *fmt, ...);

#define snprintf(buf, sz, fmt, args...)	simple_snprintf(buf, sz, fmt, ## args)
#define sprintf(buf, fmt, args...)		simple_sprintf(buf, fmt, ## args)
#define prints(fmt, args...)			simple_printf(fmt, ## args)

int sscanf(const char *buf, const char *fmt, ...);
int scanf(const char *fmt, ...);

#endif	/* #ifndef _STDIO_H */

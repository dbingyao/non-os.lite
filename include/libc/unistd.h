#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/types.h>

int     chdir(const char *path);
char   *getcwd(char *buf, size_t size);

int     close(int fildes);
ssize_t read(int fildes, void *buf, size_t nbyte);
ssize_t write(int fildes, const void *buf, size_t nbyte);
off_t   lseek(int fildes, off_t offset, int whence);
int     fsync(int fildes);
void    sync(void);

/* unsigned int sleep(unsigned int seconds); */
int     usleep(useconds_t us);

#endif

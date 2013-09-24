#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <stddef.h>

#ifndef __off_t_defined
typedef long _off_t;
#endif

#ifndef __dev_t_defined
typedef short __dev_t;
#endif

#ifndef __uid_t_defined
typedef unsigned short __uid_t;
#endif
#ifndef __gid_t_defined
typedef unsigned short __gid_t;
#endif

#ifndef __off64_t_defined
__extension__ typedef long long _off64_t;
#endif

/*
 * We need fpos_t for the following, but it doesn't have a leading "_",
 * so we use _fpos_t instead.
 */
#ifndef __fpos_t_defined
typedef long _fpos_t;		/* XXX must match off_t in <sys/types.h> */
				/* (and must be `long' for now) */
#endif

#ifdef __LARGE64_FILES
#ifndef __fpos64_t_defined
typedef _off64_t _fpos64_t;
#endif
#endif

typedef unsigned long useconds_t;
typedef long suseconds_t;

#ifndef __clock_t_defined
typedef unsigned long clock_t;
#define __clock_t_defined
#endif

#ifndef __clockid_t_defined
typedef unsigned long clockid_t;
#define __clockid_t_defined
#endif

#ifndef __timer_t_defined
typedef unsigned long timer_t;
#define __timer_t_defined
#endif

#ifndef __time_t_defined
typedef long time_t;
#define __time_t_defined

/* Time Value Specification Structures, P1003.1b-1993, p. 261 */

struct timespec {
  time_t  tv_sec;   /* Seconds */
  long    tv_nsec;  /* Nanoseconds */
};

struct itimerspec {
  struct timespec  it_interval;  /* Timer period */
  struct timespec  it_value;     /* Timer expiration */
};
#endif

typedef	long	daddr_t;
typedef	char *	caddr_t;
typedef	unsigned long	ino_t;

typedef unsigned long vm_offset_t;
typedef unsigned long vm_size_t;

typedef _off_t	off_t;
typedef __dev_t dev_t;
typedef __uid_t uid_t;
typedef __gid_t gid_t;

typedef int pid_t;
typedef	long key_t;
typedef	char * addr_t;
typedef int mode_t;
typedef unsigned short nlink_t;

#endif

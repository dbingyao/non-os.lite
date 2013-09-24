#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef int8_t              s8;
typedef uint8_t             u8;
typedef int16_t             s16;
typedef uint16_t            u16;
typedef int32_t             s32;
typedef uint32_t            u32;
#if defined(__GNUC__)
typedef int64_t             s64;
typedef uint64_t            u64;
#endif

typedef int8_t              __s8;
typedef uint8_t             __u8;
typedef int16_t             __s16;
typedef uint16_t            __u16;
typedef int32_t             __s32;
typedef uint32_t            __u32;
#if defined(__GNUC__)
typedef int64_t             __s64;
typedef uint64_t            __u64;
#endif

/* bsd */
typedef unsigned char       u_char;
typedef unsigned short      u_short;
typedef unsigned int        u_int;
typedef unsigned long       u_long;

/* sysv */
typedef unsigned char       unchar;
typedef unsigned short      ushort;
typedef unsigned int        uint;
typedef unsigned long       ulong;

/* asm */
typedef unsigned int        phys_addr_t;
typedef unsigned int        dma_addr_t;

/*
 * Below are truly Linux-specific types that should never collide with
 * any application/library that wants linux/types.h.
 */
#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#ifdef __CHECK_ENDIAN__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif

typedef __u16 __bitwise     __le16;
typedef __u16 __bitwise     __be16;
typedef __u32 __bitwise     __le32;
typedef __u32 __bitwise     __be32;
typedef __u64 __bitwise     __le64;
typedef __u64 __bitwise     __be64;
typedef __u16 __bitwise     __sum16;
typedef __u32 __bitwise     __wsum;

typedef uint __bitwise__    gfp_t;

#endif /* _LINUX_TYPES_H */

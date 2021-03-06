/*
 * The MIT License
 *
 * Copyright © 2016 2sui.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef QP_DEFINES_H
#define QP_DEFINES_H

#ifdef __cplusplus
extern "C" {
#endif



# ifndef _GNU_SOURCE
# define _GNU_SOURCE
# endif

//# ifndef _LARGEFILE64_SOURCE
//# define _LARGEFILE64_SOURCE  1
//# endif

//# ifndef _LARGEFILE_SOURCE
//# define _LARGEFILE_SOURCE  1
//# endif

# ifdef _LARGEFILE_SOURCE
# define __USE_LARGEFILE  1
# endif

# ifdef _LARGEFILE64_SOURCE
# define __USE_LARGEFILE64  1
# endif

# ifndef _FILE_OFFSET_BITS  // bit broad
# define _FILE_OFFSET_BITS  64
# endif

# if defined(_FILE_OFFSET_BITS) && (_FILE_OFFSET_BITS == 64)
# define __USE_FILE_OFFSET64  1
# endif


/**
  * Unix sys headers.
  */

/*
   The operating system, must be one of: (QP_OS_x)

     MACX   - Mac OS X
     MAC9   - Mac OS 9
     MSDOS  - MS-DOS and Windows
     OS2    - OS/2
     OS2EMX - XFree86 on OS/2 (not PM)
     WIN32  - Win32 (Windows 95/98/ME and Windows NT/2000/XP)
     CYGWIN - Cygwin
     SOLARIS    - Sun Solaris
     HPUX   - HP-UX
     ULTRIX - DEC Ultrix
     LINUX  - Linux
     FREEBSD    - FreeBSD
     NETBSD - NetBSD
     OPENBSD    - OpenBSD
     BSDI   - BSD/OS
     IRIX   - SGI Irix
     OSF    - HP Tru64 UNIX
     SCO    - SCO OpenServer 5
     UNIXWARE   - UnixWare 7, Open UNIX 8
     AIX    - AIX
     HURD   - GNU Hurd
     DGUX   - DG/UX
     RELIANT    - Reliant UNIX
     DYNIX  - DYNIX/ptx
     QNX    - QNX
     QNX6   - QNX RTP 6.1
     LYNX   - LynxOS
     BSD4   - Any BSD 4.4 system
     UNIX   - Any UNIX BSD/SYSV system
*/

# if defined(__APPLE__) && defined(__GNUC__)
#  ifndef  QP_OS_MACX
#  define QP_OS_MACX    /* MAC OSX/macOS */
#  endif
# elif defined(__MACOSX__)
#  ifndef  QP_OS_MACX
#  define QP_OS_MACX    /* MAC OSX/ macOS */
#  endif
# elif defined(macintosh)
#  ifndef  QP_OS_MAC9
#  define QP_OS_MAC9
#  endif
# elif defined(__FreeBSD__)
#  ifndef QP_OS_FREEBSD
#  define QP_OS_FREEBSD
#  endif
# elif defined(__NetBSD__)
#  ifndef  QP_OS_NETBSD
#  define QP_OS_NETBSD
#  endif
# elif defined(__OpenBSD__)
#  ifndef  QP_OS_OPENBSD
#  define QP_OS_OPENBSD
#  endif
# elif defined(__bsdi__)
#  ifndef  QP_OS_BSDI
#  define QP_OS_BSDI
#  endif
# elif defined(__linux__) || defined(__linux)
#  ifndef QP_OS_LINUX
#  define QP_OS_LINUX   /* ## LINUX ## */
#  endif
# elif defined(__sun) || defined(sun)
#  ifndef  QP_OS_SOLARIS
#  define QP_OS_SOLARIS   /* ## Solaris ## */
#  endif
# elif defined(__OS2__)
#  if defined(__EMX__)
#   ifndef  QP_OS_OS2EMX
#   define QP_OS_OS2EMX
#   endif
#  else
#   ifndef  QP_OS_OS2
#   define QP_OS_OS2
#   endif
#  endif
# elif defined(__CYGWIN__)
#  ifndef  QP_OS_CYGWIN
#  define QP_OS_CYGWIN
#  endif
# elif defined(MSDOS) || defined(_MSDOS)
#  ifndef  QP_OS_MSDOS
#  define QP_OS_MSDOS
#  endif
# elif !defined(SAG_COM) \
      && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  ifndef  QP_OS_WIN32
#  define QP_OS_WIN32
#  endif
#  ifndef  QP_OS_WIN64
#  define QP_OS_WIN64
#  endif
# elif !defined(SAG_COM) \
    && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  ifndef  QP_OS_WIN32
#  define QP_OS_WIN32
#  endif
# elif defined(__MWERKS__) && defined(__INTEL__)
#  ifndef  QP_OS_WIN32
#  define QP_OS_WIN32
#  endif
# elif defined(hpux) || defined(__hpux)
#  ifndef QP_OS_HPUX
#  define QP_OS_HPUX
#  endif
# elif defined(__ultrix) || defined(ultrix)
#  ifndef  QP_OS_ULTRIX
#  define QP_OS_ULTRIX
#  endif
# elif defined(sinix)
#  ifndef  QP_OS_RELIANT
#  define QP_OS_RELIANT
#  endif
# elif defined(__osf__)
#  ifndef  QP_OS_OSF
#  define QP_OS_OSF
#  endif
# elif defined(__sgi)
#  ifndef  QP_OS_IRIX
#  define QP_OS_IRIX
#  endif
# elif defined(_AIX)
#  ifndef  QP_OS_AIX
#  define QP_OS_AIX
#  endif
# elif defined(__Lynx__)
#  ifndef  QP_OS_LYNX
#  define QP_OS_LYNX
#  endif
# elif defined(__GNU_HURD__)
#  ifndef  QP_OS_HURD
#  define QP_OS_HURD
#  endif
# elif defined(__DGUX__)
#  ifndef  QP_OS_DGUX
#  define QP_OS_DGUX
#  endif
# elif defined(__QNXNTO__)
#  ifndef  QP_OS_QNX6
#  define QP_OS_QNX6
#  endif
# elif defined(__QNX__)
#  ifndef QP_OS_QNX
#  define QP_OS_QNX
#  endif
# elif defined(_SEQUENT_)
#  ifndef  QP_OS_DYNIX
#  define QP_OS_DYNIX
#  endif
# elif defined(_SCO_DS)   /* SCO OpenServer 5 + GCC */
#  ifndef  QP_OS_SCO
#  define QP_OS_SCO
#  endif
# elif defined(__USLC__)    /* all SCO platforms + UDK or OUDK */
#  ifndef  QP_OS_UNIXWARE
#  define QP_OS_UNIXWARE
#  endif
#  ifndef  QP_OS_UNIXWARE7
#  define QP_OS_UNIXWARE7
#  endif
# elif defined(__svr4__) && defined(i386)   /* Open UNIX 8 + GCC */
#  ifndef  QP_OS_UNIXWARE
#  define QP_OS_UNIXWARE
#  endif
#  ifndef  QP_OS_UNIXWARE7
#  define QP_OS_UNIXWARE7
#  endif
# else
# error "QPHi has not been ported to this OS!"
# endif

# if defined(QP_OS_MACX)  
# define QP_OS_MACOS   
# endif
    
# if defined(QP_OS_MACOS) || defined(QP_OS_FREEBSD) || defined(QP_OS_NETBSD) || \
    defined(QP_OS_OPENBSD) || defined(QP_OS_BSDI)
# define QP_OS_BSD         /* ## BSD ## */
# endif
    
# if defined(QP_OS_BSD) || defined(QP_OS_LINUX) || defined(QP_OS_SOLARIS)
# define QP_OS_UNIX
# else 
# error "QPHi does not match this OS!"
# endif


# ifdef QP_OS_UNIX
# include <ctype.h>
# include <errno.h>
# include <grp.h>
# include <inttypes.h>
# include <limits.h>        /* IOV_MAX */
# include <math.h>
# include <setjmp.h>        /* <less> */
# include <signal.h>
# include <stdarg.h>
# include <stdbool.h>
# include <stddef.h>        /* offsetof() */
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>
 /* posix */
# define QP_OS_POSIX
# endif

# ifdef QP_OS_POSIX
# include <aio.h>
# include <arpa/inet.h>
# include <dirent.h>
# include <dlfcn.h>
# include <fcntl.h>
# include <fnmatch.h>
# include <glob.h>
# include <grp.h>
//# include <iconv.h>
//# include <langinfo.h>
# include <netdb.h>
# include <net/if.h>         /* <less> */
# include <netinet/in.h>
# include <netinet/tcp.h>    /* TCP_NODELAY, TCP_NOPUSH */
# include <nl_types.h>
# include <poll.h>
# include <pthread.h>
# include <pwd.h>
# include <regex.h>
# include <sched.h>
# include <semaphore.h>
# include <strings.h>
# include <sys/mman.h>
# include <sys/select.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/statvfs.h>
# include <sys/times.h>
# include <sys/types.h>
# include <sys/un.h>
# include <sys/utsname.h>    /* uname() */
# include <sys/wait.h>
//# include <tar.h>
# include <termios.h>
# include <unistd.h>
//# include <wordexp.h>
/* option */
//# include <spawn.h>
 /* XSI */
# define QP_POSIX_XSI
# endif

# ifdef QP_POSIX_XSI
# include <fmtmsg.h>
# include <ftw.h>
//# include <libgen.h>
# include <search.h>
# include <syslog.h>
# include <sys/ipc.h>
# include <sys/msg.h>
# include <sys/resource.h>
# include <sys/sem.h>
# include <sys/shm.h>
# include <sys/time.h>
# include <sys/uio.h>
# include <sys/file.h>         /* ?? */
# endif

# if defined(QP_OS_BSD)   
#  include <machine/endian.h>
#  if defined(QP_OS_MACOS)
#  include <sys/ioctl.h>
#  include <xlocale.h>
#  include <utmpx.h>
#  else 
#  include <mqueue.h>       /* FIONBIO */
#  include <libutil.h>            /* setproctitle() before 4.1 */
#  include <osreldate.h>
#  endif
# include <sys/param.h>          /* ALIGN() */
# include <sys/mount.h>          /* statfs() */
# include <sys/filio.h>   
# include <sys/sysctl.h>
# include <sys/event.h>
# include <net/bpf.h>
# define __BYTE_ORDER BYTE_ORDER
# elif defined(QP_OS_SOLARIS)
# include <endian.h>
# include <mqueue.h>
# include <sys/statvfs.h>        /* statvfs() */
# include <sys/filio.h>          /* FIONBIO */
# include <sys/systeminfo.h>
# include <crypt.h>
# include <sys/ioctl.h>
# include <sys/devpoll.h>
# include <port.h>
# include <sys/sendfile.h>
# include <utmpx.h>
# else
# include <endian.h>
# include <mqueue.h>
# include <crypt.h>
# include <malloc.h>             /* memalign() */
# include <sys/mount.h>
# include <sys/vfs.h>            /* statfs() */
# include <sys/ioctl.h>
# include <sys/prctl.h>
# include <sys/sendfile.h>
# include <sys/sysctl.h>
# include <sys/epoll.h>
# include <sys/eventfd.h>
# include <sys/syscall.h>
# include <linux/aio_abi.h>
# include <utmpx.h>
# endif

/* basic type */
/*
typedef  unsigned  char       qp_uint8_t;
typedef  signed    char       qp_int8_t;
typedef  unsigned  short      qp_uint16_t;
typedef  signed    short      qp_int16_t;
typedef  unsigned  int        qp_uint32_t;
typedef  signed    int        qp_int32_t;
#if __WORDSIZE == 64
typedef  unsigned  long       qp_uint64_t;
typedef  signed    long       qp_int64_t;
#else
__extension__
typedef  unsigned  long long  qp_uint64_t;
typedef  signed    long long  qp_int64_t;
#endif

typedef  unsigned  char       qp_uchar_t;
typedef  signed    char       qp_char_t;
typedef  unsigned  short      qp_ushort_t;
typedef  signed    short      qp_short_t;
typedef  unsigned  int        qp_uint_t;
typedef  signed    int        qp_int_t;
#if __WORDSIZE == 64
typedef  unsigned  long       qp_ulong_t;
typedef  signed    long       qp_long_t;
#else
__extension__
typedef  unsigned  long long  qp_ulong_t;
typedef  signed    long long  qp_long_t;
#endif
*/

typedef  uint8_t               qp_uint8_t;
typedef  int8_t                qp_int8_t;
typedef  uint16_t              qp_uint16_t;
typedef  int16_t               qp_int16_t;
typedef  uint32_t              qp_uint32_t;
typedef  int32_t               qp_int32_t;
typedef  uint64_t              qp_uint64_t;
typedef  int64_t               qp_int64_t;

# define  qp_uchar_t           unsigned char
# define  qp_char_t            char
# define  qp_ushort_t          unsigned short
# define  qp_short_t           short
# define  qp_uint_t            unsigned int
# define  qp_int_t             int
# if __WORDSIZE == 64
# define  qp_ulong_t           unsigned long
# define  qp_long_t            long
# else
__extension__
# define  qp_ulong_t           unsigned long long
# define  qp_long_t            long long
# endif


# if defined(QP_OS_WIN32) || defined(QP_OS_WIN64) /* WIN */
# define __QP_LITTLE_ENDIAN__ 
# endif /* WIN */

# if !defined(__QP_LITTLE_ENDIAN__) && !defined(__QP_BIG_ENDIAN__) /* ORDER */
/* Kernel modules */
#  if defined(__LITTLE_ENDIAN) /* __LITTLE_ENDIAN */
#  define __QP_LITTLE_ENDIAN__
#  endif /* __LITTLE_ENDIAN */
#  if defined(__BIG_ENDIAN) /* __BIG_ENDIAN */
#  define __QP_BIG_ENDIAN__
#  endif /* __BIG_ENDIAN */
/* Everything else */
#  if (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)) /* ELSE */
#   if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ /*__BYTE_ORDER__  */
#   define __QP_LITTLE_ENDIAN__
#   else
#   define __QP_BIG_ENDIAN__
#   endif /*__BYTE_ORDER__  */
#  endif /* ELSE */
# endif /* ORDER */

# define  qp_pagesize  (sysconf(_SC_PAGESIZE) > 0 ? sysconf(_SC_PAGESIZE) : 4096)
# define  qp_cpu_num   (sysconf(_SC_NPROCESSORS_CONF) > 1 ? \
         sysconf(_SC_NPROCESSORS_ONLN) : 1)
# define  qp_cache_line  64;

# define  QP_ERROR      -1
# define  QP_SUCCESS    0

//# include "qp_debug.h"

#ifdef __cplusplus
}
#endif

#endif /* QP_DEFINES_H */

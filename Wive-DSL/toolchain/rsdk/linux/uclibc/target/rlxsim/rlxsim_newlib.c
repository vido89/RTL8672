
#include "rlxsim_newlib.h"
#include "rlx_library.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>

#ifndef NULL
#define NULL 0
#endif

#define __MYPID 1

extern __PutCharacter(char, char);


#undef errno
int errno;

int* __errno(void)
{
  return &errno;
}

extern int outbyte(char ch);
extern unsigned char inbyte();
extern int havebyte();

void
__main()
{
  return;
}

int
read(fd, buf, nbytes)
     int fd;
     void *buf;
     size_t nbytes;
{
  return rlx_gdb_read(fd, (void *)buf, (unsigned int)nbytes);
}

int
write(fd, buf, nbytes)
     int fd;
     const void *buf;
     size_t nbytes;
{
  int i;

  return rlx_gdb_write(fd, (void *)buf, (unsigned int)nbytes);
}

int
_DEFUN (open, (buf, flags, ...),
     const char *buf _AND
     int flags _DOTS)
{
    /*
  errno = EIO;
  return (-1);
  */
  va_list ap;

  va_start(ap, flags);
  return rlx_gdb_open(buf, flags, va_arg(ap, mode_t));
}

int
close(fd)
     int fd;
{
  return rlx_gdb_close(fd);
}

/*
      * sbrk -- changes heap size size. Get nbytes more
      *         RAM. We just increment a pointer in what's
      *         left of memory on the board.
      */
void *
sbrk(nbytes)
     ptrdiff_t nbytes;
{
  static caddr_t heap_ptr = (caddr_t)&HEAP_START;
  caddr_t        base;

  if ((RAMSIZE - heap_ptr) >= 0) {
    base = heap_ptr;
    heap_ptr += nbytes;
    return (base);
  } else {
    errno = ENOMEM;
    return ((caddr_t)-1);
  }
}

off_t
lseek(fd,  offset, whence)
     int fd;
     off_t offset;
     int whence;
{
  return (off_t)rlx_gdb_lseek(fd, offset, whence);
}

int
rename(oldpath, newpath)
        const char *oldpath;
        const char *newpath;
{
  return rlx_gdb_rename(oldpath, newpath);
}

int
unlink(path)
        const char *path;
{
  return rlx_gdb_unlink(path);
}

int
stat(pathname, buf)
        const char *pathname;
        struct stat *buf;
{
  return rlx_gdb_stat(pathname, buf);
}

int
fstat(fd, buf)
     int fd;
     struct stat *buf;
{
  return rlx_gdb_fstat(fd, buf);
}

/* A dummy function to satisfy the linker */
clock_t
times(struct tms *buf)
{
  errno = ENOTSUP;
  return -1;
}

int
gettimeofday(tv, tz)
        struct timeval *tv;
        struct timezone *tz;
{
  return rlx_gdb_gettimeofday(tv, tz);
}

int
isatty(fd)
     int fd;
{
  // return (1);
  return rlx_gdb_isatty(fd);
}

int
system(command)
        const char *command;
{
  return rlx_gdb_system(command);
}

/*
      * getpid -- only one process, so just return 1.
      */
int
getpid()
{
  return __MYPID;
}

/*
      * kill -- go out via exit...
      */
int
kill(pid, sig)
     int pid;
     int sig;
{
  if(pid == __MYPID)
    _exit(sig);
  return 0;
}

/*
      * print -- do a raw print of a string
      */
int
print(ptr)
     char *ptr;
{
  while (*ptr) {
    outbyte (*ptr++);
  }
}

/*
      * putnum -- print a 32 bit number in hex
      */
int
putnum (num)
     unsigned int num;
{
  char  buffer[9];
  int   count;
  char  *bufptr = buffer;
  int   digit;

  for (count = 7 ; count >= 0 ; count--) {
    digit = (num >> (count * 4)) & 0xf;

    if (digit <= 9)
      *bufptr++ = (char) ('0' + digit);
    else
      *bufptr++ = (char) ('a' - 10 + digit);
  }

  *bufptr = (char) 0;
  print (buffer);
  return;
}

int outbyte(char ch)
{
  __PutCharacter(ch, ch);
}

unsigned char inbyte()
{
}

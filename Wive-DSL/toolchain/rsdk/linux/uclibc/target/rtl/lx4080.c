/*
 * $Id: lx4080.c,v 1.5 2000/11/01 22:52:29 franklin Exp $
 */

#include "lx4080.h"

#ifndef NULL
#define NULL 0
#endif

#define __MYPID 1

extern void __PutCharacter(char, char);
int outbyte(char ch);
unsigned char inbyte(void);


#undef errno
int errno;
     
void
__main()
{
  return;
}

/*
      * read  -- read bytes from the serial port. Ignore fd, since
      *          we only have stdin.
      */
int
read(fd, buf, nbytes)
     int fd;
     char *buf;
     int nbytes;
{
  int i = 0;
     
  for (i = 0; i < nbytes; i++) {
    *(buf + i) = inbyte();
    if ((*(buf + i) == '\n') || (*(buf + i) == '\r')) {
      (*(buf + i)) = 0;
      break;
    }
  }
  return (i);
}
     
/*
      * write -- write bytes to the serial port. Ignore fd, since
      *          stdout and stderr are the same. Since we have no filesystem,
      *          open will only return an error.
      */
int
write(fd, buf, nbytes)
     int fd;
     char *buf;
     int nbytes;
{
  int i;
     
  for (i = 0; i < nbytes; i++) {
    if (*(buf + i) == '\r') {
      outbyte ('\n');
    }
    outbyte (*(buf + i));
  }
  return (nbytes);
}
     
/*
      * open -- open a file descriptor. We don't have a filesystem, so
      *         we return an error.
      */
int
open(buf, flags, mode)
     char *buf;
     int flags;
     int mode;
{
  errno = EIO;
  return (-1);
}
     
/*
      * close -- close a file descriptor. We don't need
      *          to do anything, but pretend we did.
      */
int
close(fd)
     int fd;
{
  return (0);
}

/*
      * sbrk -- changes heap size size. Get nbytes more
      *         RAM. We just increment a pointer in what's
      *         left of memory on the board.
      */
caddr_t
sbrk(nbytes)
     int nbytes;
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

/*
      * isatty -- returns 1 if connected to a terminal device,
      *           returns 0 if not. Since we're hooked up to a
      *           serial port, we'll say yes and return a 1.
      */
int
isatty(fd)
     int fd;
{
  return (1);
}

/*
      * lseek -- move read/write pointer. Since a serial port
      *          is non-seekable, we return an error.
      */
off_t
lseek(fd,  offset, whence)
     int fd;
     off_t offset;
     int whence;
{
  errno = ESPIPE;
  return ((off_t)-1);
}

/*
      * fstat -- get status of a file. Assume it is a
      * terminal (character special file).
      */
int
fstat(fd, buf)
     int fd;
     struct stat *buf;
{
  buf->st_mode = S_IFCHR;
  return 0;
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
  return 1;
}

unsigned char inbyte()
{
  return 0;
}

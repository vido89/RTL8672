/*
 * Copyright (c) 2006, Realtek Semiconductor Corp.
 *
 * rlx_library.h:
 *
 * Zhe Jiang (zhe_jiang@realsil.com.cn)
 * Ling Sun (ling_sun@realsil.com.cn)
 * Tony Wu (tonywu@realtek.com.tw)
 * Jul. 20, 2006
 */

#ifndef _RLX_LIBRARY_H_
#define _RLX_LIBRARY_H_

#ifdef RLX_UNCACHEABLE
  #define LN_ATTR  __attribute__((far_call))
#else
  #define LN_ATTR 
#endif

/*
 * RLX code coverage analysis
 */
#ifndef __ASM__
  void rlx_cov_init(void)  LN_ATTR;
  void rlx_cov_exit(void)  LN_ATTR;
#endif

/*
 * RLX GDB Remote I/O 
 */
#ifndef __ASM__
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#define GDB_SYSCALL_NUM         5
#define GDB_SERV_OPEN           0
#define GDB_SERV_CLOSE          1
#define GDB_SERV_READ           2
#define GDB_SERV_WRITE          3
#define GDB_SERV_LSEEK          4
#define GDB_SERV_RENAME         5
#define GDB_SERV_UNLINK         6
#define GDB_SERV_STAT           7
#define GDB_SERV_FSTAT          8
#define GDB_SERV_GETTIMEOFDAY   9
#define GDB_SERV_ISATTY         10
#define GDB_SERV_SYSTEM         11

#if 0
#define O_RDONLY   0x0
#define O_WRONLY   0x1
#define O_RDWR     0x2
#define O_APPEND   0x8
#define O_CREAT    0x200
#define O_TRUNC    0x400
#define O_EXCL     0x800

#define S_IWOTH    0x2
#define S_IROTH    0x4
#define S_IWGRP    0x10
#define S_IRGRP    0x20
#define S_IWUSR    0x80
#define S_IRUSR    0x100

#define SEEK_SET   0x0
#define SEEK_CUR   0x1
#define SEEK_END   0x2
#endif


#ifndef __ASM__
int rlx_gdb_open(const char *pathname, int flags, mode_t mode)      LN_ATTR;
int rlx_gdb_close(int fd)                                           LN_ATTR;
int rlx_gdb_read(int fd, void *buf, unsigned int count)             LN_ATTR;
int rlx_gdb_write(int fd, void *buf, unsigned int count)            LN_ATTR;
int rlx_gdb_lseek(int fd, long offset, int flag)                    LN_ATTR;
int rlx_gdb_rename(const char *oldpath, const char *newpath)        LN_ATTR;
int rlx_gdb_unlink(const char *pathname)                            LN_ATTR;
int rlx_gdb_stat(const char *pathname, struct stat *buf)            LN_ATTR;
int rlx_gdb_fstat(int fd, struct stat *buf)                         LN_ATTR;
int rlx_gdb_gettimeofday(struct timeval *tv, struct timezone *tz)   LN_ATTR;
int rlx_gdb_isatty(int fd)                                          LN_ATTR;
void rlx_gdb_printf(char *, ...)                                    LN_ATTR;
void rlx_gdb_set_param_addr(unsigned int)                           LN_ATTR;
unsigned int rlx_gdb_get_parm_addr(void)                            LN_ATTR;
#endif

/*
 * RLX CP3 Performance 
 */

/* set counter controls:	0       1       2       3
**   counter0:	       cycles (0x10)  (0x10)  (0x10)  (0x10)   
**   counter1:	     ifetches (0x11)  (0x11)  (0x11)
**   counter1:	   dmiss busy                         (0x1b)
**   counter2:	    ld+stores (0x16)                  (0x16)
**   counter2:	icache misses         (0x12)  (0x12)
**   counter3:	dcache misses (0x1a)  (0x1a)          (0x1a)
**   counter3:	   imiss busy                 (0x13)
*/

#define CP3_PERFMODE0 0x1a161110
#define CP3_PERFMODE1 0x1a121110
#define CP3_PERFMODE2 0x13121110
#define CP3_PERFMODE3 0x1a161b10


#ifndef __ASM__
typedef unsigned long long CP3_COUNTER;

void rlx_cp3_init(void)                                  LN_ATTR;
void rlx_cp3_start(unsigned int)                         LN_ATTR;
void rlx_cp3_stop(void)                                  LN_ATTR;
void rlx_cp3_print_counters(unsigned int, CP3_COUNTER *) LN_ATTR;
void rlx_cp3_get_counters(CP3_COUNTER *)                 LN_ATTR;
unsigned int rlx_cp3_get_counter_hi(int)                 LN_ATTR;
unsigned int rlx_cp3_get_counter_lo(int)                 LN_ATTR;
#endif

/*
 * RLX profiler 
 */
#ifndef __ASM__
void rlx_prof_init(void)        LN_ATTR;
void rlx_prof_start(void)       LN_ATTR;
void rlx_prof_stop(void)        LN_ATTR;
int rlx_prof_save_result(void)  LN_ATTR;
void rlx_prof_disable_int(void) LN_ATTR;
void rlx_prof_enable_int(void)  LN_ATTR;
#endif

#endif

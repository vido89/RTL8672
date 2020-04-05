/*
 * Copyright (c) 2006, Realtek Semiconductor Corp.
 *
 * gdbio.c:
 *   GDB remote I/O example program
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Jul. 26, 2006
 */

#include <stdio.h>
#include "rlx_library.h"


int
main(int argc, char **argv)
{
  int fd;
  char buff[] = "This is a test string";

  fd = rlx_gdb_open("test.txt", O_CREAT | O_RDWR | O_TRUNC,
                     S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  if (fd < 0)
    {
        rlx_gdb_printf("ERROR: unable to open the test.txt\n");
        return -1;
    }

  rlx_gdb_write(fd, buff, sizeof(buff));
  rlx_gdb_close(fd);
  return 0;
}

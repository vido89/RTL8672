CC=../../../../toolchain/rsdk-1.3.6-5181-EB-2.6.19-0.9.30/bin/rsdk-linux-gcc
# define __UCLIBC_HAS_IPV6__ if support ipv6
CFLAGS=-D__UCLIBC_HAS_THREADS__ -DL_dnslookup
LDFLAGS=-shared
all:
	- $(CC) $(LDFLAGS) $(CFLAGS) -o resolv_0.9.21.so resolv_0.9.21.c
	- $(CC) $(LDFLAGS) $(CFLAGS) -o resolv_0.9.26.so resolv_0.9.26.c
	- $(CC) $(LDFLAGS) $(CFLAGS) -o resolv_0.9.27.so resolv_0.9.27.c

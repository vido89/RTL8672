x86 = n

ifeq ($(x86),y)
CROSS_COMPILE =
DNS_LIB = libdns.a
CFLAGS = -g -Wall
else
#	ifeq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
#		CROSS_COMPILE = /rsdk-1.3.6-5181-EB-2.6.19-0.9.30/bin/mips-linux- 
#	else
#	CROSS_COMPILE = mips-linux-
#	endif
include ../build/Makeparm.cfg
include ../build/Makeparm.def
endif

CC		=	$(CROSS_COMPILE)gcc
OBJS	=	dns_task.o dns_api.o dns_test.o memwatch.o
LDFLAGS	=	
DEPENDENCY	= $(OBJS:.o=.d)

all: $(OBJS)
#	$(CC) $(LDFLAGS) -o dns_task dns_task.o memwatch.o -lpthread
#	$(CC) $(LDFLAGS) -o dns_test dns_api.o dns_test.o memwatch.o
	$(CC) $(LDFLAGS) -o dns_task dns_task.o -lpthread
	$(CC) $(LDFLAGS) -o dns_test dns_api.o dns_test.o
	ln -fs dns_test dns_test1
	ln -fs dns_test dns_test2 
	ln -fs dns_test dns_test3
	$(AR) rcs $(DNS_LIB) dns_api.o
	make -f resolv.mak

clean: 
	rm -f $(OBJS) $(DEPENDENCY)
	rm -f dns_task dns_test dns_test2 dns_test3 $(DNS_LIB)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

# pull in dependency info for existing .o files
-include $(DEPENDENCY)

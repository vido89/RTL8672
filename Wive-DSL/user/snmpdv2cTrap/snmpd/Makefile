EXEC = snmpd
OBJS = snmpd.o trap.o getip.o

ifdef CONFIG_USER_REMOTE_MANAGEMENT
OBJS += xgetcwd.o xmalloc.o
endif

CFLAGS += -I../snmplib -I../modules
CFLAGS += -DHAVE_CONFIG_H=1 -I.

# Need a bigger stack than the default
FLTFLAGS += -s 8000

USELIBS = ../modules/libmodules.a ../snmplib/libsnmplib.a

all: $(EXEC) rmping

test:

$(EXEC): $(OBJS) $(USELIBS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(USELIBS) $(LDLIBS) -lpthread -lmib

rmping: rmping.o
	$(CC) $(LDFLAGS) -o $@ rmping.o

romfs:
	$(ROMFSINST) /bin/$(EXEC)
ifdef CONFIG_USER_REMOTE_MANAGEMENT
	$(ROMFSINST) /bin/rmping
endif

clean:
	-rm -f $(EXEC) rmping *.gdb *.elf *.o

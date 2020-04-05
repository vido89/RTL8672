
OBJS=	af.o if.o input.o main.o output.o startup.o \
	tables.o timer.o trace.o inet.o

CFLAGS += -I$(ROOTDIR)/linux/include $(INCNET) -I. -DRIPVERSION=2

all: routed

routed: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) -L../boa/src/LINUX -lmib

romfs:
	$(ROMFSINST) /bin/routed

clean:
	rm -f *.elf *.gdb *.o routed

$(OBJS): defs.h trace.h interface.h table.h af.h pathnames.h version.h


EXEC = mpoad
EXEC1 = mpoactl
OBJS = mpoad.o text2atm.o ans.o misc.o text2qos.o
OBJS1 = mpoactl.o
CFLAGS += $(INCATM)

all: $(EXEC) $(EXEC1)

$(EXEC): $(OBJS) ../../lib/uClibc/include/rtk/options.h
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBATM) $(LDLIBS$(LDLIBS-$(@))) -lmib

$(EXEC1): $(OBJS1)
	$(CC) $(LDFLAGS) -o $@ $(OBJS1) $(LIBATM) $(LDLIBS$(LDLIBS-$(@)))

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	$(ROMFSINST) /bin/$(EXEC1)

clean:
	-rm -f $(EXEC) $(EXEC1) *.elf *.gdb *.o


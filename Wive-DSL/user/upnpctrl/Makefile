
CFLAGS += -Os
EXEC = upnpctrl
OBJS = main.o

all: $(EXEC)


$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS-$(@))) -L../boa/src/LINUX -lmib

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o *.bak
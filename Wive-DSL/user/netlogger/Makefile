
EXEC = netlogger 
OBJS = netlogger.o

all: $(EXEC)


$(EXEC): $(OBJS) ../../lib/uClibc/include/rtk/options.h
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) -L../boa/src/LINUX -lmib


romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o


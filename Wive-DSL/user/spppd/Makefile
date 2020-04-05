EXEC = spppd
EXEC2 = spppctl
OBJS = timeout.o pppoe.o if_spppsubr.o md5.o main.o sys-linux.o \
		text2atm.o text2qos.o ans.o misc.o pppoatm.o
		
OBJS2 = text2atm.o text2qos.o ans.o misc.o spppctl.o

all: $(EXEC) $(EXEC2)


$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS-$(@))) -L../boa/src-project/LINUX -lmib


$(EXEC2): $(OBJS2)
	$(CC) $(LDFLAGS) -o $@ $(OBJS2) $(LDLIBS$(LDLIBS-$(@)))

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	$(ROMFSINST) /bin/$(EXEC2)

clean:
	-rm -f $(EXEC) $(EXEC2) *.elf *.o



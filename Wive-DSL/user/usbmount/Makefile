include ../../config/.config

EXEC = usbmount 
OBJS = usbmount.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)  -L../boa/src/LINUX -lmib

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	#$(ROMFSINST) -p 0755 /usr/hotplug
ifeq ($(CONFIG_BOA_WEB_E8B_CH),y)
	$(ROMFSINST) -p 0755 hotplug_e8b /usr/hotplug
else
	$(ROMFSINST) -p 0755 /usr/hotplug
endif

clean:
	-rm -f $(EXEC) *.elf *.o


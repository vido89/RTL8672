include ../.linux_config

#CC = gcc
EXE = cvvoip
OBJS = cvvoip.o voip_flash_mib.o voip_flash_tool.o

CFLAGS = -g -Wall -DCVVOIP -I../includes
LDFLAGS = --static

ifneq ($(BUILD_DIALPLAN), 0)
CFLAGS += -DCONFIG_RTK_VOIP_DIALPLAN
endif

DEPENDENCY	= $(OBJS:.o=.d)

all: $(EXE)
	make -f Makefile-client 

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

clean:
	rm -f $(EXE) $(OBJS) *.d
	rm -f WIN32/voip_flash.h
	rm -f WIN32/voip_flash_mib.c
	rm -f WIN32/voip_flash_mib.h
	rm -f WIN32/voip_flash_tool.c
	rm -f WIN32/voip_flash_tool.h
	rm -f cvvoip_w32.tgz
	make -f Makefile-client clean

w32:
	cp -f voip_flash_mib.c WIN32/
	cp -f voip_flash_tool.c WIN32/
	cp -f ../includes/voip_flash.h WIN32/
	cp -f ../includes/voip_flash_mib.h WIN32/
	cp -f ../includes/voip_flash_tool.h WIN32/
	tar cvfz cvvoip_w32.tgz WIN32

-include $(DEPENDENCY)

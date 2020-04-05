include ../.linux_config

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_865X), y)
CROSS_COMPILE = mips-linux-
CFLAGS += -I../includes
DOXYFILE = ./document/Doxyfile.865x
else
CROSS_COMPILE = mips-uclibc-
CFLAGS += -I../includes
DOXYFILE = ./document/Doxyfile
endif

ifdef CONFIG_RTK_VOIP_IP_PHONE
CFLAGS += -I../ui/include
endif

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
LIBA = libvoip_manager.a
OBJS = voip_manager.o

DEPENDENCY	= $(OBJS:.o=.d)

all: $(OBJS) doc
	$(AR) rcs $(LIBA) $(OBJS)
	make -C document/examples
	
romfs:
	make -C document/examples romfs

clean:
	rm -f $(LIBA) $(OBJS) *.d
	rm -rf document/html
	rm -rf document/latex
	make -C document/examples clean

doc:
	doxygen $(DOXYFILE)
	-cp ./document/realtek.jpg ./document/html

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

-include $(DEPENDENCY)

include ../../.linux_config

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
#CC			=	/opt/toolchain_mips_nofpu/bin/mips-linux-gcc
#AR			=	/opt/toolchain_mips_nofpu/bin/mips-linux-ar
CC			=	$(CROSS_COMPILE)gcc
AR          =   $(CROSS_COMPILE)ar
else
CC			=	mips-linux-gcc
AR			=	mips-linux-ar
endif

#TARGET		=	fwupdate
#OBJS		=	fwupdate.o
LIBS		=	../../voip_manager/libvoip_manager.a ../../flash/voip_flash_client.o
DEPENDENCY	=	$(OBJS:.o=.d)
LDFLAGS		=
CFLAGS		=	-I../../includes/

# 865x only
CFLAGS		+=	-I../../../../include/

CFLAGS		+=	-DVOIP_SUPPORT -DHOME_GATEWAY

#all: $(OBJS)
#	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIBS)

clean:
	rm -f $(TARGET) $(OBJS) $(DEPENDENCY)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

-include $(DEPENDENCY)

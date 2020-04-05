include ../../.linux_config

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
#CC			=	/opt/toolchain_mips_nofpu/bin/mips-linux-gcc
#AR			=	/opt/toolchain_mips_nofpu/bin/mips-linux-ar
CC          =   $(CROSS_COMPILE)gcc
AR          =   $(CROSS_COMPILE)ar

else
CC			=	mips-linux-gcc
AR			=	mips-linux-ar
endif

TARGET		=	netlink
OBJS		=	netlink.o
LIBS		=	../../voip_manager/libvoip_manager.a
DEPENDENCY	=	$(OBJS:.o=.d)
LDFLAGS		=
CFLAGS		=	-I../../includes/

ifdef CONFIG_RTK_VOIP_IP_PHONE
CFLAGS		+=	-I../../ui/include/
endif

# 865x only
CFLAGS		+=	-I../../../../include/

#all: $(OBJS)
#	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIBS)

clean:
	rm -f $(TARGET) $(OBJS) $(DEPENDENCY)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

-include $(DEPENDENCY)

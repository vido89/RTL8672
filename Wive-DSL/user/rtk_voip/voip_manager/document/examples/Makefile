include ../../../.linux_config

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
CC = /opt/Wive-DSL/toolchain/bin/mips-linux-gcc --static
else
CC = mips-linux-gcc --static
endif

CFLAGS += -g -I../../../includes
LIBS = ../../../voip_manager/libvoip_manager.a

ifdef CONFIG_RTK_VOIP_IP_PHONE
CFLAGS += -I../../../ui/include
endif

#----------------------------------------------------------------------
all:
	$(CC) ring.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o ring
	$(CC) phone.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o phone
	$(CC) rtp.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o rtp
	$(CC) daa.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o daa
	$(CC) caller_id.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o caller_id
	$(CC) vmwi.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o vmwi
	$(CC) fxo.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxo
	$(CC) conference.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o conference
	
romfs:

clean:
	rm -f *.o ring phone rtp daa caller_id vmwi fxo conference

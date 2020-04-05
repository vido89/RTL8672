
include ../.linux_config

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
CC = /opt/Wive-DSL/toolchain/bin/mips-linux-gcc
else
CC = mips-linux-gcc
endif
CFLAGS += -I../includes
LIBS = ../voip_manager/libvoip_manager.a
#LIBS = ../voip_manager/voip_manager.o

LDFLAGS += --static


VOIPBOX_DBG=1
VOIPBOX_FSKGEN=1
#VOIPBOX_LED=1
VOIPBOX_PHONERECORD=1
VOIPBOX_REG=1
VOIPBOX_RAM=1
VOIPBOX_SLIC_RESET=1
VOIPBOX_SWITCHMII=1
#VOIPBOX_TEST=1
VOIPBOX_VMWIGEN=1
VOIPBOX_VOICERECORD=1
VOIPBOX_RTCP_STATISTIC=1
#VOIPBOX_RING_TEST=1

objs = voipbox.o
TARGET = voipbox

ifdef VOIPBOX_DBG
objs += dbg.o
CFLAGS += -DVOIPBOX_DBG
endif

ifdef VOIPBOX_FSKGEN
objs += fskgen.o
CFLAGS += -DVOIPBOX_FSKGEN
endif

ifdef VOIPBOX_LED
objs += led.o
CFLAGS += -DVOIPBOX_LED
endif

ifdef VOIPBOX_PHONERECORD
objs += phonerecord.o
CFLAGS += -DVOIPBOX_PHONERECORD
endif

ifdef VOIPBOX_REG
objs += reg.o
CFLAGS += -DVOIPBOX_REG
endif

ifdef VOIPBOX_RAM
objs += ram.o
CFLAGS += -DVOIPBOX_RAM
endif

ifdef VOIPBOX_SLIC_RESET
objs += slic_reset.o
CFLAGS += -DVOIPBOX_SLIC_RESET
endif

ifdef VOIPBOX_SWITCHMII
objs += switchmii.o
CFLAGS += -DVOIPBOX_SWITCHMII
endif

ifdef VOIPBOX_TEST
objs += test.o
CFLAGS += -DVOIPBOX_TEST
endif

ifdef VOIPBOX_VMWIGEN
objs += vmwigen.o
CFLAGS += -DVOIPBOX_VMWIGEN
endif

ifdef VOIPBOX_VOICERECORD
objs += voicerecord.o
CFLAGS += -DVOIPBOX_VOICERECORD
endif

ifdef VOIPBOX_RTCP_STATISTIC
objs += rtcp_statistic.o
CFLAGS += -DVOIPBOX_RTCP_STATISTIC
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
objs += iphone_test.o
CFLAGS += -DCONFIG_RTK_VOIP_DRIVERS_IP_PHONE
endif

ifdef VOIPBOX_RING_TEST
objs += ring_test.o
CFLAGS += -DVOIPBOX_RING_TEST
endif

#----------------------------------------------------------------------
all: $(objs)
#	make -C pcm-talk
	$(CC) -o $(TARGET) $(objs) $(CFLAGS) $(LIBS) $(LDFLAGS)
	- mkdir build
ifdef VOIPBOX_DBG
	ln -fs $(TARGET) build/dbg
endif
ifdef VOIPBOX_FSKGEN
	ln -fs $(TARGET) build/fskgen
endif
ifdef VOIPBOX_LED
	ln -fs $(TARGET) build/led
endif
ifdef VOIPBOX_PHONERECORD
	ln -fs $(TARGET) build/phonerecord
endif
ifdef VOIPBOX_REG
	ln -fs $(TARGET) build/reg
endif
ifdef VOIPBOX_RAM
	ln -fs $(TARGET) build/ram
endif
ifdef VOIPBOX_SLIC_RESET
	ln -fs $(TARGET) build/slic_reset
endif
ifdef VOIPBOX_SWITCHMII
	ln -fs $(TARGET) build/switchmii
endif
ifdef VOIPBOX_TEST
	ln -fs $(TARGET) build/crash
endif
ifdef VOIPBOX_VMWIGEN
	ln -fs $(TARGET) build/vmwigen
endif
ifdef VOIPBOX_VOICERECORD
	ln -fs $(TARGET) build/voicerecord
endif
ifdef VOIPBOX_RTCP_STATISTIC
	ln -fs $(TARGET) build/rtcp_statistic
endif
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
	ln -fs $(TARGET) build/iphone_test
endif
ifdef VOIPBOX_RING_TEST
	ln -fs $(TARGET) build/ring_test
endif

linkclean:
	-rm $(ROMFSDIR)/bin/dbg
	-rm $(ROMFSDIR)/bin/fskgen
	-rm $(ROMFSDIR)/bin/led
	-rm $(ROMFSDIR)/bin/phonerecord
	-rm $(ROMFSDIR)/bin/reg
	-rm $(ROMFSDIR)/bin/slic_reset
	-rm $(ROMFSDIR)/bin/switchmii
	-rm $(ROMFSDIR)/bin/crash
	-rm $(ROMFSDIR)/bin/vmwigen
	-rm $(ROMFSDIR)/bin/voicerecord
	-rm $(ROMFSDIR)/bin/rtcp_statistic
	-rm $(ROMFSDIR)/bin/iphone_test
	-rm $(ROMFSDIR)/bin/ring_test
	-rm $(ROMFSDIR)/bin/ram

romfs: linkclean
	$(ROMFSINST)	$(TARGET)			/bin/$(TARGET)


link:
ifdef VOIPBOX_DBG
	$(ROMFSINST)	-s /bin/$(TARGET)	/bin/dbg
endif
ifdef VOIPBOX_FSKGEN
	$(ROMFSINST)    -s /bin/$(TARGET)	/bin/fskgen
endif
ifdef VOIPBOX_LED
	$(ROMFSINST)    -s /bin/$(TARGET)	/bin/led
endif
ifdef VOIPBOX_PHONERECORD
	$(ROMFSINST)    -s /bin/$(TARGET)	/bin/phonerecord
endif
ifdef VOIPBOX_REG
	$(ROMFSINST)	-s /bin/$(TARGET)	/bin/reg
endif
ifdef VOIPBOX_RAM
	$(ROMFSINST)	-s /bin/$(TARGET)	/bin/ram
endif
ifdef VOIPBOX_SLIC_RESET
	$(ROMFSINST)	-s /bin/$(TARGET)	/bin/slic_reset
endif
ifdef VOIPBOX_SWITCHMII
	$(ROMFSINST)    -s /bin/$(TARGET)	/bin/switchmii
endif
ifdef VOIPBOX_TEST
	$(ROMFSINST)    -s /bin/$(TARGET)	/bin/crash
endif
ifdef VOIPBOX_VMWIGEN
	$(ROMFSINST)    -s /bin/$(TARGET)	/bin/vmwigen
endif
ifdef VOIPBOX_VOICERECORD
	$(ROMFSINST)    -s /bin/$(TARGET)	/bin/voicerecord
endif
ifdef VOIPBOX_RTCP_STATISTIC
	$(ROMFSINST)    -s /bin/$(TARGET)	/bin/rtcp_statistic
endif
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
	$(ROMFSINST)    -s /bin/$(TARGET)	/bin/iphone_test
endif
ifdef VOIPBOX_RING_TEST
	$(ROMFSINST)    -s /bin/$(TARGET)	/bin/ring_test
endif

clean:
	make -C pcm-talk clean
	rm -f *.o a.out $(TARGET)
	rm -rf build

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

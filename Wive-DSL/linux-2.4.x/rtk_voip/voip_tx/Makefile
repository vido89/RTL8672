
O_TARGET := voip_tx.o

#ifeq ($(pp), 1)
#	EXTRA_CFLAGS += -DPP1
#endif
#ifeq ($(pp), 2)
#	EXTRA_CFLAGS += -DPP2
#endif

direct-send-mod-objs := direct_send_mgr.o 

obj-$(CONFIG_RTK_VOIP_TX) := direct_send_mod.o

EXTRA_CFLAGS += -I../include
EXTRA_CFLAGS +=  -I../voip_dsp/include
EXTRA_CFLAGS += -I../voip_dsp/dsp_r1/include
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651),y)
EXTRA_CFLAGS += -I../../drivers/net/re865x/rtl865x
endif

ifeq ($(CONFIG_RTK_VOIP_SRTP),y)
EXTRA_CFLAGS +=  -I../voip_dsp/cp3
#EXTRA_CFLAGS+=-DFEATURE_COP3_PROFILE
EXTRA_CFLAGS += -I../srtp/include
EXTRA_CFLAGS += -I../srtp/crypto/include
endif

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make

direct_send_mod.o: $(direct-send-mod-objs)
	        $(LD) -r -o $@ $(direct-send-mod-objs)



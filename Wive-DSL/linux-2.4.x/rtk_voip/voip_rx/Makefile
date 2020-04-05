
O_TARGET := voip_rx.o

trap-mod-objs    := rtk_trap.o

obj-$(CONFIG_RTK_VOIP_RX)            += trap_mod.o

EXTRA_CFLAGS +=  -I../include
EXTRA_CFLAGS +=  -I../voip_dsp/include
EXTRA_CFLAGS +=  -I../voip_dsp/dsp_r1/include

ifeq ($(CONFIG_RTK_VOIP_SRTP),y)
EXTRA_CFLAGS += -I../srtp/include
EXTRA_CFLAGS += -I../srtp/crypto/include
EXTRA_CFLAGS +=  -I../voip_dsp/cp3
#EXTRA_CFLAGS+=-DFEATURE_COP3_PROFILE
endif

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make

trap_mod.o: $(trap-mod-objs)
	$(LD) -r -o $@ $(trap-mod-objs)



O_TARGET := voip_manager.o

voip-manager-mod-objs    := voip_mgr.o voip_mgr_netfilter.o

obj-$(CONFIG_RTK_VOIP_MANAGER) := voip_manager_mod.o

EXTRA_CFLAGS += -I../include
EXTRA_CFLAGS += -I../voip_drivers

ifneq "$(CONFIG_AUDIOCODES_VOIP)" "y"
EXTRA_CFLAGS += -I../voip_dsp/include
EXTRA_CFLAGS += -I../voip_dsp/dsp_r1/include
endif

ifeq "$(CONFIG_AUDIOCODES_VOIP)" "y"
EXTRA_CFLAGS += -I../../acmw_lx/ac_drv
EXTRA_CFLAGS += -I../../acmw_lx/include
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651),y)
EXTRA_CFLAGS += -I../../drivers/net/re865x/rtl865x
endif

ifeq ($(CONFIG_RTK_VOIP_SRTP),y)
EXTRA_CFLAGS += -I../srtp/include
EXTRA_CFLAGS += -I../srtp/crypto/include
endif

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make

voip_manager_mod.o: $(voip-manager-mod-objs)
	        $(LD) -r -o $@ $(voip-manager-mod-objs)


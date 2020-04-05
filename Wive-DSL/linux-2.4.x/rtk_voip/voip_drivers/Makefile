
O_TARGET	:= voip_drivers.o

#CONFIG_VOIP_SDK=y

#pcm-mod-objs    := pcmctrl.o codec.o
pcm-mod-objs    := gpio.o spi.o Slic_api.o led.o

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
pcm-mod-objs	+= iphone/iphone.o
subdir-y += iphone
endif

ifeq ($(CONFIG_VOIP_SDK),y)

ifneq ($(CONFIG_AUDIOCODES_VOIP),y)
pcm-mod-objs   +=caller.obj
endif

else

pcm-mod-objs2    := gpio.o spi.o Slic_api.o led.o pcm_interface.o si3210init.o Legerity88221.o virtual_daa.o daa.o w682388.o

ifneq ($(CONFIG_AUDIOCODES_VOIP),y)
pcm-mod-objs2  +=caller.o
pcm-mod-objs   +=caller.o
endif

endif

pcm-mod-objs   += pcm_interface.o

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210),y)
pcm-mod-objs   += si3210init.o
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215),y)
pcm-mod-objs   += si3210init.o
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221),y)
pcm-mod-objs   += Legerity88221.o
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226),y)
pcm-mod-objs   += Legerity88221.o
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388),y)
pcm-mod-objs   += w682388.o
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_DAA_SI3050),y)
pcm-mod-objs   += daa.o
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA),y)
pcm-mod-objs   += virtual_daa.o
endif

obj-$(CONFIG_RTK_VOIP_DRIVERS)	+= pcm_mod.o
obj-$(CONFIG_RTK_VOIP_DRIVERS)	+= t38_handler.o

EXTRA_CFLAGS += -I../include
EXTRA_CFLAGS += -I../voip_dsp/include
EXTRA_CFLAGS += -I../voip_dsp/cp3
ifeq "$(CONFIG_AUDIOCODES_VOIP)" "y"
EXTRA_CFLAGS += -I../../acmw_lx/ac_drv
endif

#EXTRA_CFLAGS += -DFEATURE_COP3_PROFILE
#EXTRA_CFLAGS += -DFEATURE_COP3_PCMISR
#EXTRA_CFLAGS += -DFEATURE_COP3_PCM_RX
#EXTRA_CFLAGS += -DFEATURE_COP3_PCM_HANDLER
#EXTRA_CFLAGS += -DFEATURE_COP3_LEC
#EXTRA_CFLAGS += -DFEATURE_COP3_DTMFDEC

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651),y)
EXTRA_CFLAGS += -I../../drivers/net/re865x/rtl865x
endif

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make


ifeq ($(CONFIG_VOIP_SDK),y)
pcm_mod.o: $(pcm-mod-objs)
	        $(LD) -r -o $@ $(pcm-mod-objs)
else
pcm_mod.o: $(pcm-mod-objs2)
	        $(LD) -r -o $@ $(pcm-mod-objs)
endif


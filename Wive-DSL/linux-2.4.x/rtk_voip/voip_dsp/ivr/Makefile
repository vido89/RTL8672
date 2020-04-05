AFLAGS=-I../dsp_r1/include -fno-pic 

.S.s:
		$(CPP) $(AFLAGS) $< -o $@
.S.o:
ifeq ($(CONFIG_RTL865XC),y)
		$(CC) $(AFLAGS) -march=5181 -c $< -o $@
else
		$(CC) $(AFLAGS) -march=5280 -c $< -o $@
endif
		
O_TARGET := ivr_sup.o
subdir-y :=
obj-y    :=
#CONFIG_VOIP_SDK=y

ifeq ($(CONFIG_VOIP_SDK),y)
obj-y += ivr.obj 
ifeq ($(CONFIG_RTK_VOIP_G7231),y)
ifneq ($(CONFIG_AUDIOCODES_VOIP),y)
obj-y += ivr_g723sup.obj 
endif
endif
else
obj-y += ivr.o 
ifeq ($(CONFIG_RTK_VOIP_G7231),y)
ifneq ($(CONFIG_AUDIOCODES_VOIP),y)
obj-y += ivr_g723sup.o 
endif
endif
endif


obj-y += ivr_dummy.o

obj-y += ivr_speech.o

EXTRA_CFLAGS +=  -I../../include

ifeq ($(CONFIG_RTK_VOIP_G7231),y)
EXTRA_CFLAGS +=  -I../dsp_r1/
EXTRA_CFLAGS +=  -I../dsp_r1/include/
EXTRA_CFLAGS +=  -I../include/
endif

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make


O_TARGET := dsp_r0.o
subdir-y :=
obj-y    :=

obj-y += dspcodec.o dspparam.o dspcodechelp.o

#EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS +=  -I../../include
EXTRA_CFLAGS +=  -I../include
EXTRA_CFLAGS +=  -I../dsp_r1/include

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make

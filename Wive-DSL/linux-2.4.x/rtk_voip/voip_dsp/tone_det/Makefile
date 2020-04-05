AFLAGS=-I../dsp_r1/include -fno-pic 

.S.s:
		$(CPP) $(AFLAGS) $< -o $@
.S.o:
ifeq ($(CONFIG_RTL865XC),y)
		$(CC) $(AFLAGS) -march=5181 -c $< -o $@
else
		$(CC) $(AFLAGS) -march=5280 -c $< -o $@
endif

O_TARGET := tone_detection.o
subdir-y :=
obj-y    :=

obj-y += busytone_det.o \
	 tone_det.o	\
	 tone_dets.o	\




#EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS +=  -I../../include

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make

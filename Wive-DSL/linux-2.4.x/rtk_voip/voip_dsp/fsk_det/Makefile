AFLAGS=-I../dsp_r1/include -fno-pic 

.S.s:
		$(CPP) $(AFLAGS) $< -o $@
.S.o:
ifeq ($(CONFIG_RTL865XC),y)
		$(CC) $(AFLAGS) -march=5181 -c $< -o $@
else
		$(CC) $(AFLAGS) -march=5280 -c $< -o $@
endif

O_TARGET := fsk_det.o
subdir-y :=
obj-y    :=



obj-y += cid_det.o	\
	 cid_fsk_tab.o  \
	 fsk.o		\
	 fsks.o




#EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS +=  -I../../include

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make


O_TARGET := cp3.o
subdir-y :=
obj-y    :=

obj-y += cp3_profile.o

#EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS+=-I../../include
EXTRA_CFLAGS+=-I../include
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8186),y)
EXTRA_CFLAGS+=-DFEATURE_COP3_PROFILE
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651),y)
EXTRA_CFLAGS+=-DFEATURE_COP3_PROFILE
endif

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make

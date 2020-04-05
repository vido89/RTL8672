#
# Makefile for the IEEE 802.1d ethernet bridging layer.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

export-objs := br.o

O_TARGET	:= bridge.o

ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-y           := br_device.o16 br_fdb.o16 br_forward.o16 br_if.o16 br_input.o16 \
			br_ioctl.o br_stp_bpdu.o16
obj-y           += br.o16 br_notify.o16 br_stp.o16 br_stp_if.o16 br_stp_timer.o16

ifeq ($(CONFIG_NETFILTER),y)
obj-y           += br_netfilter.o16
endif

else
obj-y		:= br.o br_device.o br_fdb.o br_forward.o br_if.o br_input.o \
			br_ioctl.o br_notify.o br_stp.o br_stp_bpdu.o \
			br_stp_if.o br_stp_timer.o

ifeq ($(CONFIG_NETFILTER),y)
obj-y		+= br_netfilter.o
endif
endif

obj-m		:= $(O_TARGET)

include $(TOPDIR)/Rules.make

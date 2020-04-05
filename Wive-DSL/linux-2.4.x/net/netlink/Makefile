#
# Makefile for the netlink driver.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET := netlink.o

export-objs := af_netlink.o

ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-y  := af_netlink.o16
else
obj-y  := af_netlink.o
endif

obj-m  :=

obj-$(CONFIG_NETLINK_DEV) += netlink_dev.o

include $(TOPDIR)/Rules.make

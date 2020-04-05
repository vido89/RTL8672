#
# Makefile for the packet AF.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET := packet.o

ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
#af_packet.o use mips16, crash
obj-$(CONFIG_PACKET) += af_packet.o16
else
obj-$(CONFIG_PACKET) += af_packet.o
endif

include $(TOPDIR)/Rules.make

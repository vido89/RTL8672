#
# Makefile for the Tulip ethernet driver
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

O_TARGET := re8650.o

obj-y   :=  swCore.o re_core.o  swCore.o swTable.o \
	 swUtil.o vlanTable.o l2SwitchTable.o phy.o swNicEnd.o

obj-m   := $(O_TARGET)

include $(TOPDIR)/Rules.make

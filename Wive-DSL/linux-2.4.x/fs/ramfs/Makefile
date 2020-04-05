#
# Makefile for the linux ramfs routines.
#

O_TARGET := ramfs.o

ifdef CONFIG_RTL867X_KERNEL_MIPS16_FS
obj-y := inode.o16
else
obj-y := inode.o
endif

obj-m := $(O_TARGET)

include $(TOPDIR)/Rules.make

#
# Makefile for the linux squashfs routines.
#

O_TARGET := squashfs.o

ifdef CONFIG_RTL867X_KERNEL_MIPS16_FS
obj-y  := inode.o
obj-$(CONFIG_SQUASHFS_LZMA) += LzmaDecode.o16
else
obj-y  := inode.o
obj-$(CONFIG_SQUASHFS_LZMA) += LzmaDecode.o
endif

obj-m := $(O_TARGET)

include $(TOPDIR)/Rules.make

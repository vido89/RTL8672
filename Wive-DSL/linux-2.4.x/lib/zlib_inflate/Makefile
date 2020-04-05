#
# This is a modified version of zlib, which does all memory
# allocation ahead of time.
#
# This is only the decompression, see zlib_deflate for the
# the compression
#
# Decompression needs to be serialized for each memory
# allocation.
#
# (The upsides of the simplification is that you can't get in
# any nasty situations wrt memory management, and that the
# uncompression can be done without blocking on allocation).
#

O_TARGET    := zlib_inflate.o

export-objs := inflate_syms.o

ifdef CONFIG_RTL867X_KERNEL_MIPS16_LIBMMU
obj-y := inflate_syms.o
obj-y += infblock.o16 infcodes.o16 inffast.o16 inflate.o16 \
	 inftrees.o16 infutil.o16
else
obj-y := infblock.o infcodes.o inffast.o inflate.o \
	 inftrees.o infutil.o inflate_syms.o
endif
obj-m := $(O_TARGET)

EXTRA_CFLAGS += -I $(TOPDIR)/lib/zlib_inflate

include $(TOPDIR)/Rules.make

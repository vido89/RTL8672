#
# Makefile for dsp common routines
#
#	./basic_op	: ITU basic operations.
#	./util		: memory, stack, IMEM/DMEM operations.
#
AFLAGS :=-I../include -fno-pic -mno-abicalls


.S.s:
		$(CPP) $(AFLAGS) $< -o $@
.S.o:
ifeq ($(CONFIG_RTL865XC),y)
		$(CC) $(AFLAGS) -march=5181 -c $< -o $@
else
		$(CC) $(AFLAGS) -march=5280 -c $< -o $@
endif
        
O_TARGET := common.o
subdir-y := 
obj-y    :=

export-objs = 

ifeq ($(CONFIG_RTK_VOIP),m)
obj-y += 	util/dspfuncs.o util/dspfunc.o util/dspfunc_tab.o util/oper_32b.o util/oper_32bs.o util/pre_proc.o util/pre_procs.o \
			util/memset.o util/utility.o util/utilitys.o util/dmem_stack.o util/my_head.o	\
			basic_op/basic_op.o basic_op/basic_ops.o 
else
obj-y += 	util/dspfuncs.o util/dspfunc.o util/dspfunc_tab.o util/oper_32b.o util/oper_32bs.o util/pre_proc.o util/pre_procs.o \
			util/memset.o util/utility.o util/utilitys.o util/dmem_stack.o util/codec_mem.o util/my_head.o	\
			basic_op/basic_op.o basic_op/basic_ops.o 
endif

EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS+=-I../../../include
EXTRA_CFLAGS+=-I../common/basic_op
EXTRA_CFLAGS+=-I../include
EXTRA_CFLAGS+=-I../../include
EXTRA_CFLAGS+=-I../common/util
EXTRA_CFLAGS+=-I..

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make

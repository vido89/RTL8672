#
# Makefile for the ATM Protocol Families.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET	:= atm.o

export-objs 	:= common.o atm_misc.o raw.o resources.o ipcommon.o proc.o

list-multi	:= mpoa.o
mpoa-objs	:= mpc.o mpoa_caches.o mpoa_proc.o

ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_ATM) := common.o16 atm_misc.o16 raw.o16 resources.o16
obj-$(CONFIG_ATM) += pvc.o16
else
obj-$(CONFIG_ATM) := pvc.o common.o atm_misc.o raw.o resources.o
endif
obj-$(CONFIG_ATM_SVC) := addr.o signaling.o svc.o

ifeq ($(CONFIG_ATM_CLIP),y)
obj-y += clip.o
NEED_IPCOM = ipcommon.o
endif

ifeq ($(CONFIG_ATM_BR2684),y)
  NEED_IPCOM = ipcommon.o
else
  ifeq ($(CONFIG_ATM_BR2684),m)
	NEED_IPCOM = ipcommon.o
  endif
endif
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_ATM_BR2684) += br2684.o16
else
obj-$(CONFIG_ATM_BR2684) += br2684.o
endif

ifeq ($(CONFIG_NET_SCH_ATM),y)
NEED_IPCOM = ipcommon.o
endif

obj-y += $(NEED_IPCOM)

ifeq ($(CONFIG_PROC_FS),y)
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-y += proc.o16
else
obj-y += proc.o
endif
endif

obj-$(CONFIG_ATM_LANE) += lec.o
obj-$(CONFIG_ATM_MPOA) += mpoa.o
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_PPPOATM) += pppoatm.o16
else
obj-$(CONFIG_PPPOATM) += pppoatm.o
endif

include $(TOPDIR)/Rules.make

mpoa.o: $(mpoa-objs)
	$(LD) -r -o mpoa.o $(mpoa-objs)

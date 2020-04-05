#
# linux/drivers/chips/Makefile
#
# $Id: Makefile,v 1.3 2008/10/22 09:41:01 tylo Exp $

O_TARGET	:= chipslink.o

export-objs	:= chipreg.o gen_probe.o

#                       *** BIG UGLY NOTE ***
#
# The removal of get_module_symbol() and replacement with
# inter_module_register() et al has introduced a link order dependency
# here where previously there was none.  We now have to ensure that
# the CFI command set drivers are linked before cfi_probe.o

ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_MTD
obj-$(CONFIG_MTD)               += chipreg.o16
obj-$(CONFIG_MTD_AMDSTD)        += amd_flash.o
obj-$(CONFIG_MTD_CFI)           += cfi_probe.o16
obj-$(CONFIG_MTD_CFI_AMDSTD)    += cfi_cmdset_0002.o16 #16, writing has problems
obj-$(CONFIG_MTD_CFI_INTELEXT)  += cfi_cmdset_0001.o
obj-$(CONFIG_MTD_GEN_PROBE)     += gen_probe.o16
obj-$(CONFIG_MTD_JEDEC)         += jedec.o
obj-$(CONFIG_MTD_JEDECPROBE)    += jedec_probe.o
obj-$(CONFIG_MTD_RAM)           += map_ram.o
obj-$(CONFIG_MTD_ROM)           += map_rom.o
obj-$(CONFIG_MTD_SHARP)         += sharp.o
obj-$(CONFIG_MTD_ABSENT)        += map_absent.o
else
obj-$(CONFIG_MTD)		+= chipreg.o
obj-$(CONFIG_MTD_AMDSTD)	+= amd_flash.o 
obj-$(CONFIG_MTD_CFI)		+= cfi_probe.o
obj-$(CONFIG_MTD_CFI_AMDSTD)	+= cfi_cmdset_0002.o
obj-$(CONFIG_MTD_CFI_INTELEXT)	+= cfi_cmdset_0001.o
obj-$(CONFIG_MTD_GEN_PROBE)	+= gen_probe.o
obj-$(CONFIG_MTD_JEDEC)		+= jedec.o
obj-$(CONFIG_MTD_JEDECPROBE)	+= jedec_probe.o
obj-$(CONFIG_MTD_RAM)		+= map_ram.o
obj-$(CONFIG_MTD_ROM)		+= map_rom.o
obj-$(CONFIG_MTD_SHARP)		+= sharp.o
obj-$(CONFIG_MTD_ABSENT)	+= map_absent.o
endif
obj-$(CONFIG_RTL8672_SPI_FLASH)	+= spi_probe.o spi_cmd.o spi_flash.o
include $(TOPDIR)/Rules.make

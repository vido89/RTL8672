#
# Makefile for the common code of XIPOS MIPS Simulator.
#
.S.s:
	$(CPP) $(AFLAGS) $< -o $*.s
.S.o:../../../../../lib/uClibc/include/rtk/options.h
	$(CC) $(AFLAGS) -c $< -o $*.o

O_TARGET:= rtl8670.o
obj-y    += muldiv_lib.o init.o gdbstub.o int.o timer.o irq.o gpio.o prom.o rtl_gpio.o watchdog.o 
ifdef CONFIG_PCI
obj-y += pci.o
endif
ifdef CONFIG_RTL8672
obj-y += led-generic.o pushbutton.o
ifdef CONFIG_BOARD_001
obj-y += board-001.o
endif
ifdef CONFIG_BOARD_003
obj-y += board-003.o
endif
ifdef CONFIG_GPIO_LED_TR068
obj-y += led-tr068.o
endif
endif
ifdef CONFIG_EOC
obj-y += netadsl.o
endif
ifndef CONFIG_DSL_CODESWAP
ifdef CONFIG_RTL8670
obj-y += adsl70.lib
endif
ifdef CONFIG_RTL8672
obj-y += adsl72.lib
else
obj-y += adsl71.lib
endif
			 
endif
			 
include $(TOPDIR)/Rules.make

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
O_TARGET := iphone.o
obj-y    :=
obj-y += base_gpio.o lcm.o WM8510.o

obj-$(CONFIG_RTK_VOIP_IP_PHONE)   += keypad_interface.o lcm_interface.o

EXTRA_CFLAGS += -I../../include


include $(TOPDIR)/Rules.make
endif

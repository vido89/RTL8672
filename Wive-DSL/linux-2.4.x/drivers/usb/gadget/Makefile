#
# Makefile for USB peripheral controller and gadget drivers
# for kbuild 2.4
#

# for static linking
O_TARGET	:= built-in.o

list-multi			:= g_zero.o g_ether.o g_file_storage.o

# Use one of the following flags to compile the software in host-only or
# device-only mode.
#CPPFLAGS        += -DDWC_HOST_ONLY

ifeq ($(CONFIG_USB_OTG_DEVICE_RTL8672),y)
EXTRA_CFLAGS        += -DDWC_DEVICE_ONLY
else
EXTRA_CFLAGS        += -DDWC_HOST_ONLY
endif


obj-$(CONFIG_USB_NET2280)	+= net2280.o
obj-$(CONFIG_USB_GOKU)		+= goku_udc.o

#obj-m	:= dwc_otg.o
#dwc_otg-objs			:= dwc_otg_driver.o 
#dwc_otg-objs			+= dwc_otg_cil.o dwc_otg_cil_intr.o 
#dwc_otg-objs			+= dwc_otg_pcd.o dwc_otg_pcd_intr.o 

#obj-$(CONFIG_USB_OTG_DEVICE_RTL8672)	+= dwc_otg.o 

obj-m	:= dwc_otg.o

#dwc_otg-objs	:= dwc_otg_driver.o dwc_otg_attr.o 
dwc_otg-objs	:= dwc_otg_driver.o
dwc_otg-objs	+= dwc_otg_cil.o dwc_otg_cil_intr.o 
ifeq ($(CONFIG_USB_OTG_DEVICE_RTL8672),y)
dwc_otg-objs	+= dwc_otg_pcd.o dwc_otg_pcd_intr.o 
else
dwc_otg-objs	+= dwc_otg_hcd.o dwc_otg_hcd_intr.o dwc_otg_hcd_queue.o
endif

obj-y	+= dwc_otg.o 




# only one of these may be statically linked ...
controller-$(CONFIG_USB_NET2280)	+= net2280.o
controller-$(CONFIG_USB_GOKU)		+= goku_udc.o
controller-$(CONFIG_USB_OTG_DEVICE_RTL8672)		+= dwc_otg.o

# ... and only one of these, too; kbuild/kconfig don't help though.
g_zero-objs			:= zero.o usbstring.o config.o epautoconf.o
obj-$(CONFIG_USB_ZERO)		+= g_zero.o

g_ether-objs			:= ether.o usbstring.o config.o epautoconf.o
ifeq ($(CONFIG_USB_ETH_RNDIS),y)
	g_ether-objs            += rndis.o
endif
obj-$(CONFIG_USB_ETH)		+= g_ether.o

g_file_storage-objs		:= file_storage.o usbstring.o config.o \
						epautoconf.o
obj-$(CONFIG_USB_FILE_STORAGE)	+= g_file_storage.o

export-objs :=			$(controller-y) $(controller-m)

include $(TOPDIR)/Rules.make

dwc_otg.o: $(dwc_otg-objs)
	$(LD) -r -o $@ $(dwc_otg-objs)
g_zero.o: $(g_zero-objs)
	$(LD) -r -o $@ $(g_zero-objs)
g_ether.o: $(g_ether-objs)
	$(LD) -r -o $@ $(g_ether-objs)
g_file_storage.o: $(g_file_storage-objs)
	$(LD) -r -o $@ $(g_file_storage-objs)

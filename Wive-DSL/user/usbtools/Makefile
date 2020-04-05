
.EXPORT_ALL_VARIABLES:

USBTOOLS_ROOT=$(shell pwd)
LIBUSB_DIR=libusb-0.1.12
USBMODESWITCH_DIR=usb_modeswitch-1.0.5
USBUTILS_DIR=usbutils-0.84
PRE_INSTALL_DIR=$(USBTOOLS_ROOT)/preinstall

CFLAGS+= -I$(PRE_INSTALL_DIR)/include
LIBUSB_CFLAGS = -I$(PRE_INSTALL_DIR)/include
LIBUSB_LIBS = -L$(PRE_INSTALL_DIR)/lib -lusb

all:
	mkdir -p $(PRE_INSTALL_DIR)
ifeq ($(CONFIG_USER_USBTOOLS_LIBUSB),y)
	#LIBUSB
	if [ -d $(LIBUSB_DIR) ]; then \
		cd $(LIBUSB_DIR); \
		if [ ! -f config.h ]; then \
			./configure --host=mips-linux --prefix=$(PRE_INSTALL_DIR) --exec-prefix=$(PRE_INSTALL_DIR); \
		fi; \
		$(MAKE); \
		$(MAKE) install; \
	fi
endif
ifeq ($(CONFIG_USER_USBTOOLS_USBMODESWITCH),y)
	#USBMODESWITCH
	if [ -d $(USBMODESWITCH_DIR) ]; then \
		$(MAKE) -C $(USBMODESWITCH_DIR); \
		$(MAKE) -C $(USBMODESWITCH_DIR) install; \
	fi
endif
ifeq ($(CONFIG_USER_USBTOOLS_LSUSB),y)
	#USBUTILS
	if [ -d $(USBUTILS_DIR) ]; then \
		cd $(USBUTILS_DIR); \
		if [ ! -f config.h ]; then \
			./configure --host=mips-linux --prefix=$(PRE_INSTALL_DIR) --exec-prefix=$(PRE_INSTALL_DIR) --disable-zlib; \
		fi; \
		$(MAKE); \
		$(MAKE) install; \
	fi
endif
	
clean:
#ifeq ($(CONFIG_USER_USBTOOLS_LSUSB),y)
	if [ -d $(USBUTILS_DIR) ]; then \
		$(MAKE) -C $(USBUTILS_DIR) clean; \
	fi
#endif
#ifeq ($(CONFIG_USER_USBTOOLS_USBMODESWITCH),y)
	if [ -d $(USBMODESWITCH_DIR) ]; then \
		$(MAKE) -C $(USBMODESWITCH_DIR) clean; \
	fi
#endif
#ifeq ($(CONFIG_USER_USBTOOLS_LIBUSB),y)
	if [ -d $(LIBUSB_DIR) ]; then \
		$(MAKE) -C $(LIBUSB_DIR) clean; \
	fi
#endif
	rm -rf $(PRE_INSTALL_DIR)


romfs:
ifeq ($(CONFIG_USER_USBTOOLS_USBMODESWITCH),y)
	cp -f $(PRE_INSTALL_DIR)/etc/usb_modeswitch.conf $(USBTOOLS_ROOT)/../../romfs/etc
	cp -f $(PRE_INSTALL_DIR)/usr/sbin/usb_modeswitch $(USBTOOLS_ROOT)/../../romfs/bin
	cp -f $(PRE_INSTALL_DIR)/usr/sbin/ozerocdoff $(USBTOOLS_ROOT)/../../romfs/bin
endif
ifeq ($(CONFIG_USER_USBTOOLS_LSUSB),y)
	cp -f $(PRE_INSTALL_DIR)/share/usb.ids $(USBTOOLS_ROOT)/../../romfs/etc
	cp -f $(PRE_INSTALL_DIR)/sbin/lsusb $(USBTOOLS_ROOT)/../../romfs/bin
	cp -f ../../toolchain/mips-linux-uclibc/lib/libgcc_s.so.1 $(USBTOOLS_ROOT)/../../romfs/lib/
endif
ifeq ($(CONFIG_USER_USBTOOLS_LIBUSB),y)
	cp -f $(PRE_INSTALL_DIR)/lib/libusb-0.1.so.4.4.4 $(USBTOOLS_ROOT)/../../romfs/lib/libusb-0.1.so.4
endif

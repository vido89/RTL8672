#
# Realtek Semiconductor Corp.
#
# uclibc Makefile for the RSDK/uclibc toolchain
#
# Tony Wu (tonywu@realtek.com.tw)
# Oct. 10, 2005
#

#----------------------------------------------------------------------------
# uClibc local variables
#----------------------------------------------------------------------------

# Anticipate the change.
UCLIBC_SRC := $(BASE_DIR)/uClibc-$(UCLIBC_VER)
UCLIBC_DIR := $(STAGE_DIR)/uClibc-$(UCLIBC_VER)

#----------------------------------------------------------------------------
# uClibc targets
#----------------------------------------------------------------------------

$(UCLIBC_DIR)/.prepared: $(BUILD_DIR)/.ulib-make-init
	rm -rf $(UCLIBC_DIR)
	cp -r $(UCLIBC_SRC) $(UCLIBC_DIR)
	touch $(UCLIBC_DIR)/.prepared

$(UCLIBC_DIR)/.configured: $(UCLIBC_DIR)/.prepared
# copy default uclibc config file
	(cd $(UCLIBC_DIR); ./gen_config.pl $(UCLIBC_DIR) 4180 EB n)
	mkdir -p $(STAGE_DIR)/uclibc_dev/usr/include
	mkdir -p $(STAGE_DIR)/uclibc_dev/usr/lib
	mkdir -p $(STAGE_DIR)/uclibc_dev/lib
	$(MAKE) -C $(UCLIBC_DIR) \
		PREFIX=$(STAGE_DIR)/uclibc_dev/ \
		DEVEL_PREFIX=/usr/ \
		RUNTIME_PREFIX=$(STAGE_DIR)/uclibc_dev/ \
		HOSTCC="$(HOST_CC)" \
		headers install_header;
	touch $(UCLIBC_DIR)/.configured

$(UCLIBC_DIR)/.compiled: $(UCLIBC_DIR)/.configured
	(cd $(UCLIBC_DIR); ./make.pl $(BUILD_DIR) $(UCLIBC_DIR))
	touch $(UCLIBC_DIR)/.compiled

$(UCLIBC_DIR)/.installed: $(UCLIBC_DIR)/.compiled
	$(MAKE) -C $(UCLIBC_DIR) \
		PREFIX=$(BUILD_DIR)/ \
		DEVEL_PREFIX=/ \
		RUNTIME_PREFIX=/ \
		install_header
	$(MAKE) -C $(UCLIBC_DIR)/utils \
		PREFIX=$(BUILD_DIR)/ \
		HOSTCC="$(HOST_CC)" \
		hostutils
	touch $(UCLIBC_DIR)/.installed


uclibc-configured: $(UCLIBC_DIR)/.configured

uclibc: gcc-step1 $(UCLIBC_DIR)/.installed
#$(UCLIBC_TARGETS)

uclibc-clean:
	-$(MAKE) -C $(UCLIBC_DIR) clean
	rm -f $(UCLIBC_DIR)/.config

uclibc-dirclean:
	rm -rf $(UCLIBC_DIR)

#----------------------------------------------------------------------------
# uClibc for the target just needs its header files
# and whatnot installed.
#----------------------------------------------------------------------------

UCLIBC_TARGETS=$(TARGET_DIR)/lib/libc.so.0 $(TARGET_DIR)/lib/ld.so.1

uclibc-target-utils: $(TARGET_DIR)/usr/bin/ldd

$(TARGET_DIR)/lib/libc.so.0: uclibc-util
	$(MAKE) -C $(UCLIBC_DIR) \
		PREFIX=$(TARGET_DIR) \
		DEVEL_PREFIX=/usr/ \
		RUNTIME_PREFIX=/ \
		install_runtime
	(cd $(TARGET_DIR)/lib; ln -s ld-uClibc-0.9.28.so ld.so.1)
	touch -c $(TARGET_DIR)/lib/libc.so.0

$(TARGET_DIR)/lib/ld.so.1: $(TARGET_DIR)/lib/ld-uClibc-0.9.28.so
	(cd $(TARGET_DIR)/lib; ln -s ld-uClibc-0.9.28.so ld.so.1)

$(TARGET_DIR)/usr/bin/ldd:
	$(MAKE) -C $(UCLIBC_DIR) $(TARGET_CONFIGURE_OPTS) \
		PREFIX=$(TARGET_DIR) utils install_utils
	touch -c $(TARGET_DIR)/usr/bin/ldd

$(TARGET_DIR)/usr/lib/libc.a: $(BUILD_DIR)/$(TARGET_NAME)/lib/libc.a
	$(MAKE) -C $(UCLIBC_DIR) \
		PREFIX=$(TARGET_DIR) \
		DEVEL_PREFIX=/usr/ \
		RUNTIME_PREFIX=/ \
		install_header
	touch -c $(TARGET_DIR)/usr/lib/libc.a

uclibc-target: $(TARGET_DIR)/usr/lib/libc.a $(TARGET_DIR)/usr/bin/ldd

uclibc_target-clean:
	rm -f $(TARGET_DIR)/include

uclibc_target-dirclean:
	rm -f $(TARGET_DIR)/include


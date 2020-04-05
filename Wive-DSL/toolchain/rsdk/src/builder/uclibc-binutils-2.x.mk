#
# Realtek Semiconductor Corp.
# 
# binutils-2.16 makefile for uclibc
#
# Tony Wu (tonywu@realtek.com.tw)
# Oct. 10, 2005

#############################################################
#
# build binutils for use on the host system
#
#############################################################
BINUTILS_DIR:=$(BASE_DIR)/binutils-$(BINUTILS_VER)
BINUTILS_DIR1:=$(STAGE_DIR)/binutils-$(BINUTILS_VER)-build


$(BINUTILS_DIR1)/.configured: $(BUILD_DIR)/.ulib-make-init
	mkdir -p $(BINUTILS_DIR1)
	(cd $(BINUTILS_DIR1); \
		$(BINUTILS_DIR)/configure \
		--prefix=$(BUILD_DIR) \
		--build=$(BUILD_NAME) \
		--host=$(HOST_NAME) \
		--target=$(TARGET_NAME) \
		--disable-nls \
		--enable-multilib \
		--with-float=soft );
	touch $(BINUTILS_DIR1)/.configured

$(BINUTILS_DIR1)/.compiled: $(BINUTILS_DIR1)/.configured
	$(MAKE) -C $(BINUTILS_DIR1) all
	touch $(BINUTILS_DIR1)/.compiled

# Make install will put gettext data in staging_dir/share/locale.
# Unfortunatey, it isn't configureable.
$(BINUTILS_DIR1)/.installed: $(BINUTILS_DIR1)/.compiled
	$(MAKE) -C $(BINUTILS_DIR1) install
	touch $(BINUTILS_DIR1)/.installed

binutils-dependancies:
	@if ! which bison > /dev/null ; then \
		echo -e "\n\nYou must install 'bison' on your build machine\n"; \
		exit 1; \
	fi;
	@if ! which flex > /dev/null ; then \
		echo -e "\n\nYou must install 'flex' on your build machine\n"; \
		exit 1; \
	fi;
	@if ! which msgfmt > /dev/null ; then \
		echo -e "\n\nYou must install 'gettext' on your build machine\n"; \
		exit 1; \
	fi;

binutils: binutils-dependancies $(BINUTILS_DIR1)/.installed

binutils-clean:
	rm -f $(BUILD_DIR)/bin/$(TARGET_NAME)*
	-$(MAKE) -C $(BINUTILS_DIR1) clean

binutils-dirclean:
	rm -rf $(BINUTILS_DIR1)


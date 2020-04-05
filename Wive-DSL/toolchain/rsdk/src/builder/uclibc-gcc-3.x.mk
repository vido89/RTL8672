#
# Realtek Semiconductor Corp.
# 
# gcc 3.x Makefile for the RSDK/uclibc toolchain
#
# Tony Wu (tonywu@realtek.com.tw)
# Oct. 10, 2005 
# 

#-----------------------------------------------------------------------------
# gcc 3.x local variables
#-----------------------------------------------------------------------------
GCC_SRC := $(BASE_DIR)/gcc-$(GCC_VER)
GCC_DIR := $(BASE_DIR)/gcc-$(GCC_VER)

#-----------------------------------------------------------------------------
# GCC1 (first pass compiler) targets
#-----------------------------------------------------------------------------
GCC_BUILD_DIR1:=$(STAGE_DIR)/gcc-$(GCC_VER)-step1
GCC_BUILD_DIR2:=$(STAGE_DIR)/gcc-$(GCC_VER)-step2

# The --without-headers option stopped working with gcc 3.0 and has never been
# fixed, so we need to actually have working C library header files prior to
# the step or libgcc will not build...
$(GCC_BUILD_DIR1)/.configured: $(BUILD_DIR)/.ulib-make-init
	mkdir -p $(GCC_BUILD_DIR1)
	(cd $(GCC_BUILD_DIR1); PATH=$(TARGET_PATH) \
		$(GCC_DIR)/configure \
		--prefix=$(BUILD_DIR) \
		--build=$(BUILD_NAME) \
		--host=$(HOST_NAME) \
		--target=$(TARGET_NAME) \
		--enable-languages=c \
		--enable-multilib \
		--disable-shared \
		--disable-threads \
		--disable-__cxa_atexit \
		--disable-nls \
		--with-arch=4180 \
		--with-sysroot=$(STAGE_DIR)/uclibc_dev \
		--with-gnu-ld \
		--with-gnu-as \
		--with-dwarf2 \
		--with-float=soft);
	touch $(GCC_BUILD_DIR1)/.configured

$(GCC_BUILD_DIR1)/.compiled: $(GCC_BUILD_DIR1)/.configured
	(PATH=$(TARGET_PATH) $(MAKE) -C $(GCC_BUILD_DIR1) all-gcc;)
	touch $(GCC_BUILD_DIR1)/.compiled

$(GCC_BUILD_DIR1)/.installed: $(GCC_BUILD_DIR1)/.compiled
	(PATH=$(TARGET_PATH) $(MAKE) -C $(GCC_BUILD_DIR1) install-gcc;)
	touch $(GCC_BUILD_DIR1)/.installed

gcc-step1: binutils $(GCC_BUILD_DIR1)/.installed

gcc-step1-clean:
	rm -rf $(GCC_BUILD_DIR1)
	rm -f $(BUILD_DIR)/bin/$(TARGET_NAME)*

gcc-step1-dirclean:
	rm -rf $(GCC_BUILD_DIR1)


#-----------------------------------------------------------------------------
# GCC2 (second pass compiler) targets
#-----------------------------------------------------------------------------
$(GCC_BUILD_DIR2)/.configured: $(BUILD_DIR)/.ulib-make-init $(BUILD_DIR)/lib/libc.a
	mkdir -p $(GCC_BUILD_DIR2)
	# Important!  Required for limits.h to be fixed.
	ln -sf ../include $(BUILD_DIR)/$(TARGET_NAME)/sys-include
	ln -sf ../include $(BUILD_DIR)/$(TARGET_NAME)/include
	(cd $(GCC_BUILD_DIR2); PATH=$(TARGET_PATH) \
		$(GCC_DIR)/configure \
		--prefix=$(BUILD_DIR) \
		--build=$(BUILD_NAME) \
		--host=$(HOST_NAME) \
		--target=$(TARGET_NAME) \
		--enable-languages=c,c++ \
		--enable-shared \
		--enable-multilib \
		--enable-sjlj-exceptions \
		--disable-__cxa_atexit \
		--disable-nls \
		--with-arch=4180 \
		--with-gnu-ld \
		--with-gnu-as \
		--with-dwarf2 \
		--with-float=soft);
	touch $(GCC_BUILD_DIR2)/.configured

$(GCC_BUILD_DIR2)/.compiled: $(GCC_BUILD_DIR2)/.configured
	(PATH=$(TARGET_PATH) $(MAKE) -C $(GCC_BUILD_DIR2) all;)
	touch $(GCC_BUILD_DIR2)/.compiled

$(GCC_BUILD_DIR2)/.installed: $(GCC_BUILD_DIR2)/.compiled
	(PATH=$(TARGET_PATH) $(MAKE) -C $(GCC_BUILD_DIR2) install;)
	touch $(GCC_BUILD_DIR2)/.installed

gcc: gcc-step1 $(LIBFLOAT_TARGET) uclibc $(GCC_BUILD_DIR2)/.installed

gcc-clean:
	rm -rf $(GCC_BUILD_DIR2)
	rm -f $(BUILD_DIR)/bin/$(TARGET_NAME)*

gcc-dirclean:
	rm -rf $(GCC_BUILD_DIR2)

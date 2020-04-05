#
#	Makefile -- Master makefile for all libraries.
#

.EXPORT_ALL_VARIABLES:

#
# Include architecture specific build rules.
#

ifndef ROOTDIR
ROOTDIR=..
endif

UCLINUX_BUILD_LIB=1
include $(LINUX_CONFIG)
include $(CONFIG_CONFIG)
include $(ARCH_CONFIG)

#
# always build these dir_
#
dir_y  = $(LIBCDIR) libnet libcrypt_old uClibc++
dir_n  =
dir_   =

ifeq ($(LIBCDIR),libc)
dir_y += libm
endif

#fullllll cleanup if no select
SUBDIRS_APP   = uClibc uClibc++
SUBDIRS_APP  += libatm libccmalloc libcurl libgmp libm libmd5 libnet libpcap libssl libsig
SUBDIRS_APP  += libc libcrypt_old libg libjpeg libmatrixssl libncurses libpam  libpng zlib

#
# and build these if someone needs them
#
dir_$(CONFIG_LIB_SNAPGEAR)     += libsnapgear
dir_$(CONFIG_LIB_LIBSSL)       += libssl
dir_$(CONFIG_LIB_LIBSSL_FORCE) += libssl
dir_$(CONFIG_LIB_LIBMATRIXSSL) += libmatrixssl
dir_$(CONFIG_LIB_LIBGMP)       += libgmp
dir_$(CONFIG_LIB_LIBGMP_FORCE) += libgmp
dir_$(CONFIG_LIB_LIBG)         += libg
dir_$(CONFIG_LIB_LIBG_FORCE)   += libg
dir_$(CONFIG_LIB_LIBPAM)       += libpam
dir_$(CONFIG_LIB_LIBPAM_FORCE) += libpam
dir_$(CONFIG_LIB_LIBPCAP)      += libpcap
dir_$(CONFIG_LIB_LIBPCAP_FORCE)+= libpcap
dir_$(CONFIG_LIB_ZLIB)         += zlib
dir_$(CONFIG_LIB_ZLIB_FORCE)   += zlib
dir_$(CONFIG_LIB_LIBATM)       += libatm
dir_$(CONFIG_LIB_LIBATM_FORCE) += libatm
dir_$(CONFIG_LIB_LIBPNG)       += libpng
dir_$(CONFIG_LIB_LIBPNG_FORCE) += libpng
dir_$(CONFIG_LIB_LIBJPEG)      += libjpeg
dir_$(CONFIG_LIB_LIBJPEG_FORCE)+= libjpeg
dir_$(CONFIG_LIB_LIBSNAPGEARPP) += libsnapgear++
dir_$(CONFIG_LIB_LIBCCMALLOC)  += libccmalloc
dir_$(CONFIG_LIB_LIBMD5_FORCE) += libmd5
dir_$(CONFIG_LIB_NCURSES)      += libncurses
dir_$(CONFIG_LIB_LIBSIG)       += libsig

.PHONY: all
all: pre_compile
ifeq ($(findstring glibc,$(LIBCDIR)),glibc)
	[ -d $(LIBCDIR) ] || mkdir $(LIBCDIR)
	if [ ! -f $(LIBCDIR)/config.cache ]; then \
		cd $(LIBCDIR); \
		export CFLAGS="$(VENDOR_CFLAGS) -DEMBED -Os"; \
		unset LDFLAGS; \
		../configure  --prefix= \
			$(GLIBC_TARGET_OPTS) --build=i386-redhat-linux \
			--with-headers=$(ROOTDIR)/$(LINUXDIR)/include \
			--without-cvs --without-gd \
			--disable-profile --disable-force-install \
			--enable-add-ons ; \
	fi
endif
	for i in $(dir_y) ; do [ ! -d $$i ] || make -C $$i || exit $? ; done
ifeq ($(CONFIG_LIB_CURL),y)
	cd libcurl && ./DoConfigure.sh && make && make install && cd ..
endif
ifeq ($(CONFIG_LIB_NCURSES),y)
	cd libncurses && make install && cd ..
endif
ifeq ($(CONFIG_LIB_LIBSIG),y)
	cd libsig && ./DoConfigure.sh && make && make install && cd ..
endif

romfs:
	for i in $(dir_y) ; do \
		[ ! -d $$i ] || make -C $$i romfs || exit $$? ; \
	done
ifeq ($(CONFIG_LIB_CURL),y)
	cd libcurl && make romfs && cd ..
endif
ifeq ($(CONFIG_LIB_NCURSES),y)
	cd libncurses && make romfs && cd ..
endif
ifeq ($(CONFIG_LIB_LIBSIG),y)
	cd libsig && make romfs && cd ..
endif
	-$(STRIP) $(ROMFSDIR)/lib/*.so
	-$(STRIP) $(ROMFSDIR)/lib/*.a

clean:
ifeq ($(findstring glibc,$(LIBCDIR)),glibc)
	-rm -rf $(LIBCDIR)
endif
	-for i in $(dir_y) $(dir_n) $(dir_); do \
		[ ! -d $$i ] || make -C $$i clean ; \
	done

fullclean:
	echo "Full cleanUP"
	@set -e; for i in $(SUBDIRS_APP); do $(MAKE) -C $$i clean; rm -rfd $$i/filesystem/*; done
	echo "All clean"

pre_compile:
ifeq ($(CONFIG_LIB_LIBSSL),y)
	if [ ! -f libssl/config_done ]; then \
	cd libssl;./Configure no-zlib no-asm no-hw no-dso shared no-comp no-engine no-ssl2 no-ssl3 no-sse2 no-aes no-bf no-cast no-dh  no-dsa no-ec no-ecdh no-ecdsa no-idea no-md2 no-md4 no-mdc2 no-rc2 no-rc5 no-ripemd no-rmd160 no-krb5  mips-linux;make depend;touch config_done; \
	fi
endif
	@echo "finished pre_compile"

#cd libssl;./Configure no-zlib no-asm no-hw no-dso no-comp no-engine shared no-ssl2 no-ssl3 no-sse2 no-aes no-bf no-cast no-dh  no-dsa no-ec no-ecdh no-ecdsa no-idea no-md2 no-md4 no-mdc2 no-rc2 no-rc5 no-ripemd no-rmd160 no-krb5  mips-linux;make depend;touch config_done; \

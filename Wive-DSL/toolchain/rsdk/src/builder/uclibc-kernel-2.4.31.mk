#############################################################
#
# Setup the kernel headers.  I include a generic package of
# kernel headers here, so you shouldn't need to include your
# own.  Be aware these kernel headers _will_ get blown away
# by a 'make clean' so don't put anything sacred in here...
#
#############################################################
HEADERS := linux-libc-headers
HEADERS_SRC := $(HEADERS)-$(HEADERS_VER).tar.gz
HEADERS_DIR := $(BASE_DIR)/linux

HEADERS_MV :=2
HEADERS_PL :=4
HEADERS_SL :=31


$(HEADERS_DIR)/.unpacked: $(BUILD_DIR)/.ulib-make-init
	(cd $(BASE_DIR); \
		rm -f linux; \
		ln -sf $(HEADERS)-$(HEADERS_VER) linux)
	touch $(HEADERS_DIR)/.unpacked

$(HEADERS_DIR)/.patched: $(HEADERS_DIR)/.unpacked
	touch $(HEADERS_DIR)/.patched

$(HEADERS_DIR)/.configured: $(HEADERS_DIR)/.patched
	rm -f $(HEADERS_DIR)/include/asm
	@if [ ! -f $(HEADERS_DIR)/Makefile ] ; then \
	    echo -e "HEADERS_MV = $(HEADERS_MV)\nHEADERS_PL = $(HEADERS_PL)\n" > \
		    $(HEADERS_DIR)/Makefile; \
	    echo -e "HEADERS_SL = $(HEADERS_SL)\nEXTRAVERSION =\n" >> \
		    $(HEADERS_DIR)/Makefile; \
	    echo -e "KERNELRELEASE=\$$(HEADERS_MV).\$$(HEADERS_PL).\$$(HEADERS_SL)\$$(EXTRAVERSION)" >> \
		    $(HEADERS_DIR)/Makefile; \
	fi;
	(cd $(HEADERS_DIR)/include; ln -fs asm-mips asm;)
	touch $(HEADERS_DIR)/include/linux/autoconf.h;
	touch $(HEADERS_DIR)/.configured

#
# exported target
#
kernel-headers: $(HEADERS_DIR)/.configured

kernel-headers-clean: clean
	rm -f $(KERNEL)
	rm -rf $(HEADERS_DIR)

kernel-headers-dirclean:
	rm -rf $(HEADERS_DIR)

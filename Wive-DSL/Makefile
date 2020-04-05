############################################################################

#
# Makefile -- Top level uClinux makefile.
#
# Copyright (c) 2001-2002, SnapGear (www.snapgear.com)
# Copyright (c) 2001, Lineo
#

############################################################################
#
# Software Version
#
include version
VERSIONSTR = $(VERSIONPKG)


############################################################################
#
# Lets work out what the user wants, and if they have configured us yet
#

ifeq (.config,$(wildcard .config))
include .config

all: subdirs romfs modules modules_install image
else
all: config_error
endif


############################################################################
#
# Get the core stuff worked out
#

LINUXDIR = $(CONFIG_LINUXDIR)
LIBCDIR  = $(CONFIG_LIBCDIR)
ROOTDIR  = $(shell pwd)
PATH	 := $(PATH):$(ROOTDIR)/tools
HOSTCC   = unset GCC_EXEC_PREFIX; cc
IMAGEDIR = $(ROOTDIR)/images
ROMFSDIR = $(ROOTDIR)/romfs
ROMFSINST= romfs-inst.sh
SCRIPTSDIR = $(ROOTDIR)/config/scripts
TFTPDIR    = /tftpboot

LINUX_CONFIG  = $(ROOTDIR)/$(LINUXDIR)/.config
CONFIG_CONFIG = $(ROOTDIR)/config/.config
MODULES_CONFIG = $(ROOTDIR)/modules/.config

CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

ifeq (config.arch,$(wildcard config.arch))
include config.arch
ARCH_CONFIG = $(ROOTDIR)/config.arch
export ARCH_CONFIG
endif

ifneq ($(SUBARCH),)
# Using UML, so make the kernel and non-kernel with different ARCHs
MAKEARCH = $(MAKE) ARCH=$(SUBARCH) CROSS_COMPILE=$(CROSS_COMPILE)
MAKEARCH_KERNEL = $(MAKE) ARCH=$(ARCH) SUBARCH=$(SUBARCH) CROSS_COMPILE=$(CROSS_COMPILE)
else
MAKEARCH = $(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
MAKEARCH_KERNEL = $(MAKEARCH)
endif


DIRS    = $(VENDOR_TOPDIRS) lib user
VENDDIR = $(ROOTDIR)/vendors/$(CONFIG_VENDOR)/$(CONFIG_PRODUCT)/.

export VENDOR PRODUCT ROOTDIR LINUXDIR HOSTCC CONFIG_SHELL
export CONFIG_CONFIG LINUX_CONFIG ROMFSDIR SCRIPTSDIR
export VERSIONPKG VERSIONSTR ROMFSINST PATH IMAGEDIR TFTPDIR

############################################################################

#
# Config stuff,  we recall ourselves to load the new config.arch before
# running the kernel and other config scripts
#

.PHONY: config.tk config.in

config.in:
	config/mkconfig > config.in

config.tk: config.in
	$(MAKE) -C $(SCRIPTSDIR) tkparse
	ARCH=dummy $(SCRIPTSDIR)/tkparse < config.in > config.tmp
	@if [ -f /usr/local/bin/wish ];	then \
		echo '#!'"/usr/local/bin/wish -f" > config.tk; \
	else \
		echo '#!'"/usr/bin/wish -f" > config.tk; \
	fi
	cat $(SCRIPTSDIR)/header.tk >> ./config.tk
	cat config.tmp >> config.tk
	rm -f config.tmp
	echo "set defaults \"/dev/null\"" >> config.tk
	echo "set help_file \"config/Configure.help\"" >> config.tk
	cat $(SCRIPTSDIR)/tail.tk >> config.tk
	chmod 755 config.tk

.PHONY: xconfig
xconfig: config.tk
	@wish -f config.tk
	@if [ ! -f .config ]; then \
		echo; \
		echo "You have not saved your config, please re-run make config"; \
		echo; \
		exit 1; \
	 fi
	@config/setconfig defaults
	@if egrep "^CONFIG_DEFAULTS_KERNEL=y" .config > /dev/null; then \
		$(MAKE) linux_xconfig; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_MODULES=y" .config > /dev/null; then \
		$(MAKE) modules_xconfig; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_VENDOR=y" .config > /dev/null; then \
		$(MAKE) config_xconfig; \
	 fi
	@config/setconfig final

.PHONY: config
config: config.in
	@HELP_FILE=config/Configure.help \
		$(CONFIG_SHELL) $(SCRIPTSDIR)/Configure config.in
	@config/setconfig defaults
	@if egrep "^CONFIG_DEFAULTS_KERNEL=y" .config > /dev/null; then \
		$(MAKE) linux_config; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_MODULES=y" .config > /dev/null; then \
		$(MAKE) modules_config; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_VENDOR=y" .config > /dev/null; then \
		$(MAKE) config_config; \
	 fi
	@config/setconfig final

.PHONY: menuconfig
menuconfig: config.in
	$(MAKE) -C $(SCRIPTSDIR)/lxdialog all
	@HELP_FILE=config/Configure.help \
		$(CONFIG_SHELL) $(SCRIPTSDIR)/Menuconfig config.in
	@if [ ! -f .config ]; then \
		echo; \
		echo "You have not saved your config, please re-run make config"; \
		echo; \
		exit 1; \
	 fi
	@config/setconfig defaults
	@if egrep "^CONFIG_DEFAULTS_KERNEL=y" .config > /dev/null; then \
		$(MAKE) linux_menuconfig; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_MODULES=y" .config > /dev/null; then \
		$(MAKE) modules_menuconfig; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_VENDOR=y" .config > /dev/null; then \
		$(MAKE) config_menuconfig; \
	 fi
	@config/setconfig final

.PHONY: oldconfig
oldconfig:
	@$(MAKE) oldconfig_linux
	@$(MAKE) oldconfig_modules
	@$(MAKE) oldconfig_config
	@config/setconfig final

.PHONY: modules
modules:
	. $(LINUXDIR)/.config; if [ "$$CONFIG_MODULES" = "y" ]; then \
		[ -d $(LINUXDIR)/modules ] || mkdir $(LINUXDIR)/modules; \
		$(MAKEARCH_KERNEL) -C $(LINUXDIR) modules; \
	fi

.PHONY: modules_install
modules_install:
	. $(LINUXDIR)/.config; if [ "$$CONFIG_MODULES" = "y" ]; then \
		[ -d $(ROMFSDIR)/lib/modules ] || mkdir -p $(ROMFSDIR)/lib/modules; \
		$(MAKEARCH_KERNEL) -C $(LINUXDIR) INSTALL_MOD_PATH=$(ROMFSDIR) DEPMOD=true modules_install; \
		rm -f $(ROMFSDIR)/lib/modules/*/build; \
		find $(ROMFSDIR)/lib/modules -type f | xargs -r $(STRIP) -g; \
	fi

linux_xconfig:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) xconfig
linux_menuconfig:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) menuconfig
linux_config:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) config
modules_xconfig:
	[ ! -d modules ] || $(MAKEARCH) -C modules xconfig
modules_menuconfig:
	[ ! -d modules ] || $(MAKEARCH) -C modules menuconfig
modules_config:
	[ ! -d modules ] || $(MAKEARCH) -C modules config
modules_clean:
	-[ ! -d modules ] || $(MAKEARCH) -C modules clean
config_xconfig:
	$(MAKEARCH) -C config xconfig
config_menuconfig:
	$(MAKEARCH) -C config menuconfig
config_config:
	$(MAKEARCH) -C config config
oldconfig_config:
	$(MAKEARCH) -C config oldconfig
oldconfig_modules:
	[ ! -d modules ] || $(MAKEARCH) -C modules oldconfig
oldconfig_linux:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) oldconfig

	
.PHONY: uc26_combo uc26_1pwifi uc26_4pwifi uc26_4pwifiotg uc26_iad
uc26_combo:
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/combo/config .config
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/combo/config-kernel26 linux-2.6.x/.config
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/combo/config-user config/.config

uc26_1pwifi:
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/combo/config .config
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/1Pwifi/config_kernel linux-2.6.x/.config
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/1Pwifi/config_user config/.config

uc26_4pwifi:
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/combo/config .config
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/4Pwifi/config_kernel linux-2.6.x/.config
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/4Pwifi/config_user config/.config

uc26_4pwifiotg:
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/combo/config .config
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/4PwifiOTGhost/config_kernel linux-2.6.x/.config
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/4PwifiOTGhost/config_user config/.config

uc26_4pwifi_11n:
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/combo/config .config
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/4Pwifi_11n/config_kernel linux-2.6.x/.config
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/4Pwifi_11n/config_user config/.config

uc26_iad:
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/combo/config .config
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/voip-iad/config-kernel2.6 linux-2.6.x/.config
	cp $(ROOTDIR)/vendors/Realtek/RTL8670/voip-iad/config-user config/.config

	
############################################################################
#
# normal make targets
#

.PHONY: romfs
romfs:
	for dir in $(DIRS) ; do $(MAKEARCH) -C $$dir romfs || exit 1 ; done
	-find $(ROMFSDIR)/. -name CVS | xargs -r rm -rf
	date +%m%d%H%M%Y > $(ROMFSDIR)/etc/compile-date

.PHONY: dev
dev:
	$(MAKEARCH) -C user dev

.PHONY: image
include $(LINUXDIR)/.config
include config/.config
image:
ifdef CONFIG_ROMFS_FS
	[ -d $(IMAGEDIR) ] || mkdir $(IMAGEDIR)
	$(MAKEARCH) -C $(VENDDIR) image
endif
ifdef CONFIG_CRAMFS
	[ -d $(IMAGEDIR) ] || mkdir $(IMAGEDIR)
	./mkcramfs $(ROMFSDIR) rootfs
endif
ifdef CONFIG_SQUASHFS
	[ -d $(IMAGEDIR) ] || mkdir $(IMAGEDIR)
	./copy_and_strip.sh
	./mksquashfs-lzma $(ROMFSDIR) rootfs -be -noappend
ifeq ($(CONFIG_DEFAULT_WEB),y)
	@printf "Now the image you are compiling is  for:\33[0;31mdefault\n\33[0m"
endif
endif
#
# fancy target that allows a vendor to have other top level
# make targets,  for example "make vendor_flash" will run the
# vendor_flash target in the vendors directory
#

vendor_%:
	$(MAKEARCH) -C $(VENDDIR) $@

.PHONY: linux
linux linux%_only:
	@if [ $(LINUXDIR) != linux-2.5.x -a $(LINUXDIR) != linux-2.6.x -a ! -f $(LINUXDIR)/.depend ] ; then \
		echo "ERROR: you need to do a 'make dep' first" ; \
		exit 1 ; \
	fi
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) $(LINUXTARGET) || exit 1
	if [ -f $(LINUXDIR)/vmlinux ]; then \
		ln -f $(LINUXDIR)/vmlinux $(LINUXDIR)/linux ; \
	fi

.PHONY: subdirs
subdirs: linux
	for dir in $(DIRS) ; do $(MAKEARCH_KERNEL) -C $$dir || exit 1 ; done

dep:
	@if [ ! -f $(LINUXDIR)/.config ] ; then \
		echo "ERROR: you need to do a 'make config' first" ; \
		exit 1 ; \
	fi
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) dep

# This one removes all executables from the tree and forces their relinking
.PHONY: relink
relink:
	find user -name '*.gdb' | sed 's/^\(.*\)\.gdb/\1 \1.gdb/' | xargs rm -f
	find $(VENDDIR) -name '*.gdb' | sed 's/^\(.*\)\.gdb/\1 \1.gdb/' | xargs rm -f

# This one build a rom image including boot code and run-time image
.PHONY: rom vmimg
rom:
	@if [ ! -f dummy.S ] ; then \
		touch dummy.S; \
	fi
	$(CC) $(CFLAG) -c dummy.S -o dummy.o
	$(OBJCOPY) --add-section .boot=boot.bin dummy.o
	$(OBJCOPY) --add-section .pad=pad_file dummy.o
	$(OBJCOPY) --add-section .run=linux-2.4.x/vmbin.img dummy.o
	$(OBJCOPY) --remove-section=.text --remove-section=.bss --remove-section=.reginfo --remove-section=.pdr dummy.o
	$(LD) -nostdlib -Trom.script dummy.o -o rom.elf
	$(OBJCOPY) -Obinary rom.elf rom.img

vmimg:
	$(MAKEARCH) -C $(LINUXDIR) vmimg

dep26:
	find ./user/boa/ -type l | xargs rm
	rm user/boa/default_web_id

clean: modules_clean
	#for dir in $(LINUXDIR) $(DIRS); do $(MAKEARCH) -C $$dir clean ; done
	for dir in  $(DIRS); do $(MAKEARCH) -C $$dir clean ; done
	rm -rf $(ROMFSDIR)/*
	rm -rf $(IMAGEDIR)/*
	rm -f config.tk
	#rm -f $(LINUXDIR)/linux
	rm -rf $(LINUXDIR)/net/ipsec/alg/libaes $(LINUXDIR)/net/ipsec/alg/perlasm

real_clean mrproper: clean
	-$(MAKEARCH_KERNEL) -C $(LINUXDIR) mrproper
	-$(MAKEARCH) -C config clean
	rm -rf romfs config.in config.arch config.tk images
	rm -f modules/config.tk
	rm -rf .config .config.old .oldconfig autoconf.h

distclean: mrproper
	-$(MAKEARCH_KERNEL) -C $(LINUXDIR) distclean

%_only:
	@if [ '$(@:user/%_only=)' ] ; then \
		[ ! -d "$(@:_only=)" ] || $(MAKEARCH) -C $(@:_only=); \
	else \
		$(MAKEARCH) -C user $(@:user/%=%); \
	fi

%_romfs:
	@if [ '$(@:user/%_romfs=)' ] ; then \
		[ ! -d "$(@:_romfs=)" ] || $(MAKEARCH) -C $(@:_romfs=) romfs; \
		find $(ROMFSDIR)/. -name CVS | xargs -r rm -rf; \
	else \
		$(MAKEARCH) -C user $(@:user/%=%); \
	fi


%_clean:
	[ ! -d "$(@:_clean=)" ] || $(MAKEARCH) -C $(@:_clean=) clean

config_error:
	@echo "*************************************************"
	@echo "You have not run make config."
	@echo "The build sequence for this source tree is:"
	@echo "1. 'make config' or 'make xconfig'"
	@echo "2. 'make dep'"
	@echo "3. 'make'"
	@echo "*************************************************"
	@exit 1

prune:
	$(MAKE) -C user prune

dist-prep:
	-find $(ROOTDIR) -name 'Makefile*.bin' | while read t; do \
		$(MAKEARCH) -C `dirname $$t` -f `basename $$t` $@; \
	 done

############################################################################

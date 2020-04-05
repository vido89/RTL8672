#
# Realtek Semiconductor Corp.
#
# RSDK Toolchain Top-Level Makefile for uclibc
# Adopted from buildroot makefiles
#
# Tony Wu (tonywu@realtek.com.tw)
# Oct. 10, 2005
#

include builder.cfg

#
# CFLAGS/CXXFLAGS
#
ifeq ($(origin CFLAGS), undefined)
    CFLAGS := -g
endif
                                                                                                    
ifeq ($(origin CXXFLAGS), undefined)
    CXXFLAGS := -g
endif
                                                                                                    
export CFLAGS
export CXXFLAGS

LIBC_NAME := uclibc
LIBC_ARCH := linux

HOST_ARCH := i386
ifeq ($(origin HOST_TYPE), undefined)
	HOST_TYPE := $(shell uname | sed -e 's,linux.*,linux,i' | sed -e 's,cygwin.*,cygwin,i')
	export HOST_TYPE
endif

BUILD_ARCH := i386
TARGET_ARCH := mips

HOST_NAME := i386-pc-$(HOST_TYPE)
BUILD_NAME := i386-pc-$(HOST_TYPE)
TARGET_NAME = $(TARGET_ARCH)-$(LIBC_ARCH)

BUILD_DIR = $(INST_DIR)/$(HOST_TYPE)/$(LIBC_NAME)
STAGE_DIR = $(SUMP_DIR)/$(LIBC_NAME)
TARGET_DIR = $(BUILD_DIR)/root

TARGET_PATH = $(BUILD_DIR)/bin:$(ENV_PATH)
KERNEL_CROSS = $(BUILD_DIR)/bin/$(TARGET_ARCH)-$(LIBC_ARCH)-
TARGET_CROSS = $(BUILD_DIR)/bin/$(TARGET_ARCH)-$(LIBC_ARCH)-

HOST_CC = gcc
TARGET_CC =$(TARGET_CROSS)gcc
STRIP=$(TARGET_CROSS)strip --remove-section=.comment --remove-section=.note

TARGET_CONFIGURE_OPTS=PATH=$(TARGET_PATH) \
		AR=$(TARGET_CROSS)ar \
		AS=$(TARGET_CROSS)as \
		LD=$(TARGET_CROSS)ld \
		NM=$(TARGET_CROSS)nm \
		CC=$(TARGET_CROSS)gcc \
		GCC=$(TARGET_CROSS)gcc \
		CXX=$(TARGET_CROSS)g++ \
		RANLIB=$(TARGET_CROSS)ranlib

TARGET_CFLAGS=$(TARGET_OPTIMIZATION) $(TARGET_DEBUGGING)

CROSS_COMPILER_PREFIX = $(BUILD_DIR)/bin/mips-linux-

#--------------------------------------------------------------------------
# Tool configuration
#--------------------------------------------------------------------------
MAKE = make

#--------------------------------------------------------------------------
# The list of stuff to build for the target toolchain
# along with the packages to build for the target.
#--------------------------------------------------------------------------
TARGETS := binutils uclibc-configured gcc insight

#-----------------------------------------------------------------------------
# target and default definition
#-----------------------------------------------------------------------------
INIT_FLAG = $(shell (test -e $(BUILD_DIR)/.ulib-make-init && echo -n 'yes'))

.PHONY: all clean distclean pre_build pos_build $(TARGETS)

all: pre_build $(TARGETS) pos_build

#############################################################
#
# staging and target directories do NOT list these as
# dependancies anywhere else
#
#############################################################
pre_build:
ifneq ($(INIT_FLAG), 'yes')
	mkdir -p $(STAGE_DIR)
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/lib
	mkdir -p $(BUILD_DIR)/include
	mkdir -p $(BUILD_DIR)/$(TARGET_NAME)
	ln -sf ../lib $(BUILD_DIR)/$(TARGET_NAME)/lib
	touch $(BUILD_DIR)/.ulib-make-init
endif

pos_build:
	(cd $(BUILD_DIR)/bin;                           \
        ln -sf mips-linux-addr2line rsdk-elf-addr2line;  \
        ln -sf mips-linux-ar        rsdk-elf-ar;         \
        ln -sf mips-linux-as        rsdk-elf-as;         \
        ln -sf mips-linux-c++filt   rsdk-elf-c++filt;    \
        ln -sf mips-linux-cpp       rsdk-elf-cpp;        \
        ln -sf mips-linux-gcc       rsdk-elf-gcc;        \
		ln -sf mips-linux-c++       rsdk-elf-c++;        \
		ln -sf mips-linux-g++       rsdk-elf-g++;        \
        ln -sf mips-linux-gccbug    rsdk-elf-gccbug;     \
        ln -sf mips-linux-gcov      rsdk-elf-gcov;       \
        ln -sf mips-linux-ld        rsdk-elf-ld;         \
        ln -sf mips-linux-nm        rsdk-elf-nm;         \
        ln -sf mips-linux-objcopy   rsdk-elf-objcopy;    \
        ln -sf mips-linux-objdump   rsdk-elf-objdump;    \
        ln -sf mips-linux-ranlib    rsdk-elf-ranlib;     \
        ln -sf mips-linux-readelf   rsdk-elf-readelf;    \
        ln -sf mips-linux-size      rsdk-elf-size;       \
        ln -sf mips-linux-strings   rsdk-elf-strings;    \
		ln -sf mips-linux-gdb       rsdk-elf-gdb;        \
		ln -sf mips-linux-insight   rsdk-elf-insight;    \
        ln -sf mips-linux-strip     rsdk-elf-strip);
	touch $(BUILD_DIR)/.ulib-make-done


#############################################################
#
# Cleanup and misc junk
#
#############################################################
clean:
	rm -rf $(BUILD_DIR) $(TARGET_DIR) $(STAGE_DIR)
	rm -f $(BASE_DIR)/linux

dirclean: $(TARGETS_DIRCLEAN)
	rm -rf $(BUILD_DIR) $(TARGET_DIR)

distclean:
	rm -rf $(BUILD_DIR) $(LINUX_KERNEL)


include uclibc-kernel-2.4.31.mk
include uclibc-binutils-2.x.mk
include uclibc-uclibc-0.9.28.mk
include uclibc-insight-6.x.mk
include uclibc-gcc-3.x.mk

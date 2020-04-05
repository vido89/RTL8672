# Rules.make for uClibc
#
# Copyright (C) 2000 by Lineo, inc.
# Copyright (C) 2000-2002 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


#-----------------------------------------------------------
# This file contains rules which are shared between multiple
# Makefiles.  All normal configuration options live in the 
# file named ".config".  Don't mess with this file unless 
# you know what you are doing.


#-----------------------------------------------------------
# If you are running a cross compiler, you will want to set 
# 'CROSS' to something more interesting ...  Target 
# architecture is determined by asking the CC compiler what 
# arch it compiles things for, so unless your compiler is 
# broken, you should not need to specify TARGET_ARCH.
#
# Most people will set this stuff on the command line, i.e.
#        make CROSS=arm-linux-
# will build uClibc for 'arm'.

ifndef CROSS
CROSS= $(TARGET_CROSS)
endif
CC         = $(CROSS)gcc
AR         = $(CROSS)ar
LD         = $(CROSS)ld
NM         = $(CROSS)nm
RANLIB     = $(CROSS)ranlib
STRIPTOOL  = $(CROSS)strip

INSTALL    = install
LN         = ln
RM         = rm -f

# Select the compiler needed to build binaries for your development system
HOSTCC     = gcc
HOSTCFLAGS = -O2 -Wall


#---------------------------------------------------------
# Nothing beyond this point should ever be touched by mere
# mortals.  Unless you hang out with the gods, you should
# probably leave all this stuff alone.
MAJOR_VERSION := 0
MINOR_VERSION := 9
SUBLEVEL      := 28
VERSION       := $(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL)
# Ensure consistent sort order, 'gcc -print-search-dirs' behavior, etc.
LC_ALL := C
export MAJOR_VERSION MINOR_VERSION SUBLEVEL VERSION LC_ALL

SHARED_FULLNAME:=libuClibc-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so
SHARED_MAJORNAME:=libc.so.$(MAJOR_VERSION)
UCLIBC_LDSO:=ld-uClibc.so.$(MAJOR_VERSION)
LIBNAME:=libc.a
LIBC:=$(TOPDIR)libc/$(LIBNAME)

# Make sure DESTDIR and PREFIX can be used to install
# PREFIX is a uClibcism while DESTDIR is a common GNUism
ifndef PREFIX
PREFIX = $(DESTDIR)
endif

# Pull in the user's uClibc configuration
ifeq ($(filter $(noconfig_targets),$(MAKECMDGOALS)),)
-include $(TOPDIR).config
endif

ifndef CROSS
CROSS=$(subst ",, $(strip $(CROSS_COMPILER_PREFIX)))
endif

# A nifty macro to make testing gcc features easier
check_gcc=$(shell \
	if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; \
	then echo "$(1)"; else echo "$(2)"; fi)
check_as=$(shell \
	if $(CC) -Wa,$(1) -Wa,-Z -c -o /dev/null -xassembler /dev/null > /dev/null 2>&1; \
	then echo "-Wa,$(1)"; fi)

# Setup some shortcuts so that silent mode is silent like it should be
ifeq ($(subst s,,$(MAKEFLAGS)),$(MAKEFLAGS))
export MAKE_IS_SILENT=n
SECHO=@echo
SHELL_SET_X=set -x
else
export MAKE_IS_SILENT=y
SECHO=-@false
SHELL_SET_X=set +x
endif

# Make certain these contain a final "/", but no "//"s.
TARGET_ARCH:=$(shell grep -s ^TARGET_ARCH $(TOPDIR)/.config | sed -e 's/^TARGET_ARCH=//' -e 's/"//g')
RUNTIME_PREFIX:=$(strip $(subst //,/, $(subst ,/, $(subst ",, $(strip $(RUNTIME_PREFIX))))))
DEVEL_PREFIX:=$(strip $(subst //,/, $(subst ,/, $(subst ",, $(strip $(DEVEL_PREFIX))))))
export RUNTIME_PREFIX DEVEL_PREFIX

ARFLAGS:=cr

OPTIMIZATION:=
PICFLAG:=-fPIC
PIEFLAG_NAME:=-fPIE

# Keep the check_gcc from being needlessly executed
ifndef PIEFLAG
ifneq ($(UCLIBC_BUILD_PIE),y)
export PIEFLAG:=
else
export PIEFLAG:=$(call check_gcc,$(PIEFLAG_NAME),$(PICFLAG))
endif
endif
# We need to keep track of both the CC PIE flag (above) as 
# well as the LD PIE flag (below) because we can't rely on 
# gcc passing -pie if we used -fPIE
ifndef LDPIEFLAG
ifneq ($(UCLIBC_BUILD_PIE),y)
export LDPIEFLAG:=
else
export LDPIEFLAG:=$(shell $(LD) --help | grep -q pie && echo "-Wl,-pie")
endif
endif

# Use '-Os' optimization if available, else use -O2, allow Config to override
OPTIMIZATION+=$(call check_gcc,-Os,-O2)
# Use the gcc 3.4 -funit-at-a-time optimization when available
OPTIMIZATION+=$(call check_gcc,-funit-at-a-time,)

# Add a bunch of extra pedantic annoyingly strict checks
XWARNINGS=$(subst ",, $(strip $(WARNINGS))) -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing
XARCH_CFLAGS=$(subst ",, $(strip $(ARCH_CFLAGS)))
CPU_CFLAGS=$(subst ",, $(strip $(CPU_CFLAGS-y)))

LDADD_LIBFLOAT=
ifeq ($(strip $(UCLIBC_HAS_SOFT_FLOAT)),y)
# Add -msoft-float to the CPU_FLAGS since ldso and libdl ignore CFLAGS.
# If -msoft-float isn't supported, we want an error anyway.
# Hmm... might need to revisit this for arm since it has 2 different
# soft float encodings.
    CPU_CFLAGS += -msoft-float
ifeq ($(strip $(TARGET_ARCH)),arm)
# No longer needed with current toolchains, but leave it here for now.
# If anyone is actually still using gcc 2.95 (say), they can uncomment it.
#    LDADD_LIBFLOAT=-lfloat
endif
endif

SSP_DISABLE_FLAGS:=$(call check_gcc,-fno-stack-protector,)
ifeq ($(UCLIBC_BUILD_SSP),y)
SSP_CFLAGS:=$(call check_gcc,-fno-stack-protector-all,)
SSP_CFLAGS+=$(call check_gcc,-fstack-protector,)
SSP_ALL_CFLAGS:=$(call check_gcc,-fstack-protector-all,)
else
SSP_CFLAGS:=$(SSP_DISABLE_FLAGS)
endif

ifeq ($(strip $(TARGET_ARCH)),mips)
    CPU_LDFLAGS-$(ARCH_LITTLE_ENDIAN)+=-EL
    CPU_LDFLAGS-$(ARCH_BIG_ENDIAN)+=-EB
endif

# Some nice CFLAGS to work with
CFLAGS:=$(XWARNINGS) $(CPU_CFLAGS) $(SSP_CFLAGS) \
	-fno-builtin -nostdinc -D_LIBC -I$(TOPDIR)include -I.
LDFLAGS_NOSTRIP:=$(CPU_LDFLAGS-y) -shared --warn-common --warn-once -z combreloc -z defs

ifeq ($(DODEBUG),y)
    #CFLAGS += -g3
    CFLAGS += -O0 -g3 $(XARCH_CFLAGS)
    LDFLAGS := $(LDFLAGS_NOSTRIP)
    STRIPTOOL:= true -Since_we_are_debugging
else
    CFLAGS += $(OPTIMIZATION) $(XARCH_CFLAGS)
    LDFLAGS := $(LDFLAGS_NOSTRIP) -s
endif

ifeq ($(UCLIBC_BUILD_RELRO),y)
LDFLAGS+=-z relro
endif

ifeq ($(UCLIBC_BUILD_NOW),y)
LDFLAGS+=-z now
endif

# Sigh, some stupid versions of gcc can't seem to cope with '-iwithprefix include'
#CFLAGS+=-iwithprefix include
CFLAGS+=-isystem $(shell $(CC) -print-file-name=include)

ifneq ($(DOASSERTS),y)
    CFLAGS += -DNDEBUG
endif

CFLAGS_NOPIC:=$(CFLAGS)
ifeq ($(DOPIC),y)
    CFLAGS += $(PICFLAG)
endif

ifeq ($(DL_FINI_CRT_COMPAT),y)
CFLAGS += -D_DL_FINI_CRT_COMPAT
endif

# Keep the check_as from being needlessly executed
ASFLAGS = $(CFLAGS)
ifndef ASFLAGS_NOEXEC
ifeq ($(UCLIBC_BUILD_NOEXECSTACK),y)
export ASFLAGS_NOEXEC := $(call check_as,--noexecstack)
else
export ASFLAGS_NOEXEC :=
endif
endif
ASFLAGS += $(ASFLAGS_NOEXEC)

LIBGCC_CFLAGS ?= $(CFLAGS) $(CPU_CFLAGS-y)
LIBGCC:=$(shell $(CC) $(LIBGCC_CFLAGS) -print-libgcc-file-name)
LIBGCC_DIR:=$(dir $(LIBGCC))

########################################
#
# uClinux shared lib support
#

ifeq ($(CONFIG_BINFMT_SHARED_FLAT),y)
  # For the shared version of this, we specify no stack and its library ID
  FLTFLAGS += -s 0
  LIBID=1
  export LIBID FLTFLAGS
  SHARED_TARGET = lib/libc
endif

TARGET_ARCH:=$(strip $(subst ",, $(strip $(TARGET_ARCH))))

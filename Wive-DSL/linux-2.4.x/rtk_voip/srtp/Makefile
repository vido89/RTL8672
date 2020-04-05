# Makefile for secure rtp 
#
# David A. McGrew
# Cisco Systems, Inc.

# targets:
#
# runtest       runs test applications 
# test		builds test applications
# libcrypt.a	static library implementing crypto engine
# libsrtp.a	static library implementing srtp
# clean		removes objects, libs, and executables
# distribution  cleans and builds a .tgz
# tags          builds etags file from all .c and .h files


O_TARGET := srtp.o
subdir-y :=
obj-y := 
export-objs =

# gdoi is the group domain of interpretation for isakmp, a group key
# management system which can provide keys for srtp
gdoi	= 
# Random source.
RNG_OBJS = rand_source.o

# libcrypt.a (the crypto engine) 
ciphers = crypto/cipher/cipher.o crypto/cipher/null_cipher.o      \
          crypto/cipher/aes.o crypto/cipher/aes_icm.o             \
          crypto/cipher/aes_cbc.o

hashes  = crypto/hash/null_auth.o crypto/hash/sha1.o \
          crypto/hash/hmac.o crypto/hash/auth.o # crypto/hash/tmmhv2.o 

replay  = crypto/replay/rdb.o crypto/replay/rdbx.o               \
          #crypto/replay/ut_sim.o 

math    = crypto/math/datatypes.o #crypto/math/stat.o

#ust     = crypto/ust/ust.o 

rng     = crypto/rng/$(RNG_OBJS) crypto/rng/prng.o crypto/rng/ctr_prng.o

err     = crypto/kernel/err.o

kernel  = crypto/kernel/crypto_kernel.o  crypto/kernel/alloc.o   \
          crypto/kernel/key.o $(rng) $(err) # $(ust) 

cryptobj =  $(ciphers) $(hashes) $(math) $(stat) $(kernel) $(replay)

# libsrtp.a (implements srtp processing)

srtpobj = srtp/srtp.o 

obj-y += $(srtpobj) $(cryptobj) $(gdoi)
subdir-y += 


EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I./include
EXTRA_CFLAGS += -I../include
EXTRA_CFLAGS += -I./crypto/include
EXTRA_CFLAGS += -I../voip_dsp/cp3
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8186),y)
EXTRA_CFLAGS += -I../../net/ipsec
endif
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651),y)
EXTRA_CFLAGS += -I../../drivers/net/re865x/crypto/865xb
endif

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make


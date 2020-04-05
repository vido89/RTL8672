
.S.s:
		$(CPP) $(AFLAGS) $< -o $@
.S.o:
ifeq ($(CONFIG_RTL865XC),y)
		$(CC) $(AFLAGS) -march=5181 -c $< -o $@
else
		$(CC) $(AFLAGS) -march=5280 -c $< -o $@
endif

O_TARGET := dsp_r1.o
subdir-y :=
obj-y    :=
			
#CONFIG_VOIP_SDK=y

ifeq ($(CONFIG_VOIP_SDK),y)
obj-y += r1_main.obj
obj-y += dtmf.obj
obj-y += aes.obj
obj-y += table.obj
obj-y += jbc.obj 	

obj-y    += g7111.obj
obj-y    += common/common.obj
obj-y    += wsola.obj

obj-y += r1_helpG711.obj

else

obj-y += r1_main.o
obj-y += dtmf.o
obj-y += aes.o
obj-y += table.o
obj-y += jbc.o 	

subdir-y += g711
obj-y    += g711/g7111.o

subdir-y += common
obj-y    += common/common.o

subdir-y += wsola
obj-y    += wsola/wsola.o

subdir-y += help
obj-y    += help/r1_help.o

endif

obj-y += common/util/codec_mem_api.o

# For SDK not including g729/7231/726

ifeq ($(CONFIG_RTK_VOIP_G7231),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += g723.obj
obj-y    += r1_helpG723.obj
else 
subdir-y += g723
obj-y    += g723/g723.o
endif
endif

ifeq ($(CONFIG_RTK_VOIP_G729AB),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += g729.obj
obj-y    += r1_helpG729.obj
else 
subdir-y += g729
obj-y    += g729/g729.o
endif
endif

ifeq ($(CONFIG_RTK_VOIP_G726),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += g726.obj
obj-y    += r1_helpG726.obj
else 
subdir-y += g726
obj-y    += g726/g726.o
endif
endif

ifeq ($(CONFIG_RTK_VOIP_GSMFR),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += gsm_fr.obj
obj-y    += r1_helpGSMfr.obj
else 
subdir-y += gsm_fr
obj-y    += gsm_fr/gsm_fr.o
endif
endif

ifeq ($(CONFIG_RTK_VOIP_T38),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += t38.obj
obj-y    += r1_helpT38.obj
else 
subdir-y += t38
obj-y    += t38/t38.o
endif
endif

# tone_.o is not used now!
#obj-y += tone_.o

###### RULES

#.PHONY: all clean

#all: r1_main

#clean:
#	-rm -f r1_main.exe r1_main.bin *.o
#	-rm -f *.o

EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS +=  -I../../include
EXTRA_CFLAGS +=  -I../include
EXTRA_CFLAGS +=  -I./include
ifeq ($(CONFIG_VOIP_SDK),y)
else
EXTRA_CFLAGS +=  -I./common/basic_op
EXTRA_CFLAGS +=  -I./common/util
endif
EXTRA_CFLAGS +=	-I../cp3

#EXTRA_CFLAGS+=-DFEATURE_COP3_PROFILE
#EXTRA_CFLAGS+=-DFEATURE_COP3_PLAYTONE
#EXTRA_CFLAGS+=-DFEATURE_COP3_G711DECPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_G729DECPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_G7231DECPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_G726DECPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_GSMFRDECPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_T38DECPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_PRE_PROCESS
#EXTRA_CFLAGS+=-DFEATURE_COP3_AES
#EXTRA_CFLAGS+=-DFEATURE_COP3_G711ENCPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_G729ENCPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_G7231ENCPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_G726ENCPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_GSMFRENCPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_T38ENCPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_G729JBC
#EXTRA_CFLAGS+=-DFEATURE_COP3_G729DEC

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make

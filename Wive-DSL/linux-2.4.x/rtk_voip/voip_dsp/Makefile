#
# Makefile for the DSP and PCM driver
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...


O_TARGET := voip_dsp.o
subdir-y :=
obj-y := 
export-objs =
dsp-mod-objs :=
#CONFIG_VOIP_SDK=y

ifeq ($(CONFIG_VOIP_SDK),y)

dsp-mod-objs += dsp_main.obj
dsp-mod-objs += pcm_handler.obj
#obj-y += lec.obj
dsp-mod-objs += lec_g168.obj
dsp-mod-objs += dtmf.obj
dsp-mod-objs += ced_det.obj
dsp-mod-objs += voice_gain.obj
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
dsp-mod-objs += aec_g168.obj
#dsp-mod-objs += nr.obj
endif

ifeq ($(CONFIG_RTK_SLIC_DAA_NEGOTIATION),y)
dsp-mod-objs += tone_detection.obj
dsp-mod-objs += fsk_det.obj
endif

else


dsp-mod-objs += dsp_main.o
dsp-mod-objs += pcm_handler.o
#obj-y += lec/lec.obj
dsp-mod-objs += dtmf_dec/dtmf.o
subdir-y += dtmf_dec
dsp-mod-objs += ced/ced_det.o
subdir-y += ced
dsp-mod-objs += lec/lec_g168.o
subdir-y += lec
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
subdir-y += aec
dsp-mod-objs += aec/aec_g168.o
#subdir-y += nr
#dsp-mod-objs += nr/nr.o
endif

subdir-y += tone_det
subdir-y += fsk_det
subdir-y += voice_gain
dsp-mod-objs += voice_gain/voice_gain.o
ifeq ($(CONFIG_RTK_SLIC_DAA_NEGOTIATION),y)
dsp-mod-objs += tone_det/tone_detection.o
dsp-mod-objs += fsk_det/fsk_det.o
endif

endif

ifeq ($(CONFIG_RTK_VOIP_IVR),y)
subdir-y += ivr
dsp-mod-objs += ivr/ivr_sup.o
endif

dsp-mod-objs += codec_descriptor.o rtp/rtp.o dsp_r1/dsp_r1.o dsp_r0/dsp_r0.o cp3/cp3.o

subdir-y += rtp
subdir-y += dsp_r1
subdir-y += dsp_r0
subdir-y += cp3

dsp-mod-objs2  := dsp_main.o pcm_handler.o dtmf_dec/dtmf.o ced/ced_det.o lec/lec_g168.o  \
				  codec_descriptor.o rtp/rtp.o dsp_r1/dsp_r1.o dsp_r0/dsp_r0.o cp3/cp3.o \
                  tone_det/tone_detection.o fsk_det/fsk_det.o voice_gain/voice_gain.o

ifeq ($(CONFIG_RTK_VOIP_IVR),y)
dsp-mod-objs2 += ivr/ivr.o
dsp-mod-objs2 += ivr/ivr_speech.o
endif

obj-y += dsp_mod.o

EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I./include
EXTRA_CFLAGS += -I../include
EXTRA_CFLAGS += -I./dsp_r1/include
EXTRA_CFLAGS += -I./cp3

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651),y)
EXTRA_CFLAGS += -I../../drivers/net/re865x/rtl865x
endif

#EXTRA_CFLAGS += -DFEATURE_COP3_PROFILE
#EXTRA_CFLAGS += -DFEATURE_COP3_DTMFDEC
#EXTRA_CFLAGS += -DFEATURE_COP3_LEC
#EXTRA_CFLAGS += -DFEATURE_COP3_DSPPROCESS

#obj-m   := $(O_TARGET)

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make


ifeq ($(CONFIG_VOIP_SDK),y)
dsp_mod.o: $(dsp-mod-objs)
			$(LD) -r -o $@ $(dsp-mod-objs)
else
dsp_mod.o: $(dsp-mod-objs2)
			$(LD) -r -o $@ $(dsp-mod-objs)
endif



O_TARGET := rtp.o
subdir-y :=
obj-y    :=

obj-y += RtpPacket.o rtpTools.o RtpReceiver.o RtpTransmitter.o rtpterm.o rtp_main.o\
         RtcpReceiver.o RtcpTransmitter.o RtcpPacket.o NtpTime.o
#EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS +=  -I../../include
EXTRA_CFLAGS +=  -I../include
EXTRA_CFLAGS +=  -I../dsp_r1/include

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(TOPDIR)/Rules.make

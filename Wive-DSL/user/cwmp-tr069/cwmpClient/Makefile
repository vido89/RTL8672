CWMPCLIENT_ROOT=$(shell pwd)

ifeq ($(CONFIG_USER_TR143),y)
TR143_EXEC = udpechoserver
TR143_LIBOBJS = udpechoserverlib.o
TR143_LIBDEPH = udpechoserverlib.h
else
TR143_EXEC =
TR143_LIBOBJS =
TR143_LIBDEPH =
endif

EXEC = cwmpClient
OBJS =	prmt_igd.o prmt_deviceinfo.o prmt_mngmtserver.o \
	prmt_layer3fw.o prmt_landevice.o prmt_landevice_eth.o prmt_landevice_wlan.o \
	prmt_wandevice.o prmt_wandsldiagnostics.o \
	prmt_wancondevice.o prmt_ippingdiag.o \
	prmt_time.o prmt_wanatmf5loopback.o prmt_ddns.o \
	prmt_services.o prmt_userif.o prmt_ctcom.o \
	prmt_captiveportal.o prmt_traceroute.o prmt_landevice_usb.o \
	prmt_queuemnt.o prmt_tr143.o prmt_ctcom_queuemnt.o prmt_apply.o porting.o main.o
DEPH =	$(CWMPCLIENT_ROOT)/../../../config/autoconf.h \
	$(CWMPCLIENT_ROOT)/../../boa/src-project/LINUX/options.h \
	prmt_igd.h prmt_deviceinfo.h prmt_landevice_eth.h prmt_mngmtserver.h \
	prmt_layer3fw.h prmt_landevice.h prmt_landevice_wlan.h \
	prmt_wandevice.h prmt_wandsldiagnostics.h \
	prmt_wancondevice.h prmt_ippingdiag.h prmt_time.h prmt_ddns.h \
	prmt_wanatmf5loopback.h prmt_services.h prmt_userif.h prmt_ctcom.h \
	prmt_captiveportal.h prmt_traceroute.h prmt_landevice_usb.h \
	prmt_queuemnt.h prmt_ctcom_queuemnt.h prmt_apply.h

CFLAGS += -I$(CWMPCLIENT_ROOT)/../include -I$(CWMPCLIENT_ROOT)/../../boa/src-project/LINUX

# To support TR104 
ifeq ($(CONFIG_USER_RTK_VOIP),y)
CFLAGS += -I$(CWMPCLIENT_ROOT)/../../rtk_voip/tr104 -I$(CWMPCLIENT_ROOT)
endif

#set the ssl libraries
ifeq ($(CONFIG_USER_CWMP_WITH_OPENSSL),y)
LIBCWMP = ../lib/libcwmpssl.a ../lib/libgsoapssl.a
LIBCWMPFLAG = -lssl -lcrypto
endif
ifeq ($(CONFIG_USER_CWMP_WITH_MATRIXSSL),y)
LIBCWMP = ../lib/libcwmpmssl.a ../lib/libgsoapmssl.a
LIBCWMPFLAG = -lc -L$(CWMPCLIENT_ROOT)/../../../lib/libmatrixssl -lmatrixssl
endif
ifneq ($(CONFIG_USER_CWMP_WITH_SSL),y)
LIBCWMP = ../lib/libcwmp.a ../lib/libgsoapck.a
LIBCWMPFLAG =
endif

# To Support TR104
ifeq ($(CONFIG_USER_RTK_VOIP),y)
LIBCWMP += ../../rtk_voip/tr104/voip_tr104.a ../../rtk_voip/voip_manager/libvoip_manager.a
endif

#additional linking library
#-lcrypt for crypt() when creating  the /etc/passwd file
LIBCWMP += -lcrypt

all: $(EXEC) $(TR143_EXEC)

udpechoserver.o: udpechoserver.c $(TR143_LIBDEPH)
	$(CC) -Os $(CFLAGS) -c -o $@ $<

$(TR143_LIBOBJS): udpechoserverlib.c $(TR143_LIBDEPH)
	$(CC) -Os $(CFLAGS) -c -o $@ $<

$(TR143_EXEC): udpechoserver.o $(TR143_LIBOBJS)
	$(CC) -Os $(LDFLAGS) -o $@ udpechoserver.o $(TR143_LIBOBJS)

%.o: %.c $(DEPH) $(TR143_LIBDEPH)
	$(CC) -Os $(CFLAGS) -c -o $@ $<

$(EXEC): $(OBJS) $(LIBCWMP) $(TR143_LIBOBJS) $(TR143_LIBDEPH)
	$(CC) -Os $(LDFLAGS) -o $@ $(OBJS) $(LIBCWMP) $(TR143_LIBOBJS) -lmib -lpthread $(LIBCWMPFLAG)


clean:
	-rm -f $(EXEC) *.o udpechoserver


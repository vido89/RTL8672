
.EXPORT_ALL_VARIABLES:

CWMP_ROOT=$(shell pwd)
GSOAP_DIR=gsoaplib
MATRIXSSL_DIR=matrixssl-1-8-3-open
LIBOPENSSL_INC_DIR=$(CWMP_ROOT)/../../lib/libssl/include
LIBMATRIXSSL_INC_DIR=$(CWMP_ROOT)/$(MATRIXSSL_DIR)

CC += -Os

all:
#	if [ -d $(MATRIXSSL_DIR) ]; then \
#		$(MAKE) -C $(MATRIXSSL_DIR)/src; \
#		if [ ! -d lib ]; then mkdir lib; fi; \
#		cp -f $(MATRIXSSL_DIR)/src/libmatrixsslstatic.a $(CWMP_ROOT)/lib/; \
#	fi
#	$(MAKE) -C matrixssl-1-8-3-open/examples
	if [ -d $(GSOAP_DIR) ]; then \
		cd $(GSOAP_DIR); \
		if [ ! -f config.h ]; then \
			CPPFLAGS='-DLINUX -I$(LIBOPENSSL_INC_DIR) -I$(LIBMATRIXSSL_INC_DIR)' ./configure --host=mips-linux --prefix=$(CWMP_ROOT); \
		fi; \
		$(MAKE); \
		$(MAKE) install; \
	fi
# To support TR104
ifeq ($(CONFIG_USER_RTK_VOIP),y)
	$(MAKE) -C $(CWMP_ROOT)/../rtk_voip/tr104/
	$(MAKE) -C $(CWMP_ROOT)/../rtk_voip/voip_manager/
endif
	$(MAKE) -C cwmpClient

romfs:
	cp -f $(CWMP_ROOT)/cwmpClient/cwmpClient $(CWMP_ROOT)/../../romfs/bin
ifeq ($(CONFIG_USER_TR143),y)
	cp -f $(CWMP_ROOT)/cwmpClient/udpechoserver $(CWMP_ROOT)/../../romfs/bin
endif
ifeq ($(CONFIG_USER_CWMP_WITH_SSL),y)
	cp -f $(CWMP_ROOT)/cwmpClient/client.pem $(CWMP_ROOT)/../../romfs/etc
	cp -f $(CWMP_ROOT)/cwmpClient/cacert.pem $(CWMP_ROOT)/../../romfs/etc
endif

clean:
	if [ -d $(MATRIXSSL_DIR) ]; then \
		$(MAKE) -C $(MATRIXSSL_DIR)/src clean; \
	fi
	if [ -d $(GSOAP_DIR) ]; then \
		$(MAKE) -C $(GSOAP_DIR) clean; \
		rm -f  $(GSOAP_DIR)/config.h; \
	fi
	$(MAKE) -C cwmpClient clean
	$(MAKE) -C gsoaplib/soapcpp2/samples/libcwmp clean


CRYPTLIB = portingssl
WSCSRC = src

all:
	make -C $(CRYPTLIB)
	make -C $(WSCSRC)
	
clean:
	make -C $(CRYPTLIB) clean
	make -C $(WSCSRC) clean

romfs:
ifeq ($(CONFIG_USB_RTL8187SU_SOFTAP),y)
	$(ROMFSINST) wscd.conf_e8b /etc/wscd.conf
else
	$(ROMFSINST) /etc/wscd.conf
endif
	$(ROMFSINST) src/wscd /bin/wscd
	

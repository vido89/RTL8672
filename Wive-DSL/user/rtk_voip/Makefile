include .linux_config

all:
ifdef CONFIG_RTK_VOIP_IP_PHONE
	make -C ui
endif
	make -C flash
	make -C voip_manager
	make -C src/ivr
ifdef CONFIG_RTK_VOIP_TEST_TOOL
	make -C test
endif


romfs:
ifdef CONFIG_RTK_VOIP_IP_PHONE
	$(ROMFSINST)	ui/tmp/ui	/bin/ui
endif
	make -C voip_manager romfs
	$(ROMFSINST)    src/solar_monitor       /bin/solar_monitor
	$(ROMFSINST)	src/solar	/bin/solar
	$(ROMFSINST)	src/dns_task /bin/dns_task
	$(ROMFSINST)	src/ivr/ivrserver /bin/ivrserver

ifdef CONFIG_RTK_VOIP_TEST_TOOL
	make -C test romfs
else
	make -C test linkclean
endif
clean:
ifdef CONFIG_RTK_VOIP_IP_PHONE
	make -C ui clean
endif
	make -C voip_manager clean
	make -C src/ivr clean
	make -C test clean
	make -C flash clean


link:
ifdef CONFIG_RTK_VOIP_TEST_TOOL
	make -C test link
else
	@echo [RTK VoIP] No test link created!
endif
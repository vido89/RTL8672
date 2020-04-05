
.EXPORT_ALL_VARIABLES:

DIRS = build
TARGET = solar

all:
	for i in $(DIRS) ; do make -C $$i || exit $? ; done
	make -C tools

romfs:
	$(ROMFSINST)	build/all	/bin/solar
	$(ROMFSINST)	dns/dns_task /bin/dns_task
	$(ROMFSINST)	ivr/ivrserver /bin/ivrserver
	$(ROMFSINST)	tools/solar_monitor /bin/solar_monitor

clean:
	-for i in $(DIRS) ; do make -C $$i clean ; done
	make -C tools clean

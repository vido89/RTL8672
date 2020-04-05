SUBDIRS = src src/dlisten



all:
	${MAKE} -C src
	${MAKE} -C src/dlisten

clean:
	${MAKE} -C src clean
	${MAKE} -C src/dlisten clean

romfs:
	for x in $(SUBDIRS); do make -C $$x $@; done
	$(ROMFSINST) -e CONFIG_USER_WIRELESS_TOOLS_RTL8185_AUTH /bin/auth
	$(ROMFSINST) -e CONFIG_USER_WIRELESS_TOOLS_RTL8185_IAPP /bin/iapp

#
#	Makefile -- Build instructions for Realtek/RTL8670
#
ifdef CONFIG_BOA_WEB_E8B_CH
CONFIGPATH=$(shell pwd)/e8b
else
CONFIGPATH=$(shell pwd)
endif
include $(ROOTDIR)/$(LINUXDIR)/.config
ROMFSIMG = $(IMAGEDIR)/romfs.img
IMAGE    = $(IMAGEDIR)/image.bin
ifdef CONFIG_ROMFS_FS
	DEVCMD = for i in $(DEVICES); do \
			touch $(ROMFSDIR)/dev/@$$i; \
		done
	ROMFS_DIRS = bin dev etc home lib mnt proc usr var etc/scripts 
else
	DEVCMD =
	ROMFS_DIRS = bin etc home lib mnt proc usr var etc/scripts 

ifdef CONFIG_USER_FLATFSD_XXX
	ROMFS_DIRS += etc/default
endif

endif

ROMFS_DIRS = bin dev etc home lib proc usr var etc/scripts usr/local usr/local/ct

ifdef CONFIG_DEFAULTS_KERNEL_2_6
	ROMFS_DIRS += sys
endif

DEVICES = \
	tty,c,5,0      console,c,5,1      cua0,c,5,64      cua1,c,5,65  \
	\
	mem,c,1,1      kmem,c,1,2         null,c,1,3       ram0,b,1,0 \
	ram1,b,1,1 \
	\
	ptyp0,c,2,0    ptyp1,c,2,1        ptyp2,c,2,2      ptyp3,c,2,3 \
	ptyp4,c,2,4    ptyp5,c,2,5        ptyp6,c,2,6      ptyp7,c,2,7 \
	ptyp8,c,2,8    ptyp9,c,2,9        ptypa,c,2,10     ptypb,c,2,11 \
	ptypc,c,2,12   ptypd,c,2,13       ptype,c,2,14     ptypf,c,2,15 \
	\
	rom0,b,31,0    rom1,b,31,1        rom2,b,31,2      rom3,b,31,3 \
	rom4,b,31,4    rom5,b,31,5        rom6,b,31,6      rom7,b,31,7 \
	rom8,b,31,8    rom9,b,31,9 \
	\
	tty0,c,4,0     tty1,c,4,1         tty2,c,4,2       tty3,c,4,3 \
	ttyS0,c,4,64   ttyS1,c,4,65 \
	\
	ttyp0,c,3,0    ttyp1,c,3,1        ttyp2,c,3,2      ttyp3,c,3,3 \
	ttyp4,c,3,4    ttyp5,c,3,5        ttyp6,c,3,6      ttyp7,c,3,7 \
	ttyp8,c,3,8    ttyp9,c,3,9        ttypa,c,3,10     ttypb,c,3,11 \
	ttypc,c,3,12   ttypd,c,3,13       ttype,c,3,14     ttypf,c,3,15 \
	\
	mtd0,c,90,0    mtd1,c,90,1        mtd2,c,90,2      mtd3,c,90,3 \
	\
	zero,c,1,5     random,c,1,8       urandom,c,1,9    ppp,c,108,0 \
	\
	adsl0,c,100,0  wlchr,c,13,0


clean:

romfs:
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	$(DEVCMD)
	$(ROMFSINST) -s /var/tmp /tmp
	$(ROMFSINST) -s /var/mnt /mnt
	$(ROMFSINST) -s /bin /sbin
	$(ROMFSINST) -s /var/ppp /etc/ppp
	$(ROMFSINST) -s /var/config /etc/config
	$(ROMFSINST) -s /var/passwd /etc/passwd
	$(ROMFSINST) -s /var/TZ /etc/TZ
	$(ROMFSINST) /etc/rc
	$(ROMFSINST) -s /etc/rc /bin/rc
ifdef CONFIG_USER_USBMOUNT_USBMOUNT
	$(ROMFSINST) -a "echo /usr/hotplug > /proc/sys/kernel/hotplug" /etc/rc
	$(ROMFSINST) -a "mkdir -p /tmp/usb/" /etc/rc
endif
	$(ROMFSINST) -a "echo Acorp LAN > /proc/sys/kernel/hostname" /etc/rc
ifdef CONFIG_TCP_WESTWOOD
	$(ROMFSINST) -a "echo 1 > /proc/sys/net/ipv4/tcp_westwood" /etc/rc
endif
	$(ROMFSINST) -a "echo 1 > /proc/sys/net/ipv4/tcp_rfc1337" /etc/rc
	$(ROMFSINST) -a "echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse" /etc/rc
	$(ROMFSINST) -a "echo 1 > /proc/sys/net/ipv4/tcp_tw_recycle" /etc/rc
	$(ROMFSINST) -a "echo 2048 3072 > /proc/sys/net/ipv4/ip_local_port_range" /etc/rc
	$(ROMFSINST) -a "echo 512 > /proc/sys/net/ipv4/route/max_size" /etc/rc
	$(ROMFSINST) -a "echo 512 > /proc/sys/net/ipv4/route/gc_thresh" /etc/rc
	$(ROMFSINST) -a "echo 8 > /proc/sys/net/ipv4/route/gc_elasticity" /etc/rc
	$(ROMFSINST) -a "echo 60 > /proc/sys/net/ipv4/route/gc_interval" /etc/rc
	$(ROMFSINST) -a "echo 20 > /proc/sys/net/ipv4/route/gc_timeout" /etc/rc
	$(ROMFSINST) -a "echo 10 > /proc/sys/net/ipv4/tcp_keepalive_intvl" /etc/rc
	$(ROMFSINST) -a "echo 3 > /proc/sys/net/ipv4/tcp_keepalive_probes" /etc/rc
	$(ROMFSINST) -a "echo 10 > /proc/sys/net/ipv4/tcp_keepalive_time" /etc/rc
	$(ROMFSINST) -a "echo 1 > /proc/sys/net/ipv4/icmp_ignore_bogus_error_responses" /etc/rc
	$(ROMFSINST) -a "echo 1 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts" /etc/rc
	$(ROMFSINST) -a "echo 0 > /proc/sys/net/ipv4/tcp_timestamps" /etc/rc
	$(ROMFSINST) -a "echo 99 512 32 512 0 300000 60 0 0 > /proc/sys/vm/bdflush" /etc/rc
	$(ROMFSINST) -a "echo 0 0 > /proc/sys/vm/pagetable_cache" /etc/rc
	$(ROMFSINST) -a "echo 1 > /proc/sys/vm/page-cluster" /etc/rc
	$(ROMFSINST) -a "echo 0 0 0 > /proc/sys/vm/kswapd" /etc/rc
	$(ROMFSINST) -a "echo 1 > /proc/sys/vm/max-readahead" /etc/rc
	$(ROMFSINST) -a "echo 0 > /proc/sys/net/bridge/bridge-nf-call-arptables" /etc/rc
	$(ROMFSINST) -a "echo 0 > /proc/sys/net/bridge/bridge-nf-call-ip6tables" /etc/rc
	$(ROMFSINST) -a "echo 0 > /proc/sys/net/ipv4/tcp_ecn" /etc/rc
	$(ROMFSINST) -a "echo 10 > /proc/sys/net/ipv4/ipfrag_time" /etc/rc
	$(ROMFSINST) -a "echo 16384 > /proc/sys/net/ipv4/ipfrag_low_thresh" /etc/rc
	$(ROMFSINST) -a "echo 65536 > /proc/sys/net/ipv4/ipfrag_high_thresh" /etc/rc
	$(ROMFSINST) -a "echo 768 > /proc/sys/net/ipv4/ip_conntrack_max" /etc/rc
	$(ROMFSINST) -a "echo 512 > /proc/sys/net/ipv4/ip_conntrack_tcp" /etc/rc
	$(ROMFSINST) -a "echo 128 > /proc/sys/net/ipv4/ip_conntrack_udp" /etc/rc
	$(ROMFSINST) -a "echo 768     768    1024 > /proc/sys/net/ipv4/tcp_mem" /etc/rc
	$(ROMFSINST) -a "echo 768     768    1024 > /proc/sys/net/ipv4/tcp_rmem" /etc/rc
	$(ROMFSINST) -a "echo 768     768    1024 > /proc/sys/net/ipv4/tcp_wmem" /etc/rc
	$(ROMFSINST) -a "wdg on &" /etc/rc
	$(ROMFSINST) -a "wdg kick 10 &" /etc/rc
	$(ROMFSINST) -a "wdg timeout 60 &" /etc/rc
	$(ROMFSINST) -a "wdg start 10 &" /etc/rc
	
ifeq ($(CONFIG_RTK_VOIP),y)
	$(ROMFSINST) /etc/rc_voip
	$(ROMFSINST) -a "/etc/rc_voip" /etc/rc
endif
	$(ROMFSINST) /etc/motd
	$(ROMFSINST) /etc/group
	$(ROMFSINST) /etc/shells
	$(ROMFSINST) /etc/inittab
	$(ROMFSINST) /etc/termcap
	$(ROMFSINST) scripts/udhcpc.sh /etc/scripts/udhcpc.sh
	$(ROMFSINST) scripts/udhcpc.deconfig /etc/scripts/udhcpc.deconfig
	$(ROMFSINST) services /etc/services
	$(ROMFSINST) ethertypes /etc/ethertypes
	echo "$(VERSIONSTR) -- " `date` > $(ROMFSDIR)/etc/version
dev:
ifdef CONFIG_SQUASHFS
	[ -d $(ROMFSDIR) ] || mkdir -p $(ROMFSDIR)
	[ -d $(ROMFSDIR)/dev ] || mkdir -p $(ROMFSDIR)/dev
	rm $(ROMFSDIR)/dev/*
	for i in $(DEVICES); do \
		name=`echo $$i |cut -d"," -f1`;\
		type=`echo $$i |cut -d"," -f2`;\
		major=`echo $$i |cut -d"," -f3`;\
		minor=`echo $$i |cut -d"," -f4`;\
		mknod -m644 $(ROMFSDIR)/dev/$$name $$type $$major $$minor;\
	done
endif

image:
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	genromfs -v -V "ROMdisk" -f $(ROMFSIMG) -d $(ROMFSDIR)
	$(CROSS_COMPILE)objcopy -O binary --remove-section=.romvec \
			--remove-section=.text --remove-section=.ramvec \
			--remove-secti/on=.init \
			--remove-section=.bss --remove-section=.eram \
			$(ROOTDIR)/$(LINUXDIR)/linux $(IMAGEDIR)/linux.data
	$(CROSS_COMPILE)objcopy -O binary --remove-section=.ramvec \
			--remove-section=.bss --remove-section=.data \
			--remove-section=.eram \
			--set-section-flags=.romvec=CONTENTS,ALLOC,LOAD,READONLY,CODE \
			$(ROOTDIR)/$(LINUXDIR)/linux $(IMAGEDIR)/linux.text
	cat $(IMAGEDIR)/linux.text $(IMAGEDIR)/linux.data $(ROMFSIMG) > $(IMAGE)
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(IMAGE) /tftpboot


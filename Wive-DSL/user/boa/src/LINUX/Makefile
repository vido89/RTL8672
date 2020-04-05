#
# Makefile for the GoAhead web server reference source base
#  for the Linux OS
#
# Copyright (c) GoAhead Software, Inc. 1995-2000
#
# $Id: Makefile,v 1.55 2009/02/10 13:03:26 xl Exp $
#

ifndef UCLINUX_BUILD_USER
include ../../../../config/.config
LIBCRYPT = -lcrypt
LDFLAGS = -L. -L/lib
endif
all:	compile 

WEB_LIB	= libwebs.a
MIB_LIB = libmib.a
MIB_LIB_SO = libmib.so

# Dick
ifdef NOT_IN_UCLINUX_DIST
CC = mips-uclibc-gcc
STRIP = mips-uclibc-strip
endif

# User Management switch
UMSW	= -DUSER_MANAGEMENT_SUPPORT

ifeq ($(ACTION),1)
RTFLAGS += -DAPPLY_CHANGE
endif

CONFIG_MMU = 1

ifdef CONFIG_MMU
RTFLAGS += -DCONFIG_MMU
FLASHDRV = rtl_flashdrv_mmu
else
FLASHDRV = rtl_flashdrv
endif

# Digest Access switch
#DASW	= -DDIGEST_ACCESS_SUPPORT

# SSL switches (enable all to activate)
# SSLINC = /RSASSL/library/include
# SSLLIB = /RSASSL/library/lib/libsslc.a
# SSLSW	= -DWEBS_SSL_SUPPORT -I$(SSLINC)
# SSLPATCHFILE = ../websSSL.o

# If-Modified-Support switches (requires math library, libm.a)
# IFMODSW = -DWEBS_IF_MODIFIED_SUPPORT
# IFMODLIB = /usr/lib/libm.a

# Dependencies
ifdef CONFIG_GUI_WEB
DEPEND_FILES = fmget.o fmtcpip.o fmfwall.o fmmgmt.o \
		  fmwan.o fmbridge.o fmroute.o fmdns.o \
		  fmping.o fmreboot.o \
		  fmoamlb.o fmdhcpd.o fmigmproxy.o \
		  fmothers.o fmstatus.o fmupnp.o fmdiagtest.o\
		  fmeth2pvc.o fmqos.o fmacc.o fmacl.o fmdomainblk.o fmmacbase.o fmurl.o fmddns.o fmdhcrelay.o \
		  fmdhcpmode.o boamain.o fmlink.o fmguimenu.o saved.o 
else
DEPEND_FILES = fmget.o fmtcpip.o fmfwall.o fmmgmt.o \
		  fmwan.o fmbridge.o fmroute.o fmdns.o \
		  fmping.o fmreboot.o \
		  fmoamlb.o fmdhcpd.o fmigmproxy.o \
		  fmothers.o fmstatus.o fmupnp.o fmdiagtest.o\
		  fmeth2pvc.o fmeth2pvc_vlan.o fmqos.o fmacc.o fmacl.o fmdomainblk.o fmmacbase.o fmurl.o fmddns.o fmdhcrelay.o \
		  fmdhcpmode.o fmmenucreate.o fmmenucreate_user.o boamain.o fmlink.o saved.o fmalgonoff.o fmaddressmap.o
endif

obj_y =
exec_y =

obj_$(CONFIG_USER_CWMP_TR069)			+= fmtr069.o
obj_$(CONFIG_USER_AUTO_PROVISIONING)		+= fmautoprovision.o
obj_$(CONFIG_USER_CMD_CLI)		+= cfgutility.o
obj_$(CONFIG_USER_CMD_CLI)		+= cmdedit.o

DEPEND_FILES += $(obj_y)
# MIB_DEPEND_FILES = mib.o mibtbl.o $(FLASHDRV).o utility.o adsl_drv.o debug.o arping.o ../msgutil.o sysconfig.o
MIB_DEPEND_FILES = $(FLASHDRV).o utility.o adsl_drv.o debug.o arping.o ../msgutil.o ../md5.o \
				   sysconfig.o lzss.o

ifdef CONFIG_USER_WIRELESS_TOOLS
WLAN_FILES	= fmwlan.o
endif

FLASH   = -DINCLUDE_DEFAULT_VALUE
RTFLAGS += -DNO_ACTION
WEBFLAG	= -DWEBS -DUEMF -DOS="LINUX" -DLINUX $(WLANSUPP) $(UMSW) $(DASW) \
	$(SSLSW) $(IFMODSW) $(RTFLAGS)
OTHERS	= -DB_STATS -DB_FILL 
#-DDEBUG
#DEBUG	= -g -Wall
#DEBUG	=
#DEBUG	= -D_DEBUG_MESSAGE
IFLAGS 	= -I..
#LDFLAGS	= $(SSLLIB) $(IFMODLIB)

ifeq ($(CONFIG_USER_RTK_VOIP),y)
#+++++add by Jack for VoIP project 20/03/07+++++
PWD := $(shell pwd)
TOPDIR := $(shell pwd)/../../../..
VOIP_APP = $(TOPDIR)/user/rtk_voip
# -include ../../../../linux-2.4.x/.config
ifdef CONFIG_RTK_VOIP
# have VOIP support
FLASH_FILE = voip_flash.o voip_flash_mib.o voip_flash_tool.o voip_flash_server.o
MIB_DEPEND_FILES += $(FLASH_FILE)
CFLAGS += -DVOIP_SUPPORT -I$(VOIP_APP)/includes -I$(VOIP_APP)/web \
	-I$(PWD) -I$(PWD)/..
endif
	
all:
ifdef CONFIG_RTK_VOIP
		make -C $(VOIP_APP)/web -f rtl867x.mak WEB_FLAGS='$(DEBUG) $(CFLAGS) $(IFLAGS)' WEB_DIR=$(TOPDIR)/romfs/home/httpd/web
endif
	@make compile
#-----end-----
endif

#compile: cleanmib $(WEB_LIB) flash compweb webpage startup
ifdef UCLINUX_BUILD_USER
exec_$(CONFIG_USER_AUTO_PROVISIONING)		+= parser auto_provisioning
compile: cleanmib $(WEB_LIB) $(MIB_LIB) $(exec_y) cli cli2 flash startup qc ShowStatus CreatexmlConfig LoadxmlConfig configd httpd slogd saveconfig loadconfig
else
compile: cleanmib $(WEB_LIB) $(MIB_LIB)
endif

#
#	Build archive of objects
#
$(WEB_LIB): $(DEPEND_FILES) $(WLAN_FILES)
	$(AR) $(ARFLAGS) $(WEB_LIB) $?
	
$(MIB_LIB): $(MIB_DEPEND_FILES)
	$(AR) $(ARFLAGS) $(MIB_LIB) $?
ifdef UCLINUX_BUILD_USER
	$(LD) -shared --warn-common --warn-once -z combreloc -soname=$(MIB_LIB_SO).0 \
		-o $(MIB_LIB_SO) --whole-archive $(MIB_LIB)
	install -m 644 $(MIB_LIB_SO) ../../../../uClibc/lib/$(MIB_LIB_SO)
endif
	
#
#	Primary link
#
$(NAME): Makefile $(WEB_LIB)
	$(CC) -o $(NAME) \
		main.o $(WEB_LIB) $(LDFLAGS) 	
		
ifeq ($(CONFIG_USER_CLI),y)		
ifeq ($(CONFIG_USER_MENU_CLI),y)
cli: climenu.c $(FLASHDRV).c mib.h mibtbl.h rtl_flashdrv.h $(MIB_LIB)
	$(CC) -c -o climenu.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) climenu.c
	$(CC) -o cli \
		climenu.o saved.o $(LIBCRYPT) $(LDFLAGS) -lpthread -lmib
cli2: climenu2.c $(FLASHDRV).c mib.h mibtbl.h rtl_flashdrv.h $(MIB_LIB)                                                      
	$(CC) -c -o climenu2.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) climenu2.c                                   
	$(CC) -o cli2 climenu2.o saved.o $(LIBCRYPT) $(LDFLAGS) -lpthread -lmib                 
endif
#added by xl_yue
ifeq ($(CONFIG_USER_CMD_CLI),y)
cli: clicmd.c $(FLASHDRV).c mib.h mibtbl.h rtl_flashdrv.h $(MIB_LIB)
	$(CC) -c -o clicmd.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) clicmd.c 
	$(CC) -o cli \
		clicmd.o cfgutility.o cmdedit.o saved.o $(LIBCRYPT) $(LDFLAGS) -lpthread -lmib
endif
endif

flash: flash.c $(FLASHDRV).c mib.h mibtbl.h rtl_flashdrv.h mib.o mibtbl.o lzss.o $(MIB_LIB)
	$(CC) -c -o flash.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) flash.c
	$(CC) -o flash \
		flash.o mib.o mibtbl.o lzss.o $(LDFLAGS) -lmib

qc: qc.c mib.c mibtbl.c $(FLASHDRV).c mib.h mibtbl.h rtl_flashdrv.h $(MIB_LIB)
	$(CC) -c -o qc.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) qc.c
	$(CC) -o qc \
		qc.o $(LDFLAGS) -lmib
		
parser: parser.c mib.c mibtbl.c $(FLASHDRV).c mib.h mibtbl.h utility.c rtl_flashdrv.h $(MIB_LIB)
	$(CC) -c -o parser.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) parser.c
	$(CC) -o parser \
		parser.o $(LDFLAGS) -lmib

auto_provisioning: auto_provisioning.c
	$(CC) -c -o auto_provisioning.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) auto_provisioning.c
	$(CC) -o auto_provisioning \
		auto_provisioning.o $(LDFLAGS) -lpthread

CreatexmlConfig: CreatexmlConfig.c mib.c mibtbl.c $(FLASHDRV).c mib.h mibtbl.h rtl_flashdrv.h $(MIB_LIB)
	$(CC) -c -o CreatexmlConfig.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) CreatexmlConfig.c
	$(CC) -o CreatexmlConfig \
		CreatexmlConfig.o $(LDFLAGS) -lmib
		
LoadxmlConfig: LoadxmlConfig.c $(FLASHDRV).c mib.h mibtbl.h rtl_flashdrv.h $(MIB_LIB)
	$(CC) -c -o LoadxmlConfig.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) LoadxmlConfig.c
	$(CC) -o LoadxmlConfig \
		LoadxmlConfig.o $(LDFLAGS) -lmib

saveconfig: saveconfig.c $(FLASHDRV).c mib.h mibtbl.h rtl_flashdrv.h $(MIB_LIB)
	$(CC) -c -o saveconfig.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) saveconfig.c
	$(CC) -o saveconfig \
		saveconfig.o $(LDFLAGS) -lmib

loadconfig: loadconfig.c $(FLASHDRV).c mib.h mibtbl.h rtl_flashdrv.h $(MIB_LIB)
	$(CC) -c -o loadconfig.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) loadconfig.c
	$(CC) -o loadconfig \
		loadconfig.o $(LDFLAGS) -lmib

ShowStatus: ShowStatus.c mib.c mibtbl.c $(FLASHDRV).c mib.h mibtbl.h rtl_flashdrv.h $(MIB_LIB) 
	$(CC) -c -o ShowStatus.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) ShowStatus.c
	$(CC) -o ShowStatus \
		ShowStatus.o $(LDFLAGS) -lpthread -lmib

startup: Makefile startup.c mib.h mibtbl.h rtl_flashdrv.h $(MIB_LIB)
	$(CC) -c -o startup.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) startup.c
	$(CC) -o startup \
		startup.o $(LDFLAGS) -lcrypt  -lmib

configd: Makefile configd.c msgparser.c mib.o mibtbl.o lzss.o mib_reserve.o $(MIB_LIB)
	$(CC) -c -o configd.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) configd.c
	$(CC) -c -o msgparser.o $(FLASH) $(DEBUG) $(CFLAGS) $(WEBFLAG) $(IFLAGS) msgparser.c
	$(CC) -o configd \
		configd.o msgparser.o mib.o mibtbl.o lzss.o mib_reserve.o $(LDFLAGS) -lpthread -lmib

httpd: Makefile httpd.c $(MIB_LIB)
	$(CC) -c -o httpd.o $(DEBUG) $(CFLAGS) $(IFLAGS) httpd.c
	$(CC) -o httpd \
		httpd.o $(LDFLAGS) -lmib

slogd: slogd.c
	$(CC) -c -o slogd.o $(DEBUG) $(CFLAGS) $(IFLAGS) slogd.c
	$(CC) -o slogd \
		slogd.o $(LDFLAGS)

compweb: Makefile compweb.o
	gcc -c -o compweb.o $(FLASH) $(DEBUG) $(WEBFLAG) $(IFLAGS) compweb.c
	gcc  -o compweb $(DEBUG) $(WEBFLAG) $(IFLAGS) compweb.o

webpage:
	rm webpages.bin -f
	find ../web -name "*.*" >web_files
	./compweb web_files webpages.bin

romfs:
	mkdir -p $(ROMFSDIR)/home/httpd
	$(ROMFSINST) ../web  /home/httpd/web
	$(ROMFSINST) ../web/graphics /home/httpd/web/admin/graphics
	$(ROMFSINST) ../web/menu-images /home/httpd/web/admin/menu-images
	$(ROMFSINST) -s /home/httpd/web/mtmcode.js  /home/httpd/web/admin/mtmcode.js
	$(ROMFSINST) -s /home/httpd/web/share.js  /home/httpd/web/admin/share.js
	$(ROMFSINST) -s /home/httpd/web/acl.asp  /home/httpd/web/admin/acl.asp
	$(ROMFSINST) -s /home/httpd/web/url_blocking.asp  /home/httpd/web/admin/url_blocking.asp
	$(ROMFSINST) -s /home/httpd/web/date.asp  /home/httpd/web/admin/date.asp
	$(ROMFSINST) -s /home/httpd/web/fw-macfilter.asp  /home/httpd/web/admin/fw-macfilter.asp
	$(ROMFSINST) -s /home/httpd/web/reboot.asp  /home/httpd/web/admin/reboot.asp
	$(ROMFSINST) -s /home/httpd/web/saveconf.asp  /home/httpd/web/admin/saveconf.asp
	$(ROMFSINST) -s /home/httpd/web/status.asp  /home/httpd/web/admin/status.asp
	$(ROMFSINST) -s /home/httpd/web/title.html  /home/httpd/web/admin/title.html
	$(ROMFSINST) -s /home/httpd/web/upload.asp  /home/httpd/web/admin/upload.asp
	$(ROMFSINST) -s /home/httpd/web/upload2.asp  /home/httpd/web/admin/upload2.asp
	$(ROMFSINST) -s /home/httpd/web/upload3.asp  /home/httpd/web/admin/upload3.asp
	$(ROMFSINST) -s /home/httpd/web/wladvanced.asp  /home/httpd/web/admin/wladvanced.asp
	$(ROMFSINST) -s /home/httpd/web/wlbasic.asp  /home/httpd/web/admin/wlbasic.asp
	$(ROMFSINST) -s /home/httpd/web/wlstatbl.asp  /home/httpd/web/admin/wlstatbl.asp
	$(ROMFSINST) -s /home/httpd/web/wlwpa.asp  /home/httpd/web/admin/wlwpa.asp
	$(ROMFSINST) -s /home/httpd/web/wlwep.asp  /home/httpd/web/admin/wlwep.asp
	$(ROMFSINST) -s /home/httpd/web/wlactrl.asp  /home/httpd/web/admin/wlactrl.asp
	$(ROMFSINST) -e CONFIG_USER_WIRELESS_WDS -s /home/httpd/web/wlwds.asp  /home/httpd/web/admin/wlwds.asp
	$(ROMFSINST) -s /home/httpd/web/wlsurvey.asp  /home/httpd/web/admin/wlsurvey.asp
	$(ROMFSINST) -e CONFIG_WIFI_SIMPLE_CONFIG -s /home/httpd/web/wlwps.asp  /home/httpd/web/admin/wlwps.asp
	$(ROMFSINST) -e CONFIG_USER_WIRELESS_MBSSID -s /home/httpd/web/wlmbssid.asp  /home/httpd/web/admin/wlmbssid.asp
	$(ROMFSINST) -s /home/httpd/web/wlwpa_mbssid.asp  /home/httpd/web/admin/wlwpa_mbssid.asp
	$(ROMFSINST) -s /home/httpd/web/wlwep_mbssid.asp  /home/httpd/web/admin/wlwep_mbssid.asp
	$(ROMFSINST) -s /home/httpd/web/graphics/del.gif  /home/httpd/web/admin/graphics/del.gif
	$(ROMFSINST) -s /home/httpd/web/graphics/edit.gif  /home/httpd/web/admin/graphics/edit.gif
	$(ROMFSINST) -s /home/httpd/web/graphics/goahead_logo.gif  /home/httpd/web/admin/graphics/goahead_logo.gif
	$(ROMFSINST) -s /home/httpd/web/graphics/topbar.png  /home/httpd/web/admin/graphics/topbar.png
	$(ROMFSINST) -s /home/httpd/web/menu-images/menu_bar.gif  /home/httpd/web/admin/menu-images/menu_bar.gif
	$(ROMFSINST) -s /home/httpd/web/menu-images/menu_corner.gif  /home/httpd/web/admin/menu-images/menu_corner.gif
	$(ROMFSINST) -s /home/httpd/web/menu-images/menu_corner_minus.gif  /home/httpd/web/admin/menu-images/menu_corner_minus.gif
	$(ROMFSINST) -s /home/httpd/web/menu-images/menu_corner_plus.gif  /home/httpd/web/admin/menu-images/menu_corner_plus.gif
	$(ROMFSINST) -s /home/httpd/web/menu-images/menu_folder_closed.gif  /home/httpd/web/admin/menu-images/menu_folder_closed.gif
	$(ROMFSINST) -s /home/httpd/web/menu-images/menu_folder_open.gif  /home/httpd/web/admin/menu-images/menu_folder_open.gif
	$(ROMFSINST) -s /home/httpd/web/menu-images/menu_link_default.gif  /home/httpd/web/admin/menu-images/menu_link_default.gif
	$(ROMFSINST) -s /home/httpd/web/menu-images/menu_pixel.gif  /home/httpd/web/admin/menu-images/menu_pixel.gif
	$(ROMFSINST) -s /home/httpd/web/menu-images/menu_root.gif  /home/httpd/web/admin/menu-images/menu_root.gif
	$(ROMFSINST) -s /home/httpd/web/menu-images/menu_tee.gif  /home/httpd/web/admin/menu-images/menu_tee.gif
	$(ROMFSINST) -s /home/httpd/web/menu-images/menu_tee_minus.gif  /home/httpd/web/admin/menu-images/menu_tee_minus.gif
	$(ROMFSINST) -s /home/httpd/web/menu-images/menu_tee_plus.gif  /home/httpd/web/admin/menu-images/menu_tee_plus.gif
	#added by xl_yue
	$(ROMFSINST) -s /home/httpd/web/wlbasic_sc.asp  /home/httpd/web/admin/wlbasic_sc.asp
	$(ROMFSINST) -s /home/httpd/web/wladvanced_sc.asp  /home/httpd/web/admin/wladvanced_sc.asp
	$(ROMFSINST) -s /home/httpd/web/status_sc.asp  /home/httpd/web/admin/status_sc.asp
	$(ROMFSINST) -s /home/httpd/web/wlactrl_sc.asp  /home/httpd/web/admin/wlactrl_sc.asp
	$(ROMFSINST) -s /home/httpd/web/wlmbssid_sc.asp  /home/httpd/web/admin/wlmbssid_sc.asp
	$(ROMFSINST) -s /home/httpd/web/wlwpa_mbssid_sc.asp  /home/httpd/web/admin/wlwpa_mbssid_sc.asp
	$(ROMFSINST) -s /home/httpd/web/wlwep_mbssid_sc.asp  /home/httpd/web/admin/wlwep_mbssid_sc.asp
	$(ROMFSINST) -s /home/httpd/web/wlwds_sc.asp  /home/httpd/web/admin/wlwds_sc.asp
	$(ROMFSINST) -s /home/httpd/web/wlwpa_sc.asp  /home/httpd/web/admin/wlwpa_sc.asp
	$(ROMFSINST) -s /home/httpd/web/wlwep_sc.asp  /home/httpd/web/admin/wlwep_sc.asp
	$(ROMFSINST) -s /home/httpd/web/wlwps_sc.asp  /home/httpd/web/admin/wlwps_sc.asp
	# jim  for ZTE 531B case,  
	$(ROMFSINST) -s /home/httpd/web/eth2pvc_sc.asp  /home/httpd/web/admin/eth2pvc_sc.asp
	$(ROMFSINST) -s /home/httpd/web/password_sc.asp  /home/httpd/web/admin/password_sc.asp
	$(ROMFSINST) -s /home/httpd/web/saveconf_sc.asp  /home/httpd/web/admin/saveconf_sc.asp
	$(ROMFSINST) -s /home/httpd/web/upload_sc.asp  /home/httpd/web/admin/upload_sc.asp
	$(ROMFSINST) -s /home/httpd/web/reboot_sc.asp  /home/httpd/web/admin/reboot_sc.asp
	$(ROMFSINST) -s /home/httpd/web/syslog_sc.asp  /home/httpd/web/admin/syslog_sc.asp
	$(ROMFSINST) -s /home/httpd/web/acl_sc.asp  /home/httpd/web/admin/acl_sc.asp
	$(ROMFSINST) -s /home/httpd/web/snmp_sc.asp  /home/httpd/web/admin/snmp_sc.asp
	$(ROMFSINST) -s /home/httpd/web/share_sc.js  /home/httpd/web/admin/share_sc.js
	$(ROMFSINST) -s /home/httpd/web/fc-page1-ch.asp /home/httpd/web/admin/fc-page1-ch.asp
	$(ROMFSINST) -s /home/httpd/web/fc-page2-ch.asp /home/httpd/web/admin/fc-page2-ch.asp
	$(ROMFSINST) -s /home/httpd/web/fc-page3-ch.asp /home/httpd/web/admin/fc-page3-ch.asp
	$(ROMFSINST) -s /home/httpd/web/fc-page4-ch.asp /home/httpd/web/admin/fc-page4-ch.asp
	$(ROMFSINST) -s /home/httpd/web/fc-ppp-type.asp /home/httpd/web/admin/fc-ppp-type.asp
	$(ROMFSINST) -s /home/httpd/web/fc-ppp-wan.asp /home/httpd/web/admin/fc-ppp-wan.asp
	$(ROMFSINST) -s /home/httpd/web/fc-mer-wan.asp /home/httpd/web/admin/fc-mer-wan.asp
	$(ROMFSINST) -s /home/httpd/web/fc-ipoa.asp /home/httpd/web/admin/fc-ipoa.asp
	$(ROMFSINST) -s /home/httpd/web/bridging_sc.asp /home/httpd/web/admin/bridging_sc.asp
	$(ROMFSINST) -s /home/httpd/web/fdbtbl_sc.asp /home/httpd/web/admin/fdbtbl_sc.asp
	$(ROMFSINST) -s /home/httpd/web/upnp_sc.asp /home/httpd/web/admin/upnp_sc.asp
	$(ROMFSINST) -s /home/httpd/web/fw-macfilter_sc.asp /home/httpd/web/admin/fw-macfilter_sc.asp
	$(ROMFSINST) -s /home/httpd/web/date_sc.asp /home/httpd/web/admin/date_sc.asp
	$(ROMFSINST) -s /home/httpd/web/vrtsrv_sc.asp /home/httpd/web/admin/vrtsrv_sc.asp
	$(ROMFSINST) -s /home/httpd/web/vrtsrv_tbl.asp /home/httpd/web/admin/vrtsrv_tbl.asp
	$(ROMFSINST) -s /home/httpd/web/fw-dmz_sc.asp /home/httpd/web/admin/fw-dmz_sc.asp
	#add by ramen for ZTE user account
	$(ROMFSINST) -s /home/httpd/web/lan_status_sc.asp /home/httpd/web/admin/lan_status_sc.asp	
	$(ROMFSINST) -s /home/httpd/web/wlan_status_sc.asp /home/httpd/web/admin/wlan_status_sc.asp
	$(ROMFSINST) -s /home/httpd/web/wan_status_sc.asp /home/httpd/web/admin/wan_status_sc.asp
	$(ROMFSINST) -s /home/httpd/web/status_eth2pvc_sc.asp /home/httpd/web/admin/status_eth2pvc_sc.asp
	$(ROMFSINST) -s /home/httpd/web/stats_sc.asp /home/httpd/web/admin/stats_sc.asp
	$(ROMFSINST) -s /home/httpd/web/fw-dmz_sc.asp /home/httpd/web/admin/fw-dmz_sc.asp
	$(ROMFSINST) -s /home/httpd/web/adv/adsl-statis_sc.asp /home/httpd/web/admin/adsl-statis_sc.asp
	$(ROMFSINST) -s /home/httpd/web/routetbl_sc.asp /home/httpd/web/admin/routetbl_sc.asp
	$(ROMFSINST) -s /home/httpd/web/arptable_sc.asp /home/httpd/web/admin/arptable_sc.asp
	$(ROMFSINST) -s /home/httpd/web/adv/adsl-statis_sc.asp /home/httpd/web/admin/adsl-statis_sc.asp
	

	$(ROMFSINST) /bin/startup
	$(ROMFSINST) /bin/flash
	$(ROMFSINST) /bin/qc
	$(ROMFSINST) /bin/ShowStatus
	$(ROMFSINST) /bin/qc
	$(ROMFSINST) /bin/CreatexmlConfig
	$(ROMFSINST) /bin/LoadxmlConfig
	$(ROMFSINST) /bin/configd
	$(ROMFSINST) /bin/httpd
	$(ROMFSINST) /bin/slogd
	$(ROMFSINST) -e CONFIG_USER_AUTO_PROVISIONING /bin/parser
	$(ROMFSINST) -e CONFIG_USER_AUTO_PROVISIONING /bin/auto_provisioning
	$(ROMFSINST) -e CONFIG_USER_CLI /bin/cli
	$(ROMFSINST) -e CONFIG_USER_CLI /bin/cli2
	$(ROMFSINST) /lib/libmib.so
	$(ROMFSINST) -s /lib/libmib.so /lib/libmib.so.0
	rm -f $(ROMFSDIR)/web/*.bak
	rm -f $(ROMFSDIR)/web/adv/*.bak

clean:
ifeq ($(CONFIG_USER_RTK_VOIP),y)
	rm -f $(NAME) $(WEB_LIB) $(MIB_LIB) $(MIB_LIB_SO) flash compweb webpages.bin startup cli qc parser auto_provisioning CreatexmlConfig LoadxmlConfig ShowStatus configd httpd slogd *.gdb $(FLASH_FILE) libflashlib.a cli2 saveconfig loadconfig

	make -C $(VOIP_APP)/web -f rtl867x.mak clean WEB_DIR=$(TOPDIR)/romfs/home/httpd/web
else
	rm -f $(NAME) $(WEB_LIB) $(MIB_LIB) $(MIB_LIB_SO) flash compweb webpages.bin startup cli qc parser auto_provisioning CreatexmlConfig LoadxmlConfig ShowStatus configd httpd slogd *.gdb cli2 saveconfig loadconfig
endif

cleanall:
	rm -f *.o

cleanmib:
	rm -f flash.o mib.o mibtbl.o lzss.o $(FLASHDRV).o flash

#
#	Dependencies
#
../asp.o:  ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../balloc.o: ../balloc.c ../uemf.h

../base64.o:  ../base64.c ../webs.h ../wsIntrn.h  ../ej.h ../ejIntrn.h ../uemf.h

../cgi.o:  ../webs.h ../wsIntrn.h ../uemf.h

../default.o:  ../default.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../ejlex.o:  ../ejlex.c ../ej.h ../ejIntrn.h ../uemf.h

../ejparse.o:  ../ejparse.c ../ej.h ../ejIntrn.h ../uemf.h

../emfdb.o:  ../emfdb.h ../wsIntrn.h ../uemf.h

../form.o:  ../form.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../h.o:  ../h.c ../uemf.h

../handler.o:  ../handler.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../md5c.o:  ../md5.h ../wsIntrn.h ../uemf.h

../mime.o:  ../mime.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../misc.o:  ../misc.c ../uemf.h

../page.o:  ../page.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../ringq.o:  ../ringq.c ../uemf.h

../rom.o:  ../rom.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../msgutil.o: ../msgutil.c ../uemf.h

../security.o:  ../security.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../sock.o:  ../sock.c ../uemf.h

../sockGen.o:  ../sockGen.c ../uemf.h

../sym.o:  ../sym.c ../uemf.h

../uemf.o:  ../uemf.c ../uemf.h

../um.o:  ../webs.h ../wsIntrn.h ../um.h ../uemf.h

../umui.o:  ../webs.h ../wsIntrn.h ../um.h ../uemf.h

../url.o:  ../url.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../value.o:  ../value.c ../uemf.h

../webrom.o:  ../webrom.c ../webs.h ../wsIntrn.h ../uemf.h

../webs.o:  ../webs.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../websda.o:  ../webs.h ../wsIntrn.h ../websda.h ../uemf.h

../websuemf.o:  ../websuemf.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../websSSL.o:  ../websSSL.c ../websSSL.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

main.o:  main.c ../wsIntrn.h ../webs.h ../ej.h ../ejIntrn.h ../uemf.h

$(DEPEND_FILES): options.h

$(MIB_DEPEND_FILES): options.h

ifdef CONFIG_USER_WIRELESS_TOOLS
$(WLAN_FILES): options.h
endif

fmget.o: fmget.c

fmtcpip.o: fmtcpip.c

fmfwall.o: fmfwall.c

fmmgmt.o:fmmgmt.c

mibtbl.o: mibtbl.c mib.h

lzss.o:lzss.c

mib.o: mib.c lzss.c mib.h

fmwan.o:fmwan.c

fmbridge.o:fmbridge.c

fmroute.o:fmroute.c

fmacl.o:fmacl.c

fmurl.o:fmurl.c

fmdhcrelay.o:fmdhcrelay.c
ifdef CONFIG_GUI_WEB
fmguimenu.o:fmguimenu.c
else
#added by xl_yue
fmmenucreate.o:fmmenucreate.c
#added by xl_yue
fmmenucreate_user.o:fmmenucreate_user.c
endif

fmdhcpmode.o:fmdhcpmode.c

fmdhcpd.o:fmdhcpd.c

fmdns.o:fmdns.c

utility.o:utility.c utility.h

ifeq ($(CONFIG_USER_CMD_CLI),y)
cfgutility.o:cfgutility.c cfgutility.h
cmdedit.o:cmdedit.c
endif

debug.o:debug.c debug.h

$(FLASHDRV).o:$(FLASHDRV).c rtl_types.h rtl_board.h rtl_flashdrv.h

#genscript_ui.o:genscript_ui.c

adsl_drv.o:adsl_drv.c

fmping.o:fmping.c

fmoamlb.o:fmoamlb.c

fmreboot.o:fmreboot.c

fmigmproxy.o:fmigmproxy.c

msgparser.o:msgparser.c

fmothers.o:fmothers.c

fmstatus.o:fmstatus.c

fmautoprovision.o: fmautoprovision.c

fmdiagtest.o: fmdiagtest.c

startup.o:startup.c mibtbl.h

arping.o:arping.c utility.h

sysconfig.o:sysconfig.c sysconfig.h ../msgq.h mibtbl.h ../../../../config/autoconf.h

mib_reserve.o:mib_reserve.c mib_reserve.h mib.h mibtbl.h

ShowStatus.o: ShowStatus.c

ifdef CONFIG_USER_WIRELESS_TOOLS
fmwlan.o:fmwlan.c
endif

ifdef CONFIG_USER_CWMP_TR069
fmtr069.o:fmtr069.c
endif

ifeq ($(CONFIG_USER_RTK_VOIP),y)
#+++++add by Jack for VoIP project 20/03/07+++++
voip_flash.o: $(VOIP_APP)/flash/voip_flash.c $(VOIP_APP)/includes/voip_flash.h
	$(CC) -c -o $@ $(DEBUG) $(FLASH) $(CFLAGS) $(WEBFLAG) $(IFLAGS) $<
	
voip_flash_mib.o: $(VOIP_APP)/flash/voip_flash_mib.c $(VOIP_APP)/includes/voip_flash_mib.h
	$(CC) -c -o $@ $(DEBUG) $(FLASH) $(CFLAGS) $(WEBFLAG) $(IFLAGS) $<
	
voip_flash_tool.o: $(VOIP_APP)/flash/voip_flash_tool.c $(VOIP_APP)/includes/voip_flash_tool.h
	$(CC) -c -o $@ $(DEBUG) $(FLASH) $(CFLAGS) $(WEBFLAG) $(IFLAGS) $<
	
voip_flash_server.o: $(VOIP_APP)/flash/voip_flash_server.c $(VOIP_APP)/includes/voip_flash.h 
	$(CC) -c -o $@ $(DEBUG) $(FLASH) $(CFLAGS) $(WEBFLAG) $(IFLAGS) $<
#-----end-----
endif

#flash_ui.o:flash_ui.c

#
#	Transition rules (add -o to put object in right directory)
#
.c.o:
	$(CC) -c -o $@ $(DEBUG) $(FLASH) $(CFLAGS) $(WEBFLAG) $(IFLAGS) $<

#
#	Makefile -- Build instructions for user level apps
#

.EXPORT_ALL_VARIABLES:

#
# Include architecture specific build rules.
#

ifndef ROOTDIR
ROOTDIR=..
endif

UCLINUX_BUILD_USER=1
-include $(LINUX_CONFIG)
-include $(CONFIG_CONFIG)
-include $(ARCH_CONFIG)
-include $(MODULES_CONFIG)

VEND=$(ROOTDIR)/vendors

##### add for VoIP merging ##########
ifeq ($(CONFIG_USER_RTK_VOIP),y)
VOIPDIR=$(ROOTDIR)/user/rtk_voip
VOIP_CONFIG = $(VOIPDIR)/.linux_config
endif
######################################
#

# must run the vendor build first
#
dir_v = $(VEND)/$(CONFIG_VENDOR)/$(CONFIG_PRODUCT)/.

dir_y =
dir_n =
dir_  =

#fullllll cleanup if no select
SUBDIRS_APP  = adslctrl flatfsd inetd mini_upnp tcpdump_web upnpctrl
SUBDIRS_APP += auth dhcp-isc init miniupnpd samba telnetd usbmount
SUBDIRS_APP += dnsmasq fswcert iproute2 netlogger sarctl tftpd  usbtools
SUBDIRS_APP += br2684ctl dropbear ftp iptables openssl setserial torrent vsntp
SUBDIRS_APP += bridge-utils ebtables ftpd login route snmpdv2cTrap udhcp wireless_tools
SUBDIRS_APP += busybox ethctl ftp_manage routed spppd updatedd wlan_daemon
SUBDIRS_APP += cifs flash igmpproxy mc tcpdump updateddctrl wsc
SUBDIRS_APP += boa

#RTL UTILS
dir_y += ethctl
dir_y += sarctl
dir_y += udhcp
dir_y += spppd
dir_y += bridge-utils
dir_y += adslctrl
dir_y += busybox
dir_y += br2684ctl
dir_y += boa
dir_y += watchdog

dir_$(CONFIG_USER_IPROUTE2)                 += iproute2
dir_$(CONFIG_USER_IPTABLES_IPTABLES)        += iptables
dir_$(CONFIG_BRIDGE_NF_EBTABLES)	    += ebtables
dir_$(CONFIG_USER_WIRELESS_TOOLS)           += wireless_tools
dir_$(CONFIG_USER_WIRELESS_TOOLS_RTL8185_AUTH) += auth
dir_$(CONFIG_USER_WIRELESS_TOOLS)           += wlan_daemon
dir_$(CONFIG_USB_RTL8192SU_SOFTAP)	    += rtl8192su
dir_$(CONFIG_WIFI_SIMPLE_CONFIG)	    += wsc
dir_$(CONFIG_USER_DHCP_ISC_RELAY_DHCRELAY)  += dhcp-isc
dir_$(CONFIG_USER_DNSMASQ_DNSMASQ)          += dnsmasq
dir_$(CONFIG_USER_FLASH_FLASH)              += flash
dir_$(CONFIG_USER_FREESWAN)                 += freeswan
dir_$(CONFIG_USER_FSWCERT_FSWCERT)          += fswcert
dir_$(CONFIG_USER_FTP_FTP_FTP)              += ftp
dir_$(CONFIG_USER_FTPD_FTPD)                += ftpd
dir_$(CONFIG_USER_BFTPD_BFTPD)              += ftp_manage
dir_$(CONFIG_USER_INETD_INETD)              += inetd
dir_$(CONFIG_USER_INIT_EXPAND)              += init
dir_$(CONFIG_USER_INIT_INIT)                += init
dir_$(CONFIG_USER_LOGIN_LOGIN)              += login
dir_$(CONFIG_USER_LOGIN_PASSWD)             += login
dir_$(CONFIG_USER_OPENSSL_APPS)             += openssl
dir_$(CONFIG_USER_ROUTE_ARP)                += route
dir_$(CONFIG_USER_ROUTE_IFCONFIG)           += route
dir_$(CONFIG_USER_ROUTE_MIITOOL)            += route
dir_$(CONFIG_USER_ROUTE_ROUTE)              += route
dir_$(CONFIG_USER_ROUTE_NETSTAT)            += route
dir_$(CONFIG_USER_ROUTED_ROUTED)            += routed
dir_$(CONFIG_USER_SETSERIAL_SETSERIAL)      += setserial
dir_$(CONFIG_USER_SNMPD_SNMPD_V2CTRAP)      += snmpdv2cTrap
dir_$(CONFIG_USER_SSH_DROPBEAR)             += dropbear
dir_$(CONFIG_USER_TCPDUMP_TCPDUMP)          += tcpdump
dir_$(CONFIG_USER_TCPDUMP_WEB)              += tcpdump_web
dir_$(CONFIG_USER_TELNETD_TELNETD)          += telnetd
dir_$(CONFIG_USER_TFTPD_TFTPD)              += tftpd
dir_$(CONFIG_USER_USBTOOLS)                 += usbtools
dir_$(CONFIG_USER_USBMOUNT_USBMOUNT)	    += usbmount
dir_$(CONFIG_USER_RTK_VOIP)                 += rtk_voip
dir_$(CONFIG_USER_DDNS)			    += updatedd
dir_$(CONFIG_USER_DDNS)			    += updateddctrl
dir_$(CONFIG_USER_IGMPPROXY)                += igmpproxy
dir_$(CONFIG_USER_VSNTP)                    += vsntp
dir_$(CONFIG_USER_MINIUPNPD)		    += miniupnpd
dir_$(CONFIG_USER_MINIUPNPD)		    += upnpctrl
dir_$(CONFIG_USER_MINI_UPNPD)		    += mini_upnp
dir_$(CONFIG_USER_CWMP_TR069)               += cwmp-tr069
ifeq ($(CONFIG_USER_CWMP_TR069),y)
dir_$(CONFIG_USER_FLATFSD_XXX)              += flatfsd 
endif
dir_$(CONFIG_RTL867X_NETLOG)		    += netlogger
dir_$(CONFIG_CIFS)			    += cifs
dir_$(CONFIG_USER_SAMBA) 	            += samba
dir_$(CONFIG_USER_MC)			    += mc
dir_$(CONFIG_USER_CTORRENT) 		    += torrent


all:
ifeq ($(CONFIG_USER_MC),y)
	cd mc && ./DoConfigure.sh && make && cd ..
endif
ifeq ($(CONFIG_USER_CTORRENT),y)
	cd torrent && ./DoConfigure.sh && cd ..
endif
ifeq ($(CONFIG_USER_RTK_VOIP),y)
	-ln -fs $(ROOTDIR)/$(CONFIG_LINUXDIR)/include/linux/autoconf.h $(VOIPDIR)/includes/kernel_config.h	
	-ln -fs $(ROOTDIR)/$(CONFIG_LINUXDIR)/.config $(VOIPDIR)/.linux_config	
	-ln -fs $(ROOTDIR)/$(CONFIG_LINUXDIR)/rtk_voip/include/rtk_voip.h $(VOIPDIR)/includes/rtk_voip.h	
	-ln -fs $(ROOTDIR)/$(CONFIG_LINUXDIR)/rtk_voip/include/voip_control.h $(VOIPDIR)/includes/voip_control.h	
	-ln -fs $(ROOTDIR)/$(CONFIG_LINUXDIR)/rtk_voip/include/voip_params.h $(VOIPDIR)/includes/voip_params.h	
	-ln -fs $(ROOTDIR)/$(CONFIG_LINUXDIR)/rtk_voip/include/voip_feature.h $(VOIPDIR)/includes/voip_feature.h
endif
	for i in $(dir_v) $(sort $(dir_y)) ; do  make -C $$i || exit $? ; done
romfs:
ifeq ($(CONFIG_USER_MC),y)
	cd mc && make install && make && cd ..
endif
ifeq ($(CONFIG_USER_CTORRENT),y)
	cd torrent && make install && cd ..
endif
	for i in $(dir_v) $(sort $(dir_y)) ; do  make -C $$i romfs || exit $? ; done
ifeq ($(CONFIG_USER_RTK_VOIP),y)
	make -C rtk_voip
endif
%_romfs:
	[ ! -d "$(@:_romfs=)" ] || make -C $(@:_romfs=) romfs


dev:
	make -C $(dir_v) dev
	

%_only:
	[ ! -d "$(@:_only=)" ] || make -C $(@:_only=)

clean:
	-for i in $(dir_v) $(sort $(dir_y) $(dir_n) $(dir_)) ; do \
		[ ! -d $$i ] || make -C $$i clean; \
	done

fullclean:
	echo "Full cleanUP"
	@set -e; for i in $(SUBDIRS_APP); do $(MAKE) -C $$i clean; rm -rfd $$i/filesystem/*; done
	echo "All clean"

prune:
	-for i in $(sort $(dir_n) $(dir_)) ; do \
		found=0; \
		for j in $(sort $(dir_y)) ; do \
			if [ $$i == $$j ]; then \
				found=1; \
			fi; \
		done; \
		if [ $$found == 0 ]; then \
			rm -r $$i; \
		fi; \
	done

boa_compile:
	make -C boa
	make romfs -C boa

boa_clean:
	make -C boa clean

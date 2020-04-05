#
# Makefile for the netfilter modules on top of IPv4.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET := netfilter.o

#export-objs = ip_conntrack_standalone.o ip_fw_compat.o ip_nat_standalone.o ip_tables.o arp_tables.o
# Modified by Mason Yu
export-objs = ip_conntrack_standalone.o ip_conntrack_ftp.o ip_fw_compat.o ip_nat_standalone.o ip_tables.o arp_tables.o ip_conntrack_pptp.o

# Multipart objects.
list-multi		:= ip_conntrack.o iptable_nat.o ipfwadm.o ipchains.o

# objects for the conntrack and NAT core (used by standalone and backw. compat)
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
ip_nf_conntrack-objs    := ip_conntrack_core.o16 ip_conntrack_proto_tcp.o16 ip_conntrack_proto_udp.o16 ip_conntrack_proto_icmp.o16
ip_nf_conntrack-objs    += ip_conntrack_proto_generic.o16 
ip_nf_nat-objs          := ip_nat_core.o16 ip_nat_helper.o16
ip_nf_nat-objs          += ip_nat_proto_unknown.o16 ip_nat_proto_tcp.o16 ip_nat_proto_udp.o16 ip_nat_proto_icmp.o16
else
ip_nf_conntrack-objs	:= ip_conntrack_core.o ip_conntrack_proto_generic.o ip_conntrack_proto_tcp.o ip_conntrack_proto_udp.o ip_conntrack_proto_icmp.o
ip_nf_nat-objs		:= ip_nat_core.o ip_nat_helper.o ip_nat_proto_unknown.o ip_nat_proto_tcp.o ip_nat_proto_udp.o ip_nat_proto_icmp.o
endif

# objects for the standalone - connection tracking / NAT
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
ip_conntrack-objs       := ip_conntrack_standalone.o16 $(ip_nf_conntrack-objs)
iptable_nat-objs        := ip_nat_standalone.o16 ip_nat_rule.o16 $(ip_nf_nat-objs)
else
ip_conntrack-objs	:= ip_conntrack_standalone.o $(ip_nf_conntrack-objs)
iptable_nat-objs	:= ip_nat_standalone.o ip_nat_rule.o $(ip_nf_nat-objs)
endif

# objects for backwards compatibility mode
ip_nf_compat-objs	:= ip_fw_compat.o ip_fw_compat_redir.o ip_fw_compat_masq.o $(ip_nf_conntrack-objs) $(ip_nf_nat-objs)

ipfwadm-objs		:= $(ip_nf_compat-objs) ipfwadm_core.o
ipchains-objs		:= $(ip_nf_compat-objs) ipchains_core.o

# connection tracking
obj-$(CONFIG_IP_NF_CONNTRACK) += ip_conntrack.o
 
# H.323 support
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_H323) += ip_conntrack_h323.o16
ifdef CONFIG_IP_NF_H323
        export-objs += ip_conntrack_h323.o
endif
obj-$(CONFIG_IP_NF_NAT_H323) += ip_nat_h323.o16
else
obj-$(CONFIG_IP_NF_H323) += ip_conntrack_h323.o
ifdef CONFIG_IP_NF_H323
	export-objs += ip_conntrack_h323.o
endif
obj-$(CONFIG_IP_NF_NAT_H323) += ip_nat_h323.o
endif


# connection tracking helpers

# rtsp protocol support
obj-$(CONFIG_IP_NF_RTSP) += ip_conntrack_rtsp.o
ifdef CONFIG_IP_NF_NAT_RTSP
       export-objs += ip_conntrack_rtsp.o
endif
obj-$(CONFIG_IP_NF_NAT_RTSP) += ip_nat_rtsp.o

obj-$(CONFIG_IP_NF_QUAKE3) += ip_conntrack_quake3.o
ifdef CONFIG_IP_NF_NAT_QUAKE3
	export-objs += ip_conntrack_quake3.o
endif

ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_FTP) += ip_conntrack_ftp.o16
ifdef CONFIG_IP_NF_NAT_FTP
        export-objs += ip_conntrack_ftp.o
endif
else
obj-$(CONFIG_IP_NF_FTP) += ip_conntrack_ftp.o
ifdef CONFIG_IP_NF_NAT_FTP
	export-objs += ip_conntrack_ftp.o
endif
endif

# Added by Mason Yu for TFTP ALG
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_TFTP) += ip_conntrack_tftp.o16
ifdef CONFIG_IP_NF_NAT_TFTP
        export-objs += ip_conntrack_tftp.o
endif
else
obj-$(CONFIG_IP_NF_TFTP) += ip_conntrack_tftp.o
ifdef CONFIG_IP_NF_NAT_TFTP
	export-objs += ip_conntrack_tftp.o
endif
endif

obj-$(CONFIG_IP_NF_IRC) += ip_conntrack_irc.o
ifdef CONFIG_IP_NF_NAT_IRC
	export-objs += ip_conntrack_irc.o
endif
# Added by Mason Yu
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_PPTP) += ip_conntrack_pptp.o16
obj-$(CONFIG_IP_NF_IPSEC) += ip_conntrack_ipsec.o16
obj-$(CONFIG_IP_NF_SIP) += ip_conntrack_sip.o16
ifdef CONFIG_IP_NF_SIP
        export-objs += ip_conntrack_sip.o
endif
else
obj-$(CONFIG_IP_NF_PPTP) += ip_conntrack_pptp.o
obj-$(CONFIG_IP_NF_IPSEC) += ip_conntrack_ipsec.o
obj-$(CONFIG_IP_NF_SIP) += ip_conntrack_sip.o
ifdef CONFIG_IP_NF_SIP
	export-objs += ip_conntrack_sip.o
endif
endif

# NAT helpers 
obj-$(CONFIG_IP_NF_NAT_CUSEEME) += ip_nat_cuseeme.o
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_NAT_FTP) += ip_nat_ftp.o16
else
obj-$(CONFIG_IP_NF_NAT_FTP) += ip_nat_ftp.o
endif
# Added by Mason Yu for TFTP ALG
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_NAT_TFTP) += ip_nat_tftp.o16
else
obj-$(CONFIG_IP_NF_NAT_TFTP) += ip_nat_tftp.o
endif
obj-$(CONFIG_IP_NF_NAT_IRC) += ip_nat_irc.o
obj-$(CONFIG_IP_NF_NAT_QUAKE3) += ip_nat_quake3.o
# Added by Mason Yu
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_NAT_PPTP) += ip_nat_pptp.o16
obj-$(CONFIG_IP_NF_NAT_L2TP) += ip_nat_l2tp.o16
obj-$(CONFIG_IP_NF_NAT_IPSEC) += ip_nat_ipsec.o16
obj-$(CONFIG_IP_NF_NAT_SIP) += ip_nat_sip.o16
else
obj-$(CONFIG_IP_NF_NAT_PPTP) += ip_nat_pptp.o
obj-$(CONFIG_IP_NF_NAT_L2TP) += ip_nat_l2tp.o
obj-$(CONFIG_IP_NF_NAT_IPSEC) += ip_nat_ipsec.o
obj-$(CONFIG_IP_NF_NAT_SIP) += ip_nat_sip.o
endif

# generic IP tables 
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_IPTABLES) += ip_tables.o16
else
obj-$(CONFIG_IP_NF_IPTABLES) += ip_tables.o
endif

# the three instances of ip_tables
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_FILTER) += iptable_filter.o16
obj-$(CONFIG_IP_NF_MANGLE) += iptable_mangle.o16
obj-$(CONFIG_IP_NF_NAT) += iptable_nat.o
else
obj-$(CONFIG_IP_NF_FILTER) += iptable_filter.o
obj-$(CONFIG_IP_NF_MANGLE) += iptable_mangle.o
obj-$(CONFIG_IP_NF_NAT) += iptable_nat.o
endif

# matches
obj-$(CONFIG_IP_NF_MATCH_HELPER) += ipt_helper.o
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_MATCH_LIMIT) += ipt_limit.o16
else
obj-$(CONFIG_IP_NF_MATCH_LIMIT) += ipt_limit.o
endif
obj-$(CONFIG_IP_NF_MATCH_IPLIMIT) += ipt_iplimit.o
obj-$(CONFIG_IP_NF_MATCH_IPRANGE) += ipt_iprange.o
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_MATCH_MARK) += ipt_mark.o16
obj-$(CONFIG_IP_NF_MATCH_MAC) += ipt_mac.o16
else
obj-$(CONFIG_IP_NF_MATCH_MARK) += ipt_mark.o
obj-$(CONFIG_IP_NF_MATCH_MAC) += ipt_mac.o
endif

obj-$(CONFIG_IP_NF_MATCH_CONNLIMIT) += ipt_connlimit.o
obj-$(CONFIG_IP_NF_MATCH_UDPLIMIT) += ipt_udplimit.o


obj-$(CONFIG_IP_NF_MATCH_PKTTYPE) += ipt_pkttype.o
obj-$(CONFIG_IP_NF_MATCH_MULTIPORT) += ipt_multiport.o
obj-$(CONFIG_IP_NF_MATCH_OWNER) += ipt_owner.o
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_MATCH_TOS) += ipt_tos.o16
else
obj-$(CONFIG_IP_NF_MATCH_TOS) += ipt_tos.o
endif
obj-$(CONFIG_IP_NF_MATCH_ECN) += ipt_ecn.o
obj-$(CONFIG_IP_NF_MATCH_DSCP) += ipt_dscp.o
obj-$(CONFIG_IP_NF_MATCH_AH_ESP) += ipt_ah.o ipt_esp.o

obj-$(CONFIG_IP_NF_MATCH_LENGTH) += ipt_length.o

obj-$(CONFIG_IP_NF_MATCH_TTL) += ipt_ttl.o
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_MATCH_STATE) += ipt_state.o16
else
obj-$(CONFIG_IP_NF_MATCH_STATE) += ipt_state.o
endif
obj-$(CONFIG_IP_NF_MATCH_CONNTRACK) += ipt_conntrack.o
obj-$(CONFIG_IP_NF_MATCH_UNCLEAN) += ipt_unclean.o
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_MATCH_STRING) += ipt_string.o16
obj-$(CONFIG_IP_NF_MATCH_TCPMSS) += ipt_tcpmss.o16
else
obj-$(CONFIG_IP_NF_MATCH_STRING) += ipt_string.o
obj-$(CONFIG_IP_NF_MATCH_TCPMSS) += ipt_tcpmss.o
endif
obj-$(CONFIG_IP_NF_MATCH_LAYER7) += ipt_layer7.o
obj-$(CONFIG_IP_NF_MATCH_IPP2P) += ipt_ipp2p.o

obj-$(CONFIG_IP_NF_MATCH_PHYSDEV) += ipt_physdev.o

# targets
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_TARGET_REJECT) += ipt_REJECT.o16
obj-$(CONFIG_IP_NF_TARGET_MIRROR) += ipt_MIRROR.o16
else
obj-$(CONFIG_IP_NF_TARGET_REJECT) += ipt_REJECT.o
obj-$(CONFIG_IP_NF_TARGET_MIRROR) += ipt_MIRROR.o
endif
obj-$(CONFIG_IP_NF_TARGET_TOS) += ipt_TOS.o
obj-$(CONFIG_IP_NF_TARGET_ECN) += ipt_ECN.o
obj-$(CONFIG_IP_NF_TARGET_DSCP) += ipt_DSCP.o
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_TARGET_MARK) += ipt_MARK.o16
obj-$(CONFIG_IP_NF_TARGET_MASQUERADE) += ipt_MASQUERADE.o16
obj-$(CONFIG_IP_NF_TARGET_REDIRECT) += ipt_REDIRECT.o16
else
obj-$(CONFIG_IP_NF_TARGET_MARK) += ipt_MARK.o
obj-$(CONFIG_IP_NF_TARGET_IMQ) += ipt_IMQ.o
obj-$(CONFIG_IP_NF_TARGET_MASQUERADE) += ipt_MASQUERADE.o
obj-$(CONFIG_IP_NF_TARGET_REDIRECT) += ipt_REDIRECT.o
endif
obj-$(CONFIG_IP_NF_NAT_SNMP_BASIC) += ip_nat_snmp_basic.o
obj-$(CONFIG_IP_NF_TARGET_LOG) += ipt_LOG.o
obj-$(CONFIG_IP_NF_TARGET_ULOG) += ipt_ULOG.o
ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
obj-$(CONFIG_IP_NF_TARGET_TCPMSS) += ipt_TCPMSS.o16
else
obj-$(CONFIG_IP_NF_TARGET_TCPMSS) += ipt_TCPMSS.o
endif
obj-$(CONFIG_IP_NF_TARGET_TRIGGER) += ipt_TRIGGER.o

# generic ARP tables
obj-$(CONFIG_IP_NF_ARPTABLES) += arp_tables.o

# just filtering instance of ARP tables for now
obj-$(CONFIG_IP_NF_ARPFILTER) += arptable_filter.o

# backwards compatibility 
obj-$(CONFIG_IP_NF_COMPAT_IPCHAINS) += ipchains.o
obj-$(CONFIG_IP_NF_COMPAT_IPFWADM) += ipfwadm.o

obj-$(CONFIG_IP_NF_QUEUE) += ip_queue.o

include $(TOPDIR)/Rules.make

ip_conntrack.o: $(ip_conntrack-objs)
	$(LD) -r -o $@ $(ip_conntrack-objs)

iptable_nat.o: $(iptable_nat-objs)
	$(LD) -r -o $@ $(iptable_nat-objs)

ipfwadm.o: $(ipfwadm-objs)
	$(LD) -r -o $@ $(ipfwadm-objs)

ipchains.o: $(ipchains-objs)
	$(LD) -r -o $@ $(ipchains-objs)

# Generated automatically from Makefile.in by configure.  

#KERNEL_HEADERS=-I$(ROOTDIR)/linux-2.4.x

ifndef UCLINUX_BUILD_USER
LDFLAGS = -LLINUX
LIBCRYPT = -lcrypt
endif

GCC_FLAGS = -Os -finline-small-functions -Wstrict-prototypes -Wpointer-arith -Wcast-align -Wcast-qual\
  -Wtraditional\
  -Wshadow\
  -Wconversion\
  -Waggregate-return\
  -Wmissing-prototypes\
  -Wnested-externs\
  -Wall

# Add -DNO_DAEMON if you do not want the server to fork upon launch
#LDFLAGS = -lnsl 
#CFLAGS = -O -DLINUX -I$(ROOTDIR)/lib/include -I$(ROOTDIR)/lib/include/sys -I$(ROOTDIR)/$(LINUXDIR)/include -I$(ROOTDIR)/$(LINUXDIR)/include/asm-mips -I$(ROOTDIR)/include -I. -I./rtl865x 

#CFLAGS += -DSERVER_SSL -I$(ROOTDIR)/lib/libssl/include
#SSLLIB =$(ROOTDIR)/lib/libssl/libssl.a $(ROOTDIR)/lib/libssl/libcrypto.a
#LDFLAGS	+= $(SSLLIB) $(IFMODLIB)
MANDIR = /usr/local/man
BINDIR = /usr/local/sbin

# FreeBSD's make doesn't like `ifdef`, but likes `.ifdef`
ifdef INET6
LDFLAGS += -L/usr/inet6/lib -linet6
CFLAGS += -DINET6 -I/usr/inet6/include
endif

###### Merge for VoIP project ######
ifeq ($(CONFIG_USER_RTK_VOIP),y)
-include $(ROOTDIR)/$(LINUXDIR)/.config
ifdef CONFIG_RTK_VOIP
PWD := $(shell pwd)
TOPDIR := $(shell pwd)/../../..
VOIP_APP = $(TOPDIR)/user/rtk_voip
CFLAGS += -DVOIP_SUPPORT -I$(VOIP_APP)/includes -I$(VOIP_APP)/web \
	-I$(PWD) -I$(PWD)/..
VOIPLIB=$(VOIP_APP)/web/libweb_voip.a
endif
################end#################
endif


## --- HTTPS server support --- ##
ifeq ($(CONFIG_USER_BOA_WITH_SSL),y)
# ---- HTTPS with Matrix SSL library ---
ifeq ($(CONFIG_USER_MATRIXSSL),y)
LIBMATRIXSSL_INC_DIR=$(ROOTDIR)/lib/libmatrixssl/
CFLAGS +=  -DLINUX -I$(LIBMATRIXSSL_INC_DIR)
LDFLAGS += -L$(ROOTDIR)/lib/libmatrixssl/
SSLLIB = -lmatrixssl
else
LIBSSL_INC_DIR=$(ROOTDIR)/lib/libssl/include/
CFLAGS +=  -DLINUX -I$(LIBSSL_INC_DIR)
SSLLIB = -lssl -lcrypto
endif
endif
# --- End of HTTPS ---

# Change these if necessary

YACC = bison -y 
LEX = flex 

SUBDIRS=LINUX
WEBLIB=LINUX/libwebs.a

SOURCES = alias.c boa.c cgi.c config.c get.c hash.c log.c \
    queue.c read.c request.c response.c signals.c util.c \
    cgi_header.c pipe.c nls.c auth.c md5.c globals.c \
    asp_page.c port.c msgutil.c

ifneq ($(CONFIG_GUI_WEB),y)
PAGE_ADMIN_DEP1 = mtmcode.js share.js acl.asp date.asp fw-macfilter.asp parental-ctrl.asp reboot.asp \
	saveconf.asp status.asp title.html upload.asp upload2.asp upload3.asp toupgrade.html index2.html wladvanced.asp \
	wlbasic.asp wlstatbl.asp wlwpa.asp wlwep.asp wlactrl.asp wlwds.asp wlsurvey.asp wlwps.asp connlimit.asp 
# add for 11n wagem logo

else
PAGE_ADMIN_DEP1 = stm31.js css.css share.js status_pl.asp\
	acl.asp date.asp fw-macfilter.asp parental-ctrl.asp reboot.asp \
	saveconf.asp status.asp title.asp upload.asp upload2.asp upload3.asp toupgrade.html index2.html wladvanced.asp \
	wlbasic.asp wlstatbl.asp wlwpa.asp wlwep.asp wlactrl.asp wlwds.asp wlsurvey.asp wlwps.asp connlimit.asp 
# add for 11n wagem logo

endif

ifneq ($(CONFIG_GUI_WEB),y)
PAGE_ADMIN_DEP2 = graphics/del.gif graphics/edit.gif 	graphics/sagemlogo2.gif graphics/sagemlogo1.gif \
	graphics/goahead_logo.gif graphics/topbar.gif menu-images/menu_bar.gif \
	menu-images/menu_corner.gif menu-images/menu_corner_minus.gif \
	menu-images/menu_corner_plus.gif menu-images/menu_folder_closed.gif \
	menu-images/menu_folder_open.gif menu-images/menu_link_default.gif \
	menu-images/menu_pixel.gif menu-images/menu_root.gif \
	menu-images/menu_tee.gif menu-images/menu_tee_minus.gif \
	menu-images/menu_tee_plus.gif
# add for 11n wagem logo

else
PAGE_ADMIN_DEP2 = graphics/del.gif graphics/edit.gif \
	images/bg1.gif images/bg2.gif images/bg3.gif images/bg4.gif \
	images/blank.gif images/logo.gif images/spacer.gif images/arrow_r.gif \
	images/img-admin-a.gif images/img-advance-a.gif \
	images/img-admin-b.gif images/img-advance-b.gif \
	images/img-firewall-a.gif images/img-internet-a.gif \
	images/img-firewall-b.gif images/img-internet-b.gif \
	images/img-lan-a.gif images/img-setup-a.gif \
	images/img-lan-b.gif images/img-setup-b.gif \
	images/img-status-a.gif images/img-wireless-a.gif \
	images/img-status-b.gif images/img-wireless-b.gif \
	images/left_disabled.gif images/left_disabled.gif \
	images/right_disabled.gif images/right_disabled.gif \
	images/line.gif images/bottomline.gif \
	images/topright.gif images/topleft.gif \
	images/straightright.gif images/straightleft.gif \
	images/bottomright.gif images/bottomleft.gif \
	images/btn-commitreboot.gif images/config-bg.gif images/conn-top-bg.gif\
	images/config-content-bg.gif images/config-content-btm.gif \
	images/config-content-titlebarbg.gif images/config-content-top.gif \
	images/config-login-admin.gif images/config-login-lock.gif \
	images/config-tabtop-bg.gif images/conn-bg.gif \
	images/conn-btn-abort.gif images/conn-btn-abort-over.gif \
	images/conn-btn-again.gif images/conn-btn-again-over.gif \
	images/conn-dialog-bg.gif images/conn-dialog-bgbtm.gif \
	images/conn-dialog-top.gif images/connect_fail.gif \
	images/conn-notice-bg.gif images/conn-prolink-logo.png
endif

# SOURCES += LINUX/fmget.c LINUX/mibtbl.c LINUX/mib.c LINUX/rtl_flashdrv.c \
#   LINUX/adsl_drv.c LINUX/utility.c LINUX/fmfwall.c 
	
OBJS = y.tab.o lex.yy.o ${SOURCES:.c=.o} timestamp.o

.PHONY: clean mrclean depend all
 
ifdef UCLINUX_BUILD_USER
all:	${OBJS} subdir boa
else
all:	${OBJS} subdir boa webs_link
endif

subdir:
ifeq ($(CONFIG_GUI_WEB),y)
	cp -f $(shell pwd)/web_default/*.asp $(shell pwd)/web_gui/
	cp -f $(shell pwd)/web_default/share.js $(shell pwd)/web_gui/
	cp -f $(shell pwd)/web_default/toupgrade.html $(shell pwd)/web_gui/
	cp -f $(shell pwd)/web_default/index2.html $(shell pwd)/web_gui/
	cp -f $(shell pwd)/web_default/adv/adsl-statis.asp $(shell pwd)/web_gui/adv/
	cp -f $(shell pwd)/web_default/adv/adsl-set.asp $(shell pwd)/web_gui/adv/
	cp -f $(shell pwd)/web_default/adv/adsl-diag.asp $(shell pwd)/web_gui/adv/
	cp -f $(shell pwd)/web_default/graphics/del.gif $(shell pwd)/web_gui/graphics/
	cp -f $(shell pwd)/web_default/graphics/edit.gif $(shell pwd)/web_gui/graphics/
	for i in $(PAGE_ADMIN_DEP1) ; do \
		[ -e web/admin/$$i ] || ln -s ../$$i web/admin/$$i; \
	done
	for i in $(PAGE_ADMIN_DEP2) ; do \
		[ -e web/admin/$$i ] || ln -s ../../$$i web/admin/$$i; \
	done
endif
	for x in $(SUBDIRS); do (cd $$x && $(MAKE) ); done

boa:	${OBJS} $(WEBLIB)
	${CC} $(LDFLAGS) -o $@ ${OBJS} ${WEBLIB} $(MIBLIB) $(LIBCRYPT) $(LDLIBS$(LDLIBS-$(@))) $(VOIPLIB) -lpthread -lmib $(SSLLIB)
webs_link:
	for i in $(PAGE_ADMIN_DEP1) ; do \
		[ -e web/admin/$$i ] || ln -s ../$$i web/admin/$$i; \
	done
	for i in $(PAGE_ADMIN_DEP2) ; do \
		[ -e web/admin/$$i ] || ln -s ../../$$i web/admin/$$i; \
	done

clean:
	find . -name '*.o' | xargs rm -f
	rm -f boa
	for i in $(PAGE_ADMIN_DEP1) $(PAGE_ADMIN_DEP2); do \
		rm -f web/admin/$$i; \
	done
	rm -f 
	for x in $(SUBDIRS); do (cd $$x && $(MAKE) clean ); done
#	rm -f ${OBJS} boa core lex.yy.c y.tab.c y.tab.h rtl865x/*.o LINUX/*.o LINUX/*.a
	
mrclean:	clean
	rm -f config.status config.cache config.h Makefile config.log

# parser dependencies

y.tab.c y.tab.h:	boa_grammar.y
	${YACC} -d boa_grammar.y

lex.yy.c:	boa_lexer.l
	${LEX} boa_lexer.l

#escape.h:	escape.pl
#	perl escape.pl >escape.h

%.d: %.c
	$(CC) -E $(CFLAGS) $(EXTRACFLAGS) -Wp,-MD,$*.d $< >/dev/null
	
depend:
	rm *.o rtl865x/*.o -f
	${CC} $(CFLAGS) -E -MM ${SOURCES} > .depend
	
-include .depend

timestamp.c:	${SOURCES} boa_grammar.y boa_lexer.l
	touch timestamp.c

install:	boa
	install -c -m 755 boa ${BINDIR} 
	install -c -m 755 ../utils/boa_indexer ${BINDIR}
	install -c -m 644 ../docs/boa.1 ${MANDIR}/man1

SOCKET = socket localhost 80
ECHO = /bin/echo

.PHONY: test dne cgi_bin dir normal

test:	dne cgi_bin dir normal

dne:
	${ECHO} 'GET /file_that_does_not_exit HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'GET /file_that_does_not_exit\n' | ${SOCKET}
	${ECHO} 'HEAD /file_that_does_not_exit HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'HEAD /file_that_does_not_exit\n' | ${SOCKET}

cgi_bin:
	${ECHO} 'GET /cgi-bin/cgi-test.cgi HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'GET /cgi-bin/cgi-test.cgi\n' | ${SOCKET}
	${ECHO} 'HEAD /cgi-bin/cgi-test.cgi HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'HEAD /cgi-bin/cgi-test.cgi\n' | ${SOCKET}

DIR = /doc/

dir:
	${ECHO} 'GET ${DIR} HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'GET ${DIR}\n' | ${SOCKET}
	${ECHO} 'HEAD ${DIR} HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'HEAD ${DIR}\n' | ${SOCKET}

NORMAL = /test_file.gz

normal:
	${ECHO} 'GET ${NORMAL} HTTP/1.0\n\n' | ${SOCKET} | wc -c
	${ECHO} 'GET ${NORMAL}\n' | ${SOCKET} | wc -c
	${ECHO} 'HEAD ${NORMAL} HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'HEAD ${NORMAL}' | ${SOCKET}

GZ = /test_file

# Note: to test automatic gunzipping a file named "test_file.gz" must be in
# the document root.

gz:	
	${ECHO} 'GET ${GZ} HTTP/1.0\n\n' | ${SOCKET} | wc -c
	${ECHO} 'GET ${GZ}\n' | ${SOCKET} | wc -c
	${ECHO} 'HEAD ${GZ} HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'HEAD ${GZ}\n' | ${SOCKET}


romfs:
	for x in $(SUBDIRS); do make -C $$x $@; done
	$(ROMFSINST) /bin/boa
	$(ROMFSINST) /home/httpd/boa.conf
#ifeq ($(CONFIG_GUI_WEB),y)
#	$(ROMFSINST) $(TOP_DIR)/user/boa/src/web /home/httpd/web
#endif
ifdef CONFIG_RTK_VOIP	
	$(ROMFSINST) $(VOIP_APP)/web/voip_script.js /home/httpd/web
	$(ROMFSINST) $(VOIP_APP)/web/voip_general.asp /home/httpd/web
	$(ROMFSINST) $(VOIP_APP)/web/voip_tone.asp /home/httpd/web
	$(ROMFSINST) $(VOIP_APP)/web/voip_ring.asp /home/httpd/web
	$(ROMFSINST) $(VOIP_APP)/web/voip_other.asp /home/httpd/web
	$(ROMFSINST) $(VOIP_APP)/web/voip_config.asp /home/httpd/web
	$(ROMFSINST) $(VOIP_APP)/web/voip_fxo.asp /home/httpd/web
ifeq ($(CONFIG_RTK_VOIP_IVR),y)
	$(ROMFSINST) $(VOIP_APP)/web/voip_ivr_req.asp /home/httpd/web
endif
	$(ROMFSINST) $(VOIP_APP)/web/voip_sip_status.asp /home/httpd/web
	$(ROMFSINST) -s /var/config_voip.dat  /home/httpd/web/config_voip.dat
endif
ifeq ($(CONFIG_USER_BOA_WITH_SSL),y)
	$(ROMFSINST) /etc/ssl_key.pem
	$(ROMFSINST) /etc/ssl_cert.pem
endif

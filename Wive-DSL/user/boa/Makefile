.EXPORT_ALL_VARIABLES:
DEFAULT_WEB_ID=default_web_id
GUI_WEB_ID=gui_web_id
ZTE_531B_BRIDGE_WEB_ID=zte_531b_bridge_web_id
ZTE_531B_GENERAL_ROUTER_WEB_ID=zte_531b_router_web_id
ZTE_531B_GENERAL_ROUTER_WEB_EN_ID=zte_531b_router_web_en_id
CONFIG_BOA_WEB_E8B_CH_ID=config_boa_web_e8b_ch_id
SRCNAME=src-project
DIRS = $(SRCNAME)

ifndef UCLINUX_BUILD_USER
include ../../config/.config
endif

all romfs:
ifeq ($(CONFIG_DEFAULT_WEB),y)
ifneq ($(DEFAULT_WEB_ID),$(wildcard $(DEFAULT_WEB_ID)))
	rm -rf $(shell pwd)/$(SRCNAME)/web
	rm -rf $(shell pwd)/$(SRCNAME)
	ln -s $(shell pwd)/src  $(shell pwd)/$(SRCNAME)
	ln -s $(shell pwd)/$(SRCNAME)/web_default $(shell pwd)/$(SRCNAME)/web
	make -C $(shell pwd)/$(SRCNAME) clean
	$(shell touch $(DEFAULT_WEB_ID))
	$(shell rm -rf  $(GUI_WEB_ID))
	$(shell rm -rf  $(ZTE_531B_GENERAL_ROUTER_WEB_ID))
	$(shell rm -rf  $(ZTE_531B_BRIDGE_WEB_ID))
	$(shell rm -rf  $(ZTE_531B_GENERAL_ROUTER_WEB_EN_ID))
	$(shell rm -rf  $(CONFIG_BOA_WEB_E8B_CH_ID))
endif
endif
ifeq ($(CONFIG_GUI_WEB),y)
ifneq ($(GUI_WEB_ID),$(wildcard $(GUI_WEB_ID)))
	rm -rf $(shell pwd)/$(SRCNAME)/web
	rm -rf $(shell pwd)/$(SRCNAME)
	ln -s $(shell pwd)/src  $(shell pwd)/$(SRCNAME)
	ln -s $(shell pwd)/$(SRCNAME)/web_gui $(shell pwd)/$(SRCNAME)/web
	make -C $(shell pwd)/$(SRCNAME) clean
	$(shell rm -rf $(DEFAULT_WEB_ID))
	$(shell touch  $(GUI_WEB_ID))
	$(shell rm -rf  $(ZTE_531B_GENERAL_ROUTER_WEB_ID))
	$(shell rm -rf  $(ZTE_531B_BRIDGE_WEB_ID))
	$(shell rm -rf  $(ZTE_531B_GENERAL_ROUTER_WEB_EN_ID))
	$(shell rm -rf  $(CONFIG_BOA_WEB_E8B_CH_ID))
endif
endif
ifeq ($(ZTE_531B_BRIDGE_SC),y)
ifneq ($(ZTE_531B_BRIDGE_WEB_ID),$(wildcard $(ZTE_531B_BRIDGE_WEB_ID)))
	rm -rf $(shell pwd)/$(SRCNAME)/web
	rm -rf $(shell pwd)/$(SRCNAME)
	ln -s $(shell pwd)/src  $(shell pwd)/$(SRCNAME)
	ln -s $(shell pwd)/$(SRCNAME)/web_zte531b_bridge $(shell pwd)/$(SRCNAME)/web
	make -C $(shell pwd)/$(SRCNAME) clean
	$(shell rm -rf $(DEFAULT_WEB_ID))
	$(shell rm -rf  $(GUI_WEB_ID))
	$(shell rm -rf $(ZTE_531B_GENERAL_ROUTER_WEB_ID))
	$(shell touch  $(ZTE_531B_BRIDGE_WEB_ID))
	$(shell rm -rf  $(ZTE_531B_GENERAL_ROUTER_WEB_EN_ID))
	$(shell rm -rf  $(CONFIG_BOA_WEB_E8B_CH_ID))
endif
endif
ifeq ($(ZTE_GENERAL_ROUTER_SC),y)
ifneq ($(ZTE_531B_GENERAL_ROUTER_WEB_ID),$(wildcard $(ZTE_531B_GENERAL_ROUTER_WEB_ID)))
	rm -rf $(shell pwd)/$(SRCNAME)/web
	rm -rf $(shell pwd)/$(SRCNAME)
	ln -s $(shell pwd)/src  $(shell pwd)/$(SRCNAME)
	ln -s $(shell pwd)/$(SRCNAME)/web_zte531b_router $(shell pwd)/$(SRCNAME)/web
	make -C $(shell pwd)/$(SRCNAME) clean
	$(shell rm -rf $(DEFAULT_WEB_ID))
	$(shell rm -rf  $(GUI_WEB_ID))
	$(shell touch  $(ZTE_531B_GENERAL_ROUTER_WEB_ID))
	$(shell rm -rf  $(ZTE_531B_BRIDGE_WEB_ID))
	$(shell rm -rf  $(ZTE_531B_GENERAL_ROUTER_WEB_EN_ID))
	$(shell rm -rf  $(CONFIG_BOA_WEB_E8B_CH_ID))
endif
endif
ifeq ($(ZTE_GENERAL_ROUTER_EN),y)
ifneq ($(ZTE_531B_GENERAL_ROUTER_WEB_EN_ID),$(wildcard $(ZTE_531B_GENERAL_ROUTER_WEB_EN_ID)))
	rm -rf $(shell pwd)/$(SRCNAME)/web
	rm -rf $(shell pwd)/$(SRCNAME)
	ln -s $(shell pwd)/src  $(shell pwd)/$(SRCNAME)
	ln -s $(shell pwd)/$(SRCNAME)/zte531b_router_en $(shell pwd)/$(SRCNAME)/web
	make -C $(shell pwd)/$(SRCNAME) clean
	$(shell rm -rf $(DEFAULT_WEB_ID))
	$(shell rm -rf  $(GUI_WEB_ID))
	$(shell rm -fr  $(ZTE_531B_GENERAL_ROUTER_WEB_ID))
	$(shell rm -rf  $(ZTE_531B_BRIDGE_WEB_ID))
	$(shell touch  $(ZTE_531B_GENERAL_ROUTER_WEB_EN_ID))
	$(shell rm -rf  $(CONFIG_BOA_WEB_E8B_CH_ID))
endif
endif
ifeq ($(CONFIG_BOA_WEB_E8B_CH),y)
ifneq ($(CONFIG_BOA_WEB_E8B_CH_ID),$(wildcard $(CONFIG_BOA_WEB_E8B_CH_ID)))
	rm -rf $(shell pwd)/$(SRCNAME)/web
	rm -rf $(shell pwd)/$(SRCNAME)
	ln -s $(shell pwd)/src-e8b  $(shell pwd)/$(SRCNAME)
	ln -s $(shell pwd)/$(SRCNAME)/web_e8b_ch $(shell pwd)/$(SRCNAME)/web
	make -C $(shell pwd)/$(SRCNAME) clean
	$(shell rm -rf $(DEFAULT_WEB_ID))
	$(shell rm -rf  $(GUI_WEB_ID))
	$(shell rm -fr  $(ZTE_531B_GENERAL_ROUTER_WEB_ID))
	$(shell rm -rf  $(ZTE_531B_BRIDGE_WEB_ID))
	$(shell rm -rf  $(ZTE_531B_GENERAL_ROUTER_WEB_EN_ID))
	$(shell touch  $(CONFIG_BOA_WEB_E8B_CH_ID))
endif
endif
	for i in $(DIRS) ; do make -C $$i $@ || exit $?; done

clean:
	for i in $(DIRS) ; do make -C $$i clean ; done


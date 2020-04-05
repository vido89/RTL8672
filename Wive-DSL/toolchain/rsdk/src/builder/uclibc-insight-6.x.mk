# Realtek Semiconductor Corp.
#
# insight-6.0 makefile for uclibc
#
# Tony Wu (tonywu@realtek.com.tw)
# Oct. 10, 2005
#

#-----------------------------------------------------------------------------
# build insight for use on the host system
#-----------------------------------------------------------------------------

INSIGHT_DIR:=$(BASE_DIR)/insight-$(INSIGHT_VER)
INSIGHT_DIR1:=$(STAGE_DIR)/insight-$(INSIGHT_VER)-build


$(INSIGHT_DIR1)/.configured: $(BUILD_DIR)/.ulib-make-init
	mkdir -p $(INSIGHT_DIR1)
	(cd $(INSIGHT_DIR1); \
		$(INSIGHT_DIR)/configure \
		--prefix=$(BUILD_DIR) \
		--build=$(BUILD_NAME) \
		--host=$(HOST_NAME) \
		--target=$(TARGET_NAME));
	touch $(INSIGHT_DIR1)/.configured

$(INSIGHT_DIR1)/.compiled: $(INSIGHT_DIR1)/.configured
	$(MAKE) -C $(INSIGHT_DIR1) all
	touch $(INSIGHT_DIR1)/.compiled

$(INSIGHT_DIR1)/.installed: $(INSIGHT_DIR1)/.compiled
	$(MAKE) -C $(INSIGHT_DIR1) install
	touch $(INSIGHT_DIR1)/.installed

insight: $(INSIGHT_DIR1)/.installed

insight-clean:
	rm -f $(BUILD_DIR)/bin/$(TARGET_NAME)*
	-$(MAKE) -C $(INSIGHT_DIR1) clean

insight-dirclean:
	rm -rf $(INSIGHT_DIR1)


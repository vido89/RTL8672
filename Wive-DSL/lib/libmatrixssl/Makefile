
MATRIXSSL_DIR=../../user/cwmp-tr069/matrixssl-1-8-3-open/
LIBRARY=libmatrixssl.so

.PHONY:	all romfs
all:
	if [ -d $(MATRIXSSL_DIR) ]; then \
		$(MAKE) -C $(MATRIXSSL_DIR)/src; \
		cp -f $(MATRIXSSL_DIR)/src/libmatrixssl.so .; \
	fi


romfs:
	$(ROMFSINST) /lib/$(LIBRARY)
	$(ROMFSINST) -s /lib/$(LIBRARY) /lib/$(LIBRARY).0	

clean:
	rm -f *.so
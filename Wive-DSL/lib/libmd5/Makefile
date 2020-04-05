
all: libmd5.so

libmd5.o: libmd5.h libmd5.c
	$(CC) -Os -c -o $@ $(CFLAGS) libmd5.c

libmd5.a: libmd5.o
	$(AR) $(ARFLAGS) $@ $?

libmd5.so: libmd5.a
	$(LD) -shared --warn-common --warn-once -z combreloc -soname=$@.0 \
		-o $@ --whole-archive $?

clean:
	rm -rf libmd5.o libmd5.a libmd5.so

romfs:
	$(ROMFSINST) /lib/libmd5.so
	$(ROMFSINST) -s /lib/libmd5.so /lib/libmd5.so.0

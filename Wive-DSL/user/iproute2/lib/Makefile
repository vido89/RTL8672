CC=mips-linux-gcc

UTILOBJ=utils.o rt_names.o ll_types.o ll_proto.o ll_addr.o inet_proto.o

NLOBJ=ll_map.o libnetlink.o
CFLAGS+=-fPIC -DPIC

all: libnetlink.a libutil.a libnetlink.so

libnetlink.a: $(NLOBJ)
	$(AR) rcs $@ $(NLOBJ)

libutil.a: $(UTILOBJ) $(ADDLIB)
	$(AR) rcs $@ $(UTILOBJ) $(ADDLIB)

libnetlink.so: $(NLOBJ)
	$(CC) -shared -fPIC -DPIC -o $@ $(NLOBJ)

install:

romfs:
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_TC_TC /lib/libnetlink.so

clean:
	rm -f $(NLOBJ) $(UTILOBJ) $(ADDLIB) libnetlink.a libutil.a
	rm -f *.so

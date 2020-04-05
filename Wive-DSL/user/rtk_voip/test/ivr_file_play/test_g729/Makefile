CC = mips-linux-gcc
CFLAGS = -I../../../includes/

CFLAGS += -DCONFIG_RTK_VOIP_IVR

all: 
	$(CC) $(CFLAGS) -o test_g729.o test_g729.c

nfs:
	cp -f test_g729.o /work/nfs/ivr/
	cp -f 729_raw /work/nfs/ivr/

clean:
	rm -rf *.o

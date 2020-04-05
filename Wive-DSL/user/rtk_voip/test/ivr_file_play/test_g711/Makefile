CC = mips-linux-gcc
CFLAGS = -I../../../includes/

CFLAGS += -DCONFIG_RTK_VOIP_IVR

all: 
	$(CC) $(CFLAGS) -o test_g711.o test_g711.c

nfs:
	cp -f test_g711.o /work/nfs/ivr/
	cp -f 711_raw /work/nfs/ivr/

clean:
	rm -rf *.o

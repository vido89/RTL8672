CC = mips-linux-gcc
CFLAGS = -I../../../includes/


CFLAGS += -DCONFIG_RTK_VOIP_IVR

all: 
	$(CC) $(CFLAGS) -o test_g723.o test_g723.c -lpthread

nfs:
	cp -f test_g723.o /work/nfs/ivr/
	cp -f 723_raw /work/nfs/ivr/

clean:
	rm -rf *.o

PRE = mips-uclibc-
CC = $(PRE)gcc
NM = $(PRE)nm
STRIP = $(PRE)strip
CFLAGS = -lpthread -static -DUSE_UDP
#CFLAGS = -lpthread -static -DTWO_THREAD
#CFLAGS = -lpthread -static
CFLAGS_SERVER = $(CFLAGS) -DTALK_SERVER

all: udptalk

udptalk: talk.c talk.h
	$(CC) -o $@ talk.c $(CFLAGS_SERVER)
#	$(NM) $@ | sort > talksvr.map
	$(STRIP) $@
#	cp $@ /root/nfs/.


clean:
	rm -f udptalk
	rm -f *.map

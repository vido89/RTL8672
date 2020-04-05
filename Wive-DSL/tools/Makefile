CC=gcc
STRIP=strip

all :	install

install :
	make -C squashfs
	make -C libstrip
	make -C sstrip
	make -C packimg

clean:
	make -C squashfs clean
	make -C sstrip clean
	make -C packimg clean

romfs: install
	echo "OK"
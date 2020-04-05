CC       = rsdk-elf-gcc
AR       = rsdk-elf-ar
OBJDUMP  = rsdk-elf-objdump -S -D

CFLAGS   = -c -G0 -O1 -g -msoft-float -march=4180 -I.

all: liblx4080.a

liblx4080.a: lx4080.o
	$(AR) r liblx4080.a lx4080.o

lx4080.o: lx4080.c lx4080.h 
	$(CC) -D__ASM $(CFLAGS) -o $@ lx4080.c
	$(OBJDUMP) $@ > $(basename $@).text

.PHONY:	clean all
clean:
	/bin/rm -f *.o *.text *.hex liblx4080.a



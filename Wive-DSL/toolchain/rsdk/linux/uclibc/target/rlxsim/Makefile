CC       = rsdk-elf-gcc
AR       = rsdk-elf-ar
OBJDUMP  = rsdk-elf-objdump -S -D

CFLAGS   = -c -G0 -O1 -g -msoft-float -mcpu=4180 -I. -I./include -I../../library/include

all: librlxsim_gdb.a

librlxsim_gdb.a: rlxsim_newlib.o cache_ctrl.o
	$(AR) r librlxsim_gdb.a rlxsim_newlib.o  cache_ctrl.o

rlxsim_fileio.o: rlxsim_fileio.S rlxsim_fileio.h 
	$(CC) -D__ASM -x assembler-with-cpp $(CFLAGS) -o $@ rlxsim_fileio.S
	$(OBJDUMP) $@ > $(basename $@).text

cache_ctrl.o: cache_ctrl.S cache_ctrl.h 
	$(CC) -D__ASM -x assembler-with-cpp $(CFLAGS) -o $@ cache_ctrl.S
	$(OBJDUMP) $@ > $(basename $@).text

rlxsim_newlib: rlxsim_newlib.c rlxsim_newlib.h 
	$(CC) $(CFLAGS) -o $@ rlxsim_newlib.c
	$(OBJDUMP) $@ > $(basename $@).text

.PHONY:	clean all
clean:
	/bin/rm -f *.o *.text *.hex librlxsim_gdb.a



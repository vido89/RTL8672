#
# Realtek Semiconductor Corp.
#
# RSDK GDB Remote IO Library 
#
# Tony Wu (tonywu@realtek.com.tw)
# Jul. 07, 2006
#

#//////////////////////////////////////////////
ARCH = 4180
PROGRAM = testgdbio
RSDKDIR = /rsdk/rsdk-1.2.7/linux/newlib

CC = $(RSDKDIR)/bin/rsdk-elf-gcc
LD = $(RSDKDIR)/bin/rsdk-elf-ld
 
CFLAGS = -march=$(ARCH) -c -G0 -fno-pic
IFLAGS = -I$(RSDKDIR)/include
LFLAGS = -Ttext 80000000 -e start -N -n
LIBS = -L$(RSDKDIR)/lib/$(ARCH) -lrlx -lm -lc -lgcc -L. -lrlxsim_gdb

#//////////////////////////////////////////////
all: $(PROGRAM)

.S.o:
	$(CC) -D__ASM__ -x assembler-with-cpp $(CFLAGS) $(IFLAGS) $<

.c.o:
	$(CC) $(CFLAGS) $(IFLAGS) $<

testgdbio: c.o gdbio.o
	$(LD) -T target-rlxsim.ld -o testgdbio $(LFLAGS) c.o gdbio.o $(LIBS)

clean:
	rm -f *.o $(PROGRAM)

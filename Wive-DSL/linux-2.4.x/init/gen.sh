mips-linux-gcc -D__KERNEL__ -I/home/mips/linux-2.4.19/include -Wall -Wstrict-prototypes -Wno-trigraphs -O1 -gdwarf-2 -fno-strict-aliasing -fno-common -DNO_MM -I /home/mips/linux-2.4.19/include/asm/gcc -G 0 -mno-abicalls -fno-pic -pipe -mcpu=r3000 -mips1   -DKBUILD_BASENAME=do_mounts -c -o do_mounts.o do_mounts.c


mips-linux-gcc -D__KERNEL__ -I/home/mips/linux-2.4.19/include -Wall -Wstrict-prototypes -Wno-trigraphs -O1 -gdwarf-2 -fno-strict-aliasing -fno-common -DNO_MM -I /home/mips/linux-2.4.19/include/asm/gcc -G 0 -mno-abicalls -fno-pic -pipe -mcpu=r3000 -mips1   -DKBUILD_BASENAME=do_mounts -S main.c 

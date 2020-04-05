# RTL8672

Here you can find RTK for RTL8670, RTL8671 and RTL8672 cpu's

You need to setup i386 chroot environment https://wiki.debian.org/chroot

Make sure you place RTK it in `/opt/Wive-DSL` and make sure that you have those 2 symlinks
```
$ ln -s Wive-DSL Wive-DSL-2.6
$ ln -s Wive-DSL Wive
```

You need to isntall those packages 
```
apt-get	install git gcc binutils bzip2 flex python perl make \
grep diffutils unzip gawk getopt subversion libz-dev \
libc-dev ncurses-dev g++ bison
```
If you need clean build then
```
git clean -fdx; git clean -fdX; git reset --hard
```
Do not select `Build libncurses`

And just run
```
PATH=/opt/Wive-DSL/toolchain/bin:$PATH ./compile
```

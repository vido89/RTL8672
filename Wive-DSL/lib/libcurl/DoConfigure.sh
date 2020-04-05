#!/bin/bash

ROOTDIR=`pwd`
LIBDIR=$FIRMROOT/lib
CONFOPTS="--host=mips --with-ssl=$LIBDIR/libssl --without-random" 
CONFOPTS="$CONFOPTS --prefix=$ROOTDIR/filesystem"
CC=mips-linux-gcc
CFLAGS="-s -Os -I$LIBDIR/libssl/include/openssl -I$LIBDIR/zlib" 
LDFLAGS="-s -Os -L$LIBDIR/libssl -L$LIBDIR/zlib "
export CC CFLAGS LDFLAGS
./configure $CONFOPTS

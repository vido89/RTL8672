#!/bin/bash

ROOTDIR=`pwd`
CONFOPTS="--host=mips-linux --build=i686-pc-linux-gnu --disable-gtk --disable-nls"
CONFOPTS="$CONFOPTS --prefix=$ROOTDIR/filesystem"

./configure $CONFOPTS

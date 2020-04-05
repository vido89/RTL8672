#!/bin/bash

ROOTDIR=`pwd`
rm -rf ./filesystem/*
CONFOPTS="--host=mips --without-subshell --with-terminfo --disable-nls"
CONFOPTS="$CONFOPTS --prefix=$ROOTDIR/filesystem"
./configure $CONFOPTS

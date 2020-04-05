#!/bin/bash

ROOTDIR=`pwd`
LIBDIR=$FIRMROOT/lib
CONFOPTS="--host=mips" 
CONFOPTS="$CONFOPTS --prefix=$ROOTDIR/filesystem"
./configure $CONFOPTS

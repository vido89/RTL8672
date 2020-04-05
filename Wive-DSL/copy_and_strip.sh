#!/bin/sh

RO_ROOT=romfs
STRIP=./toolchain/bin/mips-linux-strip
SSTRIP=./tools/sstrip/sstrip

echo ---------------------------------STRIP-LIB-SDK-------------------------------
./tools/libstrip/libstrip
#$STRIP $RO_ROOT/lib/*.a
$SSTRIP $RO_ROOT/lib/*.so
echo --------------------------------STRIP AND SSTRIP-----------------------------
echo Strip binary files
NON_STRIPS_BIN=`find $RO_ROOT/bin -type f -print -exec file {} \; | grep -v "modules" | grep -v "icon" | grep -v "rc" | grep -v "service" | grep -v "++" | grep -v ".sh" | cut -d":" -f1`
NON_STRIPS_LIB=`find $RO_ROOT/lib -type f -print -exec file {} \; | grep "not stripped" | grep -v "modules" | cut -d":" -f1`

if [ "$NON_STRIPS_BIN" != "" ]; then
  echo BIN: $NON_STRIPS_BIN
  $STRIP $NON_STRIPS_BIN
  $SSTRIP $NON_STRIPS_BIN
fi
if [ "$NON_STRIPS_LIB" != "" ]; then
  echo LIB: $NON_STRIPS_LIB
  $STRIP $NON_STRIPS_LIB
  $SSTRIP $NON_STRIPS_LIB
fi
echo -----------------------------------SYNC!!-------------------------------------
sync
echo ----------------------------APP STRIP AND COPY OK-----------------------------
              

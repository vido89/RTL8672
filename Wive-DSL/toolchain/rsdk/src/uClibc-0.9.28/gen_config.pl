#!/usr/bin/perl
#
# Copyright 2007, Realtek Semiconductor Corp.
#
# Tony Wu (tonywu@realtek.com.tw)
# Apr. 07, 2007
# 

if ($#ARGV != 3) {
  print "usage: make.pl UCLIBC_DIR", "\n";
  exit 0;
}

my $UDIR = $ARGV[0];
my $ARCH = $ARGV[1];
my $ENDN = $ARGV[2];
my $MODE = $ARGV[3];

open(FILE, "$UDIR/uclibc-config") or die "cannot open uclibc-config";
@uconfigc = <FILE>;
close(FILE);

open(FILE, "$UDIR/uclibc-config.h") or die "cannot open uclibc-config.h";
@uconfigh = <FILE>;
close(FILE);

    system("mkdir $UDIR/include/bits");
    print("INFO: mkdir $UDIR/include/bits");
    print("INFO: $UDIR/.config");

    open(PIPE, ">$UDIR/.config") or die "cannot open .config";
    foreach $t_line (@uconfigc) {
      if ($t_line =~ m|^ARCH_CFLAGS|) {
        $t_line = "ARCH_CFLAGS=-march=$ARCH -mno-split-addresses -$ENDN\n";
      }

      if ($t_line =~ m|ARCH_LITTLE_ENDIAN|) {
        if ($ENDN eq 'EL') {
          $t_line = "ARCH_LITTLE_ENDIAN=y\n";
        } else {
          $t_line = "# ARCH_LITTLE_ENDIAN is not set\n";
        }
      }

      if ($t_line =~ m|ARCH_BIG_ENDIAN|) {
        if ($ENDN eq 'EL') {
          $t_line = "# ARCH_BIG_ENDIAN is not set\n";
        } else {
          $t_line = "ARCH_BIG_ENDIAN=y\n";
        }
      }

      if ($t_line =~ m|DODEBUG_PT|) {
        if ($MODE eq 'n') {
          $t_line = "# DODEBUG_PT is not set\n";
        } else {
          $t_line = "DODEBUG_PT=y\n";
        }
      }

      if ($t_line =~ m|DODEBUG | or $t_line =~ m|DODEBUG=|) {
        if ($MODE eq 'n') {
          $t_line = "# DODEBUG is not set\n";
        } else {
          $t_line = "DODEBUG=y\n";
        }
      }
      print PIPE $t_line;
    }
    close(PIPE);

    open(PIPE, ">$UDIR/include/bits/uClibc_config.h") or die "cannot open .config";
    foreach $t_line (@uconfigh) {
      if ($t_line =~ m|ARCH_CFLAGS|) {
        $t_line = "#define __ARCH_CFLAGS__ \"-march=$ARCH -mno-split-addresses -$ENDN\"\n";
      }

      if ($t_line =~ m|ARCH_LITTLE_ENDIAN|) {
        if ($ENDN eq 'EL') {
          $t_line = "#define __ARCH_LITTLE_ENDIAN__ 1\n";
        } else {
          $t_line = "#undef __ARCH_LITTLE_ENDIAN__\n";
        }
      }

      if ($t_line =~ m|ARCH_BIG_ENDIAN|) {
        if ($ENDN eq 'EB') {
          $t_line = "#define __ARCH_BIG_ENDIAN__ 1\n";
        } else {
          $t_line = "#undef __ARCH_BIG_ENDIAN__\n";
        }
      }

      if ($t_line =~ m|__DODEBUG__|) {
        if ($MODE eq 'n') {
          $t_line = "#undef __DODEBUG__\n";
        }

        if ($MODE eq 'y') {
          $t_line = "#define __DODEBUG__ 1\n";
        }
      }


      if ($t_line =~ m|__DODEBUG_PT__|) {
        if ($MODE eq 'n') {
          $t_line = "#undef __DODEBUG_PT__\n";
        }

        if ($MODE eq 'y') {
          $t_line = "#define __DODEBUG_PT__ 1\n";
        }
      }

      print PIPE $t_line;
    }
    close(PIPE);

exit 0;

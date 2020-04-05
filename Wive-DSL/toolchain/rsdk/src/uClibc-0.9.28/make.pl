#!/usr/bin/perl
#
# Copyright 2007, Realtek Semiconductor Corp.
#
# Tony Wu (tonywu@realtek.com.tw)
# Apr. 07, 2007
# 

my @ARCH = ('4180', '4181', '5181', '5280');
my @ENDN = ('EB');
my @MODE = ('n');

if ($#ARGV != 1) {
  print "usage: make.pl BDIR UCLIBC_DIR", "\n";
  exit 0;
}

my $BDIR = $ARGV[0];
my $UDIR = $ARGV[1];

print "INFO: $BDIR", "\n";
print "INFO: $UDIR", "\n";

open(FILE, "$UDIR/uclibc-config") or die "cannot open uclibc-config";
@uconfigc = <FILE>;
close(FILE);

open(FILE, "$UDIR/uclibc-config.h") or die "cannot open uclibc-config.h";
@uconfigh = <FILE>;
close(FILE);

foreach $t_arch (@ARCH) {
  foreach $t_endn (@ENDN) {
    foreach $t_mode (@MODE) {

      &make_clean;

      &gen_multi($t_arch, $t_endn, $t_mode);

      $t_mdir = $t_arch;
      $t_mdir .= '/el' if ($t_endn eq 'EL');
      $t_mdir .= '/debug' if ($t_mode eq 'y' or $t_mode eq 'Y');

      &make_multi($t_mdir, $t_mode);
    }
  }
}

sub make_clean
{
    print("make clean CROSS=\"$BDIR/bin/mips-linux-\"");
    system("make clean CROSS=\"$BDIR/bin/mips-linux-\"");
}

sub make_multi
{
  local($MDIR1, $DEBUG) = @_;


  print "INFO: $MDIR1\n";
  print "INFO: $DEBUG\n";

  system("make -C $UDIR CROSS=\"$BDIR/bin/mips-linux-\" DODEBUG=$DEBUG DODEBUG_PT=$DEBUG all");
  system("make -C $UDIR CROSS=\"$BDIR/bin/mips-linux-\" PREFIX=$BDIR/ DEVEL_PREFIX=/lib/$MDIR1/ RUNTIME_PREFIX=/lib/$MDIR1/ install_runtime");

  system("make -C $UDIR CROSS=\"$BDIR/bin/mips-linux-\" PREFIX=$BDIR/ DEVEL_PREFIX=/lib/$MDIR1/ RUNTIME_PREFIX=/lib/$MDIR1/ install_lib");

  $MDIR2 = $MDIR1;
  $MDIR2 =~ s/4180//g;

  if ($MDIR1 ne $MDIR2) {
    system("make -C $UDIR CROSS=\"$BDIR/bin/mips-linux-\" PREFIX=$BDIR/ DEVEL_PREFIX=/lib/$MDIR2/ RUNTIME_PREFIX=/lib/$MDIR2/ install_runtime");

    system("make -C $UDIR CROSS=\"$BDIR/bin/mips-linux-\" PREFIX=$BDIR/ DEVEL_PREFIX=/lib/$MDIR2/ RUNTIME_PREFIX=/lib/$MDIR2/ install_lib");
  }
}

sub gen_multi
{
    local($ARCH, $ENDN, $MODE) = @_;

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
        if ($MODE eq 'n' or $MODE eq 'N') {
          $t_line = "#undef __DODEBUG__\n";
        }

        if ($MODE eq 'y' or $MODE eq 'Y') {
          $t_line = "#define __DODEBUG__ 1\n";
        }
      }


      if ($t_line =~ m|__DODEBUG_PT__|) {
        if ($MODE eq 'n' or $MODE eq 'N') {
          $t_line = "#undef __DODEBUG_PT__\n";
        }

        if ($MODE eq 'y' or $MODE eq 'Y') {
          $t_line = "#define __DODEBUG_PT__ 1\n";
        }
      }

      print PIPE $t_line;
    }
    close(PIPE);
}

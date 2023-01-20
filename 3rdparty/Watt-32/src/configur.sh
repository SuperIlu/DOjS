#!/bin/sh

#
# Contributed by Ozkan Sezer <sezeroz@users.sourceforge.net>
# for cross-compiling Watt-32 on Linux. Targets are:
#   djgpp, mingw32, mingw64, cygwin, clang or watcom
#
# What works:
# - generates suitable target makefile from makefile.all
# - generates the src/build/<target> objects directory
#
# What does not work:
# - generation of inc/sys/<target>.err, e.g. djgpp.err, and
#   src/build/<target>/syserr.c won't happen:
#   util/errnos.c relies on being compiled as a target-exe:
#   it relies on the sys_nerr value from the target libc and
#   the strerror() string returned from target's libc.  none
#   of these can be accomplished by a simple cross-build.
#

#
# detect which utility programs to use
#
case $(uname) in
MINGW*|MSYS*) util_dir=../util/win32 ;;
*)            util_dir=../util/linux ;;
# non-linux users will need to recompile util/linux/* for their platform.
esac

#
# target triplet for cross-djgpp toolchain:
#
if [ -z "$DJGPP_PREFIX" ]; then
  for i in $(seq 3 7); do
    prefix=i${i}86-pc-msdosdjgpp
    if which $prefix-gcc > /dev/null 2>&1; then
      DJGPP_PREFIX=$prefix
      break
    fi
  done
fi

#
# target triplet for cross-win64 toolchain, MinGW-w64:
#
MINGW64_PREFIX="x86_64-w64-mingw32"

#
# target triplet for cross-win32 toolchain, MinGW-xxx
# for MinGW x86 targets, one can use either MinGW-w64:
# MINGW_PREFIX="i686-w64-mingw32"
#
# ... or the plain old MinGW.org:
#
MINGW_PREFIX="i686-pc-mingw32"

#
# target triplet for cross-cygwin toolchain:
#
CYGWIN_PREFIX="i686-pc-cygwin"

#
# OpenWatcom linux distributions have no prefixing.
#
WATCOM_PREFIX=

#
# error-out functions:
#
missing_stuff ()
{
  echo "You must export WATT_ROOT, like:   'export WATT_ROOT=\$HOME/watt'"
  echo "and must run this script from within your \$WATT_ROOT/src directory."
  exit 4;
}

bad_usage ()
{
  echo "Unknown option '$1'."
  echo "Usage: $0 [djgpp mingw32 mingw64 cygwin clang watcom all clean]"
  exit 2;
}

usage ()
{
  echo "Configuring Watt-32 tcp/ip targets."
  echo "Usage: $0 [djgpp mingw32 mingw64 cygwin clang watcom all clean]"
  exit 1;
}

#
# generate-target functions:
#
gen_djgpp ()
{
  echo "Generating DJGPP makefile, directory and dependencies"
  echo "BIN_PREFIX = $DJGPP_PREFIX-"                      >  djgpp.mak
  echo "W32_BIN2C_ = $util_dir/bin2c"                     >> djgpp.mak
  echo "W32_NASM_ ?= nasm"                                >> djgpp.mak
  $util_dir/mkmake -d build/djgpp makefile.all DJGPP FLAT >> djgpp.mak
  $util_dir/mkdep -s.o -p\$\(OBJDIR\)/ *.[ch] > build/djgpp/watt32.dep
  echo "neterr.c: build/djgpp/syserr.c"          >> build/djgpp/watt32.dep

  #
  # these hacks won't work because errnos.c relies on being compiled as a target-exe.
  # echo "#include <errno.h>" | $DJGPP_PREFIX-gcc -E -dD - | grep "#define E" > ../util/generrno.h
  # echo "#include <sys/version.h>" | $DJGPP_PREFIX-gcc -E -dD - | grep "#define __DJGPP" >> ../util/generrno.h
  # make -C ../util -f errnox.mak dj_err
  # ../util/dj_err -s > build/djgpp/syserr.c
  # ../util/dj_err -e > ../inc/sys/djgpp.err
  #
  echo "Run GNU make to make target:"
  echo "  'make -f djgpp.mak'"
}

gen_mingw32 ()
{
  echo "Generating MinGW32 makefile, directory, errnos and dependencies"
  $util_dir/mkmake -o MinGW32.mak -d             build/MinGW32 makefile.all MINGW32 WIN32
  $util_dir/mkdep -s.o -p\$\(OBJDIR\)/ *.c *.h > build/MinGW32/watt32.dep
  echo "neterr.c: build/MinGW32/syserr.c"         >> build/MinGW32/watt32.dep

  #
  # these hacks won't work because errnos.c relies on being compiled as a target-exe.
  # echo "#include <errno.h>" | $MINGW_PREFIX-gcc -E -dD - | grep "#define E" > ../util/generrno.h
  # echo "#define __MINGW32__" >> ../util/generrno.h
  # echo "#include <_mingw.h>" | $MINGW_PREFIX-gcc -E -dD - | grep "#define __MINGW32_M" >> ../util/generrno.h
  # make -C ../util -f errnox.mak mw_err
  # ../util/mw_err -s > build/MinGW32/syserr.c
  # ../util/mw_err -e > ../inc/sys/mingw32.err
  #
  echo "Run GNU make to make target:"
  echo "  'make -f MinGW32.mak'"
  make -s -f ../util/pkg-conf.mak mingw32_pkg MINGW32_DIR=../lib
}

gen_mingw64 ()
{
  echo "Generating MinGW64-w64 makefiles, directory, errnos and dependencies"
  $util_dir/mkmake -o MinGW64_32.mak -d          build/MinGW64/32bit makefile.all MINGW64 WIN32
  $util_dir/mkmake -o MinGW64_64.mak -d          build/MinGW64/64bit makefile.all MINGW64 WIN64
  $util_dir/mkdep -s.o -p\$\(OBJDIR\)/ *.c *.h > build/MinGW64/watt32.dep
  echo "neterr.c: build/MinGW64/syserr.c"         >> build/MinGW64/watt32.dep

  #
  # these hacks won't work because errnos.c relies on being compiled as a target-exe.
  # echo "#include <errno.h>" | $MINGW64_PREFIX-gcc -E -dD - | grep "#define E" > ../util/generrno.h
  # echo "#define __MINGW32__" >> ../util/generrno.h
  ##echo "#define __MINGW64__" >> ../util/generrno.h
  # echo "#include <_mingw.h>" | $MINGW64_PREFIX-gcc -E -dD - | grep "#define __MINGW64_VERSION_M" >> ../util/generrno.h
  # make -C ../util -f errnox.mak mw64_err
  # ../util/mw64_err -s > build/MinGW64/syserr.c
  # ../util/mw64_err -e > ../inc/sys/mingw64.err
  #
  echo "Run GNU make to make target:"
  echo "  'make -f MinGW64_32.mak'"
  echo "or"
  echo "  'make -f MinGW64_64.mak'"
  make -s -f ../util/pkg-conf.mak mingw64_pkg MINGW64_DIR=../lib
}

gen_cygwin ()
{
  echo "Generating Cygwin makefiles, directories and dependencies"
  $util_dir/mkmake -o Cygwin_32.mak -d           build/CygWin/32bit makefile.all CYGWIN WIN32
  $util_dir/mkmake -o Cygwin_64.mak -d           build/CygWin/64bit makefile.all CYGWIN WIN64
  $util_dir/mkdep -s.o -p\$\(OBJDIR\)/ *.c *.h > build/CygWin/watt32.dep

  echo "Run GNU make to make target(s):"
  echo "    'make -f Cygwin_32.mak'"
  echo " or 'make -f Cygwin_64.mak'"
  make -s -f ../util/pkg-conf.mak cygwin_pkg   CYGWIN_DIR=../lib
  make -s -f ../util/pkg-conf.mak cygwin64_pkg CYGWIN_DIR=../lib
}

#
# Highly experimental. I do not have Linux (or WSL for Win-10).
# So it's completely untested.
#
gen_clang ()
{
  echo "Generating clang-cl (Win32/Win64, release/debug) makefiles, directories, errnos and dependencies"
  $util_dir/mkmake -o clang-release_32.mak -d build/clang/32bit/release makefile.all CLANG WIN32 RELEASE
  $util_dir/mkmake -o clang-release_64.mak -d build/clang/64bit/release makefile.all CLANG WIN64 RELEASE
  $util_dir/mkmake -o clang-debug_32.mak   -d build/clang/32bit/debug   makefile.all CLANG WIN32 DEBUG
  $util_dir/mkmake -o clang-debug_64.mak   -d build/clang/64bit/debug   makefile.all CLANG WIN64 DEBUG

  $util_dir/mkdep -s.obj -p\$\(OBJDIR\)/ *.[ch] > build/clang/watt32.dep
  echo "neterr.c: build/clang/syserr.c"            >> build/clang/watt32.dep

  #
  # Not sure these will work (under Linux/Wine)?
  #
  wine ../util/win32/clang_err -s > build/clang/syserr.c
  wine ../util/win32/clang_err -e > ../inc/sys/clang.err

  echo "Run GNU make to make target(s):"
  echo "  E.g. 'make -f clang-release_32.mak'"
  echo "    or 'make -f clang-release_64.mak'"
  echo "Depending on which clang-cl (32 or 64-bit) is first on your PATH, use the correct 'clang-release_32.mak' or 'clang-release_64.mak'."
}


gen_watcom ()
{
  echo "Generating Watcom makefiles, directories, errnos and dependencies"
  $util_dir/mkmake -w -o watcom_s.mak -d build/watcom/small   makefile.all WATCOM SMALL
  $util_dir/mkmake -w -o watcom_l.mak -d build/watcom/large   makefile.all WATCOM LARGE
  $util_dir/mkmake -w -o watcom_3.mak -d build/watcom/small32 makefile.all WATCOM SMALL32
  $util_dir/mkmake -w -o watcom_f.mak -d build/watcom/flat    makefile.all WATCOM FLAT
  $util_dir/mkmake -w -o watcom_x.mak -d build/watcom/x32vm   makefile.all WATCOM FLAT X32VM
  $util_dir/mkmake -w -o watcom_w.mak -d build/watcom/win32   makefile.all WATCOM WIN32

  #
  # This require dosemu be installed
  #
  dosemu -dumb -c "../dosemu.bat"

  echo "Run wmake to make target(s):"
  echo "  E.g. 'wmake -h -f watcom_s.mak' for small model (16-bit)"
  echo "       'wmake -h -f watcom_l.mak' for large model (16-bit)"
  echo "       'wmake -h -f watcom_3.mak' for small model (32-bit)"
  echo "       'wmake -h -f watcom_f.mak' for flat model  (DOS4GW)"
  echo "       'wmake -h -f watcom_x.mak' for flat model  (X32VM)"
  echo "       'wmake -h -f watcom_w.mak' for Win32"
}

gen_all ()
{
  gen_djgpp
  gen_mingw32
  gen_mingw64
  gen_cygwin
  gen_watcom
}

do_clean ()
{
  rm -f djgpp.mak watcom_{f,l,s,w,x,3}.mak MinGW32.mak MinGW64_{32,64}.mak CygWin_{32,64}.mak clang-release_{32,64}.mak clang-debug_{32,64}.mak
  rm -f build/djgpp/watt32.dep  build/MinGW32/watt32.dep build/MinGW64/watt32.dep
  rm -f build/CygWin/watt32.dep build/watcom/watt32.dep  build/clang/watt32.dep
  rm -f build/djgpp/syserr.c build/watcom/syserr.c build/MinGW32/syserr.c build/MinGW64/syserr.c build/clang/syserr.c
  rm -f ../inc/sys/djgpp.err ../inc/sys/watcom.err ../inc/sys/mingw32.err ../inc/sys/mingw64.err ../inc/sys/clang.err
}

#
# Sanity check our pwd
#
test -f makefile.all || { missing_stuff ; }
test -d ../bin       || { missing_stuff ; }
test -d ../inc       || { missing_stuff ; }
test -d ../lib       || { missing_stuff ; }
test -d ../util      || { missing_stuff ; }

#
# Make sure WATT_ROOT is set
#
if test "x$WATT_ROOT" = "x"; then
  missing_stuff
fi

#
# Check cmdline args
#
if test $# -lt 1; then
  usage
fi
case $1 in
  djgpp|mingw32|mingw64|cygwin|clang|watcom|all|clean)
      ;;
  "-h"|"-?") usage ;;
  *)  bad_usage $1 ;;
esac

#
# Process the cmdline args
#
for i in "$@"
do
 case $i in
  all)     gen_all      ;;
  clean)   do_clean     ;;
  djgpp)   gen_djgpp    ;;
  mingw32) gen_mingw32  ;;
  mingw64) gen_mingw64  ;;
  cygwin)  gen_cygwin   ;;
  clang)   gen_clang    ;;
  watcom)  gen_watcom   ;;
  *)       bad_usage $i;;
 esac
done

#! /bin/sh

kos_dir=/opt/toolchains/dc/kos
. ${kos_dir}/environ.sh

make -f Makefile.dc KOS_WRAPPERS_VERBOSE=1

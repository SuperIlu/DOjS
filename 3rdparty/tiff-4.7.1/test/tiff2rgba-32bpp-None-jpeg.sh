#!/bin/sh
# Generated file, master is Makefile.am
. ${srcdir:-.}/common.sh
infile="$srcdir/images/32bpp-None.tiff"
outfile="o-tiff2rgba-32bpp-None-jpeg.tiff"
f_test_convert "${TIFF2RGBA} -B 255 -n" $infile $outfile
f_tiffinfo_validate $outfile

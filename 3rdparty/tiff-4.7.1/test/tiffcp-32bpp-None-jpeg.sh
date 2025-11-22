#!/bin/sh
# Generated file, master is Makefile.am
. ${srcdir:-.}/common.sh
infile="$srcdir/images/32bpp-None-jpeg.tiff"
outfile="o-tiffcp-32bpp-None-jpeg-YCbCr.tiff"
f_test_convert "${TIFFCP} -c jpeg" $infile $outfile
f_tiffinfo_validate $outfile

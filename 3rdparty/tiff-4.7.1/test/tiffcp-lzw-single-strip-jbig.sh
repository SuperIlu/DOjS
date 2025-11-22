#!/bin/sh
# Generated file, master is Makefile.am
. ${srcdir:-.}/common.sh
infile="$srcdir/images/lzw-single-strip.tiff"
outfile="o-tiffcp-lzw-single-strip-jbig.tiff"
f_test_convert "${TIFFCP} -c jbig" $infile $outfile
f_tiffinfo_validate $outfile

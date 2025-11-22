#!/bin/sh
#
# check decoding of a CCITT Group 3 encoded TIFF without EOL
# hitting https://gitlab.com/libtiff/libtiff/-/issues/54
. ${srcdir:-.}/common.sh
infile="${IMAGES}/testfax3_bug54_1dnoEOL.tif"
outfile="o-testfax3_bug54_1dnoEOL.tiff"
rm -f $outfile
echo "$MEMCHECK ${TIFFCP} -c none $infile $outfile"
eval "$MEMCHECK ${TIFFCP} -c none $infile $outfile"
status=$?
if [ $status != 0 ] ; then
  echo "Returned failed status $status!"
  echo "Output (if any) is in \"${outfile}\"."
  exit $status
fi
echo "$MEMCHECK ${TIFFCMP} $outfile ${REFS}/$outfile"
eval "$MEMCHECK ${TIFFCMP} $outfile ${REFS}/$outfile"
status=$?
if [ $status != 0 ] ; then
  echo "Returned failed status $status!"
  echo "\"${outfile}\" differs from reference file."
  exit $status
fi

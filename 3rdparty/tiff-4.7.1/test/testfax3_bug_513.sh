#!/bin/sh
#
# check decoding of a CCITT Group 3 encoded TIFF
# hitting https://gitlab.com/libtiff/libtiff/-/issues/513
. ${srcdir:-.}/common.sh
infile="${IMAGES}/testfax3_bug_513.tiff"
outfile="o-testfax3_bug_513.tiff"
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

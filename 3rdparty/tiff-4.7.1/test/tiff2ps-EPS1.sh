#!/bin/sh
#
# Basic sanity check for tiffps with PostScript Level 1 encapsulated output
#
PSFILE=o-tiff2ps-EPS1.ps
. ${srcdir:-.}/common.sh
f_test_stdout "${TIFF2PS} -e -1" "${IMG_MINISWHITE_1C_1B}" "${PSFILE}"
cat "${REFS}/${PSFILE}" | grep -v '%%CreationDate:' | grep -v '%%Title:' > "${PSFILE}.ref.tmp"
cat "${PSFILE}" | grep -v '%%CreationDate:' | grep -v '%%Title:' > "${PSFILE}.tmp"
diff -b -u "${PSFILE}.ref.tmp" "${PSFILE}.tmp" || exit 1
rm -f "${PSFILE}.ref.tmp" "${PSFILE}.tmp"

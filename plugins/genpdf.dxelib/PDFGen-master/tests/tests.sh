#!/bin/sh

fail() {
	echo $*
	exit 1
}

run() {
	name=$1
	shift
	printf "Running test $name ..."
	"$@" || fail "Failed to run '$name'"
	printf "\n"
}

run_fail() {
	name=$1
	shift
	"$@"
	if [ $? -eq 0 ] ; then
		fail "Successfully ran '$name' (expected failure)"
	fi
}

# Run the test program
run "valgrind" valgrind -q --leak-check=full --error-exitcode=1 ./testprog

# We can either use pdftotext or acroread to process it. The results should
# be the same
if [ "$1" = "acroread" ] ; then
	run "acroread" acroread -toPostScript output.pdf
	run "ps2ascii" ps2ascii output.ps output.txt
else
	run "pdftotext" pdftotext -layout output.pdf
fi
run "pdftk" pdftk output.pdf dump_data output output.pdftk

# Produces output.ppm
run "pdfppm-p4" pdftoppm -f 4 -l 4 -singlefile output.pdf output

# Check for various output strings
run "check utf8 characters" grep -q "Special characters: €ÜŽžŠšÁáüöäÄÜÖß" output.txt
run "check special characters" grep -q "( ) < > \[ \] { } / %" output.txt
run_fail "check for line wrapping" grep -q "This is a great big long string that I hope will wrap properly around several lines." output.txt

# Check barcodes on page 4 as per main.c
run "zbar extract" zbarimg --quiet output.ppm > output-barcodes.txt
run "check barcode EAN-8" grep -q "EAN-8:95012346" output-barcodes.txt
run "check barcode UPCE" grep -q "EAN-13:0012345000058" output-barcodes.txt
run "check barcode EAN13" grep -q "EAN-13:4003994155486" output-barcodes.txt
run "check barcode UPC-A" grep -q "EAN-13:0003994155480" output-barcodes.txt
run "check barcode Code 128" grep -q "CODE-128:Code128" output-barcodes.txt
run "check barcode CODE 39" grep -q "CODE-39:CODE39" output-barcodes.txt

# Check for pdftk meta data
run "check page count" grep -q "NumberOfPages: 5$" output.pdftk
run "check bookmarks" grep -q "BookmarkTitle: First page$" output.pdftk
run "check for subject" grep -q "InfoValue: My subject$" output.pdftk

# Run it again in a different locale
export LC_ALL=fr_FR
run "locale" ./testprog
run "pdftotext locale" pdftotext -layout output.pdf
run "check locale text" grep -q "This is a great big long string" output.txt

echo "Tests completed successfully"

#! /bin/sh
#
#  Shell script to adjust the version numbers and dates in allegro.h,
#  readme.txt, allegro._tx, and makefile.all.


if [ $# -lt 3 -o $# -gt 4 ]; then
   echo "Usage: fixver major_num sub_num wip_num [comment]" 1>&2
   echo "Example: fixver 3 9 1 WIP" 1>&2
   exit 1
fi

# get the version and date strings in a nice format
if [ $# -eq 3 ]; then
   verstr="$1.$2.$3"
   shortver="$1.$2.$3"
else
   verstr="$1.$2.$3 ($4)"
   shortver="$4$1.$2.$3"
fi

year=$(date +%Y)
month=$(date +%m)
day=$(date +%d)
datestr="$(date +%b) $day, $year"

# patch allegro.h
echo "s/\#define ALLEGRO_VERSION .*/\#define ALLEGRO_VERSION          $1/" > fixver.sed
echo "s/\#define ALLEGRO_SUB_VERSION .*/\#define ALLEGRO_SUB_VERSION      $2/" >> fixver.sed
echo "s/\#define ALLEGRO_WIP_VERSION .*/\#define ALLEGRO_WIP_VERSION      $3/" >> fixver.sed
echo "s/\#define ALLEGRO_VERSION_STR .*/\#define ALLEGRO_VERSION_STR      \"$verstr\"/" >> fixver.sed
echo "s/\#define ALLEGRO_DATE_STR .*/\#define ALLEGRO_DATE_STR         \"$year\"/" >> fixver.sed
echo "s/\#define ALLEGRO_DATE .*/\#define ALLEGRO_DATE             $year$month$day    \/\* yyyymmdd \*\//" >> fixver.sed

echo "Patching include/allegro.h..."
cp include/allegro.h fixver.tmp
sed -f fixver.sed fixver.tmp > include/allegro.h

# patch readme.txt
echo "s/\\_\/__\/     Version .*/\\_\/__\/     Version $verstr/" > fixver.sed
echo "s/By Shawn Hargreaves, .*\./By Shawn Hargreaves, $datestr\./" >> fixver.sed

echo "Patching readme.txt..."
cp readme.txt fixver.tmp
sed -f fixver.sed fixver.tmp > readme.txt

# patch allegro._tx
echo "Patching docs/allegro._tx..."
cp docs/allegro._tx fixver.tmp
sed -f fixver.sed fixver.tmp > docs/allegro._tx

# patch makefile.all
echo "s/LIBRARY_VERSION = .*/LIBRARY_VERSION = $1$2$3/" > fixver.sed

echo "Patching makefile.all..."
cp makefile.all fixver.tmp
sed -f fixver.sed fixver.tmp > makefile.all

# patch makefile.ver
echo "s/shared_version = .*/shared_version = $1.$2.$3/" > fixver.sed

echo "Patching makefile.ver..."
cp makefile.ver fixver.tmp
sed -f fixver.sed fixver.tmp > makefile.ver

# patch allegro.spec
echo "s/\\_\/__\/     Version .*/\\_\/__\/     Version $verstr/" > fixver.sed
echo "s/Version:        .*/Version:        $shortver/" >> fixver.sed
echo "s/Buildroot: \/var\/tmp\/allegro-.*/Buildroot: \/var\/tmp\/allegro-$shortver/" >> fixver.sed
echo "s/all[0-9]*\.zip/all$1$2$3\.zip/" >> fixver.sed

echo "Patching misc/allegro.spec..."
cp misc/allegro.spec fixver.tmp
sed -f fixver.sed fixver.tmp > misc/allegro.spec

# clean up after ourselves
rm fixver.sed fixver.tmp

echo "Done!"

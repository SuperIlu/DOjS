#!/bin/sh
#  mksofile -- creates the .so file for an Allegro library
#
#          Usage: mksofile libdir stem major_version

rm -f $1/$2
cat > $1/$2 << EOF
/* GNU ld script
   Most of Allegro is in the shared library, but the assembly language 
   functions are linked statically because they are not PIC. */
GROUP ( $3 $4 )
EOF


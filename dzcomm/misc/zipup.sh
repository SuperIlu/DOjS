#! /bin/sh
#
#  Shell script to create a distribution zip, generating the manifest
#  file and optionally building diffs against a previous version. This
#  should be run from the allegro directory, but will leave the zip 
#  files in the parent directory.
#
#  If possible, this script will try to generate dependencies for all
#  the supported DOS/Windows compilers, so they can be included in the
#  distribution. This is obviously only possible on DOS machines that
#  have these compilers installed, and requires an ability to alter the
#  machine environment on the fly. This script expects you to have set
#  up a collection of batch files in a /batch directory on the current
#  drive, named msvc.bat, watcom.bat, rsxnt.bat, mingw32.bat, and djgpp.bat.
#  If any of these files are absent or are not executable (ie. we running
#  on a non-DOS system), it will skip the dependency generation for that
#  compiler. If the batch file is available, it will pass it a command
#  to run in the environment of that compiler, eg.
#  "/batch/djgpp.bat make depend". This script should set environment
#  variables ready for using djgpp, and then invoke "make depend"
#  (using 4DOS this is a %& command, or with command.com you can just
#  write a string of %1 %2 %3 etc).


if [ $# -lt 1 -o $# -gt 2 ]; then
   echo "Usage: zipup archive_name [previous_archive]" 1>&2
   exit 1
fi

# strip off the path and extension from our arguments
name=$(echo "$1" | sed -e 's/.*[\\\/]//; s/\.zip//')
prev=$(echo "$2" | sed -e 's/.*[\\\/]//; s/\.zip//')

# delete all generated files
echo "Cleaning the Allegro tree..."
make -s veryclean

# if it is possible on this machine, generate the DLL linkage files
if [ -x fixdll.bat ]; then
   ./fixdll.bat
fi

# if it is available on this machine, generate dependencies for MSVC
if [ -x /batch/msvc.bat ]; then
   ./fixmsvc.bat
   echo "Generating MSVC dependencies..."
   /batch/msvc.bat make depend
fi

# if it is available on this machine, generate dependencies for Watcom
if [ -x /batch/watcom.bat ]; then
   ./fixwat.bat
   echo "Generating Watcom dependencies..."
   /batch/watcom.bat make depend
fi

# if it is available on this machine, generate dependencies for RSXNT
if [ -x /batch/rsxnt.bat ]; then
   ./fixrsxnt.bat
   echo "Generating RSXNT dependencies..."
   /batch/rsxnt.bat make depend
fi

# if it is available on this machine, generate dependencies for Mingw32
if [ -x /batch/mingw32.bat ]; then
   ./fixming.bat
   echo "Generating Mingw32 dependencies..."
   /batch/mingw32.bat make depend
fi

# if it is available on this machine, generate dependencies for djgpp
if [ -x /batch/djgpp.bat ]; then
   ./fixdjgpp.bat
   echo "Generating djgpp dependencies..."
   /batch/djgpp.bat make depend
fi

# convert documentation from the ._tx source files
echo "Converting documentation..."
make docs
make -s distclean

# recursive helper to fill any empty directories with a tmpfile.txt
scan_for_empties()
{
   if [ -f $1/tmpfile.txt ]; then rm $1/tmpfile.txt; fi

   for file in $1/*; do
      if [ -d $file ]; then
	 scan_for_empties $file
      elif [ $file = "$1/*" ]; then
	 echo "This file is needed because some unzip programs skip empty directories." > $1/tmpfile.txt
      fi
   done
}

#echo "Filling empty directories with tmpfile.txt..."
scan_for_empties "."

# build the main zip archive
echo "Creating $name.zip..."
cd ..
if [ -f $name.zip ]; then rm $name.zip; fi
zip -9 -r $name.zip allegro

# generate the manifest file
echo "Generating allegro.mft..."
unzip -Z1 $name.zip | sort > allegro/allegro.mft
echo "allegro/allegro.mft" >> allegro/allegro.mft
zip -9 $name.zip allegro/allegro.mft

# if we are building diffs as well, do those
if [ $# -eq 2 ]; then

   echo "Inflating current version ($name.zip)..."
   mkdir current
   unzip -q $name.zip -d current

   echo "Inflating previous version ($2)..."
   mkdir previous
   unzip -q "$2" -d previous

   echo "Generating diffs..."
   diff -U 3 -N --recursive previous/ current/ > $name.diff

   echo "Deleting temp files..."
   rm -r previous
   rm -r current

   # generate the .txt file for the diff archive
   echo "This is a set of diffs to upgrade from $prev to $name." > $name.txt
   echo >> $name.txt
   echo "Date: $(date '+%A %B %d, %Y, %H:%M')" >> $name.txt
   echo >> $name.txt
   echo "To apply this patch, copy $name.diff into the same directory where you" >> $name.txt
   echo "installed Allegro (this should be one level up from the allegro/ dir, eg." >> $name.txt
   echo "if your Allegro installation lives in c:/djgpp/allegro/, you should be in" >> $name.txt
   echo "c:/djgpp/). Then type the command:" >> $name.txt
   echo >> $name.txt
   echo "   patch -p1 < $name.diff" >> $name.txt
   echo >> $name.txt
   echo "Change into the allegro directory, run make, and enjoy!" >> $name.txt

   # zip up the diff archive
   echo "Creating ${name}_diff.zip..."
   if [ -f ${name}_diff.zip ]; then rm ${name}_diff.zip; fi
   zip -9 ${name}_diff.zip $name.diff $name.txt

   # find if we need to add any binary files as well
   bin=$(sed -n -e "s/Binary files previous\/\(.*\) and current.* differ/\1/p" $name.diff)

   if [ "$bin" != "" ]; then
      echo "Adding binary diffs..."
      zip -9 ${name}_diff.zip $bin
   fi

   rm $name.diff $name.txt

fi

echo "Done!"

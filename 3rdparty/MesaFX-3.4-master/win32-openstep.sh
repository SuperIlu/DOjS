# This file should be run from the Bourne shell
# provided for OpenStep implementations under WIN32

mkdir lib
( cd src; make openstep-win32 )
( cd src-glu; make openstep-win32 )

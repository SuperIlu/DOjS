call vcvars32 x86

mkdir lib
cd src
nmake /f Makefile.fx
cd ..\src-glu
nmake /f Makefile.fx
cd ..\src-glut
nmake /f Makefile.fx
cd ..\3Dfx\demos
nmake /f Makefile.fx

echo off
rem
rem .bat file for building Mesa3d/Mingw32
rem
rem Paul Garceau, August 26, 1998
rem Updated January, 13, 2000 -- Paul Garceau
rem
rem GCC-2.95.2/Mingw32 build of Mesa 3-D Graphics Library (v3.3)

rem Build Requirements:
rem
rem .bat file uses Make 3.77
rem "touch" must be somewhere on your system path variable %PATH%
rem
rem "touch" doesn't seem to work using OS environment variables so
rem we need to directly access any "touch" directory/folder references


rem Set up Mesa Root directory/folder -- modify as needed
rem
set mesaroot=d:\mesa-3.3

rem move to Mesa3d root directory
cd %mesaroot%

rem set up Mesa3d build path
PATH=%mesaroot%;%PATH%

rem Set up Mesa Source directory/folder
set mesasrc=%mesaroot%\src

rem Set up Mesa lib directory/folder
md lib

rem Set up Mesa lib directory/folder
set mesalib=%mesaroot%\lib

touch src/depend
touch src-glu/depend

rem touch src-glut/depend

rem Generate wing32.dll for the sake of Mesa build
cd %mesasrc%\windows

rem
rem Create a .a lib file
rem
dlltool --input-def wing32.def --output-lib wing32.a --dllname wing32.dll
rem
rem Create a .dll file (wing32.dll); GCC-2.95.2/Mingw32 compiler used
rem
gcc -mdll -o wing32.dll wing32.a -WI,temp.exp
move wing32.dll %mesalib%

rem Return to mesa-3.3 'root' directory
cd %mesaroot%

rem Begin build of mesa-3.3 libs for GCC-2.95.2/Mingw32

rem  Build libGL.a
rem
make -w --directory=%mesasrc% -fmakefile.m32

rem move the completed library
move %mesasrc%\libGL.a %mesalib%\libGL.a

rem  Build libGLU.a
rem
make -w --directory=%mesaroot%\src-glu -f makefile.m32

rem move the completed library to the lib directory
move %mesaroot%\src-glu\libGLU.a %mesalib%\libGLU.a

rem Library build complete

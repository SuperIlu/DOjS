@echo off
::
:: configur.bat for the ./src/test/*.mak files.
::
set MKMAKE=..\..\util\mkmake.exe
if %OS%. == Windows_NT. set MKMAKE=..\..\util\win32\mkmake.exe
if %1. == djgpp. goto start

:start
if %1.==mingw64.  goto mingw64
if %1.==mingw.    goto mingw64
if %1.==cygwin.   goto cygwin
if %1.==djgpp.    goto djgpp
if %1.==highc.    goto highc
if %1.==visualc.  goto visualc
if %1.==clang.    goto clang
if %1.==watcom.   goto watcom
if %1.==borland.  goto borland
if %1.==all.      goto all
if %1.==clean.    goto clean
if %1.==-h.       goto usage
if %1.==-?.       goto usage
if %1.==.         goto usage
if not %1.==.     goto bad_usage
goto quit

::--------------------------------------------------------------------------
:mingw64
::
echo Generating MinGW64-w64 makefiles
%MKMAKE% -o MinGW64_32.mak makefile.all MINGW64 WIN32 IS_GCC
%MKMAKE% -o MinGW64_64.mak makefile.all MINGW64 WIN64 IS_GCC

echo Run GNU make to make target(s):
echo   make -f MinGW64_32.mak
echo   make -f MinGW64_64.mak
goto next

::--------------------------------------------------------------------------
:cygwin
::
echo Generating Cygwin (x86/x64) makefiles
%MKMAKE% -o Cygwin_32.mak makefile.all CYGWIN WIN32 IS_GCC
%MKMAKE% -o Cygwin_64.mak makefile.all CYGWIN WIN64 IS_GCC

echo Run GNU make to make target(s):
echo   "make -f Cygwin_32.mak"
echo   "make -f Cygwin_64.mak"
echo Depending on which gcc.exe (32 or 64-bit) is first on your PATH, use the correct 'Cygwin_32.mak' or 'Cygwin_64.mak'.
goto next

::--------------------------------------------------------------------------
:djgpp
::
echo Generating DJGPP makefile
%MKMAKE% -o djgpp.mak makefile.all DJGPP FLAT IS_GCC

echo Run GNU make to make target:
echo   make -f djgpp.mak
goto next

::--------------------------------------------------------------------------
:highc
::
:: Need to use GNU-make (on Windows) to build for High-C.
::
echo Generating Metaware High-C makefile
%MKMAKE% -o highc.mak makefile.all HIGHC FLAT

echo Run GNU make to make target:
echo   "make -f highc.mak"
goto next

::--------------------------------------------------------------------------
:visualc
::
echo off
echo Generating Microsoft Visual-C (x86/x64) makefiles
%MKMAKE% -o visualc_32.mak makefile.all VISUALC WIN32
%MKMAKE% -o visualc_64.mak makefile.all VISUALC WIN64

echo Run GNU make to make target(s):
echo   "make -f visualc_32.mak"
echo   "make -f visualc_64.mak"
goto next

::--------------------------------------------------------------------------
:clang
::
echo off
echo Generating clang-cl (x86/x64) makefiles
%MKMAKE% -o clang_32.mak makefile.all CLANG WIN32
%MKMAKE% -o clang_64.mak makefile.all CLANG WIN64

echo Run GNU make to make target(s):
echo   "make -f clang_32.mak"
echo   "make -f clang_64.mak"
echo Depending on which clang-cl.exe (32 or 64-bit) is first on your PATH, use the correct 'clang_32.mak' or 'clang_64.mak'.
goto next

::--------------------------------------------------------------------------
:watcom
::
echo Generating Watcom makefiles
%MKMAKE% -o watcom_s.mak makefile.all WATCOM SMALL
%MKMAKE% -o watcom_l.mak makefile.all WATCOM LARGE
%MKMAKE% -o watcom_3.mak makefile.all WATCOM SMALL32
%MKMAKE% -o watcom_f.mak makefile.all WATCOM FLAT
%MKMAKE% -o watcom_x.mak makefile.all WATCOM FLAT X32VM
%MKMAKE% -o watcom_w.mak makefile.all WATCOM WIN32

echo Run GNU make to make target(s):
echo   "make -f watcom_s.mak" for small model (16-bit)
echo   "make -f watcom_l.mak" for large model (16-bit)
echo   "make -f watcom_3.mak" for small model (32-bit DOS4GW)
echo   "make -f watcom_f.mak" for flat model  (DOS4GW)
echo   "make -f watcom_x.mak" for flat model  (X32VM)
echo   "make -f watcom_w.mak" for Win32
goto next

::--------------------------------------------------------------------------
:borland
::
echo Generating Borland Flat/Win32 makefiles
%MKMAKE% -o bcc_f.mak makefile.all BORLAND FLAT
%MKMAKE% -o bcc_w.mak makefile.all BORLAND WIN32

echo Run GNU make to make target(s):
echo   "make -f bcc_f.mak" for Flat model
echo   "make -f bcc_w.mak" for Win32
goto next

::--------------------------------------------------------------------------

:bad_usage
echo Unknown option '%1'.

::--------------------------------------------------------------------------
:usage
::
echo Configuring Watt-32 tcp/ip targets.
echo Usage: %0 {watcom, borland, highc, djgpp, mingw64, cygwin, visualc, clang, all, clean}
goto quit

::--------------------------------------------------------------------------
:clean
::
del /Q djgpp.mak
del /Q highc.mak
del /Q clang_32.mak   clang_64.mak
del /Q visualc_32.mak visualc_64.mak
del /Q MinGW64_32.mak MinGW64_64.mak
del /Q Cygwin_32.mak  Cygwin_64.mak
del /Q watcom_?.mak
del /Q bcc_?.mak
echo 'clean' done.
goto next

::------------------------------------------------------------
:all
::
call %0 djgpp   %2
call %0 clang   %2
call %0 visualc %2
call %0 mingw64 %2
call %0 cygwin  %2
call %0 watcom  %2
call %0 borland %2
call %0 highc   %2
:next
shift
echo.

if %1.==. goto quit
goto start

:quit


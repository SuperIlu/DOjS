@echo off
::
:: configur.bat:
::   This .bat file generates all makefiles from 1 source:
::   the Makefile.all file. Other generated files are placed
::   under 'build/*' to keep a flatter directory structure.
::   Files for errno values are placed in ../inc/sys/*.err.
::
:: Note: the command line options are case sensitive under the
::       brain-dead CMD shell. (use lower case args or switch to
::       JPsoft's freeware TCC/LE).
::
if %WATT_ROOT%. ==. goto not_set
if not exist %WATT_ROOT%\src\makefile.all goto not_set

::
:: These variables can be run under plain DOS or Win-XP.
:: But all of them are built using djgpp. Hence they fail
:: to run under e.g. vDOS on 64-bit Windows 10
:: (32-bit Win-10 is untested).
::
:: Note: The variables with '/' are used in the respective
::       generated makefiles.
::       The variables with '\' are used below only.
::
set     MKMAKE=..\util\mkmake.exe
set      MKDEP=..\util\mkdep.exe
set     WC_ERR=..\util\wc_err.exe
set    BCC_ERR=..\util\bcc_err.exe
set  W32_BIN2C=..\util\bin2c.exe
set W32_BIN2C_=../util/bin2c.exe
set   W32_NASM=..\util\nasm.exe
set  W32_NASM_=../util/nasm.exe
set    DJ_ERR=..\util\dj_err.exe

::
:: Check for %OS%. Ass-u-me, if set to anything, we are on Windows.
::
if %OS%. ==. goto is_dos

::
:: Use these programs under all versions of Windows.
::
set     MKMAKE=..\util\win32\mkmake.exe
set      MKDEP=..\util\win32\mkdep.exe
set     WC_ERR=..\util\win32\wc_err.exe
set    BCC_ERR=..\util\win32\bcc_err.exe
set  W32_BIN2C=..\util\win32\bin2c.exe
set W32_BIN2C_=../util/win32/bin2c.exe
set   W32_NASM=..\util\win32\nasm.exe
set  W32_NASM_=../util/win32/nasm.exe
set    DJ_ERR=..\util\win32\dj_err.exe

:is_dos

:start
if %1.==clang.    goto clang
if %1.==mingw32.  goto mingw32
if %1.==mingw64.  goto mingw64
if %1.==borland.  goto borland
if %1.==cygwin.   goto cygwin
if %1.==cygwin64. goto cygwin64
if %1.==djgpp.    goto djgpp
if %1.==djcross.  goto djcross
if %1.==digmars.  goto digmars
if %1.==ladsoft.  goto ladsoft
if %1.==lcc.      goto lcc
if %1.==pellesc.  goto pellesc
if %1.==highc.    goto highc
if %1.==visualc.  goto visualc
if %1.==watcom.   goto watcom
if %1.==all.      goto all
if %1.==clean.    goto clean
if %1.==-h.       goto usage
if %1.==-?.       goto usage
if %1.==.         goto usage
if not %1.==.     goto bad_usage
goto quit

::--------------------------------------------------------------------------
:borland
::
echo Generating Borland-C makefiles, directories, errnos and dependencies
%MKMAKE% -o bcc_s.mak -d build\borland\small makefile.all BORLAND SMALL
%MKMAKE% -o bcc_l.mak -d build\borland\large makefile.all BORLAND LARGE
%MKMAKE% -o bcc_f.mak -d build\borland\flat  makefile.all BORLAND FLAT
%MKMAKE% -o bcc_w.mak -d build\borland\win32 makefile.all BORLAND WIN32
%MKDEP%  -s.obj -p$(OBJDIR)\ *.c *.h     > build\borland\watt32.dep
echo neterr.c:  build\borland\syserr.c  >> build\borland\watt32.dep

%BCC_ERR% -s > build\borland\syserr.c
%BCC_ERR% -e > ..\inc\sys\borlandc.err

echo Run Borland or CBuilder's make to make target(s):
echo   E.g. "%%BCCDIR%%\bin\make -f bcc_l.mak" for large model
goto next

::--------------------------------------------------------------------------
:watcom
::
echo Generating Watcom makefiles, directories, errnos and dependencies
%MKMAKE% -o watcom_s.mak -d build\watcom\small makefile.all WATCOM SMALL
%MKMAKE% -o watcom_l.mak -d build\watcom\large makefile.all WATCOM LARGE
%MKMAKE% -o watcom_f.mak -d build\watcom\flat  makefile.all WATCOM FLAT
%MKMAKE% -o watcom_x.mak -d build\watcom\x32vm makefile.all WATCOM FLAT X32VM
%MKMAKE% -o watcom_w.mak -d build\watcom\win32 makefile.all WATCOM WIN32
%MKDEP%  -s.obj -p$(OBJDIR)\ *.c *.h  > build\watcom\watt32.dep
echo neterr.c: build\watcom\syserr.c >> build\watcom\watt32.dep

%WC_ERR% -s > build\watcom\syserr.c
%WC_ERR% -e > ..\inc\sys\watcom.err

echo Run wmake to make target(s):
echo   E.g. "wmake -f watcom_l.mak" for large model
goto next

::--------------------------------------------------------------------------
:highc
::
:: Need to use GNU-make (on Windows) to build for High-C.
::
echo Generating Metaware High-C makefile, directory, errnos and dependencies
%MKMAKE% -o highc.mak -d build\highc makefile.all HIGHC FLAT
%MKDEP%  -s.obj -p$(OBJDIR)/ *.c *.h > build\highc\watt32.dep
echo neterr.c: build\highc/syserr.c >> build\highc\watt32.dep

..\util\hc_err -s > build\highc\syserr.c
..\util\hc_err -e > ..\inc\sys\highc.err

echo Run GNU make to make target:
echo   "make -f highc.mak"
goto next

::--------------------------------------------------------------------------
:ladsoft
::
echo Generating LADsoft makefile, directory, errnos and dependencies
%MKMAKE% -o ladsoft.mak -d build\ladsoft makefile.all LADSOFT
%MKDEP%  -s.obj -p$(OBJDIR)\ *.c *.h   > build\ladsoft\watt32.dep
echo neterr.c: build\ladsoft\syserr.c >> build\ladsoft\watt32.dep

..\util\ls_err -s > build\ladsoft\syserr.c
..\util\ls_err -e > ..\inc\sys\ladsoft.err

echo Run a Borland compatible make to make target:
echo   "maker -f ladsoft.mak"
goto next

::--------------------------------------------------------------------------
:digmars
::
echo Generating Digital Mars makefiles, directories, errnos and dependencies
%MKMAKE% -o dmars_s.mak -d build\digmars\small makefile.all DIGMARS SMALL
%MKMAKE% -o dmars_l.mak -d build\digmars\large makefile.all DIGMARS LARGE
%MKMAKE% -o dmars_x.mak -d build\digmars\x32vm makefile.all DIGMARS FLAT X32VM
%MKMAKE% -o dmars_p.mak -d build\digmars\phar  makefile.all DIGMARS FLAT PHARLAP
%MKMAKE% -o dmars_w.mak -d build\digmars\win32 makefile.all DIGMARS WIN32
%MKDEP%  -s.obj -p$(OBJDIR)\ *.c *.h    > build\digmars\watt32.dep
echo neterr.c : build\digmars\syserr.c >> build\digmars\watt32.dep

..\util\dm_err -s > build\digmars\syserr.c
..\util\dm_err -e > ..\inc\sys\digmars.err

echo Run make to make target(s):
echo   E.g. "maker -f dmars_l.mak" for large model
goto next

::--------------------------------------------------------------------------
:djgpp
::
echo Generating DJGPP makefile, directory, errnos and dependencies
%MKMAKE% -o djgpp.mak -d build\djgpp makefile.all DJGPP FLAT
%MKDEP%  -s.o -p$(OBJDIR)/ *.c *.h   > build\djgpp\watt32.dep
echo neterr.c: build\djgpp/syserr.c >> build\djgpp\watt32.dep

%DJ_ERR% -s > build\djgpp\syserr.c
%DJ_ERR% -e > ..\inc\sys\djgpp.err

echo Run GNU make to make target:
echo   make -f djgpp.mak
goto next

::--------------------------------------------------------------------------
:djcross
::
echo Generating DJGPP makefile, directory, errnos and dependencies
%MKMAKE% -o djgpp.mak -d build\djgpp makefile.all DJGPP
%MKDEP%  -s.o -p$(OBJDIR)/ *.c *.h   > build\djgpp\watt32.dep
echo neterr.c: build\djgpp/syserr.c >> build\djgpp\watt32.dep

%DJ_ERR% -s > build\djgpp\syserr.c
%DJ_ERR% -e > ..\inc\sys\djgpp.err

echo Run GNU make to make target:
echo   make -f djgpp.mak
goto next

::--------------------------------------------------------------------------
:visualc
::
echo off
echo Generating Microsoft Visual-C (x86/x64) makefiles, directories, errnos and dependencies
%MKMAKE% -o visualc-release.mak    -d build\visualc\32bit\release makefile.all VISUALC WIN32 RELEASE
%MKMAKE% -o visualc-release_64.mak -d build\visualc\64bit\release makefile.all VISUALC WIN64 RELEASE
%MKMAKE% -o visualc-debug.mak      -d build\visualc\32bit\debug   makefile.all VISUALC WIN32 DEBUG
%MKMAKE% -o visualc-debug_64.mak   -d build\visualc\64bit\debug   makefile.all VISUALC WIN64 DEBUG

%MKDEP%  -s.obj -p$(OBJDIR)\ *.c *.h                               > build\visualc\watt32.dep
echo $(OBJDIR)\stkwalk.obj: stkwalk.cpp wattcp.h misc.h stkwalk.h >> build\visualc\watt32.dep
echo neterr.c:  build\visualc\syserr.c                            >> build\visualc\watt32.dep

..\util\vc_err -s > build\visualc\syserr.c
..\util\vc_err -e > ..\inc\sys\visualc.err

::
:: If %CL% does not contain a '-D_WIN32_WINNT' already, add a '-D_WIN32_WINNT=0x0601' to %CL%.
::
echo %CL% | %SystemRoot%\system32\find.exe "-D_WIN32_WINNT" > NUL
if errorlevel 1 (
  echo Setting "_WIN32_WINNT=0x0601". Change to suite your OS or SDK.
  set CL=%CL% -D_WIN32_WINNT=0x0601
)

echo Run nmake to make target(s):
echo   E.g. "nmake -f visualc-release.mak"
echo     or "nmake -f visualc-release_64.mak"
goto next

::--------------------------------------------------------------------------
:mingw32
::
echo Generating MinGW32 makefile, directory, errnos and dependencies
%MKMAKE% -o MinGW32.mak -d build\MinGW32 makefile.all MINGW32 WIN32
%MKDEP%  -s.o -p$(OBJDIR)/ *.c *.h     > build\MinGW32\watt32.dep
echo neterr.c: build/MinGW32/syserr.c >> build\MinGW32\watt32.dep

..\util\mw_err -s > build\MinGW32\syserr.c
..\util\mw_err -e > ..\inc\sys\mingw32.err

echo Run GNU make to make target:
echo   make -f MinGW32.mak
make.exe -s -f ../util/pkg-conf.mak mingw32_pkg
goto next

::--------------------------------------------------------------------------
:mingw64
::
echo Generating MinGW64-w64 makefile, directory, errnos and dependencies
%MKMAKE% -o MinGW64_32.mak             -d build\MinGW64\32bit makefile.all MINGW64 WIN32
%MKMAKE% -o MinGW64_64.mak             -d build\MinGW64\64bit makefile.all MINGW64 WIN64
%MKDEP%  -s.o -p$(OBJDIR)/32bit/ *.c *.h > build\MinGW64\32bit\watt32.dep
%MKDEP%  -s.o -p$(OBJDIR)/64bit/ *.c *.h > build\MinGW64\64bit\watt32.dep

echo neterr.c: build/MinGW64/syserr.c   >> build\MinGW64\32bit\watt32.dep
echo neterr.c: build/MinGW64/syserr.c   >> build\MinGW64\64bit\watt32.dep

..\util\mw64_err -s > build\MinGW64\syserr.c
..\util\mw64_err -e > ..\inc\sys\mingw64.err

echo Run GNU make to make target:
echo   make -f MinGW64_32.mak
echo or
echo   make -f MinGW64_64.mak
make.exe -s -f ../util/pkg-conf.mak mingw64_pkg
goto next

::--------------------------------------------------------------------------
:cygwin
::
echo Generating CygWin makefile, directory and dependencies
%MKMAKE% -o CygWin.mak -d build\CygWin\32bit makefile.all CYGWIN WIN32
%MKDEP%  -s.o -p$(OBJDIR)/ *.c *.h > build\CygWin\32bit\watt32.dep

echo Run GNU make to make target:
echo   make -f CygWin.mak
make.exe -s -f ../util/pkg-conf.mak cygwin_pkg
goto next

::--------------------------------------------------------------------------
:cygwin64
::
echo Generating CygWin64 makefile, directory and dependencies
%MKMAKE% -o CygWin_64.mak -d build\CygWin\64bit makefile.all CYGWIN64 WIN64
%MKDEP% -s.o -p$(OBJDIR)/ *.c *.h > build\CygWin\64bit\watt32.dep

echo Run GNU make to make target:
echo   make -f CygWin_64.mak
make.exe -s -f ../util/pkg-conf.mak cygwin64_pkg
goto next

::--------------------------------------------------------------------------
:pellesc
::
echo Generating PellesC makefile, directory, errnos and dependencies
%MKMAKE% -o pellesc.mak    -d build\pellesc\32bit makefile.all PELLESC WIN32
%MKMAKE% -o pellesc_64.mak -d build\pellesc\64bit makefile.all PELLESC WIN64
%MKDEP%  -s.obj -p$(OBJDIR)\ *.c *.h   > build\pellesc\watt32.dep
echo neterr.c: build\pellesc\syserr.c >> build\pellesc\watt32.dep

..\util\po_err -s > build\pellesc\syserr.c
..\util\po_err -e > ..\inc\sys\pellesc.err

echo Run pomake to make targets:
echo   E.g. "pomake -f pellesc.mak"
echo   E.g. "pomake -f pellesc_64.mak"
goto next


::--------------------------------------------------------------------------
:lcc
::
echo Generating LCC-Win32 makefile, directory, errnos and dependencies
%MKMAKE% -o lcc.mak -d build\lcc makefile.all LCC WIN32
%MKDEP%  -s.obj -p$(OBJDIR)\ *.c *.h > build\lcc\watt32.dep
echo neterr.c: build\lcc\syserr.c   >> build\lcc\watt32.dep

..\util\lcc_err -s > build\lcc\syserr.c
..\util\lcc_err -e > ..\inc\sys\lcc.err

echo Run make to make target:
echo   E.g. "maker -f lcc.mak"
goto next

::--------------------------------------------------------------------------
:clang
::
echo Generating CLang-Win32 makefile, directory, errnos and dependencies
%MKMAKE% -o clang32.mak -d build\clang\32bit makefile.all CLANG WIN32
%MKMAKE% -o clang64.mak -d build\clang\64bit makefile.all CLANG WIN64
%MKDEP% -s.obj -p$(OBJDIR)/ *.c *.h  > build\clang\watt32.dep
echo neterr.c: build\clang\syserr.c >> build\clang\watt32.dep

..\util\clang_err -s > build\clang\syserr.c
..\util\clang_err -e > ..\inc\sys\clang.err

echo Run GNU make to make target(s):
echo   E.g. "make -f clang32.mak"
echo     or "make -f clang64.mak"
echo Depending on which clang-cl.exe (32 or 64-bit) is first on your PATH, use the correct 'clangX.bat' above.

goto next

::--------------------------------------------------------------------------

:bad_usage
echo Unknown option '%1'.

::--------------------------------------------------------------------------
:usage
::
echo Configuring Watt-32 tcp/ip targets.
echo Usage: %0 {borland, clang, cygwin, digmars, djgpp, djcross, highc, ladsoft,
echo                      mingw32, mingw64, pellesc, visualc, watcom, all, clean}
goto quit

::--------------------------------------------------------------------------
:clean
::
del djgpp.mak
del watcom_*.mak
del bcc_*.mak
del highc.mak
del dmars_*.mak
del ladsoft.mak
del visualc-release.mak
del visualc-debug.mak
del visualc-release_64.mak
del visualc-debug_64.mak
del MinGW32.mak
del MinGW64_32.mak
del MinGW64_64.mak
del CygWin.mak
del CygWin_64.mak
del watcom_w.mak
del pellesc.mak
del pellesc_64.mak
del highc.mak
del lcc.mak
del clang32.mak
del clang64.mak

del build\djgpp\watt32.dep
del build\borland\watt32.dep
del build\highc\watt32.dep
del build\digmars\watt32.dep
del build\ladsoft\watt32.dep
del build\visualc\watt32.dep
del build\MinGW32\watt32.dep
del build\MinGW64\watt32.dep
del build\CygWin\watt32.dep
del build\CygWin\64bit\watt32.dep
del build\watcom\watt32.dep
del build\pellesc\watt32.dep
del build\highc\watt32.dep
del build\lcc\watt32.dep
del build\clang\watt32.dep

del build\djgpp\syserr.c
del build\watcom\syserr.c
del build\borland\syserr.c
del build\highc\syserr.c
del build\digmars\syserr.c
del build\ladsoft\syserr.c
del build\visualc\syserr.c
del build\MinGW32\syserr.c
del build\MinGW64\syserr.c
del build\pellesc\syserr.c
del build\highc\syserr.c
del build\lcc\syserr.c
del build\clang\syserr.c

del ..\inc\sys\djgpp.err
del ..\inc\sys\watcom.err
del ..\inc\sys\borlandc.err
del ..\inc\sys\highc.err
del ..\inc\sys\digmars.err
del ..\inc\sys\ladsoft.err
del ..\inc\sys\visualc.err
del ..\inc\sys\mingw32.err
del ..\inc\sys\mingw64.err
del ..\inc\sys\pellesc.err
del ..\inc\sys\highc.err
del ..\inc\sys\lcc.err
del ..\inc\sys\clang.err
goto next

::------------------------------------------------------------
:all
::
call %0 borland   %2
call %0 watcom    %2
call %0 djgpp     %2
call %0 djcross   %2
call %0 digmars   %2
call %0 ladsoft   %2
call %0 visualc   %2
call %0 mingw32   %2
call %0 mingw64   %2
call %0 cygwin    %2
call %0 cygwin64  %2
call %0 watcom    %2
call %0 lcc       %2
call %0 clang     %2
call %0 pellesc   %2
call %0 highc     %2
:next
shift
echo.

if %1.==. goto quit
goto start

:not_set
echo Environment variable WATT_ROOT not set (or incorrectly set).
echo Put this in your AUTOEXEC.BAT or environment:
echo   e.g. "SET WATT_ROOT=C:\NET\WATT"

:quit


@echo off
if not %1. == clean. call configur.bat mingw32 mingw64 cygwin64 visualc watcom
if %1. == clean.     call configur.bat clean

echo ---- running visualc-release.mak %1 ---- >> make.log
nmake -f visualc-release.mak %1 >> make.log

echo ---- running visualc-debug.mak %1 ---- >> make.log
nmake -f visualc-debug.mak %1 >> make.log

call cl64.bat
echo ---- running visualc-release_64.mak %1 ---- >> make.log
nmake -f visualc-release_64.mak %1 >> make.log

echo ---- running visualc-debug_64.mak %1 ---- >> make.log
nmake -f visualc-debug_64.mak %1 >> make.log

echo ---- running watcom_w.mak %1 ---- >> make.log
wmake -f watcom_w.mak %1 >> make.log

call mingw.bat
echo ---- running mingw32.mak %1 ---- > make.log
make -f mingw32.mak  %1 2>> make.log

call tdm-gcc.bat
echo ---- running mingw64_32.mak %1 ---- > make.log
make -f mingw64_32.mak  %1 2>> make.log

echo ---- running mingw64_64.mak %1 ---- > make.log
make -f mingw64_64.mak  %1 2>> make.log

call cygwin64.bat
echo ---- running cygwin_64.mak %1 ---- >> make.log
make -f cygwin_64.mak %1 2>> make.log


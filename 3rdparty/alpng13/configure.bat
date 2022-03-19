@echo off

rem AllegroPNG build configurator

if [%1] == [mingw]   goto mingw
if [%1] == [mingw32] goto mingw
if [%1] == [msvc]    goto msvc
goto help

:mingw
if [%2] == []         goto mingw_standalone
if [%2] == [zlib]     goto mingw_zlib
if [%2] == [crypto++] goto mingw_crypto
if [%2] == [cryptopp] goto mingw_crypto
goto help

:mingw_standalone
copy makefiles\makefile.mingw0 makefile
goto end

:mingw_zlib
copy makefiles\makefile.mingw1 makefile
goto end

:mingw_crypto
copy makefiles\makefile.mingw2 makefile
goto end

:msvc
if [%2] == []         goto msvc_standalone
if [%2] == [zlib]     goto msvc_zlib
if [%2] == [crypto++] goto msvc_crypto
if [%2] == [cryptopp] goto msvc_crypto
goto help

:msvc_standalone
copy makefiles\makefile.msvc0 makefile
goto end

:msvc_zlib
copy makefiles\makefile.msvc1 makefile
goto end

:msvc_crypto
copy makefiles\makefile.msvc2 makefile
goto end

:help
echo.
echo Usage: configure.bat platform [dependency]
echo.
echo Where platform is one of:
echo     mingw for MinGW
echo     msvc  for MS Visual C++
echo.
echo The optional dependency can be one of:
echo     zlib for dependency on zlib
echo     crypto++ for dependency on Crypto++
echo If you are not 100%% sure don't set this parameter.
echo.
echo See readme.txt for details.
echo.
goto realend

:end
echo Finished.

:realend


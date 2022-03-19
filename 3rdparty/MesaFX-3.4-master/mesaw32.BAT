@echo off

set _argc=0
:loops
if [%1] == [] goto loope
If _argc == 0 set _args=%1
If Not _argc == 0 set _args=%_args% %1
set _argc=1
shift
goto loops
:loope

if %_argc% == 0 set _args=help

cd win32
nmake /f nmake.mak %_args%
cd ..

set _args=
set _argc=

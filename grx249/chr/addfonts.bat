rem We are not distributing Borland BGI fonts with GRX.
rem To add them to the library put .CHR files in this
rem directory (where this script is located) and run it
rem after building GRX
rem This file is for Borland C.

if "%1"=="" addfonts bold euro goth lcom litt sans scri simp trip tscr

del *.obj
del addfonts.rsp
bcc -O -ml -ebin2c.exe ..\src\utilprog\bin2c.c

:proc
if not exist %1.chr goto next

echo Processing %1.chr
bin2c %1.chr _%1_font %1.c
bcc -c -O -ml %1.c
echo +%1.obj & >> addfonts.rsp
del %1.c

:next
shift
if not "%1"=="" goto proc

:done
echo , >> addfonts.rsp
tlib ..\lib\bcc\grx20l.lib @addfonts.rsp

del *.obj
del addfonts.rsp
del ..\lib\bcc\grx20l.bak

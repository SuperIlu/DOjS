@echo off
rem Remove next for no mapfile
echo >mesados.lnk option map=mapfile
rem Remove next line for no debug info
echo >>mesados.lnk debug all
rem Comment out next two lines if you aren`t using univbe VESA support
echo >>mesados.lnk library \scitech\lib\dos32\wc\svga.lib
echo >>mesados.lnk library \scitech\lib\dos32\wc\svga.lib
rem MGL support, isn`t finished yet. sorry!.
rem echo >>mesados.lnk library \scitech\lib\dos32\wc\mgllt.lib
echo >>mesados.lnk library \scitech\lib\dos32\wc\dos4gw\pmode.lib
rem these are the main mesa libs
echo >>mesados.lnk library ..\src-glu\dosglub.lib
echo >>mesados.lnk library ..\src-tk\tkdos.lib
echo >>mesados.lnk library ..\src\dosmesa.lib
echo >>mesados.lnk library ..\src-aux\dosaux.lib
rem set wcl386=/DDOSVGA /I\mesa\include /d3 /5r /mf /wx /x /fp5 /k80000
set wcl386=/DDOSVGA /I\mesa\include /5r /fp5 /wx /x /d1 /otexan /mf /k80000
wcl386 %1 %2 %3 %4 %5 %6 %7 @mesados
goto exit
wstrip %1.exe
del %1.obj
%1
pause
del %1.exe
:exit
rem del mesados.lnk
set wcl386=

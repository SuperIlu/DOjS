#
#  Makefile for the Errno utility used for Watt-32 developement.
#
#  NOTE: Some make programs may not be able to generate all
#        targets due to DPMI/DOS-extender conflicts etc.
#        Use your native make tool to make only the target
#        you need.
#          E.g. say: 'wmake -u -h -f errnos.mak wc_err.exe' for Watcom
#                or: 'maker -h -f errnos.mak hc_err.exe' for High-C
#
# Therefore, do not use any GNU-make specific syntax here.
# (use explicit rules only). Thus, this makefile could also
# be used with GNU-make.
#
PROGRAMS = bcc_err.exe  \
           dm_err.exe   \
           tcc_err.exe  \
           wc_err.exe   \
           hc_err.exe   \
           dj_err.exe   \
           lcc_err.exe

#
# These are the Windows version for some of the above;
# I.e. when building under Windows (%OS%=Windows_NT), one
# of these MUST be used instead of the above.
#
WIN_PROGRAMS = win32/clang_err.exe \
               win32/wc_err.exe    \
               win32/bcc_err.exe   \
               win32/dj_err.exe    \
               win32/dm_err.exe    \
               win32/ls_err.exe    \
               win32/oc_err.exe    \
               win32/vc_err.exe    \
             # win32/hc_err.exe

all:   $(PROGRAMS) $(WIN_PROGRAMS)
win32: $(WIN_PROGRAMS)

bcc_err.exe: errnos.c
	bcc -I..\inc -ml -ebcc_err.exe errnos.c

win32/bcc_err.exe: errnos.c
	bcc32c -I..\inc -w -M -ewin32/bcc_err.exe errnos.c

tcc_err.exe: errnos.c
	tcc -I..\inc -ml -etcc_err.exe errnos.c

wc_err.exe: errnos.c
	wcl -zq -bcl=dos -ml -wx -I"../inc" -I"$(%WATCOM)/h" -fe=wc_err.exe errnos.c

win32/wc_err.exe: errnos.c
	wcl386 -zq -bcl=nt -mf -wx -I"../inc" -I"$(%WATCOM)/h" -I"$(%WATCOM)/h/nt" -fe=win32/wc_err.exe errnos.c

hc_err.exe: errnos.c
	hc386 -I..\inc -Hldopt=-nomap -Hnocopyr -o hc_err.exe errnos.c

#
# Does not work
#
win32/hc_err.exe: errnos.c
	hc386 -I..\inc -Hldopt=-nomap -Hnocopyr -nostub -o win32/hc_err.exp errnos.c
	$(MWBIND) mwsup.exe win32/hc_err.exp win32/hc_err.exe
	rm -f win32/hc_err.obj

dm_err.exe: errnos.c
	dmc -ml -g -I..\inc -odm_err.exe errnos.c

win32/dm_err.exe: errnos.c
	dmc -mn -g -I..\inc -owin32\dm_err.exe errnos.c

win32/vc_err.exe: errnos.c
	cl -nologo -I..\inc -DWIN32 -Fe./win32/vc_err.exe errnos.c

#
# See errnos.c for the idea behind this.
#
mw64_err.exe: errnos.c
	gcc -m32 -s -I../inc -DMAKE_MINGW64_ERRNOS -o mw64_err.exe errnos.c

mw_err.exe: errnos.c
	gcc -s -I../inc -o mw_err.exe errnos.c

#
# Orange-C compiler
# Refs:
#   https://orangec.readthedocs.io/en/latest/Tools/
#   https://github.com/LADSoft/OrangeC/
#
win32/oc_err.exe: errnos.c
	$(ORANGEC)\bin\occ --nologo -Wc -1 -I..\inc -I$(ORANGEC)\include -owin32/oc_err.exe errnos.c
	- rm -f errnos.o

#
# Note: 'cl386' is LadSoft's compiler. If you have <WATCOM_ROOT\binnt>' in
#       your PATH, put it after Ladsoft's .'\bin' dir.
#
win32/ls_err.exe: errnos.c
	$(LADSOFT)\bin\cl386 -v+ -9 -E0 -I../inc -W32CON -e=win32\ls_err.exe errnos.c

lcc_err.exe: errnos.c
	lcc -I..\inc -A errnos.c
	lcclnk errnos.obj -o lcc_err.exe

po_err.exe: errnos.c
	pocc -Ze -c -I$(PELLESC)\include -I$(PELLESC)\include\win -I..\inc errnos.c
	polink -out:$@ -libpath:$(PELLESC)\lib errnos.obj

win32/clang_err.exe: errnos.c
	clang-cl -c -D_CRT_SECURE_NO_DEPRECATE -I../inc -o clang_err.obj errnos.c
	link -nologo -out:win32/clang_err.exe clang_err.obj
	rm -f clang_err.obj

#
# These targets requires GNU-make syntax.
# Hence put these in a separate makefile.
#
dj_err.exe:
	$(MAKE) -f dj-errno.mak dj_err.exe

win32/dj_err.exe::
	$(MAKE) -f dj-errno.mak win32/dj_err.exe

clean:
	@del bcc_err.exe win32\clang_err.exe wc_err.exe hc_err.exe \
         tcc_err.exe dj_err.exe mw_err.exe mw64_err.exe win32\ls_err.exe \
         lcc.exe win32\oc_err.exe po_err.exe win32\vc_err.exe win32\wc_err.exe \
         win32\bcc_err.exe win32\hc_err.exe errnos.obj


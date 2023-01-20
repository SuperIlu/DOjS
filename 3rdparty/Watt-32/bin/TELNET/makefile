#
#  Makefile for Waterloo miniTelnet
#
#  Targets:
#     Borland 4.0+        (DOS large model)
#     Metaware HighC 3.1+ (PharLap 386|DosX)
#     Watcom C 11+        (DOS4GW/WDOSX/Pharlap)
#

SRC = config.c keyb.c main.c telnet.c screen.c vt102.c int29.c ssh.c \
      md5.c rsa.c des.c blowfish.c script.c timer.c

EXTRA_WCLIB = dos4g.w3s\conio.lib  # MwConio lib for Watcom
EXTRA_HCLIB = conio                # MwConio lib for Metaware
HC386_DEFS  = -Hon=check_stack     # -Hon=call_trace

BORLAND_EXE = telnet.exe
PHARLAP_EXP = telnet.exp
WATCOM_EXE  = telnet4.exe
DIGMARS_EXE = nul-1
QUICKC_EXE  = nul-2

!include "..\common.mak"

################### Test programs ########################

keyb.exe: keyb.c
          bcc.exe -DTEST -I$(INC) keyb.c

keyb.exp: keyb.c
          hc386 -DTEST -w3 -g -c -I$(INC) $(HC386_DEFS) $*.c
          386link @&&|
            $*.obj
            -lib conio,hc386,hc387,dosx32,exc_hc
            -offset 1000h -stack 8192 -386 -twocase -nostub
            -unprivileged -fullwarn -maxdata 0 -fullseg -symbols
            -publist both -purge none * -mapnames 30 -mapwidth 132 -pack
            -attributes class code  er
            -attributes class data  rw
            -attributes class stack rw
|


#
#  Makefile for _some_ Watt-32 tcp/ip test programs.
#  High-C / Pharlap version only
#

CC     = hc386
CFLAGS = -g -I.. -I..\..\inc -DTEST_PROG -DWATT32 -DBUGGY_FARPTR=1 \
         -Hnocopyr -Hpragma=Offwarn(572)

LINK = 386link

all: cpu.exp cpuspeed.exp chksum.exp ttime.exp gettod.exp geteth.exp \
     getserv.exp ip4_frag.exp tftp.exp fingerd.exp ioctl.exp \
     gethost.exp gethost6.exp misc.exe idna.exp select.exp \
     language.exp

cpu.exp: hc386.arg 386link.arg cpu.c
    $(CC) -c @hc386.arg $*.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

cpuspeed.exp: hc386.arg 386link.arg cpuspeed.c
    $(CC) -c @hc386.arg $*.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

chksum.exp: hc386.arg 386link.arg chksum.c
    $(CC) -c @hc386.arg $*.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

tftp.exp: hc386.arg 386link.arg ..\tftp.c
    $(CC) -c @hc386.arg ..\tftp.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

ttime.exp: hc386.arg 386link.arg ttime.c
    $(CC) -c @hc386.arg $*.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

gettod.exp: hc386.arg 386link.arg ..\gettod.c
    $(CC) -c @hc386.arg ..\gettod.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

geteth.exp: hc386.arg 386link.arg ..\geteth.c
    $(CC) -c @hc386.arg ..\geteth.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

getserv.exp: hc386.arg 386link.arg ..\getserv.c
    $(CC) -c @hc386.arg ..\getserv.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

gethost.exp: hc386.arg 386link.arg ..\gethost.c
    $(CC) -c @hc386.arg ..\gethost.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

gethost6.exp: hc386.arg 386link.arg ..\gethost6.c
    $(CC) -c @hc386.arg ..\gethost6.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

ip4_frag.exp: hc386.arg 386link.arg ..\ip4_frag.c
    $(CC) -c @hc386.arg ..\ip4_frag.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

fingerd.exp: hc386.arg 386link.arg ..\listen.c
    $(CC) -c @hc386.arg ..\listen.c
    $(LINK) @386link.arg listen.obj -exe fingerd.exp

ioctl.exp: hc386.arg 386link.arg ..\ioctl.c
    $(CC) -c @hc386.arg ..\ioctl.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

pc_cbrk.exp: hc386.arg 386link.arg ..\pc_cbrk.c
    $(CC) -c @hc386.arg ..\pc_cbrk.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

misc.exp: hc386.arg 386link.arg ..\misc.c
    $(CC) -c @hc386.arg ..\misc.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

idna.exp: hc386.arg 386link.arg ..\idna.c
    $(CC) -c @hc386.arg ..\idna.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

language.exp: hc386.arg 386link.arg ..\language.c
    $(CC) -c @hc386.arg ..\language.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

select.exp: hc386.arg 386link.arg ..\select.c
    $(CC) -c @hc386.arg ..\select.c
    $(LINK) @386link.arg $*.obj -exe $*.exp

386link.arg: Makefile.hc
      copy &&|
        -lib ..\..\lib\wattcphf     # Watt-32 TCP/IP for MetaWare/PharLap
        -libpath $(LIBPATH)         # Path to below libs
        -lib hc386,hc387,hcna       # MetaWare HighC libraries
        -lib dosx32                 # PharLap DOSX API library
        -lib exc_hc.lib             # Exception handler library
        -offset 1000h               # start at 4kB (traps NULL-pointers)
        -stack 300000               # >256 kByte stack
        -386                        # 386 (or later) processor target
        -twocase                    # case sensitive link
        -nostub                     # don't prepend a stub
        -unprivileged               # run at ring 3
        -fullwarn                   # give all warnings
        -maxdata 0                  # limit data to what we use (no heap)
        -symbols                    # -symbols (386debug) or -cvsymbols (Mdb)
        -publist both               # map-list by name and address
        -purge none *
        -mapnames 30
        -mapwidth 132
        -pack                       # pack BSS segment
| $<

hc386.arg: Makefile.hc
         @copy &&|
           $(CFLAGS)
| $<

clean:
    - del *.obj
    - del *.map
    - del *.exp
    - del hc386.arg
    - del 386link.arg


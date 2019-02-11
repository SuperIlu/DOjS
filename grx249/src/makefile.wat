!define BLANK ""

## Object Files
OBJS = .\bmp.obj .\grxprint.obj .\prndata.obj .\bitblt.obj .\bitbltnc.obj &
 .\box.obj .\boxnc.obj .\clearclp.obj .\clearctx.obj .\clearscr.obj &
 .\drwinlne.obj .\fillbox.obj .\fillboxn.obj .\frambox.obj .\framboxn.obj &
 .\getscl.obj .\line.obj .\linenc.obj .\majorln1.obj .\majorln2.obj &
 .\majorln3.obj .\majorln4.obj .\pixel.obj .\pixelc.obj .\plot.obj &
 .\putscl.obj .\dotab8.obj .\ega4.obj .\egavga1.obj .\ftable.obj &
 .\genblit.obj .\gengiscl.obj .\genptscl.obj .\genstrch.obj .\herc1.obj &
 .\lfb16.obj .\lfb24.obj .\lfb32h.obj .\lfb32l.obj .\lfb8.obj &
 .\lfbbltrv.obj .\lfbbltvr.obj .\lfbbltvv.obj .\pblitr2r.obj .\pblitr2v.obj &
 .\pblitv2r.obj .\pblitv2v.obj .\ram1.obj .\ram16.obj .\ram24.obj &
 .\ram32h.obj .\ram32l.obj .\ram4.obj .\ram8.obj .\rblit_14.obj &
 .\svga16.obj .\svga24.obj .\svga32h.obj .\svga32l.obj .\svga4.obj &
 .\svga8.obj .\vga8x.obj .\fdtable.obj .\fdv_bgi.obj .\fdv_grx.obj &
 .\pc6x8.obj .\pc8x14.obj .\pc8x16.obj .\pc8x8.obj .\ialloc.obj &
 .\ibuild.obj .\ifbox.obj .\ihline.obj .\iinverse.obj .\imginlne.obj &
 .\iplot.obj .\istretch.obj .\bldcurs.obj .\dosinput.obj .\doskeys.obj &
 .\drawcurs.obj .\mouinfo.obj .\mouinlne.obj .\mscursor.obj .\fillpatt.obj &
 .\makepat.obj .\patfbits.obj .\patfbox.obj .\patfcvxp.obj .\patfline.obj &
 .\patfplot.obj .\patfpoly.obj .\patternf.obj .\pattfldf.obj .\pattline.obj &
 .\pattpoly.obj .\pfcirc.obj .\pfcirca.obj .\pfelli.obj .\pfellia.obj &
 .\ptcirc.obj .\ptcirca.obj .\ptelli.obj .\ptellia.obj .\clip.obj &
 .\clrinfo.obj .\clrinlne.obj .\colorbw.obj .\colorega.obj .\colors.obj &
 .\context.obj .\cxtinfo.obj .\cxtinlne.obj .\drvinfo.obj .\drvinlne.obj &
 .\fframe.obj .\fgeom.obj .\hooks.obj .\modewalk.obj .\setdrvr.obj &
 .\setmode.obj .\version.obj .\viewport.obj .\circle1.obj .\circle2.obj &
 .\circle3.obj .\circle4.obj .\drawpoly.obj .\fillcir1.obj .\fillcir2.obj &
 .\fillcnvx.obj .\fillell1.obj .\fillell2.obj .\fillpoly.obj .\flood.obj &
 .\floodfil.obj .\genellip.obj .\polygon.obj .\polyline.obj .\scancnvx.obj &
 .\scanellp.obj .\scanpoly.obj .\solidfil.obj .\buildaux.obj .\buildfnt.obj &
 .\convfont.obj .\drawstrg.obj .\drawtext.obj .\drwstrg.obj .\dumpfont.obj &
 .\dumptext.obj .\epatstrg.obj .\fntinlne.obj .\fontinfo.obj .\fontpath.obj &
 .\loadfont.obj .\pattstrg.obj .\propwdt.obj .\unloadfn.obj .\ubox.obj &
 .\ucbox.obj .\uccirc.obj .\uccirca.obj .\ucelli.obj .\ucellia.obj &
 .\ucirc.obj .\ucirca.obj .\ucircf.obj .\ucircfa.obj .\ucline.obj &
 .\ucpolyg.obj .\ucpolyl.obj .\udrwchar.obj .\udrwstrg.obj .\uelli.obj &
 .\uellia.obj .\uellif.obj .\uellifa.obj .\ufcpolyg.obj .\ufillbox.obj &
 .\uflood.obj .\ufpolyg.obj .\uframbox.obj .\ugetwin.obj .\uhline.obj &
 .\uline.obj .\upbox.obj .\upcirc.obj .\upcirca.obj .\upelli.obj &
 .\upellia.obj .\upfbox.obj .\upfcirc.obj .\upfcirca.obj .\upfcpoly.obj &
 .\upfelli.obj .\upfellia.obj .\upfflood.obj .\upfline.obj .\upfplot.obj &
 .\upfpolyg.obj .\upixel.obj .\upixelc.obj .\upline.obj .\uplot.obj &
 .\upolygon.obj .\upolylin.obj .\uppolyg.obj .\uppolyl.obj .\usercord.obj &
 .\usetwin.obj .\utextxy.obj .\uvline.obj .\ordswap.obj .\resize.obj &
 .\shiftscl.obj .\strmatch.obj .\tmpbuff.obj .\watcom32.obj .\ati28800.obj &
 .\cl5426.obj .\et4000.obj .\herc.obj .\mach64.obj .\s3.obj .\stdega.obj &
 .\stdvga.obj .\u_egavga.obj .\u_vesa.obj .\u_vsvirt.obj .\vd_mem.obj &
 .\vesa.obj .\vtable.obj .\ccirc.obj .\ccirca.obj .\celli.obj .\cellia.obj &
 .\custbox.obj .\custline.obj .\custplne.obj .\custpoly.obj .\drwcpoly.obj

OBJS += .\arc.obj .\aspectra.obj .\bar.obj .\bar3d.obj .\bccgrx.obj &
 .\bgimode.obj .\circle.obj .\clearvp.obj .\closegra.obj &
 .\clrdev.obj .\detectg.obj .\drvname.obj .\egacolor.obj .\ellipse.obj &
 .\errmsg.obj .\fellipse.obj .\fillpatt.obj .\fillpoly.obj .\fillstyl.obj &
 .\fldfill.obj .\getbkcol.obj .\getcol.obj .\getdefpa.obj .\getfillp.obj &
 .\getfills.obj .\getgramo.obj .\getimage.obj .\getmaxmo.obj .\getmoran.obj &
 .\getpixel.obj .\getviewp.obj .\getx.obj .\gety.obj .\gmaxcol.obj &
 .\gmmaxcol.obj .\gmmaxx.obj .\gmmaxy.obj .\gpalsize.obj .\graphres.obj &
 .\imagesze.obj .\instbgid.obj .\line.obj .\linerel.obj .\lineto.obj &
 .\lnestyle.obj .\modename.obj .\moverel.obj .\moveto.obj .\page1.obj &
 .\page2.obj .\page3.obj .\page4.obj .\page5.obj .\page6.obj .\palette.obj &
 .\pieslice.obj .\polygon.obj .\putimage.obj .\putpixel.obj .\rectang.obj &
 .\regbgidr.obj .\rgbpal_g.obj .\rgbpal_s.obj .\rstcrtmd.obj .\sector.obj &
 .\setbgiwh.obj .\setbkcol.obj .\setbusze.obj .\setcolor.obj .\setrgbc.obj &
 .\setviewp.obj .\setwrmod.obj .\text.obj .\text1.obj .\text2.obj &
 .\text3.obj .\text4.obj .\text5.obj .\text6.obj .\text7.obj .\text8.obj &
 .\text9.obj .\texta.obj .\textb.obj .\textc.obj .\textd.obj .\txtlnest.obj &
 .\bgiext01.obj .\bgiext01.obj

!ifdef DEBUG
OBJS += .\dbgprint.obj
!endif
OBJS += .AUTODEPEND

## implicit rules do not seem to work with wmake - it complains about
## no default action???
.c.obj:
	$(CC) $[@ $(CC_OPTS)

## Rules
.\bmp.obj : .\addons\bmp\bmp.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\grxprint.obj : .\addons\print\grxprint.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\prndata.obj : .\addons\print\prndata.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\bitblt.obj : .\src\draw\bitblt.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\bitbltnc.obj : .\src\draw\bitbltnc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\box.obj : .\src\draw\box.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\boxnc.obj : .\src\draw\boxnc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\clearclp.obj : .\src\draw\clearclp.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\clearctx.obj : .\src\draw\clearctx.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\clearscr.obj : .\src\draw\clearscr.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\drwinlne.obj : .\src\draw\drwinlne.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fillbox.obj : .\src\draw\fillbox.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fillboxn.obj : .\src\draw\fillboxn.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\frambox.obj : .\src\draw\frambox.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\framboxn.obj : .\src\draw\framboxn.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getscl.obj : .\src\draw\getscl.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\line.obj : .\src\draw\line.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\linenc.obj : .\src\draw\linenc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\majorln1.obj : .\src\draw\majorln1.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\majorln2.obj : .\src\draw\majorln2.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\majorln3.obj : .\src\draw\majorln3.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\majorln4.obj : .\src\draw\majorln4.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pixel.obj : .\src\draw\pixel.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pixelc.obj : .\src\draw\pixelc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\plot.obj : .\src\draw\plot.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\putscl.obj : .\src\draw\putscl.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\dotab8.obj : .\src\fdrivers\dotab8.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ega4.obj : .\src\fdrivers\ega4.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\egavga1.obj : .\src\fdrivers\egavga1.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ftable.obj : .\src\fdrivers\ftable.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\genblit.obj : .\src\fdrivers\genblit.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\gengiscl.obj : .\src\fdrivers\gengiscl.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\genptscl.obj : .\src\fdrivers\genptscl.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\genstrch.obj : .\src\fdrivers\genstrch.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\herc1.obj : .\src\fdrivers\herc1.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\lfb16.obj : .\src\fdrivers\lfb16.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\lfb24.obj : .\src\fdrivers\lfb24.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\lfb32h.obj : .\src\fdrivers\lfb32h.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\lfb32l.obj : .\src\fdrivers\lfb32l.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\lfb8.obj : .\src\fdrivers\lfb8.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\lfbbltrv.obj : .\src\fdrivers\lfbbltrv.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\lfbbltvr.obj : .\src\fdrivers\lfbbltvr.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\lfbbltvv.obj : .\src\fdrivers\lfbbltvv.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pblitr2r.obj : .\src\fdrivers\pblitr2r.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pblitr2v.obj : .\src\fdrivers\pblitr2v.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pblitv2r.obj : .\src\fdrivers\pblitv2r.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pblitv2v.obj : .\src\fdrivers\pblitv2v.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ram1.obj : .\src\fdrivers\ram1.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ram16.obj : .\src\fdrivers\ram16.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ram24.obj : .\src\fdrivers\ram24.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ram32h.obj : .\src\fdrivers\ram32h.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ram32l.obj : .\src\fdrivers\ram32l.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ram4.obj : .\src\fdrivers\ram4.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ram8.obj : .\src\fdrivers\ram8.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\rblit_14.obj : .\src\fdrivers\rblit_14.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\svga16.obj : .\src\fdrivers\svga16.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\svga24.obj : .\src\fdrivers\svga24.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\svga32h.obj : .\src\fdrivers\svga32h.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\svga32l.obj : .\src\fdrivers\svga32l.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\svga4.obj : .\src\fdrivers\svga4.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\svga8.obj : .\src\fdrivers\svga8.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\vga8x.obj : .\src\fdrivers\vga8x.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fdtable.obj : .\src\fonts\fdtable.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fdv_bgi.obj : .\src\fonts\fdv_bgi.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fdv_grx.obj : .\src\fonts\fdv_grx.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pc6x8.obj : .\src\fonts\pc6x8.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pc8x14.obj : .\src\fonts\pc8x14.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pc8x16.obj : .\src\fonts\pc8x16.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pc8x8.obj : .\src\fonts\pc8x8.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ialloc.obj : .\src\image\ialloc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ibuild.obj : .\src\image\ibuild.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ifbox.obj : .\src\image\ifbox.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ihline.obj : .\src\image\ihline.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\iinverse.obj : .\src\image\iinverse.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\imginlne.obj : .\src\image\imginlne.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\iplot.obj : .\src\image\iplot.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\istretch.obj : .\src\image\istretch.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\bldcurs.obj : .\src\mouse\bldcurs.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\dosinput.obj : .\src\mouse\dosinput.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\doskeys.obj : .\src\mouse\doskeys.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\drawcurs.obj : .\src\mouse\drawcurs.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\mouinfo.obj : .\src\mouse\mouinfo.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\mouinlne.obj : .\src\mouse\mouinlne.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\mscursor.obj : .\src\mouse\mscursor.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fillpatt.obj : .\src\pattern\fillpatt.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\makepat.obj : .\src\pattern\makepat.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\patfbits.obj : .\src\pattern\patfbits.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\patfbox.obj : .\src\pattern\patfbox.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\patfcvxp.obj : .\src\pattern\patfcvxp.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\patfline.obj : .\src\pattern\patfline.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\patfplot.obj : .\src\pattern\patfplot.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\patfpoly.obj : .\src\pattern\patfpoly.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\patternf.obj : .\src\pattern\patternf.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pattfldf.obj : .\src\pattern\pattfldf.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pattline.obj : .\src\pattern\pattline.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pattpoly.obj : .\src\pattern\pattpoly.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pfcirc.obj : .\src\pattern\pfcirc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pfcirca.obj : .\src\pattern\pfcirca.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pfelli.obj : .\src\pattern\pfelli.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pfellia.obj : .\src\pattern\pfellia.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ptcirc.obj : .\src\pattern\ptcirc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ptcirca.obj : .\src\pattern\ptcirca.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ptelli.obj : .\src\pattern\ptelli.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ptellia.obj : .\src\pattern\ptellia.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\clip.obj : .\src\setup\clip.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\clrinfo.obj : .\src\setup\clrinfo.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\clrinlne.obj : .\src\setup\clrinlne.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\colorbw.obj : .\src\setup\colorbw.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\colorega.obj : .\src\setup\colorega.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\colors.obj : .\src\setup\colors.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\context.obj : .\src\setup\context.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\cxtinfo.obj : .\src\setup\cxtinfo.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\cxtinlne.obj : .\src\setup\cxtinlne.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\drvinfo.obj : .\src\setup\drvinfo.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\drvinlne.obj : .\src\setup\drvinlne.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fframe.obj : .\src\setup\fframe.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fgeom.obj : .\src\setup\fgeom.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\hooks.obj : .\src\setup\hooks.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\modewalk.obj : .\src\setup\modewalk.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\setdrvr.obj : .\src\setup\setdrvr.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\setmode.obj : .\src\setup\setmode.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\version.obj : .\src\setup\version.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\viewport.obj : .\src\setup\viewport.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\circle1.obj : .\src\shape\circle1.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\circle2.obj : .\src\shape\circle2.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\circle3.obj : .\src\shape\circle3.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\circle4.obj : .\src\shape\circle4.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\drawpoly.obj : .\src\shape\drawpoly.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fillcir1.obj : .\src\shape\fillcir1.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fillcir2.obj : .\src\shape\fillcir2.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fillcnvx.obj : .\src\shape\fillcnvx.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fillell1.obj : .\src\shape\fillell1.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fillell2.obj : .\src\shape\fillell2.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fillpoly.obj : .\src\shape\fillpoly.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\flood.obj : .\src\shape\flood.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\floodfil.obj : .\src\shape\floodfil.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\genellip.obj : .\src\shape\genellip.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\polygon.obj : .\src\shape\polygon.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\polyline.obj : .\src\shape\polyline.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\scancnvx.obj : .\src\shape\scancnvx.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\scanellp.obj : .\src\shape\scanellp.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\scanpoly.obj : .\src\shape\scanpoly.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\solidfil.obj : .\src\shape\solidfil.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\buildaux.obj : .\src\text\buildaux.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\buildfnt.obj : .\src\text\buildfnt.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\convfont.obj : .\src\text\convfont.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\drawstrg.obj : .\src\text\drawstrg.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\drawtext.obj : .\src\text\drawtext.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\drwstrg.obj : .\src\text\drwstrg.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\dumpfont.obj : .\src\text\dumpfont.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\dumptext.obj : .\src\text\dumptext.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\epatstrg.obj : .\src\text\epatstrg.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fntinlne.obj : .\src\text\fntinlne.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fontinfo.obj : .\src\text\fontinfo.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fontpath.obj : .\src\text\fontpath.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\loadfont.obj : .\src\text\loadfont.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pattstrg.obj : .\src\text\pattstrg.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\propwdt.obj : .\src\text\propwdt.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\unloadfn.obj : .\src\text\unloadfn.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ubox.obj : .\src\user\ubox.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ucbox.obj : .\src\user\ucbox.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uccirc.obj : .\src\user\uccirc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uccirca.obj : .\src\user\uccirca.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ucelli.obj : .\src\user\ucelli.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ucellia.obj : .\src\user\ucellia.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ucirc.obj : .\src\user\ucirc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ucirca.obj : .\src\user\ucirca.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ucircf.obj : .\src\user\ucircf.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ucircfa.obj : .\src\user\ucircfa.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ucline.obj : .\src\user\ucline.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ucpolyg.obj : .\src\user\ucpolyg.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ucpolyl.obj : .\src\user\ucpolyl.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\udrwchar.obj : .\src\user\udrwchar.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\udrwstrg.obj : .\src\user\udrwstrg.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uelli.obj : .\src\user\uelli.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uellia.obj : .\src\user\uellia.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uellif.obj : .\src\user\uellif.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uellifa.obj : .\src\user\uellifa.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ufcpolyg.obj : .\src\user\ufcpolyg.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ufillbox.obj : .\src\user\ufillbox.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uflood.obj : .\src\user\uflood.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ufpolyg.obj : .\src\user\ufpolyg.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uframbox.obj : .\src\user\uframbox.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ugetwin.obj : .\src\user\ugetwin.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uhline.obj : .\src\user\uhline.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uline.obj : .\src\user\uline.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upbox.obj : .\src\user\upbox.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upcirc.obj : .\src\user\upcirc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upcirca.obj : .\src\user\upcirca.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upelli.obj : .\src\user\upelli.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upellia.obj : .\src\user\upellia.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upfbox.obj : .\src\user\upfbox.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upfcirc.obj : .\src\user\upfcirc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upfcirca.obj : .\src\user\upfcirca.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upfcpoly.obj : .\src\user\upfcpoly.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upfelli.obj : .\src\user\upfelli.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upfellia.obj : .\src\user\upfellia.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upfflood.obj : .\src\user\upfflood.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upfline.obj : .\src\user\upfline.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upfplot.obj : .\src\user\upfplot.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upfpolyg.obj : .\src\user\upfpolyg.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upixel.obj : .\src\user\upixel.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upixelc.obj : .\src\user\upixelc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upline.obj : .\src\user\upline.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uplot.obj : .\src\user\uplot.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upolygon.obj : .\src\user\upolygon.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\upolylin.obj : .\src\user\upolylin.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uppolyg.obj : .\src\user\uppolyg.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uppolyl.obj : .\src\user\uppolyl.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\usercord.obj : .\src\user\usercord.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\usetwin.obj : .\src\user\usetwin.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\utextxy.obj : .\src\user\utextxy.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\uvline.obj : .\src\user\uvline.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ordswap.obj : .\src\utils\ordswap.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\resize.obj : .\src\utils\resize.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\shiftscl.obj : .\src\utils\shiftscl.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\strmatch.obj : .\src\utils\strmatch.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\tmpbuff.obj : .\src\utils\tmpbuff.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\watcom32.obj : .\src\utils\watcom32.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ati28800.obj : .\src\vdrivers\ati28800.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\cl5426.obj : .\src\vdrivers\cl5426.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\et4000.obj : .\src\vdrivers\et4000.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\herc.obj : .\src\vdrivers\herc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\mach64.obj : .\src\vdrivers\mach64.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\s3.obj : .\src\vdrivers\s3.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\stdega.obj : .\src\vdrivers\stdega.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\stdvga.obj : .\src\vdrivers\stdvga.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\u_egavga.obj : .\src\vdrivers\u_egavga.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\u_vesa.obj : .\src\vdrivers\u_vesa.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\u_vsvirt.obj : .\src\vdrivers\u_vsvirt.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\vd_mem.obj : .\src\vdrivers\vd_mem.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\vesa.obj : .\src\vdrivers\vesa.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\vtable.obj : .\src\vdrivers\vtable.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ccirc.obj : .\src\wideline\ccirc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ccirca.obj : .\src\wideline\ccirca.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\celli.obj : .\src\wideline\celli.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\cellia.obj : .\src\wideline\cellia.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\custbox.obj : .\src\wideline\custbox.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\custline.obj : .\src\wideline\custline.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\custplne.obj : .\src\wideline\custplne.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\custpoly.obj : .\src\wideline\custpoly.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\drwcpoly.obj : .\src\wideline\drwcpoly.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\dbgprint.obj : .\src\utils\dbgprint.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\arc.obj : .\src\bgi\arc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\aspectra.obj : .\src\bgi\aspectra.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\bar.obj : .\src\bgi\bar.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\bar3d.obj : .\src\bgi\bar3d.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\bccgrx.obj : .\src\bgi\bccgrx.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\bgimode.obj : .\src\bgi\bgimode.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\circle.obj : .\src\bgi\circle.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\clearvp.obj : .\src\bgi\clearvp.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\closegra.obj : .\src\bgi\closegra.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\clrdev.obj : .\src\bgi\clrdev.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\detectg.obj : .\src\bgi\detectg.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\drvname.obj : .\src\bgi\drvname.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\egacolor.obj : .\src\bgi\egacolor.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ellipse.obj : .\src\bgi\ellipse.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\errmsg.obj : .\src\bgi\errmsg.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fellipse.obj : .\src\bgi\fellipse.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fillpatt.obj : .\src\bgi\fillpatt.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fillpoly.obj : .\src\bgi\fillpoly.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fillstyl.obj : .\src\bgi\fillstyl.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fldfill.obj : .\src\bgi\fldfill.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getbkcol.obj : .\src\bgi\getbkcol.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getcol.obj : .\src\bgi\getcol.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getdefpa.obj : .\src\bgi\getdefpa.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getfillp.obj : .\src\bgi\getfillp.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getfills.obj : .\src\bgi\getfills.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getgramo.obj : .\src\bgi\getgramo.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getimage.obj : .\src\bgi\getimage.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getmaxmo.obj : .\src\bgi\getmaxmo.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getmoran.obj : .\src\bgi\getmoran.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getpixel.obj : .\src\bgi\getpixel.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getviewp.obj : .\src\bgi\getviewp.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\getx.obj : .\src\bgi\getx.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\gety.obj : .\src\bgi\gety.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\gmaxcol.obj : .\src\bgi\gmaxcol.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\gmmaxcol.obj : .\src\bgi\gmmaxcol.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\gmmaxx.obj : .\src\bgi\gmmaxx.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\gmmaxy.obj : .\src\bgi\gmmaxy.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\gpalsize.obj : .\src\bgi\gpalsize.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\graphres.obj : .\src\bgi\graphres.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\imagesze.obj : .\src\bgi\imagesze.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\instbgid.obj : .\src\bgi\instbgid.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\line.obj : .\src\bgi\line.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\linerel.obj : .\src\bgi\linerel.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\lineto.obj : .\src\bgi\lineto.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\lnestyle.obj : .\src\bgi\lnestyle.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\modename.obj : .\src\bgi\modename.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\moverel.obj : .\src\bgi\moverel.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\moveto.obj : .\src\bgi\moveto.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\page1.obj : .\src\bgi\page1.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\page2.obj : .\src\bgi\page2.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\page3.obj : .\src\bgi\page3.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\page4.obj : .\src\bgi\page4.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\page5.obj : .\src\bgi\page5.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\page6.obj : .\src\bgi\page6.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\palette.obj : .\src\bgi\palette.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pieslice.obj : .\src\bgi\pieslice.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\polygon.obj : .\src\bgi\polygon.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\putimage.obj : .\src\bgi\putimage.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\putpixel.obj : .\src\bgi\putpixel.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\rectang.obj : .\src\bgi\rectang.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\regbgidr.obj : .\src\bgi\regbgidr.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\rgbpal_g.obj : .\src\bgi\rgbpal_g.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\rgbpal_s.obj : .\src\bgi\rgbpal_s.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\rstcrtmd.obj : .\src\bgi\rstcrtmd.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\sector.obj : .\src\bgi\sector.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\setbgiwh.obj : .\src\bgi\setbgiwh.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\setbkcol.obj : .\src\bgi\setbkcol.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\setbusze.obj : .\src\bgi\setbusze.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\setcolor.obj : .\src\bgi\setcolor.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\setrgbc.obj : .\src\bgi\setrgbc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\setviewp.obj : .\src\bgi\setviewp.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\setwrmod.obj : .\src\bgi\setwrmod.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\text.obj : .\src\bgi\text.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\text1.obj : .\src\bgi\text1.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\text2.obj : .\src\bgi\text2.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\text3.obj : .\src\bgi\text3.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\text4.obj : .\src\bgi\text4.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\text5.obj : .\src\bgi\text5.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\text6.obj : .\src\bgi\text6.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\text7.obj : .\src\bgi\text7.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\text8.obj : .\src\bgi\text8.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\text9.obj : .\src\bgi\text9.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\texta.obj : .\src\bgi\texta.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\textb.obj : .\src\bgi\textb.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\textc.obj : .\src\bgi\textc.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\textd.obj : .\src\bgi\textd.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\txtlnest.obj : .\src\bgi\txtlnest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\bgiext01.obj : .\src\bgi\bgiext01.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\bgiext02.obj : .\src\bgi\bgiext02.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

## wat32mak.lb1 is a text file with the names of all the object files
## this gets around DOS command line length limit

$(GRXLIB) : $(OBJS)
  %create wat32mak.lb1
!ifneq BLANK "bmp.obj grxprint.obj prndata.obj bitblt.obj bitbltnc.obj &
box.obj boxnc.obj clearclp.obj clearctx.obj clearscr.obj drwinlne.obj &
fillbox.obj fillboxn.obj frambox.obj framboxn.obj getscl.obj line.obj &
linenc.obj majorln1.obj majorln2.obj majorln3.obj majorln4.obj pixel.obj &
pixelc.obj plot.obj putscl.obj dotab8.obj ega4.obj egavga1.obj ftable.obj &
genblit.obj gengiscl.obj genptscl.obj genstrch.obj herc1.obj lfb16.obj &
lfb24.obj lfb32h.obj lfb32l.obj lfb8.obj lfbbltrv.obj lfbbltvr.obj &
lfbbltvv.obj pblitr2r.obj pblitr2v.obj pblitv2r.obj pblitv2v.obj ram1.obj &
ram16.obj ram24.obj ram32h.obj ram32l.obj ram4.obj ram8.obj rblit_14.obj &
svga16.obj svga24.obj svga32h.obj svga32l.obj svga4.obj svga8.obj vga8x.obj &
fdtable.obj fdv_bgi.obj fdv_grx.obj pc6x8.obj pc8x14.obj pc8x16.obj &
pc8x8.obj ialloc.obj ibuild.obj ifbox.obj ihline.obj iinverse.obj &
imginlne.obj iplot.obj istretch.obj bldcurs.obj dosinput.obj doskeys.obj &
drawcurs.obj mouinfo.obj mouinlne.obj mscursor.obj fillpatt.obj makepat.obj &
patfbits.obj patfbox.obj patfcvxp.obj patfline.obj patfplot.obj &
patfpoly.obj patternf.obj pattfldf.obj pattline.obj pattpoly.obj pfcirc.obj &
pfcirca.obj pfelli.obj pfellia.obj ptcirc.obj ptcirca.obj ptelli.obj &
ptellia.obj clip.obj clrinfo.obj clrinlne.obj colorbw.obj colorega.obj &
colors.obj context.obj cxtinfo.obj cxtinlne.obj drvinfo.obj drvinlne.obj &
fframe.obj fgeom.obj hooks.obj modewalk.obj setdrvr.obj setmode.obj &
version.obj viewport.obj circle1.obj circle2.obj circle3.obj circle4.obj &
drawpoly.obj fillcir1.obj fillcir2.obj fillcnvx.obj fillell1.obj &
fillell2.obj fillpoly.obj flood.obj floodfil.obj genellip.obj polygon.obj &
polyline.obj scancnvx.obj scanellp.obj scanpoly.obj solidfil.obj &
buildaux.obj buildfnt.obj convfont.obj drawstrg.obj drawtext.obj &
drwstrg.obj dumpfont.obj dumptext.obj epatstrg.obj fntinlne.obj &
fontinfo.obj fontpath.obj loadfont.obj pattstrg.obj propwdt.obj &
unloadfn.obj ubox.obj ucbox.obj uccirc.obj uccirca.obj ucelli.obj &
ucellia.obj ucirc.obj ucirca.obj ucircf.obj ucircfa.obj ucline.obj &
ucpolyg.obj ucpolyl.obj udrwchar.obj udrwstrg.obj uelli.obj uellia.obj &
uellif.obj uellifa.obj ufcpolyg.obj ufillbox.obj uflood.obj ufpolyg.obj &
uframbox.obj ugetwin.obj uhline.obj uline.obj upbox.obj upcirc.obj &
upcirca.obj upelli.obj upellia.obj upfbox.obj upfcirc.obj upfcirca.obj &
upfcpoly.obj upfelli.obj upfellia.obj upfflood.obj upfline.obj upfplot.obj &
upfpolyg.obj upixel.obj upixelc.obj upline.obj uplot.obj upolygon.obj &
upolylin.obj uppolyg.obj uppolyl.obj usercord.obj usetwin.obj utextxy.obj &
uvline.obj ordswap.obj resize.obj shiftscl.obj strmatch.obj tmpbuff.obj &
watcom32.obj ati28800.obj cl5426.obj et4000.obj herc.obj mach64.obj s3.obj &
stdega.obj stdvga.obj u_egavga.obj u_vesa.obj u_vsvirt.obj vd_mem.obj &
vesa.obj vtable.obj ccirc.obj ccirca.obj celli.obj cellia.obj custbox.obj &
custline.obj custplne.obj custpoly.obj drwcpoly.obj"
 @for %i in (bmp.obj grxprint.obj prndata.obj bitblt.obj bitbltnc.obj &
box.obj boxnc.obj clearclp.obj clearctx.obj clearscr.obj drwinlne.obj &
fillbox.obj fillboxn.obj frambox.obj framboxn.obj getscl.obj line.obj &
linenc.obj majorln1.obj majorln2.obj majorln3.obj majorln4.obj pixel.obj &
pixelc.obj plot.obj putscl.obj dotab8.obj ega4.obj egavga1.obj ftable.obj &
genblit.obj gengiscl.obj genptscl.obj genstrch.obj herc1.obj lfb16.obj &
lfb24.obj lfb32h.obj lfb32l.obj lfb8.obj lfbbltrv.obj lfbbltvr.obj &
lfbbltvv.obj pblitr2r.obj pblitr2v.obj pblitv2r.obj pblitv2v.obj ram1.obj &
ram16.obj ram24.obj ram32h.obj ram32l.obj ram4.obj ram8.obj rblit_14.obj &
svga16.obj svga24.obj svga32h.obj svga32l.obj svga4.obj svga8.obj vga8x.obj &
fdtable.obj fdv_bgi.obj fdv_grx.obj pc6x8.obj pc8x14.obj pc8x16.obj &
pc8x8.obj ialloc.obj ibuild.obj ifbox.obj ihline.obj iinverse.obj &
imginlne.obj iplot.obj istretch.obj bldcurs.obj dosinput.obj doskeys.obj &
drawcurs.obj mouinfo.obj mouinlne.obj mscursor.obj fillpatt.obj makepat.obj &
patfbits.obj patfbox.obj patfcvxp.obj patfline.obj patfplot.obj &
patfpoly.obj patternf.obj pattfldf.obj pattline.obj pattpoly.obj pfcirc.obj &
pfcirca.obj pfelli.obj pfellia.obj ptcirc.obj ptcirca.obj ptelli.obj &
ptellia.obj clip.obj clrinfo.obj clrinlne.obj colorbw.obj colorega.obj &
colors.obj context.obj cxtinfo.obj cxtinlne.obj drvinfo.obj drvinlne.obj &
fframe.obj fgeom.obj hooks.obj modewalk.obj setdrvr.obj setmode.obj &
version.obj viewport.obj circle1.obj circle2.obj circle3.obj circle4.obj &
drawpoly.obj fillcir1.obj fillcir2.obj fillcnvx.obj fillell1.obj &
fillell2.obj fillpoly.obj flood.obj floodfil.obj genellip.obj polygon.obj &
polyline.obj scancnvx.obj scanellp.obj scanpoly.obj solidfil.obj &
buildaux.obj buildfnt.obj convfont.obj drawstrg.obj drawtext.obj &
drwstrg.obj dumpfont.obj dumptext.obj epatstrg.obj fntinlne.obj &
fontinfo.obj fontpath.obj loadfont.obj pattstrg.obj propwdt.obj &
unloadfn.obj ubox.obj ucbox.obj uccirc.obj uccirca.obj ucelli.obj &
ucellia.obj ucirc.obj ucirca.obj ucircf.obj ucircfa.obj ucline.obj &
ucpolyg.obj ucpolyl.obj udrwchar.obj udrwstrg.obj uelli.obj uellia.obj &
uellif.obj uellifa.obj ufcpolyg.obj ufillbox.obj uflood.obj ufpolyg.obj &
uframbox.obj ugetwin.obj uhline.obj uline.obj upbox.obj upcirc.obj &
upcirca.obj upelli.obj upellia.obj upfbox.obj upfcirc.obj upfcirca.obj &
upfcpoly.obj upfelli.obj upfellia.obj upfflood.obj upfline.obj upfplot.obj &
upfpolyg.obj upixel.obj upixelc.obj upline.obj uplot.obj upolygon.obj &
upolylin.obj uppolyg.obj uppolyl.obj usercord.obj usetwin.obj utextxy.obj &
uvline.obj ordswap.obj resize.obj shiftscl.obj strmatch.obj tmpbuff.obj &
watcom32.obj ati28800.obj cl5426.obj et4000.obj herc.obj mach64.obj s3.obj &
stdega.obj stdvga.obj u_egavga.obj u_vesa.obj u_vsvirt.obj vd_mem.obj &
vesa.obj vtable.obj ccirc.obj ccirca.obj celli.obj cellia.obj custbox.obj &
arc.obj aspectra.obj bar.obj bar3d.obj bccgrx.obj bgimode.obj circle.obj &
clearvp.obj closegra.obj clrdev.obj detectg.obj drvname.obj egacolor.obj &
ellipse.obj errmsg.obj fellipse.obj fillpatt.obj fillpoly.obj fillstyl.obj &
fldfill.obj getbkcol.obj getcol.obj getdefpa.obj getfillp.obj getfills.obj &
getgramo.obj getimage.obj getmaxmo.obj getmoran.obj getpixel.obj &
getviewp.obj getx.obj gety.obj gmaxcol.obj gmmaxcol.obj gmmaxx.obj &
gmmaxy.obj gpalsize.obj graphres.obj imagesze.obj instbgid.obj line.obj &
linerel.obj lineto.obj lnestyle.obj modename.obj moverel.obj moveto.obj &
page1.obj page2.obj page3.obj page4.obj page5.obj page6.obj palette.obj &
pieslice.obj polygon.obj putimage.obj putpixel.obj rectang.obj regbgidr.obj &
rgbpal_g.obj rgbpal_s.obj rstcrtmd.obj sector.obj \setbgiwh.obj setbkcol.obj &
setbusze.obj setcolor.obj setrgbc.obj setviewp.obj setwrmod.obj text.obj &
text1.obj text2.obj text3.obj text4.obj text5.obj text6.obj text7.obj &
text8.obj text9.obj texta.obj textb.obj textc.obj textd.obj txtlnest.obj &
bgiext01.obj bgiext02.obj custline.obj custplne.obj custpoly.obj drwcpoly.obj&
) do @%append wat32mak.lb1 +'%i'
!endif
!ifdef DEBUG
 @for %i in (dbgprint.obj) do @%append wat32mak.lb1 +'%i'
!endif
!ifneq BLANK ""
 @for %i in () do @%append wat32mak.lb1 +'%i'
!endif
 $(LIB) $(LIB_OPTS) $(GRXLIB) @wat32mak.lb1

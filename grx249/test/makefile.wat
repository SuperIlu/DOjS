!define BLANK ""

##############
# Object Files

.\life.obj : .\test\life.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\bgilink.obj : .\test\bgilink.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\blittest.obj : .\test\blittest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\circtest.obj : .\test\circtest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\cliptest.obj : .\test\cliptest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\colorops.obj : .\test\colorops.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\curstest.obj : .\test\curstest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fnt2c.obj : .\test\fnt2c.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fnt2text.obj : .\test\fnt2text.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fonttest.obj : .\test\fonttest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\imgtest.obj : .\test\imgtest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\keys.obj : .\test\keys.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\arctest.obj : .\test\arctest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\linetest.obj : .\test\linetest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\modetest.obj : .\test\modetest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\mousetst.obj : .\test\mousetst.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\pcirctst.obj : .\test\pcirctst.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\polytest.obj : .\test\polytest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\rgbtest.obj : .\test\rgbtest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\speedtst.obj : .\test\speedtst.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\textpatt.obj : .\test\textpatt.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\winclip.obj : .\test\winclip.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\wintest.obj : .\test\wintest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\scroltst.obj : .\test\scroltst.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\sbctest.obj : .\test\sbctest.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\bccbgi.obj : .\test\bccbgi.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\colortst.obj : .\test\colortst.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\fontplay.obj : .\test\fontplay.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\small.obj : .\test\small.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\tellipse.obj : .\test\tellipse.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\tfill.obj : .\test\tfill.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\tmodes.obj : .\test\tmodes.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\tpoly.obj : .\test\tpoly.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

.\ttext.obj : .\test\ttext.c .AUTODEPEND
	$(CC) $[@ $(CC_OPTS)

##############
# Executables

$(GRX_BIN_SUBDIR)\bccbgi.exe : .\bccbgi.obj $(GRXLIB) .AUTODEPEND
 @%write bccbgi.lk1 FIL bccbgi.obj
 @%append bccbgi.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\bccbgi $(LINK_OPTS) @bccbgi.lk1

$(GRX_BIN_SUBDIR)\colortst.exe : .\colortst.obj $(GRXLIB) .AUTODEPEND
 @%write colortst.lk1 FIL colortst.obj
 @%append colortst.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\ $(LINK_OPTS) @colortst.lk1

$(GRX_BIN_SUBDIR)\fontplay.exe : .\fontplay.obj $(GRXLIB) .AUTODEPEND
 @%write fontplay.lk1 FIL fontplay.obj
 @%append fontplay.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\ $(LINK_OPTS) @fontplay.lk1

$(GRX_BIN_SUBDIR)\small.exe : .\small.obj $(GRXLIB) .AUTODEPEND
 @%write small.lk1 FIL small.obj
 @%append .lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\small $(LINK_OPTS) @small.lk1

$(GRX_BIN_SUBDIR)\tellipse.exe : .\tellipse.obj $(GRXLIB) .AUTODEPEND
 @%write tellipse.lk1 FIL tellipse.obj
 @%append tellipse.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\tellipse $(LINK_OPTS) @tellipse.lk1

$(GRX_BIN_SUBDIR)\tfill.exe : .\tfill.obj $(GRXLIB) .AUTODEPEND
 @%write tfill.lk1 FIL tfill.obj
 @%append tfill.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\tfill $(LINK_OPTS) @tfill.lk1

$(GRX_BIN_SUBDIR)\tmodes.exe : .\tmodes.obj $(GRXLIB) .AUTODEPEND
 @%write tmodes.lk1 FIL tmodes.obj
 @%append tmodes.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\tmodes $(LINK_OPTS) @tmodes.lk1

$(GRX_BIN_SUBDIR)\tpoly.exe : .\tpoly.obj $(GRXLIB) .AUTODEPEND
 @%write tpoly.lk1 FIL tpoly.obj
 @%append tpoly.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\tpoly $(LINK_OPTS) @.tpolylk1

$(GRX_BIN_SUBDIR)\ttext.exe : .\ttext.obj $(GRXLIB) .AUTODEPEND
 @%write ttext.lk1 FIL ttext.obj
 @%append ttext.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\ttext $(LINK_OPTS) @ttext.lk1

$(GRX_BIN_SUBDIR)\life.exe : .\life.obj $(GRXLIB) .AUTODEPEND
 @%write life.lk1 FIL life.obj
 @%append life.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\life $(LINK_OPTS) @life.lk1

$(GRX_BIN_SUBDIR)\bgilink.exe : .\bgilink.obj $(GRXLIB) .AUTODEPEND
 @%write bgilink.lk1 FIL bgilink.obj
 @%append bgilink.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\bgilink $(LINK_OPTS) @bgilink.lk1


$(GRX_BIN_SUBDIR)\blittest.exe : .\blittest.obj $(GRXLIB) .AUTODEPEND
 @%write blittest.lk1 FIL blittest.obj
 @%append blittest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\blittest $(LINK_OPTS) @blittest.lk1


$(GRX_BIN_SUBDIR)\circtest.exe : .\circtest.obj $(GRXLIB) .AUTODEPEND
 @%write circtest.lk1 FIL circtest.obj
 @%append circtest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\circtest $(LINK_OPTS) @circtest.lk1


$(GRX_BIN_SUBDIR)\cliptest.exe : .\cliptest.obj $(GRXLIB) .AUTODEPEND
 @%write cliptest.lk1 FIL cliptest.obj
 @%append cliptest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\cliptest $(LINK_OPTS) @cliptest.lk1

$(GRX_BIN_SUBDIR)\colorops.exe : .\colorops.obj $(GRXLIB) .AUTODEPEND
 @%write colorops.lk1 FIL colorops.obj
 @%append colorops.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\colorops $(LINK_OPTS) @colorops.lk1

$(GRX_BIN_SUBDIR)\curstest.exe : .\curstest.obj $(GRXLIB) .AUTODEPEND
 @%write curstest.lk1 FIL curstest.obj
 @%append curstest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\curstest $(LINK_OPTS) @curstest.lk1

$(GRX_BIN_SUBDIR)\fnt2c.exe : .\fnt2c.obj $(GRXLIB) .AUTODEPEND
 @%write fnt2c.lk1 FIL fnt2c.obj
 @%append fnt2c.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\fnt2c $(LINK_OPTS) @fnt2c.lk1

$(GRX_BIN_SUBDIR)\fnt2text.exe : .\fnt2text.obj $(GRXLIB) .AUTODEPEND
 @%write fnt2text.lk1 FIL fnt2text.obj
 @%append fnt2text.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\fnt2text $(LINK_OPTS) @fnt2text.lk1

$(GRX_BIN_SUBDIR)\fonttest.exe : .\fonttest.obj $(GRXLIB) .AUTODEPEND
 @%write fonttest.lk1 FIL fonttest.obj
 @%append fonttest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\fonttest $(LINK_OPTS) @fonttest.lk1

$(GRX_BIN_SUBDIR)\imgtest.exe : .\imgtest.obj $(GRXLIB) .AUTODEPEND
 @%write imgtest.lk1 FIL imgtest.obj
 @%append imgtest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\imgtest $(LINK_OPTS) @imgtest.lk1

$(GRX_BIN_SUBDIR)\keys.exe : .\keys.obj $(GRXLIB) .AUTODEPEND
 @%write keys.lk1 FIL keys.obj
 @%append keys.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\keys $(LINK_OPTS) @keys.lk1

$(GRX_BIN_SUBDIR)\arctest.exe : .\arctest.obj $(GRXLIB) .AUTODEPEND
 @%write arctest.lk1 FIL arctest.obj
 @%append arctest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\arctest $(LINK_OPTS) @arctest.lk1

$(GRX_BIN_SUBDIR)\linetest.exe : .\linetest.obj $(GRXLIB) .AUTODEPEND
 @%write linetest.lk1 FIL linetest.obj
 @%append linetest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\linetest $(LINK_OPTS) @linetest.lk1

$(GRX_BIN_SUBDIR)\modetest.exe : .\modetest.obj $(GRXLIB) .AUTODEPEND
 @%write modetest.lk1 FIL modetest.obj
 @%append modetest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\modetest $(LINK_OPTS) @modetest.lk1

$(GRX_BIN_SUBDIR)\mousetst.exe : .\mousetst.obj $(GRXLIB) .AUTODEPEND
 @%write mousetst.lk1 FIL mousetst.obj
 @%append mousetst.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\mousetst $(LINK_OPTS) @mousetst.lk1

$(GRX_BIN_SUBDIR)\pcirctst.exe : .\pcirctst.obj $(GRXLIB) .AUTODEPEND
 @%write pcirctst.lk1 FIL pcirctst.obj
 @%append pcirctst.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\pcirctst $(LINK_OPTS) @pcirctst.lk1

$(GRX_BIN_SUBDIR)\polytest.exe : .\polytest.obj $(GRXLIB) .AUTODEPEND
 @%write polytest.lk1 FIL polytest.obj
 @%append polytest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\polytest $(LINK_OPTS) @polytest.lk1

$(GRX_BIN_SUBDIR)\rgbtest.exe : .\rgbtest.obj $(GRXLIB) .AUTODEPEND
 @%write rgbtest.lk1 FIL rgbtest.obj
 @%append rgbtest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\rgbtest $(LINK_OPTS) @rgbtest.lk1

$(GRX_BIN_SUBDIR)\speedtst.exe : .\speedtst.obj $(GRXLIB) .AUTODEPEND
 @%write speedtst.lk1 FIL speedtst.obj
 @%append speedtst.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\speedtst $(LINK_OPTS) @speedtst.lk1

$(GRX_BIN_SUBDIR)\sbctest.exe : .\sbctest.obj $(GRXLIB) .AUTODEPEND
 @%write sbctest.lk1 FIL sbctest.obj
 @%append sbctest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\sbctest $(LINK_OPTS) @sbctest.lk1

$(GRX_BIN_SUBDIR)\textpatt.exe : .\textpatt.obj $(GRXLIB) .AUTODEPEND
 @%write textpatt.lk1 FIL textpatt.obj
 @%append textpatt.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\textpatt $(LINK_OPTS) @textpatt.lk1

$(GRX_BIN_SUBDIR)\winclip.exe : .\winclip.obj $(GRXLIB) .AUTODEPEND
 @%write winclip.lk1 FIL winclip.obj
 @%append winclip.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\winclip $(LINK_OPTS) @winclip.lk1

$(GRX_BIN_SUBDIR)\wintest.exe : .\wintest.obj $(GRXLIB) .AUTODEPEND
 @%write wintest.lk1 FIL wintest.obj
 @%append wintest.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\wintest $(LINK_OPTS) @wintest.lk1

$(GRX_BIN_SUBDIR)\scroltst.exe : .\scroltst.obj $(GRXLIB) .AUTODEPEND
 @%write scroltst.lk1 FIL scroltst.obj
 @%append scroltst.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\scroltst $(LINK_OPTS) @scroltst.lk1

$(GRX_BIN_SUBDIR)\arctest.dat : .\test\arctest.dat
 @copy .\test\arctest.dat $(GRX_BIN_SUBDIR)\arctest.dat

$(GRX_BIN_SUBDIR)\polytest.dat : .\test\polytest.dat
 @copy .\test\polytest.dat $(GRX_BIN_SUBDIR)\polytest.dat

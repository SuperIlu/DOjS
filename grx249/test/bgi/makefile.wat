!define BLANK ""

##############
# Object Files

.\bgilink.obj : .\test\bgilink.c .AUTODEPEND
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

$(GRX_BIN_SUBDIR)\bgilink.exe : .\bgilink.obj $(GRXLIB) .AUTODEPEND
 @%write bgilink.lk1 FIL bgilink.obj
 @%append bgilink.lk1 LIBR $(GRXLIB)
 $(LINK) name $(GRX_BIN_SUBDIR)\bgilink $(LINK_OPTS) @bgilink.lk1

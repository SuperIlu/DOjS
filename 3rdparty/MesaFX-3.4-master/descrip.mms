# Makefile for Mesa for VMS
# contributed by Jouk Jansen  joukj@hrem.stm.tudelft.nl

macro : 
        @ macro=""
.ifdef NOSHARE
.else
	@ if f$getsyi("HW_MODEL") .ge. 1024 then macro= "/MACRO=(SHARE=1)"
.endif
	$(MMS)$(MMSQUALIFIERS)'macro' all

all :
	if f$search("lib.dir") .eqs. "" then create/directory [.lib]
	set default [.src]
	$(MMS)$(MMSQUALIFIERS)
	set default [-.src-glu]
	$(MMS)$(MMSQUALIFIERS)
# PIPE is avalailable on VMS7.0 and higher. For lower versions split the
#command in two conditional command.   JJ
	if f$search("[-]SRC-GLUT.DIR") .nes. "" then pipe set default [-.src-glut] ; $(MMS)$(MMSQUALIFIERS)
	if f$search("[-]DEMOS.DIR") .nes. "" then pipe set default [-.demos] ; $(MMS)$(MMSQUALIFIERS)


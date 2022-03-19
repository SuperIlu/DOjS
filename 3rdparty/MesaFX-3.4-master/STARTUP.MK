# MSDOS DMAKE startup file.  Customize to suit your needs.
# Assumes MKS toolkit for the tool commands, and Zortech C.  Change as req'd.
# See the documentation for a description of internally defined macro#
# Disable warnings for macros redefined here that were given
# on the command line.
__.SILENT := $(.SILENT)
.SILENT   := yes

# Configuration parameters for DMAKE startup.mk file
# Set these to NON-NULL if you wish to turn the parameter on.
_HAVE_RCS	:= yes		# yes => RCS  is installed.
_HAVE_SCCS	:= 		# yes => SCCS is installed.

# Applicable suffix definitions
A := .lib	# Libraries
E := .exe	# Executables
F := .for	# Fortran
O := .obj	# Objects
P := .pas	# Pascal
S := .asm	# Assembler sources
V := 		# RCS suffix

# See if these are defined
TMPDIR := $(ROOTDIR)/tmp
.IMPORT .IGNORE : TMPDIR SHELL COMSPEC

# Recipe execution configurations
# First set SHELL, If it is not defined, use COMSPEC, otherwise
# it is assumed to be MKS Korn SHELL.
.IF $(SHELL) == $(NULL)
.IF $(COMSPEC) == $(NULL)
   SHELL := $(ROOTDIR)/bin/sh$E
.ELSE
   SHELL := $(COMSPEC)
.END
.END
GROUPSHELL := $(SHELL)

# Now set remaining arguments depending on which SHELL we
# are going to use.  COMSPEC (assumed to be command.com) or
# MKS Korn Shell.
.IF $(SHELL)==$(COMSPEC)
   SHELLFLAGS  := $(SWITCHAR)c
   GROUPFLAGS  := $(SHELLFLAGS)
   SHELLMETAS  := *"?<>
   GROUPSUFFIX := .bat
   DIRSEPSTR   := \\
   DIVFILE      = $(TMPFILE:s,/,\)
.ELSE
   SHELLFLAGS  := -c
   GROUPFLAGS  :=
   SHELLMETAS  := *"?<>|()&][$$\#`'
   GROUPSUFFIX := .ksh
   .MKSARGS    := yes
   DIVFILE      = $(TMPFILE:s,/,${DIVSEP_shell_${USESHELL}})
   DIVSEP_shell_yes := \\\
   DIVSEP_shell_no  := \\
.END

# Standard C-language command names and flags
   CC      := wcl386	# C-compiler and flags
   CFLAGS  +=

   AS      := tasm31		# Assembler and flags
   ASFLAGS +=

   LD       = wlink		# Loader and flags
   LDFLAGS +=
   LDLIBS   =

# Definition of $(MAKE) macro for recursive makes.
   MAKE = $(MAKECMD) $(MFLAGS)

# Language and Parser generation Tools and their flags
   YACC	  := yacc		# standard yacc
   YFLAGS +=
   YTAB	  := ytab		# yacc output files name stem.

   LEX	  := lex		# standard lex
   LFLAGS +=
   LEXYY  := lex_yy		# lex output file

# Other Compilers, Tools and their flags
   PC	:= any_pc		# pascal compiler
   RC	:= anyf77		# ratfor compiler
   FC	:= anyf77		# fortran compiler

   CO	   := co		# check out for RCS
   COFLAGS += -q

   AR     := ar			# archiver
   ARFLAGS+= ruv

   RM	   := del		# remove a file command
   RMFLAGS +=

# Implicit generation rules for making inferences.
# We don't provide .yr or .ye rules here.  They're obsolete.
# Rules for making *$O
   %$O : %.c ; $(CC) $(CFLAGS) -c $<
   %$O : %.cpp ; $(CC) $(CFLAGS) -c $<
   %$O : %$P ; $(PC) $(PFLAGS) -c $<
   %$O : %$S ; $(AS) $(ASFLAGS) $(<:s,/,\);
   %$O : %.cl ; class -c $<
   %$O : %.e %.r %.F %$F ; $(FC) $(RFLAGS) $(EFLAGS) $(FFLAGS) -c $<

# Executables
   %$E : %$O ; $(CC) @$(mktmp,a.lnk $(LDFLAGS)  $< $(LDLIBS))

# lex and yacc rules
   %.c : %.y ; $(YACC)  $(YFLAGS) $<; mv $(YTAB).c $@
   %.c : %.l ; $(LEX)   $(LFLAGS) $<; mv $(LEXYY).c $@

# RCS support
.IF $(_HAVE_RCS)
   % : $$(@:d)RCS$$(DIRSEPSTR)$$(@:f)$V;- $(CO) $(COFLAGS) $@
   .NOINFER : $$(@:d)RCS$$(DIRSEPSTR)$$(@:f)$V
.END

# SCCS support
.IF $(_HAVE_SCCS)
   % : s.% ; get $<
   .NOINFER : s.%
.END

# Recipe to make archive files.
%$A :
[
   $(AR) $(ARFLAGS) $@ $?
   $(RM) $(RMFLAGS) $?
]

# DMAKE uses this recipe to remove intermediate targets
.REMOVE :; $(RM) $<

# AUGMAKE extensions for SYSV compatibility
@B = $(@:b)
@D = $(@:d)
@F = $(@:f)
"*B" = $(*:b)
"*D" = $(*:d)
"*F" = $(*:f)
<B = $(<:b)
<D = $(<:d)
<F = $(<:f)
#?B = $(?:b)
#?F = $(?:f)
#?D = $(?:d)

# watcom dir
WINC =C:\watcom\h\

# Turn warnings back to previous setting.
.SILENT := $(__.SILENT)

# Local init file if any, gets parsed before user makefile
.INCLUDE .IGNORE: "_startup.mk"

# this is my stuff
# what the various flags do
# fp5 - floating point inlined for pentium
# 5r  - pentium register passing
# s   - remove stack check
# mf  - flat memory model
# zq  - operate quietly
# wx  - all warnings on
# d1  - simple debug info, shouldn`t affect code
# d1+ - as above, but with unused names
# 5s  - pentium stack passing
# zp4 - pack structures to 4 bytes
# hc  - do codeview style debug info
# oneatx - watcom recommended optimizations for pentium (see below)
# ee ep 	- add epilogue, prolog hooks for profiler
# et     - add rdtsc style profiler hooks

# /o optimizations
# t -
# e -
# x -
# e -
# a -
# n -
# i -
# l -
# r -
# + -

#CFLAGS = /oanrlt /fp5 /5r /s /mf /zq /wx
#CFLAGS = /oanrlt /d2 /fp5 /5r /mf /zq /wx
#CFLAGS = /oanrlt /fp5 /5r /mf /zq /wx
#CFLAGS = /d2 /fp5 /5s /mf /zq /wx

# watcom pentium rec.
#CFLAGS = /oneatx /zp4 /5r /fp5 /s /mf /wx
#CFLAGS  = /oneatx /zp4 /5r /fp5 /s /mf /wx

# for debugging pentium stuff
#CFLAGS = /d3 /zp4 /5r /fp5 /mf /wx

# for vtune
#CFLAGS = /hc /d2 /zp4 /5 /fp5 /s /mf /wx /oneatx

# 486
#CFLAGS = /oneatx /zp4 /4 /fp3 /s /mf /wx

# 386
#CFLAGS = /oneatx /zp4 /3 /fp3 /s /mf /wx

# super inlining
#CFLAGS =  /d1 /otiexanl+ /fp5 /5r /s /mf /zq /wx

# for mesa
CFLAGS = /d2 /5r /fp5 /wx /DDEBUG
#CFLAGS =  /otexan /zp4 /mf /5r /fp5 /wx /d1+

PROFILE = /ee /ep
#CFLAGS += $(PROFILE)

PPROF = /et
#CFLAGS += $(PPROF)

# 3dfx
#CFLAGS += /DGLIDE_HARDWARE

#CFLAGS += /D_PC_=1
#CFLAGS += /D_586_ /D__586__

ASMFLAGS = /ml /m5 /zi /p /r /t /z /w2 /kh10000 /i$(@:d)

.IMPORT : WATCOM

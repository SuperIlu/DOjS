
#	Makefile for MPW (Macintosh Programmers Workshop by Apple)
#                http://developer.apple.com/tools/mpw-tools/
#
#   File:       libmikmod.make
#   Target:     libmikmod
#
#   Builds for 680X0 and PowerPC "classic" Macintosh.
#
#   currently the 68K build doesn't work because the
#   default Symantec compiler lacks "long long" (64-bit)
#   int support and also support for prefix header files.
#   (Switching to another compiler such as the one from
#    "Metrowerks CodeWarrior for MPW" could work, though)
#   Another approach would be to rewrite the code using
#   the SInt64 type and macros. Using a "Math64" subdir.
#
#	Build with: make -f libmikmod.make [Remove|Remove-All]
#
#	Written by Anders F Bjšrklund <afb@algonet.se>

MAKEFILE        = "libmikmod.make"
MikModDir	 	=  "::"

LibStatic68K	= "libmikmod.o"
LibShared68K	= "libmikmod68K.dll"
LibStub68K		= "libmikmod68K.lib"

LibStaticPPC	= "libmikmod.x"
LibSharedPPC	= "libmikmodPPC.dll"
LibStubPPC		= "libmikmodPPC.lib"

LibNameFat		= "libmikmod"
StubNameFat		= "libmikmodStub"

Export          = "libmikmod.exp"

#------------------------------------------------------------------------------
# Choose your tools and libraries:

CC68K           = SC
CCPPC           = MrC

LIB68K          = Lib
LIBPPC          = PPCLink -xm l

LINK68K         = ILink   -xm s
LINKPPC         = PPCLink -xm s

STUB68K         = MakeStub -arch m68k
STUBPPC         = MakeStub -arch pwpc

#------------------------------------------------------------------------------
# File locations:

Headers    = -i "{MikModDir}include:" ¶
		     -i "{MikModDir}playercode:" ¶
		     -i ":"
			   
Sources      =	"{MikModDir}playercode:" ¶
				"{MikModDir}depackers:" ¶
				"{MikModDir}loaders:" ¶
				"{MikModDir}drivers:" ¶
				"{MikModDir}mmio:" ¶
				"{MikModDir}posix:" ¶
				":"

obj68k          = :obj68k:
objppc	        = :objppc:

IncLibraries68K = ¶
				"{SharedLibraries}InterfaceLib" ¶
				"{SharedLibraries}StdCLib" ¶
				"{CFM68KLibraries}NuMathLib.o" ¶
				"{CFM68KLibraries}NuMacRuntime.o" ¶
				"{CFM68KLibraries}NuToolLibs.o" ¶

IncLibrariesPPC = ¶
				"{SharedLibraries}InterfaceLib" ¶
				"{SharedLibraries}StdCLib" ¶
				"{SharedLibraries}MathLib" ¶
				"{SharedLibraries}SoundLib" ¶
				"{PPCLibraries}StdCRuntime.o" ¶
				"{PPCLibraries}PPCCRuntime.o" ¶
				"{PPCLibraries}PPCToolLibs.o"
				
#------------------------------------------------------------------------------
# Compiler settings:

Warnings        =   -w 2,6,7,35
					# avoid Warning 2: possible unintended assignment
                    # avoid Warning 7: possible extraneous ';'
                    # avoid Warning 6: value of expression is not used
                    # avoid Warning 35: Parameter '*' is not used within function '*'

Options         =   -d MIKMOD_BUILD -d DRV_MAC -d HAVE_FCNTL_H

SymOptions		=	-sym off		# turn this on to debug with SADE/R2Db
Debug68KOptions	=	-opt speed		# turn this off to build debug 680x0 code
DebugPPCOptions	=	-opt speed		# turn this off to build debug PowerPC code

LibOptStatic    =   
LibOptStatic68K	=	{LibOptStatic}
LibOptStaticPPC	=	{LibOptStatic}

LibOptShared      = -t 'shlb' -c 'cfmg' -xm s
LibOptShared68K	=	{LibOptShared}
LibOptSharedPPC	=	{LibOptShared} 

IncludesFolders	=	{Headers}

CCOptions       =   {IncludesFolders} {Options} {Warnings} {SymOptions}
CC68KOptions    =	{CCOptions} {Debug68KOptions} -model cfmflat -mc68020 -x
CCPPCOptions	=	{CCOptions} {DebugPPCOptions} -longlong on -prefix mpwmikmodheaders.h

#------------------------------------------------------------------------------
# These are modified default build rules.  
#------------------------------------------------------------------------------

{obj68k}		Ä	:Math64: ¶
                    {Sources}

.c.o			Ä	.c
	Echo "# compiling "{Default}.c" using {CC68K}"
	{CC68K} {CC68KOptions} {DepDir}{Default}.c -o {TargDir}{Default}.c.o

{objppc}		Ä	{Sources}

.c.x			Ä	.c
	Echo "# compiling "{Default}.c" using {CCPPC}"
	{CCPPC} {CCPPCOptions} {DepDir}{Default}.c -o {TargDir}{Default}.c.x

#------------------------------------------------------------------------------
# These are the objects that we want to include in the library.
#------------------------------------------------------------------------------

LibObjects68K		=	¶
					{obj68k}strcasecmp.c.o ¶
					{obj68k}mmalloc.c.o ¶
					{obj68k}mmerror.c.o ¶
					{obj68k}mmio.c.o ¶
					{obj68k}mdreg.c.o ¶
					{obj68k}mdriver.c.o ¶
					{obj68k}mlreg.c.o ¶
					{obj68k}mlutil.c.o ¶
					{obj68k}mloader.c.o ¶
					{obj68k}mplayer.c.o ¶
					{obj68k}munitrk.c.o ¶
					{obj68k}mwav.c.o ¶
					{obj68k}npertab.c.o ¶
					{obj68k}sloader.c.o ¶
					{obj68k}virtch_common.c.o ¶
					{obj68k}virtch.c.o ¶
					{obj68k}virtch2.c.o ¶
					{obj68k}mmcmp.c.o ¶
					{obj68k}pp20.c.o ¶
					{obj68k}s404.c.o ¶
					{obj68k}xpk.c.o ¶
					{obj68k}load_669.c.o ¶
					{obj68k}load_amf.c.o ¶
					{obj68k}load_asy.c.o ¶
					{obj68k}load_dsm.c.o ¶
					{obj68k}load_far.c.o ¶
					{obj68k}load_gdm.c.o ¶
					{obj68k}load_imf.c.o ¶
					{obj68k}load_it.c.o ¶
					{obj68k}load_m15.c.o ¶
					{obj68k}load_med.c.o ¶
					{obj68k}load_mod.c.o ¶
					{obj68k}load_mtm.c.o ¶
					{obj68k}load_okt.c.o ¶
					{obj68k}load_s3m.c.o ¶
					{obj68k}load_stm.c.o ¶
					{obj68k}load_stx.c.o ¶
					{obj68k}load_ult.c.o ¶
					{obj68k}load_umx.c.o ¶
					{obj68k}load_uni.c.o ¶
					{obj68k}load_xm.c.o ¶
					{obj68k}drv_nos.c.o ¶
					{obj68k}drv_stdout.c.o ¶
					{obj68k}drv_raw.c.o ¶
					{obj68k}drv_wav.c.o ¶
					{obj68k}drv_mac.c.o ¶
					#

LibObjectsPPC	=	¶
					{objppc}strcasecmp.c.x ¶
					{objppc}mmalloc.c.x ¶
					{objppc}mmerror.c.x ¶
					{objppc}mmio.c.x ¶
					{objppc}mdreg.c.x ¶
					{objppc}mdriver.c.x ¶
					{objppc}mlreg.c.x ¶
					{objppc}mlutil.c.x ¶
					{objppc}mloader.c.x ¶
					{objppc}mplayer.c.x ¶
					{objppc}munitrk.c.x ¶
					{objppc}mwav.c.x ¶
					{objppc}npertab.c.x ¶
					{objppc}sloader.c.x ¶
					{objppc}virtch_common.c.x ¶
					{objppc}virtch.c.x ¶
					{objppc}virtch2.c.x ¶
					{objppc}mmcmp.c.x ¶
					{objppc}pp20.c.x ¶
					{objppc}s404.c.x ¶
					{objppc}xpk.c.x ¶
					{objppc}load_669.c.x ¶
					{objppc}load_amf.c.x ¶
					{objppc}load_asy.c.x ¶
					{objppc}load_dsm.c.x ¶
					{objppc}load_far.c.x ¶
					{objppc}load_gdm.c.x ¶
					{objppc}load_imf.c.x ¶
					{objppc}load_it.c.x ¶
					{objppc}load_m15.c.x ¶
					{objppc}load_med.c.x ¶
					{objppc}load_mod.c.x ¶
					{objppc}load_mtm.c.x ¶
					{objppc}load_okt.c.x ¶
					{objppc}load_s3m.c.x ¶
					{objppc}load_stm.c.x ¶
					{objppc}load_stx.c.x ¶
					{objppc}load_ult.c.x ¶
					{objppc}load_umx.c.x ¶
					{objppc}load_uni.c.x ¶
					{objppc}load_xm.c.x ¶
					{objppc}drv_nos.c.x ¶
					{objppc}drv_stdout.c.x ¶
					{objppc}drv_raw.c.x ¶
					{objppc}drv_wav.c.x ¶
					{objppc}drv_mac.c.x ¶
					#

#------------------------------------------------------------------------------
# These are the targets.
#------------------------------------------------------------------------------

#All				Ä 68K PPC {LibNameFat} {StubNameFat}
                    # Avoid building 68K by default, due to missing long-longs:
All				    Ä PPC

68K				    Ä {LibStatic68K} {LibShared68K} {LibStub68K}
PPC				    Ä {LibStaticPPC} {LibSharedPPC} {LibStubPPC}

{LibStatic68K}		ÄÄ {LibObjects68K}
	Echo "# Building 680X0 static library"
	{LIB68K} {LibObjects68K} {SymOptions} {LibOptStatic68K} -o {Targ}

{LibShared68K}		ÄÄ {LibObjects68K}
	Echo "# Building 680X0 shared library"
	{LINK68K} {LibObjects68K} {SymOptions} {LibOptShared68K} {IncLibraries68K} -o {Targ}

{LibStub68K}		ÄÄ {Export}
	Echo "# Building 680X0 stub library"
	{STUB68K} {Export} -o {Targ}
	
{LibStaticPPC}		ÄÄ {LibObjectsPPC}
	Echo "# Building PowerPC static library"
	{LIBPPC} {LibObjectsPPC} {SymOptions} {LibOptStaticPPC} -o {Targ}

{LibSharedPPC}		ÄÄ {Export} {LibObjectsPPC}
	Echo "# Building PowerPC shared library"
	{LINKPPC} {LibObjectsPPC} {SymOptions} {LibOptSharedPPC} {IncLibrariesPPC} -@export {Export} -o {Targ}

{LibStubPPC}		ÄÄ {Export}
	Echo "# Building PowerPC stub library"
	{STUBPPC} {Export} -o {Targ}

{LibNameFat}		Ä {MAKEFILE} libmikmodversion.r {LibShared68K} {LibSharedPPC}
	Echo "# Building Fat shared library (merging)"
	Duplicate -y {LibShared68K} {LibNameFat}
	MergeFragment {LibSharedPPC} {LibNameFat}
	Rez  libmikmodversion.r -append -o {LibNameFat}
    SetFile -a C {LibNameFat}

{StubNameFat}		Ä {LibStub68K} {LibStubPPC}
	Echo "# Building Fat stub library
	MakeStub -arch fat {Export} -o {StubNameFat}

Remove				Ä
	Echo "# Removing objects"
	Delete -i -y {LibObjects68K}
	Delete -i -y {LibObjectsPPC}

Remove-All			Ä	Remove
	Echo "# Removing binaries"
	Delete -i {LibStatic68K} {LibShared68K} {LibStub68K}
	Delete -i {LibStaticPPC} {LibSharedPPC} {LibStubPPC}

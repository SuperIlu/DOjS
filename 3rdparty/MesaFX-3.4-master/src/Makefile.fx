
#makefile for compiling Mesa with Glide support
#tested with Microsoft tools (nmake & VC++)
#Assembly code support with nasm assembler
# http://www.web-sites.co.uk/nasm/


!include <win32.mak>


# Set NASM to the complete path and file name (minus .EXE)
# where you have (0.98 or latter) nasm installed.
#
# Comment out the following to disable using the assembly files
NASM=nasmw

# Set GLIDE2SDK to have the complete path and file name
# where you have the Glide 2 SDK installed. It will be used
# as the root to build the actual path upon for linkage later.
#
# If you do not set this, then both glide.h and glide2x.lib as
# assumed to be available in the default INCLUDE and LIB search
# paths.
#GLIDE2SDK=\glide2x

#if you have a Voodoo2 or latter activate the following
#V2_OPTS=/DFX_V2

#DO_VTUNE can be used to generate VTUNE compatible
#libraries, they can propably be used for other
#debugging purposes also.
#activating the options should not significantly
#slow down the resulting library
#DO_VTUNE=1

#the following option can be used for
#Intel compiler
#CC=icl -Qxi




!if "$(NASM)" != ""
X86OPT = -DUSE_X86_ASM -DUSE_3DNOW_ASM -DUSE_KATMAI_ASM
!else
X86OPT =
!endif


!if "$(DO_VTUNE)" != ""
dlllflags =  /NODEFAULTLIB /INCREMENTAL:NO /RELEASE /NOLOGO -entry:_DllMainCRTStartup$(DLLENTRY) -dll
VTUNE_LINK_OPTS = /pdb:..\lib\OpenGL32.pdb /map /fixed:no /debug
VTUNE_CC_OPTS = /Zi
!else
!endif



.SUFFIXES: .S


CFLAGS        = $(cvarsdll) $(VTUNE_CC_OPTS) /Ox /G6 /Gd /D__i386__ \
                /DBUILD_GL32 /DMESA_FX_DDRAW /DMESA_MINWARN $(X86OPT) \
                /D__MSC__ /DFX $(V2_OPTS) /D__WIN32__ /DWIN32 /I..\include /I. \
                /I$(GLIDE2SDK)\src\include

!if "$(GLIDE2SDK)" == ""
GLIDE2LIB=glide2x.lib
!else
CFLAGS = $(CFLAGS) /I$(GLIDE2SDK)\src\include
GLIDE2LIB=$(GLIDE2SDK)\src\lib\win32\glide2x.lib
!endif


OBJS =  aatriangle.obj accum.obj alpha.obj alphabuf.obj \
        attrib.obj bbox.obj bitmap.obj blend.obj buffers.obj \
	clip.obj colortab.obj\
        config.obj context.obj copypix.obj cva.obj depth.obj\
        dispatch.obj dlist.obj\
        drawpix.obj enable.obj enums.obj eval.obj extensions.obj\
        feedback.obj fog.obj glapi.obj glapinoop.obj glthread.obj \
        get.obj hash.obj hint.obj image.obj imaging.obj light.obj\
        lines.obj logic.obj masking.obj matrix.obj mem.obj \
        mmath.obj\
        pb.obj pipeline.obj pixel.obj pixeltex.obj points.obj polygon.obj \
        quads.obj rastpos.obj readpix.obj rect.obj scissor.obj shade.obj\
        span.obj stages.obj state.obj stencil.obj teximage.obj texobj.obj\
        texstate.obj texture.obj texutil.obj translate.obj triangle.obj \
        varray.obj vbindirect.obj winpos.obj vb.obj vbcull.obj vbfill.obj\
        vbrender.obj vbxform.obj vector.obj vertices.obj xform.obj zoom.obj


FXOBJS          = FX\fxcva.obj FX\fxwgl.obj FX\fxapi.obj \
                FX\fxclip.obj FX\fxdd.obj \
                FX\fxddtex.obj FX\fxfastpath.obj\
                FX\fxglidew.obj FX\fxpipeline.obj \
                FX\fxvsetup.obj FX\fxsetup.obj \
                FX\fxtexman.obj FX\fxrender.obj \
                FX\fxddspan.obj FX\fxtrifuncs.obj




!if "$(NASM)" != ""
X86OBJS =       X86\3dnow.obj X86\3dnow_norm_raw.obj \
        X86\3dnow_xform_masked1.obj\
        X86\3dnow_xform_masked2.obj X86\3dnow_xform_masked3.obj\
        X86\3dnow_xform_masked4.obj X86\3dnow_xform_raw1.obj\
        X86\3dnow_xform_raw2.obj X86\3dnow_xform_raw3.obj\
        X86\3dnow_xform_raw4.obj\
        X86\mmx_blend.obj X86\vertex.obj X86\vertex_3dnow.obj \
        X86\x86a.obj X86\common_x86.obj X86\common_x86_asm.obj X86\x86.obj\
        X86\katmai.obj X86\katmai_norm_raw.obj \
        X86\katmai_xform_masked1.obj X86\katmai_xform_masked2.obj\
        X86\katmai_xform_masked3.obj X86\katmai_xform_masked4.obj\
        X86\katmai_xform_raw1.obj X86\katmai_xform_raw2.obj\
        X86\katmai_xform_raw3.obj X86\katmai_xform_raw4.obj\
        X86\vertex_katmai.obj \
        FX\X86\fx_3dnow_fastpath.obj
!else
X86OBJS =
!endif



OSOBJS                  = OSmesa\osmesa.obj

PROGRAM         = ..\lib\OpenGL32.DLL

all:            $(PROGRAM)


$(PROGRAM):     $(OBJS) $(FXOBJS) $(OSOBJS) $(X86OBJS)
                $(link) $(dlllflags) /out:$(PROGRAM) $(VTUNE_LINK_OPTS)\
                 /def:FX\fxOpenGL.def $(OBJS) $(FXOBJS) $(OSOBJS)\
                 $(X86OBJS) $(guilibsdll) $(GLIDE2LIB) >link.log



.c.obj:
        $(CC) $(CFLAGS) $< /c /Fo$@



.S.obj:
        $(CC) -DNASM_ASSEMBLER /EP $< >$*.as
        $(NASM) -o $@ -f win32 $*.as

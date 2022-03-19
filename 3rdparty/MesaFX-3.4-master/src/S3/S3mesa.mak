# Microsoft Developer Studio Generated NMAKE File, Based on opengl32.dsp
!IF "$(CFG)" == ""
CFG=opengl32 - Win32 Release
!MESSAGE No configuration specified. Defaulting to opengl32 - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "opengl32 - Win32 Release" && "$(CFG)" !=\
 "opengl32 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "opengl32.mak" CFG="opengl32 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "opengl32 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "opengl32 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "opengl32 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\s3mesa.dll"

!ELSE 

ALL : "$(OUTDIR)\s3mesa.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\accum.obj"
	-@erase "$(INTDIR)\alpha.obj"
	-@erase "$(INTDIR)\alphabuf.obj"
	-@erase "$(INTDIR)\api1.obj"
	-@erase "$(INTDIR)\api2.obj"
	-@erase "$(INTDIR)\attrib.obj"
	-@erase "$(INTDIR)\bitmap.obj"
	-@erase "$(INTDIR)\blend.obj"
	-@erase "$(INTDIR)\clip.obj"
	-@erase "$(INTDIR)\colortab.obj"
	-@erase "$(INTDIR)\context.obj"
	-@erase "$(INTDIR)\copypix.obj"
	-@erase "$(INTDIR)\depth.obj"
	-@erase "$(INTDIR)\dlist.obj"
	-@erase "$(INTDIR)\drawpix.obj"
	-@erase "$(INTDIR)\enable.obj"
	-@erase "$(INTDIR)\eval.obj"
	-@erase "$(INTDIR)\feedback.obj"
	-@erase "$(INTDIR)\fog.obj"
	-@erase "$(INTDIR)\get.obj"
	-@erase "$(INTDIR)\hash.obj"
	-@erase "$(INTDIR)\image.obj"
	-@erase "$(INTDIR)\light.obj"
	-@erase "$(INTDIR)\lines.obj"
	-@erase "$(INTDIR)\logic.obj"
	-@erase "$(INTDIR)\masking.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\mmath.obj"
	-@erase "$(INTDIR)\pb.obj"
	-@erase "$(INTDIR)\pixel.obj"
	-@erase "$(INTDIR)\pointers.obj"
	-@erase "$(INTDIR)\points.obj"
	-@erase "$(INTDIR)\polygon.obj"
	-@erase "$(INTDIR)\quads.obj"
	-@erase "$(INTDIR)\rastpos.obj"
	-@erase "$(INTDIR)\readpix.obj"
	-@erase "$(INTDIR)\rect.obj"
	-@erase "$(INTDIR)\s3mesa.obj"
	-@erase "$(INTDIR)\s3mesa.res"
	-@erase "$(INTDIR)\s3wgl.obj"
	-@erase "$(INTDIR)\scissor.obj"
	-@erase "$(INTDIR)\shade.obj"
	-@erase "$(INTDIR)\span.obj"
	-@erase "$(INTDIR)\stencil.obj"
	-@erase "$(INTDIR)\teximage.obj"
	-@erase "$(INTDIR)\texobj.obj"
	-@erase "$(INTDIR)\texstate.obj"
	-@erase "$(INTDIR)\texture.obj"
	-@erase "$(INTDIR)\triangle.obj"
	-@erase "$(INTDIR)\varray.obj"
	-@erase "$(INTDIR)\vb.obj"
	-@erase "$(INTDIR)\vbfill.obj"
	-@erase "$(INTDIR)\vbrender.obj"
	-@erase "$(INTDIR)\vbxform.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\winpos.obj"
	-@erase "$(INTDIR)\xform.obj"
	-@erase "$(OUTDIR)\s3mesa.dll"
	-@erase "$(OUTDIR)\s3mesa.exp"
	-@erase "$(OUTDIR)\s3mesa.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /Gz /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "S3" /Fp"$(INTDIR)\opengl32.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\s3mesa.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\opengl32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ddraw.lib s3dtkw.lib\
 /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\s3mesa.pdb"\
 /machine:I386 /def:"..\src\s3mesa.def" /out:"$(OUTDIR)\s3mesa.dll"\
 /implib:"$(OUTDIR)\s3mesa.lib" 
DEF_FILE= \
	"..\src\s3mesa.def"
LINK32_OBJS= \
	"$(INTDIR)\accum.obj" \
	"$(INTDIR)\alpha.obj" \
	"$(INTDIR)\alphabuf.obj" \
	"$(INTDIR)\api1.obj" \
	"$(INTDIR)\api2.obj" \
	"$(INTDIR)\attrib.obj" \
	"$(INTDIR)\bitmap.obj" \
	"$(INTDIR)\blend.obj" \
	"$(INTDIR)\clip.obj" \
	"$(INTDIR)\colortab.obj" \
	"$(INTDIR)\context.obj" \
	"$(INTDIR)\copypix.obj" \
	"$(INTDIR)\depth.obj" \
	"$(INTDIR)\dlist.obj" \
	"$(INTDIR)\drawpix.obj" \
	"$(INTDIR)\enable.obj" \
	"$(INTDIR)\eval.obj" \
	"$(INTDIR)\feedback.obj" \
	"$(INTDIR)\fog.obj" \
	"$(INTDIR)\get.obj" \
	"$(INTDIR)\hash.obj" \
	"$(INTDIR)\image.obj" \
	"$(INTDIR)\light.obj" \
	"$(INTDIR)\lines.obj" \
	"$(INTDIR)\logic.obj" \
	"$(INTDIR)\masking.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\mmath.obj" \
	"$(INTDIR)\pb.obj" \
	"$(INTDIR)\pixel.obj" \
	"$(INTDIR)\pointers.obj" \
	"$(INTDIR)\points.obj" \
	"$(INTDIR)\polygon.obj" \
	"$(INTDIR)\quads.obj" \
	"$(INTDIR)\rastpos.obj" \
	"$(INTDIR)\readpix.obj" \
	"$(INTDIR)\rect.obj" \
	"$(INTDIR)\s3mesa.obj" \
	"$(INTDIR)\s3mesa.res" \
	"$(INTDIR)\s3wgl.obj" \
	"$(INTDIR)\scissor.obj" \
	"$(INTDIR)\shade.obj" \
	"$(INTDIR)\span.obj" \
	"$(INTDIR)\stencil.obj" \
	"$(INTDIR)\teximage.obj" \
	"$(INTDIR)\texobj.obj" \
	"$(INTDIR)\texstate.obj" \
	"$(INTDIR)\texture.obj" \
	"$(INTDIR)\triangle.obj" \
	"$(INTDIR)\varray.obj" \
	"$(INTDIR)\vb.obj" \
	"$(INTDIR)\vbfill.obj" \
	"$(INTDIR)\vbrender.obj" \
	"$(INTDIR)\vbxform.obj" \
	"$(INTDIR)\winpos.obj" \
	"$(INTDIR)\xform.obj"

"$(OUTDIR)\s3mesa.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "opengl32 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\s3mesa.dll"

!ELSE 

ALL : "$(OUTDIR)\s3mesa.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\accum.obj"
	-@erase "$(INTDIR)\alpha.obj"
	-@erase "$(INTDIR)\alphabuf.obj"
	-@erase "$(INTDIR)\api1.obj"
	-@erase "$(INTDIR)\api2.obj"
	-@erase "$(INTDIR)\attrib.obj"
	-@erase "$(INTDIR)\bitmap.obj"
	-@erase "$(INTDIR)\blend.obj"
	-@erase "$(INTDIR)\clip.obj"
	-@erase "$(INTDIR)\colortab.obj"
	-@erase "$(INTDIR)\context.obj"
	-@erase "$(INTDIR)\copypix.obj"
	-@erase "$(INTDIR)\depth.obj"
	-@erase "$(INTDIR)\dlist.obj"
	-@erase "$(INTDIR)\drawpix.obj"
	-@erase "$(INTDIR)\enable.obj"
	-@erase "$(INTDIR)\eval.obj"
	-@erase "$(INTDIR)\feedback.obj"
	-@erase "$(INTDIR)\fog.obj"
	-@erase "$(INTDIR)\get.obj"
	-@erase "$(INTDIR)\hash.obj"
	-@erase "$(INTDIR)\image.obj"
	-@erase "$(INTDIR)\light.obj"
	-@erase "$(INTDIR)\lines.obj"
	-@erase "$(INTDIR)\logic.obj"
	-@erase "$(INTDIR)\masking.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\mmath.obj"
	-@erase "$(INTDIR)\pb.obj"
	-@erase "$(INTDIR)\pixel.obj"
	-@erase "$(INTDIR)\pointers.obj"
	-@erase "$(INTDIR)\points.obj"
	-@erase "$(INTDIR)\polygon.obj"
	-@erase "$(INTDIR)\quads.obj"
	-@erase "$(INTDIR)\rastpos.obj"
	-@erase "$(INTDIR)\readpix.obj"
	-@erase "$(INTDIR)\rect.obj"
	-@erase "$(INTDIR)\s3mesa.obj"
	-@erase "$(INTDIR)\s3mesa.res"
	-@erase "$(INTDIR)\s3wgl.obj"
	-@erase "$(INTDIR)\scissor.obj"
	-@erase "$(INTDIR)\shade.obj"
	-@erase "$(INTDIR)\span.obj"
	-@erase "$(INTDIR)\stencil.obj"
	-@erase "$(INTDIR)\teximage.obj"
	-@erase "$(INTDIR)\texobj.obj"
	-@erase "$(INTDIR)\texstate.obj"
	-@erase "$(INTDIR)\texture.obj"
	-@erase "$(INTDIR)\triangle.obj"
	-@erase "$(INTDIR)\varray.obj"
	-@erase "$(INTDIR)\vb.obj"
	-@erase "$(INTDIR)\vbfill.obj"
	-@erase "$(INTDIR)\vbrender.obj"
	-@erase "$(INTDIR)\vbxform.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(INTDIR)\winpos.obj"
	-@erase "$(INTDIR)\xform.obj"
	-@erase "$(OUTDIR)\s3mesa.dll"
	-@erase "$(OUTDIR)\s3mesa.exp"
	-@erase "$(OUTDIR)\s3mesa.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /Gz /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "S3" /Fp"$(INTDIR)\opengl32.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\s3mesa.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\opengl32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ddraw.lib s3dtkw.lib\
 /nologo /subsystem:windows /dll /profile /debug /machine:I386\
 /def:"..\src\s3mesa.def" /out:"$(OUTDIR)\s3mesa.dll"\
 /implib:"$(OUTDIR)\s3mesa.lib" 
DEF_FILE= \
	"..\src\s3mesa.def"
LINK32_OBJS= \
	"$(INTDIR)\accum.obj" \
	"$(INTDIR)\alpha.obj" \
	"$(INTDIR)\alphabuf.obj" \
	"$(INTDIR)\api1.obj" \
	"$(INTDIR)\api2.obj" \
	"$(INTDIR)\attrib.obj" \
	"$(INTDIR)\bitmap.obj" \
	"$(INTDIR)\blend.obj" \
	"$(INTDIR)\clip.obj" \
	"$(INTDIR)\colortab.obj" \
	"$(INTDIR)\context.obj" \
	"$(INTDIR)\copypix.obj" \
	"$(INTDIR)\depth.obj" \
	"$(INTDIR)\dlist.obj" \
	"$(INTDIR)\drawpix.obj" \
	"$(INTDIR)\enable.obj" \
	"$(INTDIR)\eval.obj" \
	"$(INTDIR)\feedback.obj" \
	"$(INTDIR)\fog.obj" \
	"$(INTDIR)\get.obj" \
	"$(INTDIR)\hash.obj" \
	"$(INTDIR)\image.obj" \
	"$(INTDIR)\light.obj" \
	"$(INTDIR)\lines.obj" \
	"$(INTDIR)\logic.obj" \
	"$(INTDIR)\masking.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\mmath.obj" \
	"$(INTDIR)\pb.obj" \
	"$(INTDIR)\pixel.obj" \
	"$(INTDIR)\pointers.obj" \
	"$(INTDIR)\points.obj" \
	"$(INTDIR)\polygon.obj" \
	"$(INTDIR)\quads.obj" \
	"$(INTDIR)\rastpos.obj" \
	"$(INTDIR)\readpix.obj" \
	"$(INTDIR)\rect.obj" \
	"$(INTDIR)\s3mesa.obj" \
	"$(INTDIR)\s3mesa.res" \
	"$(INTDIR)\s3wgl.obj" \
	"$(INTDIR)\scissor.obj" \
	"$(INTDIR)\shade.obj" \
	"$(INTDIR)\span.obj" \
	"$(INTDIR)\stencil.obj" \
	"$(INTDIR)\teximage.obj" \
	"$(INTDIR)\texobj.obj" \
	"$(INTDIR)\texstate.obj" \
	"$(INTDIR)\texture.obj" \
	"$(INTDIR)\triangle.obj" \
	"$(INTDIR)\varray.obj" \
	"$(INTDIR)\vb.obj" \
	"$(INTDIR)\vbfill.obj" \
	"$(INTDIR)\vbrender.obj" \
	"$(INTDIR)\vbxform.obj" \
	"$(INTDIR)\winpos.obj" \
	"$(INTDIR)\xform.obj"

"$(OUTDIR)\s3mesa.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "opengl32 - Win32 Release" || "$(CFG)" ==\
 "opengl32 - Win32 Debug"
SOURCE=..\src\accum.c
DEP_CPP_ACCUM=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_ACCUM=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\accum.obj" : $(SOURCE) $(DEP_CPP_ACCUM) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\alpha.c
DEP_CPP_ALPHA=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_ALPHA=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\alpha.obj" : $(SOURCE) $(DEP_CPP_ALPHA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\alphabuf.c
DEP_CPP_ALPHAB=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_ALPHAB=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\alphabuf.obj" : $(SOURCE) $(DEP_CPP_ALPHAB) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\api1.c

!IF  "$(CFG)" == "opengl32 - Win32 Release"

DEP_CPP_API1_=\
	"..\src\api.h"\
	"..\src\bitmap.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\dd.h"\
	"..\src\drawpix.h"\
	"..\src\eval.h"\
	"..\src\fixed.h"\
	"..\src\image.h"\
	"..\src\macros.h"\
	"..\src\matrix.h"\
	"..\src\teximage.h"\
	"..\src\vb.h"\
	

"$(INTDIR)\api1.obj" : $(SOURCE) $(DEP_CPP_API1_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "opengl32 - Win32 Debug"

DEP_CPP_API1_=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_API1_=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\api1.obj" : $(SOURCE) $(DEP_CPP_API1_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\api2.c
DEP_CPP_API2_=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_API2_=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\api2.obj" : $(SOURCE) $(DEP_CPP_API2_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\attrib.c
DEP_CPP_ATTRI=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_ATTRI=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\attrib.obj" : $(SOURCE) $(DEP_CPP_ATTRI) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\bitmap.c
DEP_CPP_BITMA=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_BITMA=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\bitmap.obj" : $(SOURCE) $(DEP_CPP_BITMA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\blend.c
DEP_CPP_BLEND=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_BLEND=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\blend.obj" : $(SOURCE) $(DEP_CPP_BLEND) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\clip.c
DEP_CPP_CLIP_=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_CLIP_=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\clip.obj" : $(SOURCE) $(DEP_CPP_CLIP_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\colortab.c
DEP_CPP_COLOR=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_COLOR=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\colortab.obj" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\context.c
DEP_CPP_CONTE=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\quads.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_CONTE=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\context.obj" : $(SOURCE) $(DEP_CPP_CONTE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\copypix.c

!IF  "$(CFG)" == "opengl32 - Win32 Release"

DEP_CPP_COPYP=\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\macros.h"\
	"..\src\pixel.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	

"$(INTDIR)\copypix.obj" : $(SOURCE) $(DEP_CPP_COPYP) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "opengl32 - Win32 Debug"

DEP_CPP_COPYP=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_COPYP=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\copypix.obj" : $(SOURCE) $(DEP_CPP_COPYP) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\depth.c
DEP_CPP_DEPTH=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_DEPTH=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\depth.obj" : $(SOURCE) $(DEP_CPP_DEPTH) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\dlist.c
DEP_CPP_DLIST=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\rect.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_DLIST=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\dlist.obj" : $(SOURCE) $(DEP_CPP_DLIST) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\drawpix.c
DEP_CPP_DRAWP=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_DRAWP=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\drawpix.obj" : $(SOURCE) $(DEP_CPP_DRAWP) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\enable.c
DEP_CPP_ENABL=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_ENABL=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\enable.obj" : $(SOURCE) $(DEP_CPP_ENABL) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\eval.c
DEP_CPP_EVAL_=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_EVAL_=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\eval.obj" : $(SOURCE) $(DEP_CPP_EVAL_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\feedback.c
DEP_CPP_FEEDB=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_FEEDB=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\feedback.obj" : $(SOURCE) $(DEP_CPP_FEEDB) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\fog.c
DEP_CPP_FOG_C=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_FOG_C=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\fog.obj" : $(SOURCE) $(DEP_CPP_FOG_C) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\get.c
DEP_CPP_GET_C=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_GET_C=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\get.obj" : $(SOURCE) $(DEP_CPP_GET_C) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\hash.c
DEP_CPP_HASH_=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_HASH_=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\hash.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\image.c
DEP_CPP_IMAGE=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_IMAGE=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\image.obj" : $(SOURCE) $(DEP_CPP_IMAGE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\light.c
DEP_CPP_LIGHT=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_LIGHT=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\light.obj" : $(SOURCE) $(DEP_CPP_LIGHT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\lines.c
DEP_CPP_LINES=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\linetemp.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_LINES=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\lines.obj" : $(SOURCE) $(DEP_CPP_LINES) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\logic.c
DEP_CPP_LOGIC=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_LOGIC=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\logic.obj" : $(SOURCE) $(DEP_CPP_LOGIC) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\masking.c
DEP_CPP_MASKI=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_MASKI=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\masking.obj" : $(SOURCE) $(DEP_CPP_MASKI) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\matrix.c
DEP_CPP_MATRI=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_MATRI=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\matrix.obj" : $(SOURCE) $(DEP_CPP_MATRI) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\misc.c
DEP_CPP_MISC_=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_MISC_=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\misc.obj" : $(SOURCE) $(DEP_CPP_MISC_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\mmath.c
DEP_CPP_MMATH=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_MMATH=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\mmath.obj" : $(SOURCE) $(DEP_CPP_MMATH) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\pb.c
DEP_CPP_PB_C3a=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_PB_C3a=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\pb.obj" : $(SOURCE) $(DEP_CPP_PB_C3a) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\pixel.c
DEP_CPP_PIXEL=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_PIXEL=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\pixel.obj" : $(SOURCE) $(DEP_CPP_PIXEL) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\pointers.c
DEP_CPP_POINT=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\rect.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_POINT=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\pointers.obj" : $(SOURCE) $(DEP_CPP_POINT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\points.c
DEP_CPP_POINTS=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_POINTS=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\points.obj" : $(SOURCE) $(DEP_CPP_POINTS) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\polygon.c
DEP_CPP_POLYG=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_POLYG=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\polygon.obj" : $(SOURCE) $(DEP_CPP_POLYG) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\quads.c
DEP_CPP_QUADS=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\quads.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_QUADS=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\quads.obj" : $(SOURCE) $(DEP_CPP_QUADS) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\rastpos.c

!IF  "$(CFG)" == "opengl32 - Win32 Release"

DEP_CPP_RASTP=\
	"..\src\clip.h"\
	"..\src\config.h"\
	"..\src\dd.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\light.h"\
	"..\src\macros.h"\
	"..\src\matrix.h"\
	"..\src\mmath.h"\
	"..\src\shade.h"\
	"..\src\xform.h"\
	

"$(INTDIR)\rastpos.obj" : $(SOURCE) $(DEP_CPP_RASTP) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "opengl32 - Win32 Debug"

DEP_CPP_RASTP=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_RASTP=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\rastpos.obj" : $(SOURCE) $(DEP_CPP_RASTP) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\readpix.c
DEP_CPP_READP=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_READP=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\readpix.obj" : $(SOURCE) $(DEP_CPP_READP) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\rect.c
DEP_CPP_RECT_=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\rect.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_RECT_=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\rect.obj" : $(SOURCE) $(DEP_CPP_RECT_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\s3mesa.c

!IF  "$(CFG)" == "opengl32 - Win32 Release"

DEP_CPP_S3MES=\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\dd.h"\
	"..\src\fixed.h"\
	"..\src\macros.h"\
	"..\src\matrix.h"\
	"..\src\s3dtk.h"\
	"..\src\s3mesa.h"\
	"..\src\texture.h"\
	"..\src\vb.h"\
	"..\src\xform.h"\
	
CPP_SWITCHES=/nologo /G5 /Gz /MT /W3 /GX /Od /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "S3" /Fp"$(INTDIR)\opengl32.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\s3mesa.obj" : $(SOURCE) $(DEP_CPP_S3MES) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "opengl32 - Win32 Debug"

DEP_CPP_S3MES=\
	"..\src\context.h"\
	"..\src\macros.h"\
	"..\src\matrix.h"\
	"..\src\s3dtk.h"\
	"..\src\s3mesa.h"\
	"..\src\texture.h"\
	"..\src\vb.h"\
	"..\src\xform.h"\
	
CPP_SWITCHES=/nologo /G5 /Gz /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "S3" /Fp"$(INTDIR)\opengl32.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\s3mesa.obj" : $(SOURCE) $(DEP_CPP_S3MES) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\src\s3mesa.rc

!IF  "$(CFG)" == "opengl32 - Win32 Release"


"$(INTDIR)\s3mesa.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x809 /fo"$(INTDIR)\s3mesa.res" /i "\Mesa-2.5\src" /d "NDEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "opengl32 - Win32 Debug"


"$(INTDIR)\s3mesa.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x809 /fo"$(INTDIR)\s3mesa.res" /i "\Mesa-2.5\src" /d "_DEBUG"\
 $(SOURCE)


!ENDIF 

SOURCE=..\src\s3wgl.c
DEP_CPP_S3WGL=\
	"..\src\s3dtk.h"\
	"..\src\s3mesa.h"\
	{$(INCLUDE)}"GL\gl.h"\
	{$(INCLUDE)}"GL\glu.h"\
	

!IF  "$(CFG)" == "opengl32 - Win32 Release"

CPP_SWITCHES=/nologo /G5 /Gz /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "S3" /Fp"$(INTDIR)\opengl32.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\s3wgl.obj" : $(SOURCE) $(DEP_CPP_S3WGL) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "opengl32 - Win32 Debug"

CPP_SWITCHES=/nologo /G5 /Gz /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "S3" /Fp"$(INTDIR)\opengl32.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\s3wgl.obj" : $(SOURCE) $(DEP_CPP_S3WGL) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\src\scissor.c
DEP_CPP_SCISS=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_SCISS=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\scissor.obj" : $(SOURCE) $(DEP_CPP_SCISS) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\shade.c
DEP_CPP_SHADE=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_SHADE=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\shade.obj" : $(SOURCE) $(DEP_CPP_SHADE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\span.c
DEP_CPP_SPAN_=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_SPAN_=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\span.obj" : $(SOURCE) $(DEP_CPP_SPAN_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\stencil.c
DEP_CPP_STENC=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_STENC=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\stencil.obj" : $(SOURCE) $(DEP_CPP_STENC) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\teximage.c
DEP_CPP_TEXIM=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_TEXIM=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\teximage.obj" : $(SOURCE) $(DEP_CPP_TEXIM) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\texobj.c
DEP_CPP_TEXOB=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_TEXOB=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\texobj.obj" : $(SOURCE) $(DEP_CPP_TEXOB) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\texstate.c
DEP_CPP_TEXST=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_TEXST=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\texstate.obj" : $(SOURCE) $(DEP_CPP_TEXST) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\texture.c
DEP_CPP_TEXTU=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_TEXTU=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\texture.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\triangle.c
DEP_CPP_TRIAN=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\tritemp.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_TRIAN=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\triangle.obj" : $(SOURCE) $(DEP_CPP_TRIAN) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\varray.c
DEP_CPP_VARRA=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_VARRA=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\varray.obj" : $(SOURCE) $(DEP_CPP_VARRA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\vb.c
DEP_CPP_VB_C64=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_VB_C64=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\vb.obj" : $(SOURCE) $(DEP_CPP_VB_C64) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\vbfill.c
DEP_CPP_VBFIL=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_VBFIL=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\vbfill.obj" : $(SOURCE) $(DEP_CPP_VBFIL) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\vbrender.c
DEP_CPP_VBREN=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_VBREN=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\vbrender.obj" : $(SOURCE) $(DEP_CPP_VBREN) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\vbxform.c
DEP_CPP_VBXFO=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_VBXFO=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\vbxform.obj" : $(SOURCE) $(DEP_CPP_VBXFO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\winpos.c
DEP_CPP_WINPO=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_WINPO=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\winpos.obj" : $(SOURCE) $(DEP_CPP_WINPO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\xform.c
DEP_CPP_XFORM=\
	"..\src\accum.h"\
	"..\src\all.h"\
	"..\src\alpha.h"\
	"..\src\alphabuf.h"\
	"..\src\api.h"\
	"..\src\attrib.h"\
	"..\src\bitmap.h"\
	"..\src\blend.h"\
	"..\src\clip.h"\
	"..\src\colortab.h"\
	"..\src\config.h"\
	"..\src\context.h"\
	"..\src\copypix.h"\
	"..\src\dd.h"\
	"..\src\depth.h"\
	"..\src\dlist.h"\
	"..\src\drawpix.h"\
	"..\src\enable.h"\
	"..\src\eval.h"\
	"..\src\feedback.h"\
	"..\src\fixed.h"\
	"..\src\fog.h"\
	"..\src\get.h"\
	"..\src\hash.h"\
	"..\src\image.h"\
	"..\src\light.h"\
	"..\src\lines.h"\
	"..\src\logic.h"\
	"..\src\macros.h"\
	"..\src\masking.h"\
	"..\src\matrix.h"\
	"..\src\misc.h"\
	"..\src\mmath.h"\
	"..\src\pb.h"\
	"..\src\pixel.h"\
	"..\src\pointers.h"\
	"..\src\points.h"\
	"..\src\polygon.h"\
	"..\src\rastpos.h"\
	"..\src\readpix.h"\
	"..\src\scissor.h"\
	"..\src\shade.h"\
	"..\src\span.h"\
	"..\src\stencil.h"\
	"..\src\teximage.h"\
	"..\src\texobj.h"\
	"..\src\texstate.h"\
	"..\src\texture.h"\
	"..\src\triangle.h"\
	"..\src\varray.h"\
	"..\src\vb.h"\
	"..\src\vbfill.h"\
	"..\src\vbrender.h"\
	"..\src\vbxform.h"\
	"..\src\winpos.h"\
	"..\src\xform.h"\
	{$(INCLUDE)}"GL\gl.h"\
	
NODEP_CPP_XFORM=\
	"..\src\GL\osmesa.h"\
	

"$(INTDIR)\xform.obj" : $(SOURCE) $(DEP_CPP_XFORM) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


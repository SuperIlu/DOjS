# Microsoft Developer Studio Project File - Name="libmikmod" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=libmikmod - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libmikmod.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libmikmod.mak" CFG="libmikmod - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libmikmod - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libmikmod - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libmikmod - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "MIKMOD_BUILD" /D "DLL_EXPORT" /D "DRV_DS" /D "DRV_WIN" /D "DRV_AIFF" /D "DRV_WAV" /D "DRV_RAW" /D "HAVE_LIMITS_H" /D "HAVE_FCNTL_H" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\win32" /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "MIKMOD_BUILD" /D "DLL_EXPORT" /D "DRV_DS" /D "DRV_WIN" /D "DRV_AIFF" /D "DRV_WAV" /D "DRV_RAW" /D "HAVE_LIMITS_H" /D "HAVE_FCNTL_H" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib winmm.lib dsound.lib /nologo /dll /machine:I386
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "libmikmod - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "MIKMOD_BUILD" /D "DLL_EXPORT" /D "DRV_DS" /D "DRV_WIN" /D "DRV_AIFF" /D "DRV_WAV" /D "DRV_RAW" /D "HAVE_LIMITS_H" /D "HAVE_FCNTL_H" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\win32" /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "MIKMOD_BUILD" /D "DLL_EXPORT" /D "DRV_DS" /D "DRV_WIN" /D "DRV_AIFF" /D "DRV_WAV" /D "DRV_RAW" /D "HAVE_LIMITS_H" /D "HAVE_FCNTL_H" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib winmm.lib dsound.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ENDIF 

# Begin Target

# Name "libmikmod - Win32 Release"
# Name "libmikmod - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\drivers\drv_aiff.c
# End Source File
# Begin Source File

SOURCE=..\..\drivers\drv_ds.c
# End Source File
# Begin Source File

SOURCE=..\..\drivers\drv_nos.c
# End Source File
# Begin Source File

SOURCE=..\..\drivers\drv_openal.c
# End Source File
# Begin Source File

SOURCE=..\..\drivers\drv_raw.c
# End Source File
# Begin Source File

SOURCE=..\..\drivers\drv_sdl.c
# End Source File
# Begin Source File

SOURCE=..\..\drivers\drv_stdout.c
# End Source File
# Begin Source File

SOURCE=..\..\drivers\drv_wav.c
# End Source File
# Begin Source File

SOURCE=..\..\drivers\drv_win.c
# End Source File
# Begin Source File

SOURCE=..\..\drivers\drv_xaudio2.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_669.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_amf.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_asy.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_dsm.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_far.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_gdm.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_gt2.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_imf.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_it.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_m15.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_med.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_mod.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_mtm.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_okt.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_s3m.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_stm.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_stx.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_ult.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_umx.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_uni.c
# End Source File
# Begin Source File

SOURCE=..\..\loaders\load_xm.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\mdreg.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\mdriver.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\mloader.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\mlreg.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\mlutil.c
# End Source File
# Begin Source File

SOURCE=..\..\mmio\mmalloc.c
# End Source File
# Begin Source File

SOURCE=..\..\mmio\mmerror.c
# End Source File
# Begin Source File

SOURCE=..\..\mmio\mmio.c
# End Source File
# Begin Source File

SOURCE=..\..\depackers\mmcmp.c
# End Source File
# Begin Source File

SOURCE=..\..\depackers\pp20.c
# End Source File
# Begin Source File

SOURCE=..\..\depackers\s404.c
# End Source File
# Begin Source File

SOURCE=..\..\depackers\xpk.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\mplayer.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\munitrk.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\mwav.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\npertab.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\sloader.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\virtch.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\virtch2.c
# End Source File
# Begin Source File

SOURCE=..\..\playercode\virtch_common.c
# End Source File
# Begin Source File

SOURCE=..\..\posix\strcasecmp.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\mikmod.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mikmod_ctype.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mikmod_internals.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

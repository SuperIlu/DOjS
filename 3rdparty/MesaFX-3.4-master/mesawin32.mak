# Be sure to modify the definitions in this file to agree with your
# systems installation.
#
#  NOTE: be sure that the install directories use '\' not '/' for paths.
#
#  Modified by tjump@spgs.com for better functioning with the win32\ build
#  rules.

# BEGIN USER MODIFICATION ZONE ...
#
# Make sure the following definitions make sense for your system! They
# *should* pluck out reasonable defaults, however it is possible for
# this code to fail in it's setup.
#
# These definitions are used only by build files in the win32\ subdirectory.
#

# import libraries install directory
#
LIBINSTALL     = $(MSDEVDIR)\..\VC\LIB

# header file installation directory
HDRINSTALL     = $(MSDEVDIR)\..\VC\INCLUDE

# dll file installation directory
#
!IF "$(OS)" == "Windows_NT"
DLLINSTALL     = $(WINDIR)\SYSTEM32
!ELSE # "$(OS)" != "Windows_NT"
DLLINSTALL     = $(WINDIR)\SYSTEM
!ENDIF

# END USER MODIFICATION ZONE ...
#
# you should not need to change anything beyond this line manually.

!IF "$(DEVDIR)" != ""
LIBINSTALL = $(DEVDIR)\SDK\LIB\WIN32\VC
HDRINSTALL = $(DEVDIR)\SDK\H
DLLINSTALL = $(DEVDIR)\SDK\BIN
!ENDIF

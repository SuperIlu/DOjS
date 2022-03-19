# 
# 1. [Un]comment the next two as required :
# ALLEGRONOTAVAIL indicates the allegro library is not available and
#                 that the eventual programme will never be compiled with it.
# ALLEGRONOTPROGS tells the makefile to make the test and example
#                 programmes without using allegro.
# The only combination that is not allowed is ALLEGRONOTAVAIL without
# ALLEGRONOTPROGS.
#

# ALLEGRONOTAVAIL = 1
# ALLEGRONOTPROGS = 1

#
# 1. Put here the path on which the compiler and other tools
# for the target will be found with standard names (eg gcc,
# not dos-gcc).
#
XC_PATH=/users/neil/bli/sun4-sunos/i386-pc-msdosdjgpp/bin

#
# 2. Put here the include list for where the above compiler will
# find allegro headers
#
XC_INCLUDE=/users/neil/bli/pc-dos/include

#
# 3. Put here the library list for where the above compiler will
# find allegro headers
#
XC_LIBS=/users/neil/bli/pc-dos/lib

#
# 4. Put here the path for where things are to be installed.
# You should have created the lib, info and include directories
# in this directory.
#
INSTALL_BASE=/users/neil/bli/pc-dos

all : install

ifdef ALLEGRONOTAVAIL
ifdef ALLEGRONOTPROGS
main : depend
	make CROSSCOMPILE="1" \
		XC_INCLUDE="${XC_INCLUDE:%=-I%}" \
		XC_LIBS="${XC_LIBS:%=-L%}" \
		DJDIR="$(XC_PATH)" \
		PATH="$(XC_PATH):$(PATH)" \
		NATIVEPATH="$(PATH)" \
		ALLEGRONOTAVAIL="1" \
		ALLEGRONOTPROGS="1"
else
main : depend
	make CROSSCOMPILE="1" \
		XC_INCLUDE="${XC_INCLUDE:%=-I%}" \
		XC_LIBS="${XC_LIBS:%=-L%}" \
		DJDIR="$(XC_PATH)" \
		PATH="$(XC_PATH):$(PATH)" \
		NATIVEPATH="$(PATH)" \
		ALLEGRONOTAVAIL="1"
endif
else
ifdef ALLEGRONOTPROGS
main : depend
	make CROSSCOMPILE="1" \
		XC_INCLUDE="${XC_INCLUDE:%=-I%}" \
		XC_LIBS="${XC_LIBS:%=-L%}" \
		DJDIR="$(XC_PATH)" \
		PATH="$(XC_PATH):$(PATH)" \
		NATIVEPATH="$(PATH)" \
		ALLEGRONOTPROGS="1"
else
main : depend
	make CROSSCOMPILE="1" \
		XC_INCLUDE="${XC_INCLUDE:%=-I%}" \
		XC_LIBS="${XC_LIBS:%=-L%}" \
		DJDIR="$(XC_PATH)" \
		PATH="$(XC_PATH):$(PATH)" \
		NATIVEPATH="$(PATH)"
endif
endif

depend :
	make CROSSCOMPILE="1" \
		PATH="$(XC_PATH):$(PATH)" \
		depend

install : main
	make CROSSCOMPILE="1" DJDIR="$(INSTALL_BASE)" install

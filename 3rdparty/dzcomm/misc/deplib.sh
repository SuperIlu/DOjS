#! /bin/sh

# Generate dependencies and rules for building libraries.


# write_code:  Writes the Make rules to create a set of libraries.
#  Pass the library type (e.g. `dzcom') and the object list variables' prefix
#  (e.g. 'LIBDZCOM').  What a nasty mixture of shell and Make variables...
write_code() {
    staticlib="lib${1}.a"
    staticobj="\$(${2}_OBJECTS)"

    sharelib="lib${1}-\$(shared_version).so"
    soname="lib${1}.so.\$(shared_major)"
    link="lib${1}.so"
    shareobj="\$(${2}_SHARED_OBJECTS)"

    unsharelib="lib${1}_nonshared.a"
    unshareobj="\$(${2}_UNSHARED_OBJECTS)"
    
    echo "\$(LIBDIR)/${staticlib}: ${staticobj}"
    echo "	rm -f \$(LIBDIR)/${staticlib}"
    echo "	\$(AR) rvs \$(LIBDIR)/${staticlib} ${staticobj}"
    echo ""
    echo "\$(LIBDIR)/${sharelib}: ${shareobj}"
    echo "	rm -f \$(LIBDIR)/${sharelib}"
    # gf: This bit is obviously gcc-specific
    echo "	gcc -shared -Wl,-soname,${soname} -o \$(LIBDIR)/${sharelib} ${shareobj}"
    echo ""
    echo "\$(LIBDIR)/${soname}: \$(LIBDIR)/${sharelib}"
    echo "	cd \$(LIBDIR) ; ln -sf ${sharelib} ${soname}"
    echo ""
    echo "\$(LIBDIR)/${link}: \$(LIBDIR)/${soname}"
    echo "	\$(mksofile) \$(LIBDIR) ${link} ${soname} ${unsharelib}"
    echo ""
    echo "\$(LIBDIR)/${unsharelib}: ${unshareobj}"
    echo "	rm -f ${unsharelib}"
    echo "	\$(AR) rvs \$(LIBDIR)/${unsharelib} ${unshareobj}"
}


sources=`echo $* | sed 's,[^	 ]*/,,g'`
sharable_sources=`echo $sources | sed 's,[^.	 ]*\.s,,g'`
unsharable_sources=`echo $sources | sed 's,[^.	 ]*\.[^s],,g'`

objects=`echo $sources | sed 's,\.[^.	 ]*,,g'`
sharable_objects=`echo $sharable_sources | sed 's,\.[^.	 ]*,,g'`
unsharable_objects=`echo $unsharable_sources | sed 's,\.[^.	 ]*,,g'`


# Normal library.
prev="LIBDZCOM_OBJECTS ="
for file in .. $objects; do
  if test "$file" != ..; then
    echo "$prev \\"
    prev="  \$(OBJDIR)/dzcom/$file\$(OBJ)"
  fi
done
echo "$prev"
prev="LIBDZCOM_SHARED_OBJECTS ="
for file in .. $sharable_objects; do
  if test "$file" != ..; then
    echo "$prev \\"
    prev="  \$(OBJDIR)/shared/dzcom/$file\$(OBJ)"
  fi
done
echo "$prev"
prev="LIBDZCOM_UNSHARED_OBJECTS ="
for file in .. $unsharable_objects; do
  if test "$file" != ..; then
    echo "$prev \\"
    prev="  \$(OBJDIR)/shared/dzcom/$file\$(OBJ)"
  fi
done
echo "$prev"
echo ""
write_code dzcom LIBDZCOM
echo ""
echo ""


# Debugging library.
prev="LIBDZCD_OBJECTS ="
for file in .. $objects; do
  if test "$file" != ..; then
    echo "$prev \\"
    prev="  \$(OBJDIR)/dzcd/$file\$(OBJ)"
  fi
done
echo "$prev"
prev="LIBDZCD_SHARED_OBJECTS ="
for file in .. $objects; do
  if test "$file" != ..; then
    echo "$prev \\"
    prev="  \$(OBJDIR)/shared/dzcd/$file\$(OBJ)"
  fi
done
echo "$prev"
prev="LIBDZCD_UNSHARED_OBJECTS ="
for file in .. $unsharable_objects; do
  if test "$file" != ..; then
    echo "$prev \\"
    prev="  \$(OBJDIR)/shared/dzcd/$file\$(OBJ)"
  fi
done
echo "$prev"
echo ""
write_code dzcd LIBDZCD
echo ""
echo ""


# Profiling library.
prev="LIBDZCP_OBJECTS ="
for file in .. $objects; do
  if test "$file" != ..; then
    echo "$prev \\"
    prev="  \$(OBJDIR)/dzcp/$file\$(OBJ)"
  fi
done
echo "$prev"
prev="LIBDZCP_SHARED_OBJECTS ="
for file in .. $objects; do
  if test "$file" != ..; then
    echo "$prev \\"
    prev="  \$(OBJDIR)/shared/dzcp/$file\$(OBJ)"
  fi
done
echo "$prev"
prev="LIBDZCP_UNSHARED_OBJECTS ="
for file in .. $unsharable_objects; do
  if test "$file" != ..; then
    echo "$prev \\"
    prev="  \$(OBJDIR)/shared/dzcp/$file\$(OBJ)"
  fi
done
echo "$prev"
echo ""
write_code dzcp LIBDZCP
echo ""
echo ""


missing=
symbols=
for file in .. $*; do
  if test -f "$file"; then
    dir=`echo $file | sed 's,/[^/]*$,,'`
    name=`echo $file | sed 's,^.*/,,;s,\.[^.]*$,,'`
    ext=`echo $file | sed 's,^.*\.,,'`
    includes=
    deps="$file"
    while test -n "$deps"; do
      newdeps=
      for dep in $deps; do
        includes1=`grep '^[ 	]*#[ 	]*include[ 	]*[a-zA-Z0-9_][a-zA-Z0-9_]*' $dep | \
          sed 's,^[ 	]*#[ 	]*include[ 	]*\([a-zA-Z0-9_]*\),\1,'`
        includes2=`grep '^[ 	]*#[ 	]*include[ 	]*".*"' $dep | \
          sed 's,^[ 	]*#[ 	]*include[ 	]*"\(.*\)",\1,'`
        if test -n "$includes1"; then
          for include in $includes1; do
	    includes="$includes \$($include)"
	    case "$symbols" in
	    *$include* )
	      ;;
	    * )
	      symbols="$symbols $include"
	      ;;
	    esac
          done
        fi
        if test -n "$includes2"; then
          for include in $includes2; do
	    if test -f "$dir/$include"; then
              includes="$includes \$(srcdir)/$dir/$include"
	      newdeps="$newdeps $dir/$include"
	    else
	      include=`echo $include | sed 's,[-./],_,g'`
	      includes="$includes \$($include)"
	      case "$symbols" in
	      *$include* )
	        ;;
	      * )
	        symbols="$symbols $include"
	        ;;
	      esac
	    fi
          done
        fi
      done
      deps="$newdeps"
    done

    # Normal library.
    echo "\$(OBJDIR)/dzcom/$name\$(OBJ): \$(srcdir)/$file$includes"
    if test "$ext" = "c"; then
      echo "	\$(COMPILE_NORMAL) -c \$(srcdir)/$file -o \$(OBJDIR)/dzcom/$name\$(OBJ)"
    else
      echo "	\$(COMPILE_S_NORMAL) -c \$(srcdir)/$file -o \$(OBJDIR)/dzcom/$name\$(OBJ)"
    fi
    echo "\$(OBJDIR)/shared/dzcom/$name\$(OBJ): \$(srcdir)/$file$includes"
    if test "$ext" = "c"; then
      echo "	\$(COMPILE_NORMAL) \$(DZCOMM_SHAREDLIB_CFLAGS) -c \$(srcdir)/$file -o \$(OBJDIR)/shared/dzcom/$name\$(OBJ)"
    else
      echo "	\$(COMPILE_S_NORMAL) -c \$(srcdir)/$file -o \$(OBJDIR)/shared/dzcom/$name\$(OBJ)"
    fi
    echo ""

    # Debugging library.
    echo "\$(OBJDIR)/dzcd/$name\$(OBJ): \$(srcdir)/$file$includes"
    if test "$ext" = "c"; then
      echo "	\$(COMPILE_DEBUG) -c \$(srcdir)/$file -o \$(OBJDIR)/dzcd/$name\$(OBJ)"
    else
      echo "	\$(COMPILE_S_DEBUG) -c \$(srcdir)/$file -o \$(OBJDIR)/dzcd/$name\$(OBJ)"
    fi
    echo "\$(OBJDIR)/shared/dzcd/$name\$(OBJ): \$(srcdir)/$file$includes"
    if test "$ext" = "c"; then
      echo "	\$(COMPILE_DEBUG) \$(DZCOMM_SHAREDLIB_CFLAGS) -c \$(srcdir)/$file -o \$(OBJDIR)/shared/dzcd/$name\$(OBJ)"
    else
      echo "	\$(COMPILE_S_DEBUG) -c \$(srcdir)/$file -o \$(OBJDIR)/shared/dzcd/$name\$(OBJ)"
    fi
    echo ""

    # Profiling library.
    echo "\$(OBJDIR)/dzcp/$name\$(OBJ): \$(srcdir)/$file$includes"
    if test "$ext" = "c"; then
      echo "	\$(COMPILE_PROFILE) -c \$(srcdir)/$file -o \$(OBJDIR)/dzcp/$name\$(OBJ)"
    else
      echo "	\$(COMPILE_S_PROFILE) -c \$(srcdir)/$file -o \$(OBJDIR)/dzcp/$name\$(OBJ)"
    fi
    echo "\$(OBJDIR)/shared/dzcp/$name\$(OBJ): \$(srcdir)/$file$includes"
    if test "$ext" = "c"; then
      echo "	\$(COMPILE_PROFILE) \$(DZCOMM_SHAREDLIB_CFLAGS) -c \$(srcdir)/$file -o \$(OBJDIR)/shared/dzcp/$name\$(OBJ)"
    else
      echo "	\$(COMPILE_S_PROFILE) -c \$(srcdir)/$file -o \$(OBJDIR)/shared/dzcp/$name\$(OBJ)"
    fi
    echo ""
  elif test "$file" != ..; then
    missing="$missing $file"
  fi
done

if test -n "$symbols"; then
  echo "# Headers referred by symbols:"
  echo "#$symbols"
  echo ""
fi

if test -n "$missing"; then
  echo "# Missing source files:"
  echo "#$missing"
  echo ""
fi

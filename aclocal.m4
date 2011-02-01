
dnl: macro simply checks for existence of header file "$2" in dir "$1"
AC_DEFUN(PD_CHECK_INC_DIR,[
  CXXFLAGS_HOLD=$CXXFLAGS
  if test "$1" != "" ; then
    AC_MSG_CHECKING([for $2 in $1])
    CXXFLAGS="$CXXFLAGS -I$1"
  else
    AC_MSG_CHECKING([for $2])
  fi
  AC_COMPILE_IFELSE( [AC_LANG_PROGRAM([#include "$2"],[])],
                     [AC_MSG_RESULT(yes)],
                     [AC_MSG_ERROR([$2 not found in $1])] )
  CXXFLAGS=$CXXFLAGS_HOLD
])

AC_DEFUN(PD_CHECK_LIB_TYPE,[
  LIB_TYPE_DYNAMIC=`ls -H -1 $1 | grep $2 | grep '\.so' | wc | awk '{if($'2' > 0){print "true"} else {print ""}}'`
  if test "$LIB_TYPE_DYNAMIC" = "true"; then
	LIB_TYPE_STATIC=	
  else
  	LIB_TYPE_STATIC=`ls -H -1 $1 | grep $2 | grep '\.a' | wc | awk '{if($'2' > 0){print "true"} else {print ""}}'`
  fi
])

AC_DEFUN(PD_SOFT_CHECK_INC_DIR,[
  CXXFLAGS_HOLD=$CXXFLAGS
  if test "$1" != "" ; then
    AC_MSG_CHECKING([for $2 in $1])
    CXXFLAGS="$CXXFLAGS -I$1"
  else
    AC_MSG_CHECKING([for $2])
  fi
  AC_COMPILE_IFELSE( [AC_LANG_PROGRAM([#include "$2"],[])],
                     [AC_DEFINE([cap_have_$3], 1, [...yes])],
                     [AC_MSG_RESULT([$2 not found in $1])] )
  CXXFLAGS=$CXXFLAGS_HOLD
])

dnl: if first argument not set, check for function symbol $3 in lib $2
dnl: otherwise, set lib to -L$1 before check 
dnl: if $4 and $5 are set, they are additional libdirs and libs needed
dnl: for linking to resolve all references properly and test to pass
AC_DEFUN(PD_CHECK_LIB_DIR,[
  if test "$1" = "" ; then
    AC_CHECK_LIB($2, $3, [], [AC_MSG_ERROR(Can't find lib$2.)])
  else
    LIBS_HOLD=$LIBS
    if test "$4" != "" ; then
      LIBS="-L$1 -L$4"
      AC_CHECK_LIB($2, $3, [], [AC_MSG_ERROR(Can't find lib$2 in $1)], $5)
    else
      LIBS="-L$1"
      AC_CHECK_LIB($2, $3, [], [AC_MSG_ERROR(Can't find lib$2 in $1)])
    fi
    LIBS=$LIBS_HOLD
  fi
])

AC_DEFUN(PD_CHECK_LIB_DIR_WERR,[
  if test "$1" = "" ; then
    AC_CHECK_LIB($2, $3, [], $6)
  else
    LIBS_HOLD=$LIBS
    if test "$4" != "" ; then
      LIBS="-L$1 -L$4"
      AC_CHECK_LIB($2, $3, [], $6, $5)
    else
      LIBS="-L$1"
      AC_CHECK_LIB($2, $3, [], $6)
    fi
    LIBS=$LIBS_HOLD
  fi
])

AC_DEFUN(PD_SOFT_CHECK_LIB_DIR,[
  if test "$1" = "" ; then
    AC_CHECK_LIB($2, $3, [], [AC_MSG_RESULT(Can't find lib$2.)])
  else
    LIBS_HOLD=$LIBS
    if test "$4" != "" ; then
      LIBS="-L$1 -L$4"
      AC_CHECK_LIB($2, $3, [], 
	[AC_MSG_RESULT(Can't find lib$2 in $1)], $5)
    else
      LIBS="-L$1"
      AC_CHECK_LIB($2, $3, [], 
	[AC_MSG_RESULT(Can't find lib$2 in $1)])
    fi
    LIBS=$LIBS_HOLD
  fi
])

AC_DEFUN(PD_CHECK_LIB_FEATURE,[
  if test "$1" = "" ; then
    AC_CHECK_LIB($2, $3, [HAS_FEATURE="true"], [])
  else
    LIBS_HOLD=$LIBS
    if test "$4" != "" ; then
      LIBS="-L$1 -L$4"
      AC_CHECK_LIB($2, $3, [HAS_FEATURE="true"], [], $5)
    else
      LIBS="-L$1"
      AC_CHECK_LIB($2, $3, [HAS_FEATURE="true"], [])
    fi
    LIBS=$LIBS_HOLD
  fi
])

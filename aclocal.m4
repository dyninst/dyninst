
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
  LIB_TYPE_STATIC=`ls -H -1 $1 | grep $2 | grep '\.a' | wc | awk '{if($'2' > 0){print "true"} else {print ""}}'`
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
    FIRST_LIBPARAM=""
  else
    FIRST_LIBPARAM="-L$1"
  fi
  if test "$4" = "" ; then
    FOURTH_LIBPARAM=""
  else
    FOURTH_LIBPARAM="-L$4"
  fi
  LIBS_HOLD=$LIBS
  LIBS="$LIBS $FIRST_LIBPARAM $FOURTH_LIBPARAM"
  AC_CHECK_LIB($2, $3, [], [AC_MSG_ERROR(Cant find lib$2.)], $5)
  LIBS=$LIBS_HOLD
])

AC_DEFUN(PD_CHECK_LIB_DIR_WERR,[
  if test "$1" = "" ; then
    FIRST_LIBPARAM=""
  else
    FIRST_LIBPARAM="-L$1"
  fi
  if test "$4" = "" ; then
    FOURTH_LIBPARAM=""
  else
    FOURTH_LIBPARAM="-L$4"
  fi
  LIBS_HOLD=$LIBS
  LIBS="$LIBS $FIRST_LIBPARAM $FOURTH_LIBPARAM"
  AC_CHECK_LIB($2, $3, [], $6, $5)
  LIBS=$LIBS_HOLD
])

AC_DEFUN(PD_SOFT_CHECK_LIB_DIR,[
  if test "$1" = "" ; then
    FIRST_LIBPARAM=""
  else
    FIRST_LIBPARAM="-L$1"
  fi
  if test "$4" = "" ; then
    FOURTH_LIBPARAM=""
  else
    FOURTH_LIBPARAM="-L$4"
  fi
  LIBS_HOLD=$LIBS
  LIBS="$LIBS $FIRST_LIBPARAM $FOURTH_LIBPARAM"
  AC_CHECK_LIB($2, $3, [], [AC_MSG_RESULT(Cant find lib$2.)], $5)
  LIBS=$LIBS_HOLD
])

AC_DEFUN(PD_CHECK_LIB_FEATURE,[
  if test "$1" = "" ; then
    FIRST_LIBPARAM=""
  else
    FIRST_LIBPARAM="-L$1"
  fi
  if test "$4" = "" ; then
    FOURTH_LIBPARAM=""
  else
    FOURTH_LIBPARAM="-L$4"
  fi
  LIBS_HOLD=$LIBS
  LIBS="$LIBS $FIRST_LIBPARAM $FOURTH_LIBPARAM"
  AC_CHECK_LIB($2, $3, [HAS_FEATURE="true"], [], $5)
  LIBS=$LIBS_HOLD
])

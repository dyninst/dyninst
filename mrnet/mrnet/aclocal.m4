dnl Determine the compiler type
dnl Sets: COMPILER_TYPE to "gnu", "aix-native", "forte" or "unknown"

AC_DEFUN(PD_COMPILER_TYPE,[
  AC_MSG_CHECKING([for compiler type])

  COMPILER_TYPE="unknown"

  if test "$CXX" = "g++" ; then
    COMPILER_TYPE="gnu"
    AC_MSG_RESULT([gnu])
  elif test "$CXX" = "xlC" ; then
    COMPILER_TYPE="aix-native"
    AC_MSG_RESULT([aix-native])
  fi

  if test "$COMPILER_TYPE" = "unknown"; then
    tmp_out=`$CXX --version 2> /dev/null | grep GCC`
    if test "$tmp_out" = "GCC" ; then
      COMPILER_TYPE="gnu"
      AC_MSG_RESULT([gnu])
    fi
  fi
    
  if test "$COMPILER_TYPE" = "unknown"; then
    tmp_out=`$CXX -V 2>& 1 | grep GCC`
    if test "$tmp_out" = "Forte" ; then
      COMPILER_TYPE="forte"
      AC_MSG_RESULT([forte])
    fi
  fi

  if test "$COMPILER_TYPE" = "unknown"; then
    AC_MSG_RESULT([unknown])
  fi

  AC_SUBST(COMPILER_TYPE)
])dnl

AC_DEFUN(PAC_CHECK_RUSAGE,[
  AC_MSG_CHECKING([for getrusage()])
  if test -z "$RUSAGE" ; then
  AC_TRY_LINK(
[#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>],
[ int i;
  struct rusage usage;

  i = getrusage(RUSAGE_SELF, &usage);
],
    RUSAGE="HAVERUSAGE"
    AC_MSG_RESULT(yes),
    RUSAGE="NORUSAGE"
    AC_MSG_RESULT(no))
  else
    AC_MSG_RESULT([NO TEST PERFORMED: using assigned value ($RUSAGE)])
  fi
  AC_SUBST(RUSAGE)
  ])dnl


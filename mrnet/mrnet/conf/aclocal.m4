dnl Get the format of Fortran names.
dnl Uses: F77, FFLAGS
dnl Sets: F2CNAMES.
dnl CREDITS: stolen from mpich1.2's aclocal.m4
dnl
AC_DEFUN(PAC_GET_F2CNAMES,[
  AC_MSG_CHECKING([for f2c naming])
  if test -z "$F2CNAMES" ; then
     cat > confftest.f <<EOF
       program fnametest
       external netsolve_test

       call netsolve_test( )

       stop
       end
EOF
    echo "$F77 $FFLAGS -c confftest.f" >> config.log
    $F77 $FFLAGS -c confftest.f >> config.log 2>&1
    if test ! -s confftest.o ; then
      echo "Unable to test Fortran compiler" 2>&1
      echo "(compiling a test program failed to produce an " 2>&1
      echo "object file)." 2>&1
      echo "Assuming Fortran externals are lowercase with 1 trailing underscore." 2>&1
      F2CNAMES="F2CADD_"
    else
      nameform1=`strings -a confftest.o | grep netsolve_test_  | sed -n -e '1p'`
      nameform2=`strings -a confftest.o | grep NETSOLVE_TEST   | sed -n -e '1p'`
      nameform3=`strings -a confftest.o | grep netsolve_test   | sed -n -e '1p'`
      nameform4=`strings -a confftest.o | grep netsolve_test__ | sed -n -e '1p'`
      /bin/rm -f confftest.f confftest.o
      if test -n "$nameform4" ; then
        AC_MSG_RESULT([lower case w/ 1 or 2 trailing underscores])
	    F2CNAMES="F2CADD__"
      elif test -n "$nameform1" ; then
        AC_MSG_RESULT([lower case w/ 1 trailing underscore])
	    F2CNAMES="F2CADD_"
      elif test -n "$nameform2" ; then
        AC_MSG_RESULT([upper case])
	    F2CNAMES="F2CUPCASE" 
      elif test -n "$nameform3" ; then
        AC_MSG_RESULT([lower case with no trailing underscores])
	    F2CNAMES="F2CNOCHANGE"
      else
	    echo "Unable to determine the form of Fortran external names" 2>&1
	    echo "Make sure that the compiler $F77 can be run on this system" 2>&1
        echo "Assuming Fortran externals are lowercase with 1 trailing underscore." 2>&1
        F2CNAMES="F2CADD_"
      fi
    fi
  else
    AC_MSG_RESULT([NO TEST PERFORMED: using assigned value ($F2CNAMES)])
  fi
  AC_SUBST(F2CNAMES)
  ])dnl

dnl Get the format of Fortran INTEGERS.
dnl Uses: F77, FFLAGS, F2CNAMES, CC, CFLAGS
dnl Sets: F2CINT.
dnl CREDITS: motivated by the config scripts of ATLAS
dnl CREDITS: test code stolen from config scripts of ATLAS
dnl
AC_DEFUN(PAC_GET_F2CINT,[
  AC_MSG_CHECKING([for f2c integers])
  if test -z "$F2CINT" ; then
    AC_REQUIRE([PAC_GET_F2CNAMES])
    cat > confftest.f <<EOF
       program ff2cint
       integer iarr(8)
       iarr(1) = 1
       iarr(2) = -1
       iarr(3) = -1
       iarr(4) = -1
       iarr(5) = -1
       iarr(6) = -1
       iarr(7) = -1
       iarr(8) = -1
       call c2fint(iarr)
       stop
       end
EOF
    cat > confctest.c <<EOF
#if defined(F2CADD_) || defined(F2CADD__)
   #define c2fint c2fint_
#elif defined(F2CUPCASE)
   #define c2fint C2FINT
#endif
void c2fint(void *vp)
{
   int *ip=vp;
   long *lp=vp;
   short *sp=vp;

   if ( (sizeof(long) != sizeof(int)) && (*lp == 1) )
      printf("F77 INTEGER -> C long\n");
   else if (*ip == 1) printf("F77 INTEGER -> C int\n");
   else if (*sp == 1) printf("F77 INTEGER -> C short\n");
}
EOF
    echo "$F77 $FFLAGS -c confftest.f" >> config.log
    $F77 $FFLAGS -c confftest.f >> config.log 2>&1
    echo "$CC $CFLAGS -D$F2CNAMES -c confctest.c" >> config.log
    $CC $CFLAGS -D$F2CNAMES -c confctest.c >> config.log 2>&1
    echo "$F77 $FFLAGS -o confname confftest.o confctest.o" >> config.log
    $F77 $FFLAGS -o conffint confftest.o confctest.o >> config.log 2>&1
    if test ! -s confftest.o ; then
      echo "Unable to test Fortran compiler" 2>&1
      echo "(compiling a test program failed to produce an " 2>&1
      echo "object file)." 2>&1
      echo "Assuming Fortran integers are same as C integers." 2>&1
	  F2CINT="FINT2CINT"
    elif test ! -s confctest.o ; then
      echo "Unable to test C compiler"  2>&1
      echo "(compiling a test program failed to produce an " 2>&1
      echo "object file)." 2>&1
      echo "Assuming Fortran integers are same as C integers." 2>&1
	  F2CINT="FINT2CINT"
    elif test ! -s conffint ; then
      echo "Unable to test Fortran linker"  2>&1
      echo "(linking a test program failed to produce an " 2>&1
      echo "executable file)." 2>&1
      echo "Assuming Fortran integers are same as C integers." 2>&1
	  F2CINT="FINT2CINT"
    else
      F2CINT=`conffint`
      /bin/rm -f confftest.f confftest.o confctest.c confctest.o conffint
      nameform1=`echo $F2CINT | grep long`
      nameform2=`echo $F2CINT | grep int`
      nameform3=`echo $F2CINT | grep short`
      if test -n "$nameform1" ; then
        AC_MSG_RESULT([long])
        F2CINT="FINT2CLONG"
      elif test -n "$nameform2" ; then
        AC_MSG_RESULT([int])
        F2CINT="FINT2CINT"
      elif test -n "$nameform3" ; then
        AC_MSG_RESULT([short])
        F2CINT="FINT2CSHORT"
      else
	    echo "Unable to determine the correspondence of Fortran integers" 2>&1
	    echo "Make sure that the compiler $F77 can be run on this system" 2>&1
        echo "Assuming Fortran integers are same as C integers." 2>&1
	    F2CINT="FINT2CINT"
      fi
    fi
  else
    AC_MSG_RESULT([NO TEST PERFORMED: using assigned value ($F2CINT)])
  fi
  AC_SUBST(F2CINT)
  ])dnl

dnl Get the format of Fortran String arguments to C.
dnl Uses: F77, FFLAGS, F2CNAMES, CC, CFLAGS, F2CINT.
dnl Sets: F2CSTR
dnl CREDITS: motivated by the config scripts of ATLAS
dnl CREDITS: test code stolen from config scripts of ATLAS
dnl
AC_DEFUN(PAC_GET_F2CSTR,[
  AC_MSG_CHECKING([for f2c string conventions])
  if test -z "$F2CSTR" ; then
    AC_REQUIRE([PAC_GET_F2CINT])
    cat > confftest.f <<EOF
      program chartst
      external crout

      call crout('123', -1, '12345', -2)

      stop
      end
EOF
    changequote(,)
    cat > confctest.c <<EOF
#if defined(F2CADD_) || defined(F2CADD__)
   #define crout crout_
#elif defined(F2CUPCASE)
   #define crout CROUT
#endif

#if defined(FINT2CINT)
  #define F77_INTEGER int
#elif defined(FINT2CLONG)
  #define F77_INTEGER LONG
#elif defined(FINT2CSHORT)
  #define F77_INTEGER short
#endif

#ifdef SunStyle

void crout(char *str1, F77_INTEGER *n1, char *str2, F77_INTEGER *n2, 
           F77_INTEGER three, F77_INTEGER five)
{
   if ( (*n1 != -1) || (*n2 != -2) || (three != 3) || (five != 5) ) exit(-1);
   if (str1[0] != '1' || str1[1] != '2' || str1[2] != '3') exit(-1);
   if (str2[0] != '1' || str2[1] != '2' || str2[2] != '3' ||
       str2[3] != '4' || str2[4] != '5') exit(-1);
   printf("Success!\n");
}

#elif defined(CrayStyle)

#include <fortran.h>
void crout(_fcd str1, F77_INTEGER *n1, _fcd str2, F77_INTEGER *n2)
{
   if ( (*n1 != -1) || (*n2 != -2) ) exit(-1);
   if (*(_fcdtocp(str1)) != '1' || *(_fcdtocp(str2)) != '1' ) exit(-1);
   printf("Success!\n");
}

#elif defined(StructVal)

typedef struct {char *cp; F77_INTEGER len;} F77_CHAR;
void crout(F77_CHAR str1, F77_INTEGER *n1, F77_CHAR str2, F77_INTEGER *n2)
{
   if ( (*n1 != -1) || (*n2 != -2) || (str1.len != 3) || (str2.len != 5) )
      exit(-1);
   if (str1.cp[0] != '1' || str1.cp[1] != '2' || str1.cp[2] != '3') exit(-1);
   if (str2.cp[0] != '1' || str2.cp[1] != '2' || str2.cp[2] != '3' ||
       str2.cp[3] != '4' || str2.cp[4] != '5') exit(-1);
   printf("Success!\n");
}
#elif defined(StructPtr)
typedef struct {char *cp; F77_INTEGER len;} *F77_CHAR;
void crout(F77_CHAR str1, F77_INTEGER *n1, F77_CHAR str2, F77_INTEGER *n2)
{
   if ( (*n1 != -1) || (*n2 != -2) || (str1->len != 3) || (str2->len != 5) )
      exit(-1);
   if (str1->cp[0] != '1' || str1->cp[1] != '2' || str1->cp[2] != '3') exit(-1);
   if (str2->cp[0] != '1' || str2->cp[1] != '2' || str2->cp[2] != '3' ||
       str2->cp[3] != '4' || str2->cp[4] != '5') exit(-1);
   printf("Success!\n");
}
#endif
EOF
    changequote([,])
    echo "$F77 $FFLAGS -c confftest.f" >> config.log
    $F77 $FFLAGS -c confftest.f >> config.log 2>&1
    echo "$CC $CFLAGS -D$F2CNAMES  -D$F2CINT \
              -DSunStyle -c confctest.c" >> config.log
    $CC $CFLAGS -D$F2CNAMES  -D$F2CINT \
        -DSunStyle -c confctest.c >> config.log 2>&1
    echo "$F77 $FFLAGS -o conffstr confftest.o confctest.o" >> config.log
    $F77 $FFLAGS -o conffstr confftest.o confctest.o >> config.log 2>&1
    if test ! -s confftest.o ; then
      echo "Unable to test Fortran compiler" 2>&1
      echo "(compiling a test program failed to produce an " 2>&1
      echo "object file)." 2>&1
      AC_MSG_WARN([Setting Default to SunStyle])
      F2CSTR="F2CSTRSUNSTYLE"
    elif test ! -s confctest.o ; then
      echo "Unable to test C compiler"  2>&1
      echo "(compiling a test program failed to produce an " 2>&1
      echo "object file)." 2>&1
      AC_MSG_WARN([Setting Default to SunStyle])
      F2CSTR="F2CSTRSUNSTYLE"
    elif test ! -s conffstr ; then
      echo "Unable to test Fortran linker"  2>&1
      echo "(linking a test program failed to produce an " 2>&1
      echo "executable file)." 2>&1
      AC_MSG_WARN([Setting Default to SunStyle])
      F2CSTR="F2CSTRSUNSTYLE"
    else
      F2CSTR=`conffstr`
      nameform=`echo $F2CSTR | grep Success`
      if test -n "$nameform" ; then
        AC_MSG_RESULT([SunStlye])
        F2CSTR="F2CSTRSUNSTYLE"
      fi
    fi
    if test -z $F2CSTR ; then
      echo "$CC $CFLAGS -D$F2CNAMES  -D$F2CINT \
            -DCrayStyle -c confctest.c" >> config.log
      $CC $CFLAGS -D$F2CNAMES  -D$F2CINT \ 
          -DCrayStyle -c confctest.c >> config.log 2>&1
      echo "$F77 $FFLAGS -o conffstr confftest.o confctest.o" >> config.log
      $F77 $FFLAGS -o conffstr confftest.o confctest.o >> config.log 2>&1
      if test ! -s confftest.o ; then
        echo "Unable to test Fortran compiler" 2>&1
        echo "(compiling a test program failed to produce an " 2>&1
        echo "object file)." 2>&1
        AC_MSG_WARN([Setting Default to SunStyle])
        F2CSTR="F2CSTRSUNSTYLE"
      elif test ! -s confctest.o ; then
        echo "Unable to test C compiler"  2>&1
        echo "(compiling a test program failed to produce an " 2>&1
        echo "object file)." 2>&1
        AC_MSG_WARN([Setting Default to SunStyle])
        F2CSTR="F2CSTRSUNSTYLE"
      elif test ! -s conffstr ; then
        echo "Unable to test Fortran linker"  2>&1
        echo "(linking a test program failed to produce an " 2>&1
        echo "executable file)." 2>&1
        AC_MSG_WARN([Setting Default to SunStyle])
        F2CSTR="F2CSTRSUNSTYLE"
      else
        F2CSTR=`conffstr`
        nameform=`echo $F2CSTR | grep Success`
        if test -n "$nameform" ; then
          AC_MSG_RESULT([CrayStlye])
          F2CSTR="F2CSTRCRAYSTYLE"
        fi
      fi
    fi
    if test -z $F2CSTR ; then
      echo "$CC $CFLAGS -D$F2CNAMES  -D$F2CINT \
            -DStructPtr -c confctest.c" >> config.log
      $CC $CFLAGS -D$F2CNAMES  -D$F2CINT \
          -DStructPtr -c confctest.c >> config.log 2>&1
      echo "$F77 $FFLAGS -o conffstr confftest.o confctest.o" >> config.log
      $F77 $FFLAGS -o conffstr confftest.o confctest.o >> config.log 2>&1
      if test ! -s confftest.o ; then
        echo "Unable to test Fortran compiler" 2>&1
        echo "(compiling a test program failed to produce an " 2>&1
        echo "object file)." 2>&1
        AC_MSG_WARN([Setting Default to SunStyle])
        F2CSTR="F2CSTRSUNSTYLE"
      elif test ! -s confctest.o ; then
        echo "Unable to test C compiler"  2>&1
        echo "(compiling a test program failed to produce an " 2>&1
        echo "object file)." 2>&1
        AC_MSG_WARN([Setting Default to SunStyle])
        F2CSTR="F2CSTRSUNSTYLE"
      elif test ! -s conffstr ; then
        echo "Unable to test Fortran linker"  2>&1
        echo "(linking a test program failed to produce an " 2>&1
        echo "executable file)." 2>&1
        AC_MSG_WARN([Setting Default to SunStyle])
        F2CSTR="F2CSTRSUNSTYLE"
      else
        F2CSTR=`conffstr`
        nameform=`echo $F2CSTR | grep Success`
        if test -n "$nameform" ; then
          AC_MSG_RESULT([StructPtr])
          F2CSTR="F2CSTRSTRUCTPTR"
        fi
      fi
    fi
    if test -z $F2CSTR ; then
      echo "$CC $CFLAGS -D$F2CNAMES  -D$F2CINT \
            -DStructVal -c confctest.c" >> config.log
      $CC $CFLAGS -D$F2CNAMES  -D$F2CINT \
          -DStructVal -c confctest.c >> config.log 2>&1
      echo "$F77 $FFLAGS -o conffstr confftest.o confctest.o" >> config.log
        $F77 $FFLAGS -o conffstr confftest.o confctest.o >> config.log 2>&1
      if test ! -s confftest.o ; then
        echo "Unable to test Fortran compiler" 2>&1
        echo "(compiling a test program failed to produce an " 2>&1
        echo "object file)." 2>&1
        AC_MSG_WARN([Setting Default to SunStyle])
        F2CSTR="F2CSTRSUNSTYLE"
      elif test ! -s confctest.o ; then
        echo "Unable to test C compiler"  2>&1
        echo "(compiling a test program failed to produce an " 2>&1
        echo "object file)." 2>&1
        AC_MSG_WARN([Setting Default to SunStyle])
        F2CSTR="F2CSTRSUNSTYLE"
      elif test ! -s conffstr ; then
        echo "Unable to test Fortran linker"  2>&1
        echo "(linking a test program failed to produce an " 2>&1
        echo "executable file)." 2>&1
        AC_MSG_WARN([Setting Default to SunStyle])
        F2CSTR="F2CSTRSUNSTYLE"
      else
        F2CSTR=`conffstr`
        nameform=`echo $F2CSTR | grep Success`
        if test -n "$nameform" ; then
          AC_MSG_RESULT([StructVal])
          F2CSTR="F2CSTRSTRUCTVAL"
        fi
      fi
    fi
    /bin/rm -f confftest.f confftest.o confctest.c confctest.o conffstr
    if test -z $F2CSTR ; then
      AC_MSG_WARN([Unable to determine f2c String Convention])
      AC_MSG_WARN([Setting Default to SunStyle])
      F2CSTR="F2CSTRSUNSTYLE"
    fi
  else
    AC_MSG_RESULT([NO TEST PERFORMED: using assigned value ($F2CSTRSUNSTYLE)])
  fi
  AC_SUBST(F2CSTR)
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

dnl AC_TRY_COMPILE(INCLUDES, FUNCTION-BODY,
dnl             [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl copied from autoconf macro included with distribution
dnl re-edited to fix a bug when using AC_LANG_FORTRAN77
AC_DEFUN(PAC_TRY_COMPILE,
[cat > conftest.$ac_ext <<EOF
ifelse(AC_LANG, [FORTRAN77],
[      program main
[$2]
      end
],
[dnl This sometimes fails to find confdefs.h, for some reason.
dnl [#]line __oline__ "[$]0"
[#]line __oline__ "configure"
#include "confdefs.h"
[$1]
int main() {
[$2]
; return 0; }
])EOF
if AC_TRY_EVAL(ac_compile); then
  ifelse([$3], , :, [rm -rf conftest*
  $3])
else
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
ifelse([$4], , , [  rm -rf conftest*
  $4
])dnl
fi
rm -f conftest*])

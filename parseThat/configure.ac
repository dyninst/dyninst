# Process this file with autoconf to produce a configure script.
AC_INIT([parseThat], [1.0])
AC_CONFIG_SRCDIR([src/config.C])
AC_PREFIX_DEFAULT([.])

if test -n ${DYNINST_ROOT}; then
   # Add search paths to CPPFLAGS and LDFLAGS.
   #AC_SUBST([CPPFLAGS], ["-I${DYNINST_ROOT}/core -I${DYNINST_ROOT}/core/dyninstAPI/h -I${DYNINST_ROOT}/core/symtabAPI/h -I${DYNINST_ROOT}/core/dynutil/h ${CPPFLAGS}"])
   AC_SUBST([CPPFLAGS], ["-I${DYNINST_ROOT}/core -I${DYNINST_ROOT}/core/dyninstAPI/h -I${DYNINST_ROOT}/core/symtabAPI/h -I${DYNINST_ROOT}/core/instructionAPI/h -I${DYNINST_ROOT}/core/common/h -I${DYNINST_ROOT}/core/dynutil/h -I${DYNINST_ROOT}/core/external ${CPPFLAGS}"])
   AC_SUBST([LDFLAGS],  ["-L${DYNINST_ROOT}/${PLATFORM}/lib ${LDFLAGS}"])
fi

# Generate host string.
AC_CANONICAL_HOST()

# Build all configure tests with C++ compiler.
AC_LANG([C++])

# sha1.C requres these system files and endianess to be tested.
AC_CHECK_HEADERS([inttypes.h], [], [], [// Do not use default includes])
AC_CHECK_HEADERS([stdint.h], [], [], [// Do not use default includes])
AC_C_BIGENDIAN()

# Search for Dyninst in all the right places.
AC_CHECK_HEADER([BPatch.h], [],
		[AC_MSG_ERROR([Cannot find DyninstAPI library include files.])],
		[// Do not use default includes])

# Check if Dyninst is version 5.0+
AC_CHECK_HEADERS([BPatch_process.h],
		 [AC_SUBST([DYNINST_VER], [5])],
		 [AC_SUBST([DYNINST_VER], [4])],
		 [// Do not use default includes])

# Find appropriate install program
AC_PROG_INSTALL()

AC_CONFIG_FILES([Makefile])
AC_OUTPUT()

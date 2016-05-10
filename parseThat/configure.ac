# Process this file with autoconf to produce a configure script.
AC_INIT([parseThat],[1.0])
AC_CONFIG_SRCDIR([src/config.C])
AC_CONFIG_AUX_DIR([.])
AC_LANG(C++)


AC_SEARCH_LIBS(dlopen, dl)

AC_ARG_WITH(dyninst,
            [AS_HELP_STRING([--with-dyninst],[Base directory to look for dyninst headers and libraries])],
            [DYNINST_LIB="${withval}/${PLATFORM}/lib"
             DYNINST_INC="${withval}/include"],)

AC_ARG_WITH(dyninst-lib,
            [AS_HELP_STRING([--with-dyninst-lib],[Directory to look for dyninst libraries])],
            [DYNINST_LIB="${withval}"],)

AC_ARG_WITH(dyninst-include,
            [AS_HELP_STRING([--with-dyninst-include],[Directory to look for dyninst headers])],
            [DYNINST_INC="${withval}"],)

AC_ARG_WITH(dwarf-lib,
            [AS_HELP_STRING([--with-dwarf-include],[Directory to look for libdwarf])],
            [DWARF_LIB="${withval}"],)

AC_ARG_WITH(elf-lib,
            [AS_HELP_STRING([--with-elf-include],[Directory to look for libelf])],
            [ELF_LIB="${withval}"],)

AC_ARG_WITH(liberty-lib,
            [AS_HELP_STRING([--with-liberty-lib],[Link string to use for libiberty])],
            [LIBERTY_LIB="${withval}"],)

if test -n ${DYNINST_ROOT}; then
   # Add search paths to CPPFLAGS and LDFLAGS.
   if test -z ${DYNINST_LIB}; then
      DYNINST_LIB="${DYNINST_ROOT}/lib"
   fi
   if test -z ${DYNINST_INC}; then
      DYNINST_INC="${DYNINST_ROOT}/include"
   fi
fi

AC_SUBST([LDFLAGS], ["-L${DYNINST_LIB} ${LDFLAGS}"])
AC_SUBST([DWARFFLAGS], ["${DWARF_LIB}"])
AC_SUBST([ELFFLAGS], ["${ELF_LIB}"])
AC_SUBST([LIBERTYFLAGS], ["${LIBERTY_LIB}"])
AC_SUBST([CPPFLAGS], ["-I${DYNINST_INC} -D_GLIBCXX_USE_CXX11_ABI=0 ${CPPFLAGS}"])

# Generate host string.
AC_CANONICAL_HOST

# Build all configure tests with C++ compiler.
AC_LANG([C++])

# sha1.C requres these system files and endianess to be tested.
AC_CHECK_HEADERS([inttypes.h], [], [], [// Do not use default includes])
AC_CHECK_HEADERS([stdint.h], [], [], [// Do not use default includes])
AC_C_BIGENDIAN

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
AC_PROG_INSTALL

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

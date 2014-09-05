# - Find Iberty
# This module finds libiberty.
#
# It sets the following variables:
#  IBERTY_LIBRARY     - The libiberty library to link against.

# For Debian <= wheezy, use libiberty_pic.a from binutils-dev
# For Debian >= jessie, use libiberty.a from libiberty-dev
# For all RHEL/Fedora, use libiberty.a from binutils-devel
FIND_LIBRARY( IBERTY_LIBRARY NAMES iberty_pic iberty )

IF (IBERTY_LIBRARY)

   # show which libiberty was found only if not quiet
   MESSAGE( STATUS "Found libiberty: ${IBERTY_LIBRARY}")

   SET(IBERTY_FOUND TRUE)

ELSE (IBERTY_LIBRARY)

   IF ( IBERTY_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find libiberty. Try to install binutil-devel?")
   ELSE()
      MESSAGE(STATUS "Could not find libiberty; downloading binutils and building PIC libiberty.")
   ENDIF (IBERTY_FIND_REQUIRED)

ENDIF (IBERTY_LIBRARY)

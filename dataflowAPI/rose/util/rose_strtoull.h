#ifndef ROSE_strtoull_H
#define ROSE_strtoull_H

#include <inttypes.h>

/** Convert a string to an unsigned long integer.
 *
 *  This function is the same as the system strtoull() except it also allows @p base to be two, in which case it parses a
 *  binary literal consisting of '0' and '1' bits.  If @p base is zero and the first non-whitespace characters of the string
 *  are '0b' then bits follow. */
uint64_t rose_strtoull(const char *nptr, char **endptr, int base);

#endif

    

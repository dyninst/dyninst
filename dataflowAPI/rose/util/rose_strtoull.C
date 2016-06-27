#include "rose_strtoull.h"
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#ifdef _MSC_VER
#define strtoull _strtoui64
#endif

uint64_t
rose_strtoull(const char *nptr, char **endptr, int base)
{
    if (0==base && NULL!=nptr) {
        // Look for Perl regular expression "^\s*[-+]0b[01]{1,64}" and if found return the (possibly negated) binary value.
        bool negative = false;
        const char *s = nptr;
        while (isspace(*s))
            ++s;
        if ('+'==*s) {
            ++s;
        } else if ('-'==*s) {
            negative = true;
            ++s;
        }
        if (0==strncmp(s, "0b", 2) && ('0'==s[2] || '1'==s[2])) {
            errno = 0;
            unsigned long long retval = strtoull(s+2, endptr, 2);
            if (0==errno && negative)
                retval = -retval;
            return retval;
        }
    }

    return strtoull(nptr, endptr, base);
}

            


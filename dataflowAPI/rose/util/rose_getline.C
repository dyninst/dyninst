#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "rose_getline.h"

ssize_t
rose_getline (char **lineptr, size_t *n, FILE *stream)
{
    assert(lineptr);
    assert(n);
    assert(stream);
    assert((0==n && NULL==lineptr) || (0!=n && NULL!=lineptr));

    size_t nread=0;
    while (1) {
        if (nread >= *n) {
            *n = std::max((size_t)256, *n*2);
            *lineptr = (char*)realloc(*lineptr, *n);
            if (!*lineptr) {
                *n = 0;
                return -1;
            }
        }

        if (nread>0 && '\n'==(*lineptr)[nread-1]) {
            (*lineptr)[nread] = '\0';
            return nread;
        }

        int c = fgetc(stream);
        if (c<0) {
            (*lineptr)[nread] = '\0';
            return nread>0 ? nread : -1;
        } else {
            (*lineptr)[nread++] = (char)c;
        }
    }
}

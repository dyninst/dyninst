#ifndef __SHA1_H__
#define __SHA1_H__

#include <stdio.h>

#define SHA1_DIGEST_LEN 20
#define SHA1_STRING_LEN (SHA1_DIGEST_LEN * 2 + 1)

char *sha1_file(const char *filename, char *result_ptr = NULL);

#endif

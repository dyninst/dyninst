#ifndef _SHA1_H_
#define _SHA1_H_
//  defines for sha1.C, checksum string length
#define SHA1_DIGEST_LEN 20
#define SHA1_STRING_LEN (SHA1_DIGEST_LEN * 2 + 1)
char *sha1_file(const char *filename, char *result_ptr = NULL);
#endif

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>

char *chomp(char *);
char *fgets_static(FILE *);
char *sprintf_static(const char *, ...);
char *strcat_static(char *, char *);
char *encodeStr(const char *);
char *decodeStr(const char *, char ** = NULL);

#endif

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#define myprint(...) do { \
    fprintf(stdout, __VA_ARGS__); \
    fflush(stdout); \
} while(0)

#define mydebug(...) do { \
  if (getenv("DEBUG")) {   \
  		char* nodir = basename((char*)__FILE__);				 \
      fprintf(stderr, "%s [%d]: ", nodir, __LINE__); \
      fprintf(stderr, __VA_ARGS__); \
      fflush(stderr); \
    } \
} while(0)

#define myperror(...) do {\
	char* nodir = basename((char*)__FILE__);							\
    char* err = strerror(errno); \
    fprintf(stderr, "ERROR in %s [%d]: ", nodir, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "--%s", err); \
  fprintf(stderr, "\n"); \
  ::exit(0);						 \
	} while(0)


#endif


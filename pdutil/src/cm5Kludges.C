
#include "util/h/headers.h"

typedef int (*intKludge)();

void * P_memcpy (void *A1, const void *A2, size_t SIZE) {
  return (memcpy(A1, A2, SIZE));
}


#if !defined(__pdr_mem_h)
#define __pdr_mem_h 


#include "mrnet/src/pdr.h"


bool_t   pdrmem_putchar(PDR *, char *);
bool_t   pdrmem_getchar(PDR *, char *);

bool_t   pdrmem_putint16(PDR *, int16_t *);
bool_t   pdrmem_getint16(PDR *, int16_t *);
bool_t   pdrmem_getint16_swap(PDR *, int16_t *);

bool_t   pdrmem_putint32(PDR *, int32_t *);
bool_t   pdrmem_getint32(PDR *, int32_t *);
bool_t   pdrmem_getint32_swap(PDR *, int32_t *);

bool_t   pdrmem_putint64(PDR *, int64_t *);
bool_t   pdrmem_getint64(PDR *, int64_t *);
bool_t   pdrmem_getint64_swap(PDR *, int64_t *);

bool_t   pdrmem_putfloat(PDR *, float *);
bool_t   pdrmem_getfloat(PDR *, float *);
bool_t   pdrmem_getfloat_swap(PDR *, float *);

bool_t   pdrmem_putdouble(PDR *, double *);
bool_t   pdrmem_getdouble(PDR *, double *);
bool_t   pdrmem_getdouble_swap(PDR *, double *);

bool_t   pdrmem_getbytes(PDR *, char *, uint32_t);
bool_t   pdrmem_putbytes(PDR *, char *, uint32_t);

bool_t   pdrmem_setpos(PDR *, uint32_t);
uint32_t pdrmem_getpos(PDR *);
int32_t* pdrmem_inline(PDR *, int32_t);
void     pdrmem_destroy(PDR *);

#endif /* __pdr_mem_h */

#include "mrnet/src/byte_order.h"
#include "mrnet/src/utils.h"
#include "mrnet/src/pdr_mem.h"
#include "mrnet/src/config.h"

static struct pdr_ops pdrmem_ops = {
  pdrmem_putchar,
  pdrmem_getchar,
  pdrmem_putint16,
  pdrmem_getint16,
  pdrmem_putint32,
  pdrmem_getint32,
  pdrmem_putint64,
  pdrmem_getint64,
  pdrmem_putfloat,
  pdrmem_getfloat,
  pdrmem_putdouble,
  pdrmem_getdouble,
  pdrmem_putbytes,
  pdrmem_getbytes,
  pdrmem_setpos,
  pdrmem_getpos,
  pdrmem_inline,
  pdrmem_destroy
};

static struct pdr_ops pdrmem_ops_swap = {
  pdrmem_putchar,
  pdrmem_getchar,
  pdrmem_putint16,
  pdrmem_getint16_swap,
  pdrmem_putint32,
  pdrmem_getint32_swap,
  pdrmem_putint64,
  pdrmem_getint64_swap,
  pdrmem_putfloat,
  pdrmem_getfloat_swap,
  pdrmem_putdouble,
  pdrmem_getdouble_swap,
  pdrmem_putbytes,
  pdrmem_getbytes,
  pdrmem_setpos,
  pdrmem_getpos,
  pdrmem_inline,
  pdrmem_destroy
};
/*
 * The procedure pdrmem_create initializes a stream descriptor for a memory buffer. 
 */
void pdrmem_create(PDR *pdrs, char * addr, uint32_t size, enum pdr_op op)
{
#if defined(WORDS_BIGENDIAN)
  static char local_bo = 1;  /* Big Endian */
#else
  static char local_bo = 0;  /* Little Endian */
#endif
  char remote_bo;

  pdrs->cur = pdrs->base = addr;
  pdrs->space = size;
  pdrs->p_ops = &pdrmem_ops;
  pdrs->p_op = op;

  if(pdrs->p_op == PDR_ENCODE){
    if(!pdr_char(pdrs, (char *)&local_bo)){
      pdrs->space = 0;
    }
  }
  else if(pdrs->p_op == PDR_DECODE){
    if(!pdr_char(pdrs, (char *)&remote_bo)){
      pdrs->space = 0;
    }
    if(remote_bo != local_bo){
      mc_printf(MCFL, stderr, "Remote_bo:%d != local_bo:%d -- swapping\n",
                 remote_bo, local_bo);
      pdrs->p_ops = &pdrmem_ops_swap;
      return;
    }
    mc_printf(MCFL, stderr, "Remote_bo == local_bo -- not swapping\n");
  }
}

/*
 *  pdrmem_xxxchar(): Procedures for putting/getting 1 byte CHARS.
 */
bool_t pdrmem_putchar(PDR *pdrs, char *p)
{
  if((pdrs->space -= SIZEOF_CHAR) < 0){
    return FALSE;
  }
  *((char *)(pdrs->cur)) = *p;
  pdrs->cur += SIZEOF_CHAR;
  return TRUE;
}
bool_t pdrmem_getchar(PDR *pdrs, char *p)
{
  if((pdrs->space -= SIZEOF_CHAR) < 0){
    return FALSE;
  }
  *p = *((char *)(pdrs->cur));
  pdrs->cur += SIZEOF_CHAR;
  return TRUE;
}

/*
 *  pdrmem_xxxint16(): Procedures for puting/getting 16 bit INTS.
 */
bool_t pdrmem_putint16(PDR *pdrs, int16_t *p)
{
  if((pdrs->space -= SIZEOF_INT16) < 0){
    return FALSE;
  }

  assert(pdrs->p_op == PDR_ENCODE);
  *((int16_t *)(pdrs->cur)) = *p;
  pdrs->cur += SIZEOF_INT16;

  return TRUE;
}

bool_t pdrmem_getint16(PDR *pdrs, int16_t *p)
{
  if((pdrs->space -= SIZEOF_INT16) < 0){
    return FALSE;
  }
  assert(pdrs->p_op == PDR_DECODE);
  *p = *((int16_t *)(pdrs->cur));
  pdrs->cur += SIZEOF_INT16;
  return TRUE;
}

bool_t pdrmem_getint16_swap(PDR *pdrs, int16_t *p)
{
  if((pdrs->space -= SIZEOF_INT16) < 0){
    return FALSE;
  }
  assert(pdrs->p_op == PDR_DECODE);
  swap_int16(p,  pdrs->cur);
  pdrs->cur += SIZEOF_INT16;
  return TRUE;
}

/*
 *  pdrmem_xxxint32(): Procedures for puting/getting 32 bit INTS.
 */
bool_t pdrmem_putint32(PDR *pdrs, int32_t *p)
{
  if((pdrs->space -= SIZEOF_INT32) < 0){
    return FALSE;
  }

  assert(pdrs->p_op == PDR_ENCODE);
  *((int32_t *)(pdrs->cur)) = *p;
  pdrs->cur += SIZEOF_INT32;

  return TRUE;
}

bool_t pdrmem_getint32(PDR *pdrs, int32_t *p)
{
  if((pdrs->space -= SIZEOF_INT32) < 0){
    return FALSE;
  }
  assert(pdrs->p_op == PDR_DECODE);
  *p = *((int32_t *)(pdrs->cur));
  pdrs->cur += SIZEOF_INT32;
  return TRUE;
}

bool_t pdrmem_getint32_swap(PDR *pdrs, int32_t *p)
{
  if((pdrs->space -= SIZEOF_INT32) < 0){
    return FALSE;
  }
  assert(pdrs->p_op == PDR_DECODE);
  swap_int32(p,  pdrs->cur);
  pdrs->cur += SIZEOF_INT32;
  return TRUE;
}

/*
 *  pdrmem_xxxint64(): Procedures for puting/getting 64 bit INTS.
 */
bool_t pdrmem_putint64(PDR *pdrs, int64_t *p)
{
  if((pdrs->space -= SIZEOF_INT64) < 0){
    return FALSE;
  }

  assert(pdrs->p_op == PDR_ENCODE);
  *((int64_t *)(pdrs->cur)) = *p;
  pdrs->cur += SIZEOF_INT64;

  return TRUE;
}

bool_t pdrmem_getint64(PDR *pdrs, int64_t *p)
{
  if((pdrs->space -= SIZEOF_INT64) < 0){
    return FALSE;
  }
  assert(pdrs->p_op == PDR_DECODE);
  *p = *((int64_t *)(pdrs->cur));
  pdrs->cur += SIZEOF_INT64;
  return TRUE;
}

bool_t pdrmem_getint64_swap(PDR *pdrs, int64_t *p)
{
  if((pdrs->space -= SIZEOF_INT64) < 0){
    return FALSE;
  }
  assert(pdrs->p_op == PDR_DECODE);
  swap_int64(p,  pdrs->cur);
  pdrs->cur += SIZEOF_INT64;
  return TRUE;
}

/*
 *  pdrmem_xxxfloat(): Procedures for puting/getting 32 bit FLOATS.
 */
bool_t pdrmem_putfloat(PDR *pdrs, float *p)
{
  if((pdrs->space -= SIZEOF_FLOAT) < 0){
    return FALSE;
  }

  assert(pdrs->p_op == PDR_ENCODE);
  *((float *)(pdrs->cur)) = *p;
  pdrs->cur += SIZEOF_FLOAT;

  return TRUE;
}

bool_t pdrmem_getfloat(PDR *pdrs, float *p)
{
  if((pdrs->space -= SIZEOF_FLOAT) < 0){
    return FALSE;
  }
  assert(pdrs->p_op == PDR_DECODE);
  *p = *((float *)(pdrs->cur));
  pdrs->cur += SIZEOF_FLOAT;
  return TRUE;
}

bool_t pdrmem_getfloat_swap(PDR *pdrs, float *p)
{
  if((pdrs->space -= SIZEOF_FLOAT) < 0){
    return FALSE;
  }
  assert(pdrs->p_op == PDR_DECODE);
  swap_float(p,  pdrs->cur);
  pdrs->cur += SIZEOF_FLOAT;
  return TRUE;
}

/*
 *  pdrmem_xxxdouble(): Procedures for puting/getting 16 bit INTS.
 */
bool_t pdrmem_putdouble(PDR *pdrs, double *p)
{
  if((pdrs->space -= SIZEOF_DOUBLE) < 0){
    return FALSE;
  }

  assert(pdrs->p_op == PDR_ENCODE);
  *((double *)(pdrs->cur)) = *p;
  pdrs->cur += SIZEOF_DOUBLE;

  return TRUE;
}

bool_t pdrmem_getdouble(PDR *pdrs, double *p)
{
  if((pdrs->space -= SIZEOF_DOUBLE) < 0){
    return FALSE;
  }
  assert(pdrs->p_op == PDR_DECODE);
  *p = *((double *)(pdrs->cur));
  pdrs->cur += SIZEOF_DOUBLE;
  return TRUE;
}

bool_t pdrmem_getdouble_swap(PDR *pdrs, double *p)
{
  if((pdrs->space -= SIZEOF_DOUBLE) < 0){
    return FALSE;
  }
  assert(pdrs->p_op == PDR_DECODE);
  swap_double(p,  pdrs->cur);
  pdrs->cur += SIZEOF_DOUBLE;
  return TRUE;
}

bool_t pdrmem_getbytes(PDR *pdrs, char * addr,  uint32_t len)
{
  if ((pdrs->space -= len) < 0)
    return (FALSE);
  bcopy(pdrs->cur, addr, len);
  pdrs->cur += len;
  return (TRUE);
}

bool_t pdrmem_putbytes(PDR *pdrs, char * addr,  uint32_t len)
{
  if ((pdrs->space -= len) < 0)
    return (FALSE);
  bcopy(addr, pdrs->cur, len);
  pdrs->cur += len;
  return (TRUE);
}

uint32_t pdrmem_getpos( PDR *pdrs)
{
  return ((uint32_t)pdrs->cur - (uint32_t)pdrs->base);
}

bool_t pdrmem_setpos( PDR *pdrs, uint32_t pos)
{
  char * newaddr = pdrs->base + pos;
  char * lastaddr = pdrs->cur + pdrs->space;

  if ((int32_t)newaddr > (int32_t)lastaddr)
    return (FALSE);

  pdrs->cur = newaddr;
  pdrs->space = (int32_t)lastaddr - (int32_t)newaddr;
  return (TRUE);
}

int32_t * pdrmem_inline(PDR *pdrs, int32_t len)
{
  int32_t *buf = 0;

  if (pdrs->space >= len) {
    pdrs->space -= len;
    buf = (int32_t *) pdrs->cur;
    pdrs->cur += len;
  }
  return (buf);
}

void pdrmem_destroy(PDR *pdrs)
{
}

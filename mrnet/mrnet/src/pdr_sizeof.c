/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <stdlib.h>
#include <stdio.h>


#include "mrnet/src/pdr.h"
#include "mrnet/src/pdr_mem.h"
#include "src/config.h"

static bool_t _putchar(PDR *pdrs, char *)
{
    pdrs->space += SIZEOF_CHAR;
    return TRUE;
}
static bool_t _getchar(PDR *, char *)
{
    return FALSE;
}

static bool_t _putint16(PDR *pdrs, int16_t *)
{
    pdrs->space += SIZEOF_INT16;
    return TRUE;
}
static bool_t _getint16(PDR *, int16_t *)
{
    return FALSE;
}

static bool_t _putint32(PDR *pdrs, int32_t *)
{
    pdrs->space += SIZEOF_INT32;
    return TRUE;
}
static bool_t _getint32(PDR *, int32_t *)
{
    return FALSE;
}

static bool_t _putint64(PDR *pdrs, int64_t *)
{
    pdrs->space += SIZEOF_INT64;
    return TRUE;
}
static bool_t _getint64(PDR *, int64_t *)
{
    return FALSE;
}

static bool_t _putfloat(PDR *pdrs, float *)
{
    pdrs->space += SIZEOF_FLOAT;
    return TRUE;
}
static bool_t _getfloat(PDR *, float *)
{
    return FALSE;
}

static bool_t _putdouble(PDR *pdrs, double *)
{
    pdrs->space += SIZEOF_DOUBLE;
    return TRUE;
}
static bool_t _getdouble(PDR *, double *)
{
    return FALSE;
}

static bool_t _putbytes(PDR *pdrs, char *, uint32_t len)
{
    pdrs->space += SIZEOF_CHAR*len;
    return TRUE;
}
static bool_t _getbytes(PDR *, char *, uint32_t len)
{
    return FALSE;
}

static uint32_t _getpos(PDR *pdrs)
{
    return pdrs->space;
}

static bool_t _setpos(PDR *, uint32_t)
{
    return FALSE;
}

static void _destroy(PDR *pdrs)
{
    pdrs->space = 0;
    pdrs->cur = 0;
    if (pdrs->base){
        free (pdrs->base);
        pdrs->base = NULL;
    }
    return;
}


static int32_t * _inline (PDR *pdrs, int len)
{
    if (len == 0 || pdrs->p_op != XDR_ENCODE){
        return NULL;
    }

    if (len < (int) (long int) pdrs->base){
        /* cur was already allocated */
        pdrs->space += len;
        return (int32_t *) pdrs->cur;
    }
    else {
        /* Free the earlier space and allocate new area */
        if (pdrs->cur)
            free (pdrs->cur);
        if ((pdrs->cur = (char *) malloc (len)) == NULL) {
            pdrs->base = 0;
            return NULL;
        }
        pdrs->base = (char *) (long) len;
        pdrs->space += len;
        return (int32_t *) pdrs->cur;
    }
}

static struct pdr_ops _ops = {
    _putchar,
    _getchar,
    _putint16,
    _getint16,
    _putint32,
    _getint32,
    _putint64,
    _getint64,
    _putfloat,
    _getfloat,
    _putdouble,
    _getdouble,
    _putbytes,
    _getbytes,
    _setpos,
    _getpos,
    _inline,
    _destroy
};

uint32_t pdr_sizeof(pdrproc_t func, void *data)
{
    PDR pdrs;
    bool_t stat;

    pdrs.p_op = XDR_ENCODE;
    pdrs.p_ops = &_ops;
    pdrs.space = 1;  /* 1-byte byte ordering entity */
    pdrs.cur = (caddr_t) NULL;
    pdrs.base = (caddr_t) 0;

    stat = func (&pdrs, data);
    if (pdrs.cur)
        free (pdrs.cur);

    return stat == TRUE ? pdrs.space : 0;
}

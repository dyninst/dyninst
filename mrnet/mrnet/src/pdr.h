#if !defined(__PDR_HEADER__)
#define __PDR_HEADER__


#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#include "mrnet/src/Types.h"


enum pdr_op {
  PDR_FREE=0,
  PDR_ENCODE,
  PDR_DECODE
};

/*
 * The PDR handle.
 * Contains operation which is being applied to the stream,
 * an operations vector for the paticular implementation (e.g. see pdr_mem.c),
 * and two private fields for the use of the particular impelementation.
 */
typedef struct PDR PDR;
struct pdr_ops {
  bool_t  (*putchar)(PDR *pdrs, char *cp);
  bool_t  (*getchar)(PDR *pdrs, char *cp);
  bool_t  (*putint16)(PDR *pdrs, int16_t *ip);
  bool_t  (*getint16)(PDR *pdrs, int16_t *ip);
  bool_t  (*putint32)(PDR *pdrs, int32_t *ip);
  bool_t  (*getint32)(PDR *pdrs, int32_t *ip);
  bool_t  (*putint64)(PDR *pdrs, int64_t *ip);
  bool_t  (*getint64)(PDR *pdrs, int64_t *ip);
  bool_t  (*putfloat)(PDR *pdrs, float *fp);
  bool_t  (*getfloat)(PDR *pdrs, float *fp);
  bool_t  (*putdouble)(PDR *pdrs, double *dp);
  bool_t  (*getdouble)(PDR *pdrs, double *dp);
  bool_t  (*putbytes)(PDR *pdrs, char *, uint32_t);
  bool_t  (*getbytes)(PDR *pdrs, char *, uint32_t);
  bool_t  (*setpos)(PDR *pdrs, uint32_t ip);
  uint32_t (*getpos)(PDR *pdrs);
  int32_t * (*pinline)(PDR *pdrs, int32_t ip);
  void    (*destroy)(PDR *pdrs);
};

struct PDR {
  enum pdr_op p_op;       /* operation; */
  struct pdr_ops * p_ops; 

  char *        cur;      /* pointer to private data */
  char *        base;     /* private used for position info */
  int32_t        space;   /* extra private word */
};

/*
 * A pdrproc_t exists for each data type which is to be encoded or decoded.
 *
 * The second argument to the pdrproc_t is a pointer to an opaque pointer.
 * The opaque pointer generally points to a structure of the data type
 * to be decoded.  If this pointer is 0, then the type routines should
 * allocate dynamic storage of the appropriate size and return it.
 * bool_t       (*pdrproc_t)(PDR *, caddr_t *);
 */
typedef bool_t (*pdrproc_t)(PDR *, void *, ...);
#define NULL_pdrproc_t ((pdrproc_t)0)

/*
 * These are the "generic" pdr routines.
 */
extern bool_t   pdr_void(PDR *pdrs,  char * addr);
extern bool_t   pdr_char(PDR *pdrs,  char * addr);
extern bool_t   pdr_uchar(PDR *pdrs,  uchar_t * addr);
extern bool_t   pdr_int16(PDR *pdrs, int16_t *ip);
extern bool_t   pdr_uint16(PDR *pdrs, uint16_t *ip);
extern bool_t   pdr_int32(PDR *pdrs, int32_t *ip);
extern bool_t   pdr_uint32(PDR *pdrs, uint32_t *ip);
extern bool_t   pdr_int64(PDR *pdrs, int64_t *ip);
extern bool_t   pdr_uint64(PDR *pdrs, uint64_t *ip);
extern bool_t   pdr_float(PDR *pdrs, float *ip);
extern bool_t   pdr_double(PDR *pdrs, double *ip);

extern bool_t   pdr_bool(PDR *pdrs, bool_t *bp);
extern bool_t   pdr_enum(PDR *pdrs, enum_t *bp);

//extern bool_t   pdr_union();
extern bool_t   pdr_opaque(PDR *pdrs, char * cp, uint32_t cnt);
extern bool_t   pdr_bytes(PDR *pdrs, char **cpp, uint32_t *sizep,
                          uint32_t maxsize);
extern bool_t   pdr_string(PDR *pdrs, char **cpp, uint32_t maxsize);
extern bool_t   pdr_wrapstring(PDR *pdrs, char **cpp);
extern bool_t   pdr_reference(PDR *pdrs, char * *pp, uint32_t size,
                          pdrproc_t proc);
extern bool_t   pdr_pointer(PDR *pdrs, char **objpp, uint32_t obj_size,
                          pdrproc_t pdr_obj);

extern bool_t   pdr_array(PDR *pdrs, char **addrp, uint32_t *sizep,
                          uint32_t maxsize, uint32_t elsize, pdrproc_t elproc);
extern bool_t   pdr_vector(PDR *pdrs, char *basep, uint32_t nelem,
                          uint32_t elemsize, pdrproc_t pdr_elem);


/*
 * These are the public routines for the various implementations of
 * pdr streams.
 */
extern void   pdrmem_create(PDR *pdrs, char * addr, uint32_t size,
                            enum pdr_op op);          /* PDR using memory buffers */
//extern void   pdrstdio_create();        /* PDR using stdio library */
//extern void   pdrrec_create();          /* PDR pseudo records for tcp */
//extern bool_t pdrrec_endofrecord();     /* make end of pdr record */
//extern bool_t pdrrec_skiprecord();      /* move to beginning of next record */
//extern bool_t pdrrec_eof();             /* true if no more input */

extern void     pdr_free (pdrproc_t proc, char *objp);

uint32_t pdr_getpos(PDR *pdrs);
bool_t pdr_setpos(PDR *pdrs, uint32_t pos);
int32_t pdr_inline(PDR *pdrs, int32_t pos);

extern uint32_t pdr_sizeof (pdrproc_t, void *);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

/****************************/
/* XDR compatibility macros */
/****************************/

#define xdr_op pdr_op
#define XDR_ENCODE PDR_ENCODE
#define XDR_DECODE PDR_DECODE
#define XDR_FREE PDR_FREE

#define XDR PDR
#define x_op p_op

#define xdrproc_t pdrproc_t
#if !defined(NULL_xdrproc_t)
#define NULL_xdrproc_t NULL_pdrproc_t
#endif

//struct xdr_discrim
//{
  //int value;
  //xdrproc_t proc;
//};

#define xdr_void          pdr_void
#define xdr_char          pdr_char
#define xdr_u_char        pdr_uchar
#define xdr_short         pdr_int16
#define xdr_u_short       pdr_uint16
#define xdr_int           pdr_int32
#define xdr_u_int         pdr_uint32
#define xdr_long          pdr_int64
#define xdr_u_long        pdr_uint64
#define xdr_hyper         pdr_int64
#define xdr_u_hyper       pdr_uint64
#define xdr_longlong_t    pdr_int64
#define xdr_u_longlong_t  pdr_uint64
#define xdr_float         pdr_float
#define xdr_double        pdr_double
#define xdr_bool          pdr_bool
#define xdr_enum          pdr_enum
#define xdr_array         pdr_array
#define xdr_vector        pdr_vector
#define xdr_bytes         pdr_bytes
#define xdr_string        pdr_string

#define xdr_sizeof        pdr_sizeof



#define xdr_opaque        pdr_opaque
//#define xdr_union         pdr_union
#define xdr_reference     pdr_reference
#define xdr_pointer       pdr_pointer
#define xdr_wrapstring    pdr_wrapstring

/*
 * Common opaque bytes objects used by many rpc protocols;
 * declared here due to commonality.
 */
//#define MAX_NETOBJ_SZ 1024
//struct netobj
//{
  //u_int n_len;
  //char *n_bytes;
//};
//typedef struct netobj netobj;
//extern bool_t xdr_netobj (XDR *__xdrs, struct netobj *__np) __THROW;

/*
 * These are the public routines for the various implementations of
 * xdr streams.
 */

/* XDR using memory buffers */
#define xdrmem_create pdrmem_create

/* XDR using stdio library */
//extern void xdrstdio_create (XDR *__xdrs, FILE *__file, enum xdr_op __xop)
//__THROW;

/* XDR pseudo records for tcp */
//extern void xdrrec_create (XDR *__xdrs, u_int __sendsize,
			   //u_int __recvsize, caddr_t __tcp_handle,
			   //int (*__readit) (char *, char *, int),
			   //int (*__writeit) (char *, char *, int)) __THROW;

/* make end of xdr record */
//extern bool_t xdrrec_endofrecord (XDR *__xdrs, bool_t __sendnow) __THROW;

/* move to beginning of next record */
//extern bool_t xdrrec_skiprecord (XDR *__xdrs) __THROW;

/* true if no more input */
//extern bool_t xdrrec_eof (XDR *__xdrs) __THROW;

/* free memory buffers for xdr */
#define xdr_free pdr_free

/* Convenience Macros */
#define pdr_getchar(pdrs, charp)                        \
        (*(pdrs)->p_ops->getchar)(pdrs, charp)
#define pdr_putchar(pdrs, charp)                        \
        (*(pdrs)->p_ops->putchar)(pdrs, charp)

#define pdr_getint16(pdrs, int16p)                        \
        (*(pdrs)->p_ops->getint16)(pdrs, int16p)
#define pdr_putint16(pdrs, int16p)                        \
        (*(pdrs)->p_ops->putint16)(pdrs, int16p)

#define pdr_getint32(pdrs, int32p)                        \
        (*(pdrs)->p_ops->getint32)(pdrs, int32p)
#define pdr_putint32(pdrs, int32p)                        \
        (*(pdrs)->p_ops->putint32)(pdrs, int32p)

#define pdr_getint64(pdrs, int64p)                        \
        (*(pdrs)->p_ops->getint64)(pdrs, int64p)
#define pdr_putint64(pdrs, int64p)                        \
        (*(pdrs)->p_ops->putint64)(pdrs, int64p)

#define pdr_getfloat(pdrs, floatp)                        \
        (*(pdrs)->p_ops->getfloat)(pdrs, floatp)
#define pdr_putfloat(pdrs, floatp)                        \
        (*(pdrs)->p_ops->putfloat)(pdrs, floatp)

#define pdr_getdouble(pdrs, doublep)                        \
        (*(pdrs)->p_ops->getdouble)(pdrs, doublep)
#define pdr_putdouble(pdrs, doublep)                        \
        (*(pdrs)->p_ops->putdouble)(pdrs, doublep)

#define pdr_getbytes(pdrs, addr, len)                        \
        (*(pdrs)->p_ops->getbytes)(pdrs, addr, len)
#define pdr_putbytes(pdrs, addr, len)                        \
        (*(pdrs)->p_ops->putbytes)(pdrs, addr, len)

#define pdr_getpos(pdrs)                                \
        (*(pdrs)->p_ops->getpostn)(pdrs)
#define pdr_setpos(pdrs, pos)                           \
        (*(pdrs)->p_ops->setpostn)(pdrs, pos)

#define pdr_inline(pdrs, len)                           \
        (*(pdrs)->p_ops->inline)(pdrs, len)

#define pdr_destroy(pdrs)                               \
        (*(pdrs)->p_ops->destroy)(pdrs)
#endif /* !__PDR_HEADER__ */

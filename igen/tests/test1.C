#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
extern "C" {
#include <rpc/types.h>
#include <rpc/xdr.h>
}
#include "test1.h"

bool_t xdr_int_Array(XDR *__xdrs__, int_Array *__ptr__) {
if (__xdrs__->x_op == XDR_DECODE) __ptr__->data = NULL;
    xdr_array(__xdrs__, &(__ptr__->data), &__ptr__->count, ~0, sizeof(int), xdr_int);
    return(TRUE);
}

bool_t xdr_intStruct(XDR *__xdrs__, intStruct *__ptr__) {
    xdr_int(__xdrs__, &(__ptr__->style));
    return(TRUE);
}


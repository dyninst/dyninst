#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
extern "C" {
#include <rpc/types.h>
#include <rpc/xdr.h>
}
#include "test1.h"

int test::mainLoop(void)
{
    unsigned int __tag__, __status__;
    __xdrs__->x_op = XDR_DECODE;
    xdrrec_skiprecord(__xdrs__);
    __status__ = xdr_int(__xdrs__, &__tag__);
	if (!__status__) return(-1);
    switch (__tag__) {
        case 0:
            char *__ProtocolName__ = "test";
            int __val__;
            __xdrs__->x_op = XDR_ENCODE;
            __val__ = 0;
            xdr_int(__xdrs__, &__val__);
            xdr_String(__xdrs__, &__ProtocolName__);
            __val__ = 1;
            xdr_int(__xdrs__, &__val__);
            xdrrec_endofrecord(__xdrs__, TRUE);
            break;
        case test_triggerAsyncUpcall_REQ: {
            	    extern xdr_void(XDR*, void*);
            test__16 __recvBuffer__;
            xdr_int(__xdrs__, &__recvBuffer__.val);
            triggerAsyncUpcall(__recvBuffer__.val);
	    __xdrs__->x_op = XDR_ENCODE;
            __tag__ = test_triggerAsyncUpcall_RESP;
            xdr_int(__xdrs__, &__tag__);
	    xdrrec_endofrecord(__xdrs__, TRUE);
            break;
         }

        case test_triggerSyncUpcall_REQ: {
            	    extern xdr_void(XDR*, void*);
            test__14 __recvBuffer__;
            xdr_int(__xdrs__, &__recvBuffer__.val);
            triggerSyncUpcall(__recvBuffer__.val);
	    __xdrs__->x_op = XDR_ENCODE;
            __tag__ = test_triggerSyncUpcall_RESP;
            xdr_int(__xdrs__, &__tag__);
	    xdrrec_endofrecord(__xdrs__, TRUE);
            break;
         }

        case test_stringString_REQ: {
            	    String __ret__;
	    extern xdr_String(XDR*, String*);
            test__13 __recvBuffer__;
            xdr_String(__xdrs__, &__recvBuffer__.test__12);
            __ret__ = stringString(__recvBuffer__.test__12);
	    __xdrs__->x_op = XDR_ENCODE;
            __tag__ = test_stringString_RESP;
            xdr_int(__xdrs__, &__tag__);
            xdr_String(__xdrs__,&__ret__);
	    xdrrec_endofrecord(__xdrs__, TRUE);
            break;
         }

        case test_retVector_REQ: {
            	    int_Array __ret__;
	    extern xdr_int_Array(XDR*, int_Array*);
            test__11 __recvBuffer__;
            xdr_int(__xdrs__, &__recvBuffer__.len);
            xdr_int(__xdrs__, &__recvBuffer__.start);
            __ret__ = retVector(__recvBuffer__.len,__recvBuffer__.start);
	    __xdrs__->x_op = XDR_ENCODE;
            __tag__ = test_retVector_RESP;
            xdr_int(__xdrs__, &__tag__);
            xdr_int_Array(__xdrs__,&__ret__);
	    xdrrec_endofrecord(__xdrs__, TRUE);
            break;
         }

        case test_sumVector_REQ: {
            	    int __ret__;
	    extern xdr_int(XDR*, int*);
            test__10 __recvBuffer__;
            xdr_int_Array(__xdrs__, &__recvBuffer__.test__9);
            __ret__ = sumVector(__recvBuffer__.test__9);
	    __xdrs__->x_op = XDR_ENCODE;
            __tag__ = test_sumVector_RESP;
            xdr_int(__xdrs__, &__tag__);
            xdr_int(__xdrs__,&__ret__);
	    xdrrec_endofrecord(__xdrs__, TRUE);
            break;
         }

        case test_add_REQ: {
            	    int __ret__;
	    extern xdr_int(XDR*, int*);
            test__8 __recvBuffer__;
            xdr_int(__xdrs__, &__recvBuffer__.test__6);
            xdr_int(__xdrs__, &__recvBuffer__.test__7);
            __ret__ = add(__recvBuffer__.test__6,__recvBuffer__.test__7);
	    __xdrs__->x_op = XDR_ENCODE;
            __tag__ = test_add_RESP;
            xdr_int(__xdrs__, &__tag__);
            xdr_int(__xdrs__,&__ret__);
	    xdrrec_endofrecord(__xdrs__, TRUE);
            break;
         }

        case test_intString_REQ: {
            	    int __ret__;
	    extern xdr_int(XDR*, int*);
            test__5 __recvBuffer__;
            xdr_String(__xdrs__, &__recvBuffer__.test__4);
            __ret__ = intString(__recvBuffer__.test__4);
	    __xdrs__->x_op = XDR_ENCODE;
            __tag__ = test_intString_RESP;
            xdr_int(__xdrs__, &__tag__);
            xdr_int(__xdrs__,&__ret__);
	    xdrrec_endofrecord(__xdrs__, TRUE);
            break;
         }

        case test_nullStruct_REQ: {
            	    extern xdr_void(XDR*, void*);
            test__3 __recvBuffer__;
            xdr_intStruct(__xdrs__, &__recvBuffer__.test__2);
            nullStruct(__recvBuffer__.test__2);
	    __xdrs__->x_op = XDR_ENCODE;
            __tag__ = test_nullStruct_RESP;
            xdr_int(__xdrs__, &__tag__);
	    xdrrec_endofrecord(__xdrs__, TRUE);
            break;
         }

        case test_intNull_REQ: {
            	    int __ret__;
	    extern xdr_int(XDR*, int*);
            __ret__ = intNull();
	    __xdrs__->x_op = XDR_ENCODE;
            __tag__ = test_intNull_RESP;
            xdr_int(__xdrs__, &__tag__);
            xdr_int(__xdrs__,&__ret__);
	    xdrrec_endofrecord(__xdrs__, TRUE);
            break;
         }

        case test_nullNull_REQ: {
            	    extern xdr_void(XDR*, void*);
            nullNull();
	    __xdrs__->x_op = XDR_ENCODE;
            __tag__ = test_nullNull_RESP;
            xdr_int(__xdrs__, &__tag__);
	    xdrrec_endofrecord(__xdrs__, TRUE);
            break;
         }

    default:
	    return(__tag__);
  }
  return(0);
}

void test::asyncUpcall(int x) {
    unsigned int __tag__;
    __tag__ = test_asyncUpcall_REQ;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdr_int(__xdrs__, &x);
    xdrrec_endofrecord(__xdrs__, TRUE);
}


void test::syncUpcall(int x) {
    unsigned int __tag__;
    __tag__ = test_syncUpcall_REQ;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdr_int(__xdrs__, &x);
    xdrrec_endofrecord(__xdrs__, TRUE);
    __xdrs__->x_op = XDR_DECODE;
    xdrrec_skiprecord(__xdrs__);
    xdr_int(__xdrs__, &__tag__);
    assert(__tag__ == test_syncUpcall_RESP);
}


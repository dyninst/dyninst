#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
extern "C" {
#include <rpc/types.h>
#include <rpc/xdr.h>
}
#include "test1.h"


void testUser::triggerAsyncUpcall(int val) {
    unsigned int __tag__;
    __tag__ = test_triggerAsyncUpcall_REQ;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdr_int(__xdrs__, &val);
    xdrrec_endofrecord(__xdrs__, TRUE);
    awaitResponce(test_triggerAsyncUpcall_RESP);
}


void testUser::triggerSyncUpcall(int val) {
    unsigned int __tag__;
    __tag__ = test_triggerSyncUpcall_REQ;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdr_int(__xdrs__, &val);
    xdrrec_endofrecord(__xdrs__, TRUE);
    awaitResponce(test_triggerSyncUpcall_RESP);
}


String testUser::stringString(String test__12) {
    unsigned int __tag__;
    String test__22;
    __tag__ = test_stringString_REQ;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdr_String(__xdrs__, &test__12);
    xdrrec_endofrecord(__xdrs__, TRUE);
    awaitResponce(test_stringString_RESP);
    xdr_String(__xdrs__, &(test__22)); 
    return(test__22);
}


int_Array testUser::retVector(int len,int start) {
    unsigned int __tag__;
    int_Array test__23;
    __tag__ = test_retVector_REQ;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdr_int(__xdrs__, &len);
    xdr_int(__xdrs__, &start);
    xdrrec_endofrecord(__xdrs__, TRUE);
    awaitResponce(test_retVector_RESP);
    xdr_int_Array(__xdrs__, &(test__23)); 
    return(test__23);
}


int testUser::sumVector(int_Array test__9) {
    unsigned int __tag__;
    int test__24;
    __tag__ = test_sumVector_REQ;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdr_int_Array(__xdrs__, &test__9);
    xdrrec_endofrecord(__xdrs__, TRUE);
    awaitResponce(test_sumVector_RESP);
    xdr_int(__xdrs__, &(test__24)); 
    return(test__24);
}


int testUser::add(int test__6,int test__7) {
    unsigned int __tag__;
    int test__25;
    __tag__ = test_add_REQ;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdr_int(__xdrs__, &test__6);
    xdr_int(__xdrs__, &test__7);
    xdrrec_endofrecord(__xdrs__, TRUE);
    awaitResponce(test_add_RESP);
    xdr_int(__xdrs__, &(test__25)); 
    return(test__25);
}


int testUser::intString(String test__4) {
    unsigned int __tag__;
    int test__26;
    __tag__ = test_intString_REQ;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdr_String(__xdrs__, &test__4);
    xdrrec_endofrecord(__xdrs__, TRUE);
    awaitResponce(test_intString_RESP);
    xdr_int(__xdrs__, &(test__26)); 
    return(test__26);
}


void testUser::nullStruct(intStruct test__2) {
    unsigned int __tag__;
    __tag__ = test_nullStruct_REQ;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdr_intStruct(__xdrs__, &test__2);
    xdrrec_endofrecord(__xdrs__, TRUE);
    awaitResponce(test_nullStruct_RESP);
}


int testUser::intNull(void) {
    unsigned int __tag__;
    int test__28;
    __tag__ = test_intNull_REQ;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdrrec_endofrecord(__xdrs__, TRUE);
    awaitResponce(test_intNull_RESP);
    xdr_int(__xdrs__, &(test__28)); 
    return(test__28);
}


void testUser::nullNull(void) {
    unsigned int __tag__;
    __tag__ = test_nullNull_REQ;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdrrec_endofrecord(__xdrs__, TRUE);
    awaitResponce(test_nullNull_RESP);
}

void testUser::verifyProtocolAndVersion() {
    unsigned int __tag__;
    String proto;
    int version;
    __tag__ = 0;
    __xdrs__->x_op = XDR_ENCODE;
    xdr_int(__xdrs__, &__tag__);
    xdrrec_endofrecord(__xdrs__, TRUE);
    awaitResponce(0);
    xdr_String(__xdrs__, &(proto));
    xdr_int(__xdrs__, &(version));
    if ((version != 1) || (strcmp(proto, "test"))) {
        printf("protocol test version 1 expected\n");
        printf("protocol %s version %d found\n", proto, version);
	    exit(-1);
    }
}


testUser::testUser(int fd, xdrIOFunc r, xdrIOFunc w):
XDRrpc(fd, r, w) { if (__xdrs__) verifyProtocolAndVersion(); }
testUser::testUser(char *m,char *l,char *p,xdrIOFunc r,xdrIOFunc w):
    XDRrpc(m, l, p, r, w) { if (__xdrs__) 
       verifyProtocolAndVersion(); }

void testUser::awaitResponce(int __targetTag__) {
    unsigned int __tag__;
  while (1) {
    __xdrs__->x_op = XDR_DECODE;
    xdrrec_skiprecord(__xdrs__);
    xdr_int(__xdrs__, &__tag__);
    if (__tag__ == __targetTag__) return;
    switch (__tag__) {
        case test_asyncUpcall_REQ: {
            	    extern xdr_void(XDR*, void*);
            test__17 __recvBuffer__;
            xdr_int(__xdrs__, &__recvBuffer__.x);
            asyncUpcall(__recvBuffer__.x);
            break;
         }

        case test_syncUpcall_REQ: {
            	    extern xdr_void(XDR*, void*);
            test__15 __recvBuffer__;
            xdr_int(__xdrs__, &__recvBuffer__.x);
            syncUpcall(__recvBuffer__.x);
	    __xdrs__->x_op = XDR_ENCODE;
            __tag__ = test_syncUpcall_RESP;
            xdr_int(__xdrs__, &__tag__);
	    xdrrec_endofrecord(__xdrs__, TRUE);
            break;
         }

	    default: 
        abort();
    }
	if (__targetTag__ == -1) return;
  }
}
int testUser::isValidUpCall(int tag) {
    return((tag >= 2000) && (tag <= 2024));
}

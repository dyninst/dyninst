#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "test2.h"

union test__18 {
    struct test__17 asyncUpcall;
    struct test__16 triggerAsyncUpcall;
    struct test__15 syncUpcall;
    struct test__14 triggerSyncUpcall;
    struct test__13 stringString;
    struct test__11 retVector;
    struct test__10 sumVector;
    struct test__8 add;
    struct test__5 intString;
    struct test__3 nullStruct;
    struct test__1 intNull;
    struct test__0 nullNull;
};

int test::mainLoop(void)
{
  unsigned int __len__;
  unsigned int __tag__;
  union test__18 __recvBuffer__;

  __tag__ = MSG_TAG_ANY;
  __len__ = sizeof(__recvBuffer__);
  requestingThread = msg_recv(&__tag__, &__recvBuffer__, &__len__);
  switch (__tag__) {
        case test_triggerAsyncUpcall_REQ: {
            	    int __val__;
            triggerAsyncUpcall(__recvBuffer__.triggerAsyncUpcall.val);
        __val__ = msg_send(requestingThread, test_triggerAsyncUpcall_RESP, NULL, 0);
	    assert(__val__ == THR_OKAY);
            break;
         }

        case test_triggerSyncUpcall_REQ: {
            	    int __val__;
            triggerSyncUpcall(__recvBuffer__.triggerSyncUpcall.val);
        __val__ = msg_send(requestingThread, test_triggerSyncUpcall_RESP, NULL, 0);
	    assert(__val__ == THR_OKAY);
            break;
         }

        case test_stringString_REQ: {
            	    int __val__;
	    String __ret__;
            __ret__ = stringString(__recvBuffer__.stringString.test__12);
        __val__ = msg_send(requestingThread, test_stringString_RESP, (void *) &__ret__, sizeof(String));
	    assert(__val__ == THR_OKAY);
            break;
         }

        case test_retVector_REQ: {
            	    int __val__;
	    int_Array __ret__;
            __ret__ = retVector(__recvBuffer__.retVector.len,__recvBuffer__.retVector.start);
        __val__ = msg_send(requestingThread, test_retVector_RESP, (void *) &__ret__, sizeof(int_Array));
	    assert(__val__ == THR_OKAY);
            break;
         }

        case test_sumVector_REQ: {
            	    int __val__;
	    int __ret__;
            __ret__ = sumVector(__recvBuffer__.sumVector.test__9);
        __val__ = msg_send(requestingThread, test_sumVector_RESP, (void *) &__ret__, sizeof(int));
	    assert(__val__ == THR_OKAY);
            break;
         }

        case test_add_REQ: {
            	    int __val__;
	    int __ret__;
            __ret__ = add(__recvBuffer__.add.test__6,__recvBuffer__.add.test__7);
        __val__ = msg_send(requestingThread, test_add_RESP, (void *) &__ret__, sizeof(int));
	    assert(__val__ == THR_OKAY);
            break;
         }

        case test_intString_REQ: {
            	    int __val__;
	    int __ret__;
            __ret__ = intString(__recvBuffer__.intString.test__4);
        __val__ = msg_send(requestingThread, test_intString_RESP, (void *) &__ret__, sizeof(int));
	    assert(__val__ == THR_OKAY);
            break;
         }

        case test_nullStruct_REQ: {
            	    int __val__;
            nullStruct(__recvBuffer__.nullStruct.test__2);
        __val__ = msg_send(requestingThread, test_nullStruct_RESP, NULL, 0);
	    assert(__val__ == THR_OKAY);
            break;
         }

        case test_intNull_REQ: {
            	    int __val__;
	    int __ret__;
            __ret__ = intNull();
        __val__ = msg_send(requestingThread, test_intNull_RESP, (void *) &__ret__, sizeof(int));
	    assert(__val__ == THR_OKAY);
            break;
         }

        case test_nullNull_REQ: {
            	    int __val__;
            nullNull();
        __val__ = msg_send(requestingThread, test_nullNull_RESP, NULL, 0);
	    assert(__val__ == THR_OKAY);
            break;
         }

    default:
	    return(__tag__);
  }
  return(0);
}

void test::asyncUpcall(int x) {
    struct test__17 test__20;
    int __val__;
    test__20.x = x;  
    __val__ = msg_send(tid, test_asyncUpcall_REQ, (void *) &test__20, sizeof(test__20));
    assert(__val__ == THR_OKAY);
}


void test::syncUpcall(int x) {
    struct test__15 test__22;
    unsigned int __tag__;
    int __val__;
    test__22.x = x;  
    __val__ = msg_send(tid, test_syncUpcall_REQ, (void *) &test__22, sizeof(test__22));
    assert(__val__ == THR_OKAY);
    __tag__ = test_syncUpcall_RESP;
    msg_recv(&__tag__, NULL, 0); 
    assert(__tag__ == test_syncUpcall_RESP);
}


void testUser::triggerAsyncUpcall(int val) {
    struct test__16 test__24;
    unsigned int __tag__;
    int __val__;
    test__24.val = val;  
    __val__ = msg_send(tid, test_triggerAsyncUpcall_REQ, (void *) &test__24, sizeof(test__24));
    assert(__val__ == THR_OKAY);
    __tag__ = test_triggerAsyncUpcall_RESP;
    msg_recv(&__tag__, NULL, 0); 
    assert(__tag__ == test_triggerAsyncUpcall_RESP);
}


void testUser::triggerSyncUpcall(int val) {
    struct test__14 test__26;
    unsigned int __tag__;
    int __val__;
    test__26.val = val;  
    __val__ = msg_send(tid, test_triggerSyncUpcall_REQ, (void *) &test__26, sizeof(test__26));
    assert(__val__ == THR_OKAY);
    __tag__ = test_triggerSyncUpcall_RESP;
    msg_recv(&__tag__, NULL, 0); 
    assert(__tag__ == test_triggerSyncUpcall_RESP);
}


String testUser::stringString(String test__12) {
    struct test__13 test__28;
    unsigned int __tag__;
    unsigned int __len__;
    int __val__;
    String test__27;
    test__28.test__12 = test__12;  
    __val__ = msg_send(tid, test_stringString_REQ, (void *) &test__28, sizeof(test__28));
    assert(__val__ == THR_OKAY);
    __tag__ = test_stringString_RESP;
    __len__ = sizeof(test__27);
    msg_recv(&__tag__, (void *) &test__27, &__len__); 
    assert(__len__ == sizeof(test__27));
    return(test__27);
    assert(__tag__ == test_stringString_RESP);
}


int_Array testUser::retVector(int len,int start) {
    struct test__11 test__30;
    unsigned int __tag__;
    unsigned int __len__;
    int __val__;
    int_Array test__29;
    test__30.len = len;  
    test__30.start = start;  
    __val__ = msg_send(tid, test_retVector_REQ, (void *) &test__30, sizeof(test__30));
    assert(__val__ == THR_OKAY);
    __tag__ = test_retVector_RESP;
    __len__ = sizeof(test__29);
    msg_recv(&__tag__, (void *) &test__29, &__len__); 
    assert(__len__ == sizeof(test__29));
    return(test__29);
    assert(__tag__ == test_retVector_RESP);
}


int testUser::sumVector(int_Array test__9) {
    struct test__10 test__32;
    unsigned int __tag__;
    unsigned int __len__;
    int __val__;
    int test__31;
    test__32.test__9 = test__9;  
    __val__ = msg_send(tid, test_sumVector_REQ, (void *) &test__32, sizeof(test__32));
    assert(__val__ == THR_OKAY);
    __tag__ = test_sumVector_RESP;
    __len__ = sizeof(test__31);
    msg_recv(&__tag__, (void *) &test__31, &__len__); 
    assert(__len__ == sizeof(test__31));
    return(test__31);
    assert(__tag__ == test_sumVector_RESP);
}


int testUser::add(int test__6,int test__7) {
    struct test__8 test__34;
    unsigned int __tag__;
    unsigned int __len__;
    int __val__;
    int test__33;
    test__34.test__6 = test__6;  
    test__34.test__7 = test__7;  
    __val__ = msg_send(tid, test_add_REQ, (void *) &test__34, sizeof(test__34));
    assert(__val__ == THR_OKAY);
    __tag__ = test_add_RESP;
    __len__ = sizeof(test__33);
    msg_recv(&__tag__, (void *) &test__33, &__len__); 
    assert(__len__ == sizeof(test__33));
    return(test__33);
    assert(__tag__ == test_add_RESP);
}


int testUser::intString(String test__4) {
    struct test__5 test__36;
    unsigned int __tag__;
    unsigned int __len__;
    int __val__;
    int test__35;
    test__36.test__4 = test__4;  
    __val__ = msg_send(tid, test_intString_REQ, (void *) &test__36, sizeof(test__36));
    assert(__val__ == THR_OKAY);
    __tag__ = test_intString_RESP;
    __len__ = sizeof(test__35);
    msg_recv(&__tag__, (void *) &test__35, &__len__); 
    assert(__len__ == sizeof(test__35));
    return(test__35);
    assert(__tag__ == test_intString_RESP);
}


void testUser::nullStruct(intStruct test__2) {
    struct test__3 test__38;
    unsigned int __tag__;
    int __val__;
    test__38.test__2 = test__2;  
    __val__ = msg_send(tid, test_nullStruct_REQ, (void *) &test__38, sizeof(test__38));
    assert(__val__ == THR_OKAY);
    __tag__ = test_nullStruct_RESP;
    msg_recv(&__tag__, NULL, 0); 
    assert(__tag__ == test_nullStruct_RESP);
}


int testUser::intNull(void) {
    unsigned int __tag__;
    unsigned int __len__;
    int __val__;
    int test__39;
    __val__ = msg_send(tid, test_intNull_REQ, NULL, 0); 
    assert(__val__ == THR_OKAY);
    __tag__ = test_intNull_RESP;
    __len__ = sizeof(test__39);
    msg_recv(&__tag__, (void *) &test__39, &__len__); 
    assert(__len__ == sizeof(test__39));
    return(test__39);
    assert(__tag__ == test_intNull_RESP);
}


void testUser::nullNull(void) {
    unsigned int __tag__;
    int __val__;
    __val__ = msg_send(tid, test_nullNull_REQ, NULL, 0); 
    assert(__val__ == THR_OKAY);
    __tag__ = test_nullNull_RESP;
    msg_recv(&__tag__, NULL, 0); 
    assert(__tag__ == test_nullNull_RESP);
}

void testUser::awaitResponce(int __targetTag__) {
    unsigned int __tag__;
  union test__18 __recvBuffer__;
  unsigned __len__ = sizeof(__recvBuffer__);
  while (1) {
  __tag__ = MSG_TAG_ANY;
    requestingThread = msg_recv(&__tag__, (void *) &__recvBuffer__, &__len__); 
    if (__tag__ == __targetTag__) return;
    switch (__tag__) {
        case test_asyncUpcall_REQ: {
                        asyncUpcall(__recvBuffer__.asyncUpcall.x);
            break;
         }

        case test_syncUpcall_REQ: {
            	    int __val__;
            syncUpcall(__recvBuffer__.syncUpcall.x);
        __val__ = msg_send(requestingThread, test_syncUpcall_RESP, NULL, 0);
	    assert(__val__ == THR_OKAY);
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

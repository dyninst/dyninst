#include "util/h/rpcUtil.h"

/* a struct with one int */
#ifndef intStruct_TYPE
#define intStruct_TYPE
class intStruct {  
public:
    int style;
};

#endif
extern xdr_intStruct(XDR*, intStruct*);
		

/*
 *
 */
struct test__0 {
};

#define test_nullNull_REQ 2001
#define test_nullNull_RESP 2002
struct test__1 {
};

#define test_intNull_REQ 2003
#define test_intNull_RESP 2004
struct test__3 {
    intStruct test__2;
};

#define test_nullStruct_REQ 2005
#define test_nullStruct_RESP 2006
struct test__5 {
    String test__4;
};

#define test_intString_REQ 2007
#define test_intString_RESP 2008
struct test__8 {
    int test__6;
    int test__7;
};

#define test_add_REQ 2009
#define test_add_RESP 2010
#ifndef int_Array_TYPE
#define int_Array_TYPE
class int_Array {  
public:
    int count;
    int* data;
};

#endif
extern xdr_int_Array(XDR*, int_Array*);
struct test__10 {
    int_Array test__9;
};

#define test_sumVector_REQ 2011
#define test_sumVector_RESP 2012
struct test__11 {
    int len;
    int start;
};

#define test_retVector_REQ 2013
#define test_retVector_RESP 2014
struct test__13 {
    String test__12;
};

#define test_stringString_REQ 2015
#define test_stringString_RESP 2016
struct test__14 {
    int val;
};

#define test_triggerSyncUpcall_REQ 2017
#define test_triggerSyncUpcall_RESP 2018
struct test__15 {
    int x;
};

#define test_syncUpcall_REQ 2019
#define test_syncUpcall_RESP 2020
struct test__16 {
    int val;
};

#define test_triggerAsyncUpcall_REQ 2021
#define test_triggerAsyncUpcall_RESP 2022
struct test__17 {
    int x;
};

#define test_asyncUpcall_REQ 2023
#define test_asyncUpcall_RESP 2024
class testUser: public RPCUser, public XDRrpc {
  public:
    virtual void verifyProtocolAndVersion();
    testUser(int fd, xdrIOFunc r, xdrIOFunc w);
    testUser(char *,char *,char*, xdrIOFunc r, xdrIOFunc w);
    void awaitResponce(int);
    int isValidUpCall(int);
    void asyncUpcall(int x);
    void triggerAsyncUpcall(int val);
    void syncUpcall(int x);
    void triggerSyncUpcall(int val);
    String stringString(String test__12);
    int_Array retVector(int len,int start);
    int sumVector(int_Array test__9);
    int add(int test__6,int test__7);
    int intString(String test__4);
    void nullStruct(intStruct test__2);
    int intNull(void);
    void nullNull(void);
};
class test: private RPCServer, public XDRrpc {
  public:
    mainLoop(void);
    void asyncUpcall(int x);
    void triggerAsyncUpcall(int val);
    void syncUpcall(int x);
    void triggerSyncUpcall(int val);
    String stringString(String test__12);
    int_Array retVector(int len,int start);
    int sumVector(int_Array test__9);
    int add(int test__6,int test__7);
    int intString(String test__4);
    void nullStruct(intStruct test__2);
    int intNull(void);
    void nullNull(void);
};



/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include "common/h/Types.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(USE_XDR)
#include <rpc/xdr.h>
#else
#include "mrnet/src/pdr.h"
#endif

enum test_enum{ZERO,ONE,TWO};

typedef struct {
  char _char;
  unsigned char _uchar;
  bool_t _bool;
  short _short;
  unsigned short _ushort;
  int _int;
  unsigned int _uint;
  long _long;
  unsigned long _ulong;
  //long long _longlong;
  //unsigned long long _ulonglong;
  float _float;
  double _double;
  enum test_enum _enum;
  char * _string;
  int *_intarray;
  unsigned int _intarraylen;
  char *_bytearray;
  unsigned int _bytearraylen;
  float * _floatvector;
  unsigned int _floatvectorlen;
}packeddata;

void print_struct(packeddata * a)
{
  unsigned int i;
  fprintf(stdout,
          "_char   = %hhd\n"
          "_uchar  = %hhd\n"
          "_bool   = %d\n"
          "_short  = %hd\n"
          "_ushort = %hd\n"
          "_int    = %d\n"
          "_uint   = %d\n"
          "_long   = %ld\n"
          "_ulong  = %ld\n"
          "_float  = %.5f\n"
          "_double = %.14f\n"
          "_enum   = %d\n"
          "_string = %s\n",
	  a->_char, a->_uchar, a->_bool, a->_short, a->_ushort, a->_int,
          a->_uint, a->_long, a->_ulong, a->_float, a->_double, a->_enum,
          a->_string);
  fprintf(stdout, "_intarray = [ ");
  for(i=0; i<a->_intarraylen; i++){
    fprintf(stdout, "%d, ", a->_intarray[i]);
  }
  fprintf(stdout, "]\n" );

  fprintf(stdout, "_bytearray = [ ");
  for(i=0; i<a->_bytearraylen; i++){
    fprintf(stdout, "%c, ", a->_bytearray[i]);
  }
  fprintf(stdout, "]\n" );

  fprintf(stdout, "_floatvector = [ ");
  for(i=0; i<a->_floatvectorlen; i++){
    fprintf(stdout, "%f, ", a->_floatvector[i]);
  }
  fprintf(stdout, "]\n" );
}

bool_t compare_structs(packeddata * a, packeddata * b)
{
  return ( (a->_char == b->_char) &&
           (a->_uchar == b->_uchar) &&
           (a->_bool == b->_bool) &&
           (a->_short == b->_short) &&
           (a->_ushort == b->_ushort) &&
           (a->_int == b->_int) &&
           (a->_uint == b->_uint) &&
           (a->_long == b->_long) &&
           (a->_ulong == b->_ulong) &&
           (a->_float == b->_float) &&
           (a->_double == b->_double) &&
           (a->_enum == b->_enum) &&
           (!strcmp(a->_string, b->_string)) &&
           (!memcmp(a->_intarray, b->_intarray, sizeof(int)*a->_intarraylen)) &&
           (!memcmp(a->_floatvector, b->_floatvector, sizeof(float)*a->_floatvectorlen)) &&
           (!memcmp(a->_bytearray, b->_bytearray, sizeof(char)*a->_bytearraylen)) );
}

bool_t xdr_struct(XDR * xdrs, packeddata * _data)
{
  if( !xdr_char(xdrs, &(_data->_char)) ){
    printf("xdr_char() failed\n");
    return FALSE;
  }
  if( !xdr_u_char(xdrs, &(_data->_uchar)) ){
    printf("xdr_uchar() failed\n");
    return FALSE;
  }
  if( !xdr_bool(xdrs, &(_data->_bool)) ){
    printf("xdr_bool() failed\n");
    return FALSE;
  }
  if( !xdr_short(xdrs, &(_data->_short)) ){
    printf("xdr_short() failed\n");
    return FALSE;
  }
  if( !xdr_u_short(xdrs, &(_data->_ushort)) ){
    printf("xdr_ushort() failed\n");
    return FALSE;
  }
  if( !xdr_int(xdrs, &(_data->_int)) ){
    printf("xdr_int() failed\n");
    return FALSE;
  }
  if( !xdr_u_int(xdrs, &(_data->_uint)) ){
    printf("xdr_uint() failed\n");
    return FALSE;
  }
#if defined(USE_XDR)
  if( !xdr_long(xdrs, &(_data->_long)) ){
    printf("xdr_long() failed\n");
    return FALSE;
  }
  if( !xdr_u_long(xdrs, &(_data->_ulong)) ){
    printf("xdr_ulong() failed\n");
    return FALSE;
  }
#else
  if( !xdr_long(xdrs, (int64_t *)&(_data->_long)) ){
    printf("xdr_long() failed\n");
    return FALSE;
  }
  if( !xdr_u_long(xdrs, (uint64_t *)&(_data->_ulong)) ){
    printf("xdr_ulong() failed\n");
    return FALSE;
  }
#endif
  if( !xdr_float(xdrs, &(_data->_float)) ){
    printf("xdr_float() failed\n");
    return FALSE;
  }
  if( !xdr_double(xdrs, &(_data->_double)) ){
    printf("xdr_double() failed\n");
    return FALSE;
  }
  if( !xdr_enum(xdrs, (enum_t*)&(_data->_enum)) ){
    printf("xdr_enum() failed\n");
    return FALSE;
  }
  if( !xdr_string(xdrs, &(_data->_string), 256) ){
    printf("xdr_string() failed\n");
    return FALSE;
  }
  if( !xdr_array(xdrs, (char **)&(_data->_intarray), &(_data->_intarraylen),
                     128, sizeof(int), (xdrproc_t)xdr_int ) ){
    printf("xdr_array() failed\n");
    return FALSE;
  }
  if( !xdr_bytes(xdrs, (char **)&(_data->_bytearray),
                     &(_data->_bytearraylen), 128) ){
    printf("xdr_bytes() failed\n");
    return FALSE;
  }
  if( !xdr_vector(xdrs, (char *)(_data->_floatvector),
                      (_data->_floatvectorlen), sizeof(float),
                      (xdrproc_t)xdr_float )  ){
    printf("xdr_vector() failed\n");
    return FALSE;
  }
  return TRUE;
}

int main(int argc, char **argv)
{
  unsigned int i, mem_size;
  char * mem;
  XDR xdrs;

  packeddata copied_data = {0, 0, FALSE, 0, 0, 0, 0,
                            0, 0, 0,
                            0, ZERO, NULL, NULL, 0, NULL, 0, NULL, 0};

  packeddata original_data = {-67, 67, TRUE, -27, 27, -64336, 64436,
                              -333333333, 333333333, 35.111222333444555,
                              35.111222333444555, TWO, "Test String"};

  original_data._intarraylen=5;
  original_data._intarray=(int*)malloc(sizeof(int)*original_data._intarraylen);
  for(i=0; i<original_data._intarraylen; i++){
    original_data._intarray[i] = i;
  }

  original_data._bytearraylen=5;
  original_data._bytearray=(char*)malloc(sizeof(char)*original_data._bytearraylen);
  for(i=0; i<original_data._bytearraylen; i++){
    original_data._bytearray[i] = i+97;
  }

  copied_data._floatvectorlen=original_data._floatvectorlen=5;
  original_data._floatvector=(float*)malloc(sizeof(float)*original_data._floatvectorlen);
  copied_data._floatvector=(float*)malloc(sizeof(float)*copied_data._floatvectorlen);
  for(i=0; i<original_data._floatvectorlen; i++){
    original_data._floatvector[i] = i*10.0;
  }

  print_struct(&original_data);
  mem_size = xdr_sizeof((xdrproc_t)xdr_struct, &original_data);
  if(mem_size == 0){
    printf("xdr_sizeof() failed\n");
    exit(-1);
  }
  mem = (char *) malloc(sizeof(char) * mem_size);
  printf("Memory requirements: %d\n", mem_size);

  xdrmem_create(&xdrs, mem, mem_size, XDR_ENCODE);
  if( !xdr_struct(&xdrs, &original_data) ){
    printf("xdr_struct(ENCODE) failed\n");
    exit(-1);
  }

  xdrmem_create(&xdrs, mem, mem_size, XDR_DECODE);
  if( !xdr_struct(&xdrs, &copied_data) ){
    printf("xdr_struct(DECODE) failed\n");
    exit(-1);
  }

  if( !compare_structs(&original_data, &copied_data) ){
    printf("FAILURE!\n");
    print_struct(&original_data);
    print_struct(&copied_data);
    exit(-1);
  }
  else{
    printf("SUCCESS!\n");
    print_struct(&copied_data);
  }

  return 0;
}

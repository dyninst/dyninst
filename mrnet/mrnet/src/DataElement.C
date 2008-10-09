/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "DataElement.h"

namespace MRN
{

DataElement::DataElement( ) : type(UNKNOWN_T), array_len(0), destroy_data(false)
{ 
    val.uld=0;
}

DataElement::~DataElement()
{
    if( destroy_data == false ){
        return;
    }

    switch(type){
    case CHAR_ARRAY_T:
    case UCHAR_ARRAY_T:
    case STRING_T:
    case INT16_ARRAY_T:
    case UINT16_ARRAY_T:
    case INT32_ARRAY_T:
    case UINT32_ARRAY_T:
    case INT64_ARRAY_T:
    case UINT64_ARRAY_T:
    case FLOAT_ARRAY_T:
    case DOUBLE_ARRAY_T:
        if( val.p != NULL ){
            free( val.p );
        }
        break;
    case STRING_ARRAY_T:
        if( val.p != NULL ){
            char** arr = ( char** ) val.p;
            for(unsigned i=0; i < array_len; i++) {
               if( arr[i] != NULL )
                  free( arr[i] );
            }
            free( arr );
        }
        break;
    case CHAR_T:
    case UCHAR_T:
    case INT16_T:
    case UINT16_T:
    case INT32_T:
    case UINT32_T:
    case INT64_T:
    case UINT64_T:
    case FLOAT_T:
    case DOUBLE_T:
    case UNKNOWN_T:
        break;
    }
}

void DataElement::set_DestroyData( bool d )
{
    destroy_data = d;
}

DataType DataElement::get_Type( void ) const
{
    return type;
}

char DataElement::get_char( void ) const
{
    return val.c;
}
void DataElement::set_char( char c )
{
    val.c = c; type = CHAR_T;
}
unsigned char DataElement::get_uchar( void ) const
{
    return val.uc;
}
void DataElement::set_uchar( unsigned char uc )
{
    val.uc = uc; type = UCHAR_T;
}
int16_t DataElement::get_int16_t( void ) const
{
    return val.hd;
}
void DataElement::set_int16_t( int16_t hd )
{
    val.hd = hd; type = INT16_T;
}

uint16_t DataElement::get_uint16_t( void ) const
{
    return val.uhd;
}
void DataElement::set_uint16_t( uint16_t uhd )
{
    val.uhd = uhd; type = UINT16_T;
}

int32_t DataElement::get_int32_t( void ) const
{
    return val.d;
}
void DataElement::set_int32_t( int32_t d )
{
    val.d = d; type = INT32_T;
}

uint32_t DataElement::get_uint32_t( void ) const
{
    return val.ud;
}
void DataElement::set_uint32_t( uint32_t ud )
{
    val.ud = ud; type = UINT32_T;
}

int64_t DataElement::get_int64_t( void ) const
{
    return val.ld;
}
void DataElement::set_int64_t( int64_t ld )
{
    val.ld = ld; type = INT64_T;
}

uint64_t DataElement::get_uint64_t( void ) const
{
    return val.uld;
}
void DataElement::set_uint64_t( uint64_t uld )
{
    val.uld = uld; type = UINT64_T;
}

float DataElement::get_float( void ) const
{
    return val.f;
}
void DataElement::set_float( float f )
{
    val.f = f; type = FLOAT_T;
}

double DataElement::get_double( void ) const
{
    return val.lf;
}
void DataElement::set_double( double lf )
{
    val.lf = lf; type = DOUBLE_T;
}

const char * DataElement::get_string( void ) const 
{ 
    return (const char *)val.p;
}
void DataElement::set_string( const char *p )
{ 
    val.p = const_cast<void*>((const void*)p); type = STRING_T;
}

const void * DataElement::get_array( DataType *t, uint32_t *len ) const
{ 
    *t=type; 
    *len=array_len; 
    return val.p;
}
void DataElement::set_array( const void *p, DataType t, uint32_t len )
{ 
    val.p = const_cast<void*>(p); 
    type = t; 
    array_len = len;
}

DataType Fmt2Type(const char * cur_fmt)
{
    if( !strcmp(cur_fmt, "c") )
        return CHAR_T;
    else if( !strcmp(cur_fmt, "uc") )
        return UCHAR_T;
    else if( !strcmp(cur_fmt, "ac") )
        return CHAR_ARRAY_T;
    else if( !strcmp(cur_fmt, "auc") )
        return UCHAR_ARRAY_T;
    else if( !strcmp(cur_fmt, "hd") )
        return INT16_T;
    else if( !strcmp(cur_fmt, "uhd") )
        return UINT16_T;
    else if( !strcmp(cur_fmt, "d") )
        return INT32_T;
    else if( !strcmp(cur_fmt, "ud") )
        return UINT32_T;
    else if( !strcmp(cur_fmt, "ahd") )
        return INT16_ARRAY_T;
    else if( !strcmp(cur_fmt, "ld") )
        return INT64_T;
    else if( !strcmp(cur_fmt, "uld") )
        return UINT64_T;
    else if( !strcmp(cur_fmt, "auhd") )
        return UINT16_ARRAY_T;
    else if( !strcmp(cur_fmt, "ad") )
        return INT32_ARRAY_T;
    else if( !strcmp(cur_fmt, "aud") )
        return UINT32_ARRAY_T;
    else if( !strcmp(cur_fmt, "ald") )
        return INT64_ARRAY_T;
    else if( !strcmp(cur_fmt, "auld") )
        return UINT64_ARRAY_T;
    else if( !strcmp(cur_fmt, "f") )
        return FLOAT_T;
    else if( !strcmp(cur_fmt, "af") )
        return FLOAT_ARRAY_T;
    else if( !strcmp(cur_fmt, "lf") )
        return DOUBLE_T;
    else if( !strcmp(cur_fmt, "alf") )
        return DOUBLE_ARRAY_T;
    else if( !strcmp(cur_fmt, "s") )
        return STRING_T;
    else if( !strcmp(cur_fmt, "as") )
        return STRING_ARRAY_T;
    else
        return UNKNOWN_T;
}

} /* namespace MRN */

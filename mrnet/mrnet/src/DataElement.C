#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "mrnet/src/DataElement.h"

namespace MRN
{

DataElement::DataElement( const DataElement & de)
    :  val(de.val), type(de.type), array_len(de.array_len)
{
    switch(type){
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
        break;
    case STRING_T:
        if( de.val.p ){
            val.p = (void*)strdup((char *)de.val.p);
        }
        else{
            val.p = NULL;
        }
        break;
    case CHAR_ARRAY_T:
    case UCHAR_ARRAY_T:
        if( de.val.p ){
            val.p = malloc( array_len * sizeof(char) );
        }
        else{
            val.p = NULL;
        }
        memcpy(val.p, de.val.p, array_len*sizeof(char) );
        break;
    case INT16_ARRAY_T:
    case UINT16_ARRAY_T:
        if( de.val.p ){
            val.p = malloc( array_len * sizeof(int16_t) );
        }
        else{
            val.p = NULL;
        }
        memcpy(val.p, de.val.p, array_len*sizeof(int16_t) );
        break;
    case INT32_ARRAY_T:
    case UINT32_ARRAY_T:
        if( de.val.p ){
            val.p = malloc( array_len * sizeof(int32_t) );
        }
        else{
            val.p = NULL;
        }
        memcpy(val.p, de.val.p, array_len*sizeof(int32_t) );
        break;
    case INT64_ARRAY_T:
    case UINT64_ARRAY_T:
        if( de.val.p ){
            val.p = malloc( array_len * sizeof(int64_t) );
        }
        else{
            val.p = NULL;
        }
        memcpy(val.p, de.val.p, array_len*sizeof(int64_t) );
        break;
    case FLOAT_ARRAY_T:
        if( de.val.p ){
            val.p = malloc( array_len * sizeof(float) );
        }
        else{
            val.p = NULL;
        }
        memcpy(val.p, de.val.p, array_len*sizeof(float) );
        break;
    case DOUBLE_ARRAY_T:
        if( de.val.p ){
            val.p = malloc( array_len * sizeof(double) );
        }
        else{
            val.p = NULL;
        }
        memcpy(val.p, de.val.p, array_len*sizeof(double) );
        break;
    case STRING_ARRAY_T:
        //TODO
        break;
    case UNKNOWN_T:
        //TODO: error
        break;
    }
}

DataElement & DataElement::operator=( const DataElement & de)
{
    type=de.type;
    array_len=de.array_len;

    switch(type){
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
        val=de.val;
        break;
    case STRING_T:
        if( val.p != NULL ){
            //TODO: check and *NOT* free memory that will be passed to user
            //free(val.p);
        }
        if( de.val.p != NULL ){
            val.p = (void*)strdup((char *)de.val.p);
        }
        else{
            val.p = NULL;
        }
        break;
    case CHAR_ARRAY_T:
    case UCHAR_ARRAY_T:
        if( val.p != NULL ){
            //TODO: check and *NOT* free memory that will be passed to user
            //free(val.p);
        }
        if( de.val.p != NULL ){
            val.p = malloc( array_len * sizeof(char) );
        }
        else{
            val.p = NULL;
        }
        memcpy(val.p, de.val.p, array_len*sizeof(char) );
        break;
    case INT16_ARRAY_T:
    case UINT16_ARRAY_T:
        if( val.p != NULL ){
            //TODO: check and *NOT* free memory that will be passed to user
            //free(val.p);
        }
        if( de.val.p != NULL ){
            val.p = malloc( array_len * sizeof(int16_t) );
        }
        else{
            val.p = NULL;
        }
        memcpy(val.p, de.val.p, array_len*sizeof(int16_t) );
        break;
    case INT32_ARRAY_T:
    case UINT32_ARRAY_T:
        if( val.p != NULL ){
            //TODO: check and *NOT* free memory that will be passed to user
            //free(val.p);
        }
        if( de.val.p != NULL ){
            val.p = malloc( array_len * sizeof(int32_t) );
        }
        else{
            val.p = NULL;
        }
        memcpy(val.p, de.val.p, array_len*sizeof(int32_t) );
        break;
    case INT64_ARRAY_T:
    case UINT64_ARRAY_T:
        if( val.p != NULL ){
            //TODO: check and *NOT* free memory that will be passed to user
            //free(val.p);
        }
        if( de.val.p != NULL ){
            val.p = malloc( array_len * sizeof(int64_t) );
        }
        else{
            val.p = NULL;
        }
        memcpy(val.p, de.val.p, array_len*sizeof(int64_t) );
        break;
    case FLOAT_ARRAY_T:
        if( val.p != NULL ){
            //TODO: check and *NOT* free memory that will be passed to user
            //free(val.p);
        }
        if( de.val.p != NULL ){
            val.p = malloc( array_len * sizeof(float) );
        }
        else{
            val.p = NULL;
        }
        memcpy(val.p, de.val.p, array_len*sizeof(float) );
        break;
    case DOUBLE_ARRAY_T:
        if( val.p != NULL ){
            //TODO: check and *NOT* free memory that will be passed to user
            //free(val.p);
        }
        if( de.val.p != NULL ){
            val.p = malloc( array_len * sizeof(double) );
        }
        else{
            val.p = NULL;
        }
        memcpy(val.p, de.val.p, array_len*sizeof(double) );
        break;
    case STRING_ARRAY_T:
        //TODO
        break;
    case UNKNOWN_T:
        //TODO: error
        break;
    }
    return *this;
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

/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "mrnet/src/DataElement.h"

namespace MRN
{

DataElement::~DataElement()
{
    if( destroy_data == false ){
        return;
    }

    switch(type){
    case CHAR_ARRAY_T:
    case UCHAR_ARRAY_T:
    case STRING_T:
    case STRING_ARRAY_T:
    case INT16_ARRAY_T:
    case UINT16_ARRAY_T:
    case INT32_ARRAY_T:
    case UINT32_ARRAY_T:
    case INT64_ARRAY_T:
    case UINT64_ARRAY_T:
    case FLOAT_ARRAY_T:
    case DOUBLE_ARRAY_T:
        if( val.p != NULL){
            free( (void*)val.p );
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

#include <string.h>

#include "mrnet/src/DataElement.h"

namespace MRN{

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

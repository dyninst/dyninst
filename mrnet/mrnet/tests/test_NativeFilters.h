/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined(test_nativefilters_h )
#define test_nativefilters_h 1

#include "mrnet/tests/timer.h"

enum{
    PROT_CHAR_SUM=MRN::FIRST_APPL_TAG,
    PROT_UCHAR_SUM,
    PROT_INT16_SUM,
    PROT_UINT16_SUM,
    PROT_INT32_SUM,
    PROT_UINT32_SUM,
    PROT_INT64_SUM,
    PROT_UINT64_SUM,
    PROT_FLOAT_SUM,
    PROT_DOUBLE_SUM,
    PROT_EXIT
};

const char CHARVAL=-7;
const unsigned char UCHARVAL=7;
const int16_t INT16VAL=-17;
const uint16_t UINT16VAL=17;
const int32_t INT32VAL=-17;
const uint32_t UINT32VAL=17;
const int64_t INT64VAL=-17;
const uint64_t UINT64VAL=17;
const float FLOATVAL=123.456;
const double DOUBLEVAL=123.456;

#endif /* test_nativefilters_h */

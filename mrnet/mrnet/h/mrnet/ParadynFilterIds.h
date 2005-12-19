/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/
#ifndef PD_FILTERS_H
#define PD_FILTERS_H

#include "mrnet/FilterIds.h"

namespace MRN
{

extern FilterId TFILTER_SAVE_LOCAL_CLOCK_SKEW_UPSTREAM;
extern FilterId TFILTER_SAVE_LOCAL_CLOCK_SKEW_DOWNSTREAM;
extern FilterId TFILTER_GET_CLOCK_SKEW;

extern FilterId TFILTER_PD_UINT_EQ_CLASS;

} // namespace MRN

#endif  // PD_FILTERS_H

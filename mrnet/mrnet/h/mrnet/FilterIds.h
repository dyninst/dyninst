/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/
#ifndef MRN_FILTERS_H
#define MRN_FILTERS_H

namespace MRN
{

typedef unsigned short FilterId;

// IDs for built-in transformation filters
extern FilterId TFILTER_NULL;
extern FilterId TFILTER_SUM;
extern FilterId TFILTER_AVG;
extern FilterId TFILTER_MIN;
extern FilterId TFILTER_MAX;
extern FilterId TFILTER_ARRAY_CONCAT;
extern FilterId TFILTER_INT_EQ_CLASS;

// IDs for built-in synchronization filters
extern FilterId SFILTER_DONTWAIT;
extern FilterId SFILTER_WAITFORALL;
extern FilterId SFILTER_TIMEOUT;

} // namespace MRN

#endif  // MRN_FILTERS_H

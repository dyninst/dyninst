/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined( filterids_h )
#define filterids_h 1


namespace MRN
{

// IDs of built-in transformation filters
extern unsigned short TFILTER_NULL;
extern unsigned short TFILTER_SUM;
extern unsigned short TFILTER_AVG;
extern unsigned short TFILTER_MIN;
extern unsigned short TFILTER_MAX;
extern unsigned short TFILTER_ARRAY_CONCAT;
extern unsigned short TFILTER_INT_EQ_CLASS;


// IDs of built-in synchronization filters
extern unsigned short SFILTER_DONTWAIT;
extern unsigned short SFILTER_WAITFORALL;
extern unsigned short SFILTER_TIMEOUT;

} // namespace MRN

#endif  /* filterids_h */

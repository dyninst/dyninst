/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined( microbench_h )
#define microbench_h 1

typedef enum {
    MB_EXIT=FIRST_APPL_TAG,
    MB_ROUNDTRIP_LATENCY,
    MB_RED_THROUGHPUT
} Protocol;

#endif /* microbench_h */

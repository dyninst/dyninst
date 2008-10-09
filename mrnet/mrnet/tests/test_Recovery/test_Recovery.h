/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined(test_failurerecovery_h)
#define test_failurerecovery_h 1

#include <set>
#include "mrnet/MRNet.h"

//#define VALIDATE_OUTPUT 1
//#define USE_EXPLICIT_EXIT 1


#define DATA_SEND_FREQ 100 //milliseconds

#define THROUGHPUT_SAMPLING_INTERVAL 2000 //milliseconds


int dump_EqClass( std::set<uint32_t>, const char * ifilename );

typedef enum {PROT_EXIT=FirstApplicationTag,
              PROT_START, PROT_INT_EQCLASS } Protocol;

#endif /* test_test_failurerecovery_h */

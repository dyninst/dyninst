/*
 * Copyright (c) 1996-2001 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */


#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "rtinst/h/rtinst.h"
#include "RTcompat.h"

#ifdef PAPI
#include "papi.h"
#endif

int papiInitialized = 0;
int papiEventSet = -1;

#ifdef PAPI
long_long* papiValues;
#endif

int isPapiAvail() {
  return papiInitialized;
}

int initPapi() {
#ifdef PAPI
  int retval;
  int numCntrs;

  retval = PAPI_library_init(PAPI_VER_CURRENT);
  if ( retval != PAPI_VER_CURRENT) {
    papiInitialized = 0;
    return 0;
  }
  else {
 
    retval = PAPI_create_eventset(&papiEventSet);
    if (retval != PAPI_OK) {
      papiInitialized = 0;
      return 0;
    }

    numCntrs = PAPI_num_hw_counters();
    assert(numCntrs > 0);

    papiValues = (long_long*) malloc( sizeof(long_long) * numCntrs);

    papiInitialized = 1;
    return 1;
  }
#endif
  papiInitialized = 0;
  return 0; 
}

void DYNINSTsampleHwCounter(tHwCounter* cntr, int hwCntrIndex) {

#ifdef PAPI
  int ret;


  ret = PAPI_read(papiEventSet, papiValues);
  assert(ret == PAPI_OK);

  cntr->value = papiValues[hwCntrIndex];  

/*
  fprintf(stderr, "MRM_DEBUG: DYNINSTsampleHwCounter()  index %d  value %lld \n", hwCntrIndex, cntr->value);
*/

#endif

}

/************************************************************************
 * void DYNINSTstartHwTimer(tHwTimer* timer)
************************************************************************/
void
DYNINSTstartHwTimer(tHwTimer* timer, int hwCntrIndex) {

#ifdef PAPI
  int ret;


/* For shared-mem sampling only: bump protector1, do work, then bump
   protector2 */
  assert(timer->protector1 == timer->protector2);
  timer->protector1++;
  MEMORY_BARRIER;
  if (timer->counter == 0) {
    ret = PAPI_read(papiEventSet, papiValues);
    assert(ret == PAPI_OK);

    timer->start     =  papiValues[hwCntrIndex];
    //fprintf(stderr, "DYNINSTstartHwTimer()  index: %d  value: %lld \n", hwCntrIndex, timer->start);
  }
  timer->counter++;
  MEMORY_BARRIER;
  timer->protector2++; /* ie. timer->protector2 == timer->protector1 */
  assert(timer->protector1 == timer->protector2);
#endif
}

/************************************************************************
 * void DYNINSTstopHwTimer(tHwTimer* timer)
************************************************************************/
void
DYNINSTstopHwTimer(tHwTimer* timer, int hwCntrIndex) {

#ifdef PAPI

  /* MRM_DEBUG:  this is a hack because for some stupid reason,
  putting the PAPI_read call in the right spot causes a compile error */

  int ret = PAPI_read(papiEventSet, papiValues);

  assert(timer->protector1 == timer->protector2);
  timer->protector1++;
  MEMORY_BARRIER;
  if (timer->counter == 0) {
    /* a strange condition; shouldn't happen.  Should we make it an assert
       fail? */
  }
  else {
    if (timer->counter == 1) {
 
      /*ret = PAPI_read(papiEventSet, papiValues); */
      /*assert(ret == PAPI_OK); */

      const rawTime64 now = papiValues[hwCntrIndex];

      timer->total += (now - timer->start);
      
      if (now < timer->start) {
	fprintf(stderr, "rtinst: cpu timer rollback.\n");
	abort();
      }
    }
    timer->counter--;
  }
  MEMORY_BARRIER;
  timer->protector2++; /* ie. timer->protector2 == timer->protector1 */
  assert(timer->protector1 == timer->protector2);
#endif
}


#ifdef PAPI
void*
DYNINSTpapi_add_event(int eventCode) {

  int retval;

  fprintf(stderr, "MRM_DEBUG: DYNINSTpapi_add_event(0x%x) \n", eventCode);
  

  if (!papiInitialized) {
    fprintf(stderr, "DYNINSTpapi_add_event(): PAPI has not been successfully initialized\n");
    return 0;
  }

  retval = PAPI_add_event(papiEventSet, eventCode);

  if (retval != PAPI_OK) {
    fprintf(stderr, "DYNINSTpapi_add_event(): PAPI_add_event() failed \n");
    return 0;
  }

  return 1;
}

void*
DYNINSTpapi_remove_event(int eventCode) {

  int retval;

  fprintf(stderr, "MRM_DEBUG: DYNINSTpapi_remove_event(0x%x) \n", eventCode);


  if (!papiInitialized) {
    fprintf(stderr, "DYNINSTpapi_remove_event(): PAPI has not been successfully initialized\n");
    return 0;
  }

  retval = PAPI_remove_event(papiEventSet, eventCode);

  if (retval != PAPI_OK) {
    fprintf(stderr, "DYNINSTpapi_remove_event(): PAPI_remove_event() failed \n");
    return 0;
  }

  return 1;
}

void*
DYNINSTpapi_start() {
  
  int retval;

  fprintf(stderr, "MRM_DEBUG: DYNINST_papi_start() \n");

  if (!papiInitialized) {
    fprintf(stderr, "DYNINSTpapi_start(): PAPI has not been successfully initialized\n");
    return 0;
  }

  retval = PAPI_start(papiEventSet);

  if (retval != PAPI_OK) {
    fprintf(stderr, "DYNINSTpapi_start(): PAPI_start() failed \n");
    return 0;
  }

  return 1;
}

void*
DYNINSTpapi_stop() {
  
  int retval;

  fprintf(stderr, "MRM_DEBUG: DYNINST_papi_stop() \n");

  if (!papiInitialized) {
    fprintf(stderr, "DYNINSTpapi_stop(): PAPI has not been successfully initialized\n");
    return 0;
  }

  retval = PAPI_stop(papiEventSet, papiValues);

  if (retval != PAPI_OK) {
    fprintf(stderr, "DYNINSTpapi_stop(): PAPI_start() failed \n");
    return 0;
  }

  return 1;
}
#endif



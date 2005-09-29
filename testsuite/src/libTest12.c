/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: libTest12.c,v 1.1 2005/09/29 20:37:37 bpellin Exp $
#include <stdio.h>
#include <stdlib.h>
#include <dyninstRTExport.h>
#include "test12.h"

/*
  A library of functions used in test12.  
*/

/*  reportMutexCreate
    inserted at mutex init points by the mutator to test the async user messaging 
    mode.
*/
unsigned int nextid = 0;

void reportMutexInit()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = mutex_init;
  msg.tid = (unsigned long) pthread_self();
  //fprintf(stderr, "%s[%d]:  reporting init, thread %lu\n", __FILE__, __LINE__, msg.tid);
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}

/*  reportMuteDestroy()x
    inserted at mutex destroy points by the mutator to test the async user messaging 
    mode.
*/
void reportMutexDestroy()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = mutex_destroy;
  msg.tid = (unsigned long) pthread_self();
  //fprintf(stderr, "%s[%d]:  reporting destroy-%d: thread %lu\n", __FILE__, __LINE__, msg.what,msg.tid);
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}

/*  reportMutexLock()
    inserted at mutex lock points by the mutator to test the async user messaging 
    mode.
*/
void reportMutexLock()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = mutex_lock;
  msg.tid = (unsigned long) pthread_self();
  //fprintf(stderr, "%s[%d]:  reporting lock-%d: thread %lu\n", __FILE__, __LINE__, msg.what, msg.tid);
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}

/*  reportMuteUnlock()
    inserted at mutex unlock points by the mutator to test the async user messaging 
    mode.
*/
void reportMutexUnlock()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = mutex_unlock;
  msg.tid = (unsigned long) pthread_self();
  //fprintf(stderr, "%s[%d]:  reporting unlock-%d\n", __FILE__, __LINE__, msg.what);
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}

/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* $Id: libTest12.c,v 1.1 2008/10/30 19:17:17 legendre Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "test12.h"

/*
  A library of functions used in test12.  
*/


#define ldprintf if (libraryDebug) fprintf
extern int DYNINSTuserMessage(void *, unsigned int);

unsigned int nextid = 0;
int libraryDebug = 0;
unsigned int did_report_entry = 0;
unsigned int did_report_exit = 0;

void reportEntry()
{

  user_msg_t msg;
  msg.id = nextid++;
  msg.what = func_entry;
  msg.tid = /*(unsigned long) pthread_self(); */ getpid();
  ldprintf(stderr, "%s[%d]:  reporting function entry, thread %lu\n", __FILE__, __LINE__, msg.tid);
  if (did_report_entry)
  {
	  fprintf(stderr, "%s[%d]:  WARNING:  calling reportEntry AGAIN\n", __FILE__, __LINE__);
  }
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
  did_report_entry = 1;
}

void reportExit()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = func_exit;
  msg.tid = /*(unsigned long) pthread_self(); */ getpid();
  ldprintf(stderr, "%s[%d]:  reporting function exit, thread %lu\n", __FILE__, __LINE__, msg.tid);
  if (did_report_exit)
  {
	  fprintf(stderr, "%s[%d]:  WARNING:  calling reportEntry AGAIN\n", __FILE__, __LINE__);
  }
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
  did_report_exit = 1;
}

void reportCallsite()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = func_callsite;
  msg.tid = /*(unsigned long) pthread_self(); */ getpid();
  ldprintf(stderr, "%s[%d]:  reporting function callsite, thread %lu\n", __FILE__, __LINE__, msg.tid);
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}

void reportEvent1()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = test3_event1;
  msg.tid = (unsigned long) pthread_self();
  ldprintf(stderr, "%s[%d]:  reporting event 1, thread %lu\n", __FILE__, __LINE__, msg.tid);
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}

void reportEvent2()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = test3_event2;
  msg.tid = (unsigned long) pthread_self();
  ldprintf(stderr, "%s[%d]:  reporting event 2, thread %lu\n", __FILE__, __LINE__, msg.tid);
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}

void reportEvent3()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = test3_event3;
  msg.tid = (unsigned long) pthread_self();
  ldprintf(stderr, "%s[%d]:  reporting event 3, thread %lu\n", __FILE__, __LINE__, msg.tid);
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}

/*  reportMutexCreate
    inserted at mutex init points by the mutator to test the async user messaging 
    mode.
*/

void reportMutexInit()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = mutex_init;
  msg.tid = (unsigned long) pthread_self();
  /* fprintf(stderr, "%s[%d]:  reporting init, thread %lu\n", __FILE__, __LINE__, msg.tid);*/
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
  /* fprintf(stderr, "%s[%d]:  reporting destroy-%d: thread %lu\n", __FILE__, __LINE__, msg.what,msg.tid);*/
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
  /*fprintf(stderr, "%s[%d]:  reporting lock-%d: thread %lu\n", __FILE__, __LINE__, msg.what, msg.tid); */
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
  /*fprintf(stderr, "%s[%d]:  reporting unlock-%d\n", __FILE__, __LINE__, msg.what);*/
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}

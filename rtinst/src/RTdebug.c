/*
 * Copyright (c) 1996 Barton P. Miller
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtinst/h/rtinst.h"
#include "rtinst/h/rtdebug.h"


db_shmArea_t *db_data = NULL;
key_t db_shmKey = 9876;

void db_init(key_t shmkey, int shmsize) {
   int shmid = shmget(shmkey, shmsize, 0666|IPC_CREAT);
   char *memaddr;
   int i;
   fprintf(stderr, "db_init\n");
   if(shmid == -1) {
      perror("Couldn't create shared memory segment");
      exit(1);
   }
   memaddr = shmat(shmid, NULL, 0);
   memset(memaddr, 0, shmsize);

   db_data = (db_shmArea_t *) memaddr;
   db_data->mutex = 0;
   for(i=0; i<DB_NUMPOS; i++)
      db_data->lastIndex[i] = -1;
   fprintf(stderr, "leaving db_init\n");
}

void db_attach(key_t key) {
   int shmid = shmget(key, 0, 0666);
   fprintf(stderr, "db_attach\n");
   if(shmid == -1) {
      perror("Couldn't get shared memory segment");
      exit(1);
   }
   db_data = (db_shmArea_t *)(shmat(shmid, NULL, 0));
   fprintf(stderr, "leaving db_attach\n");
}


/* Call with something like this.
  unsigned pos = DYNINSTthreadPosSLOW(P_thread_self());
  recordTimer(op_stop, pos, timer);
*/

void db_recordTimer(optype op_, int pos, const tTimer *t) {
   while(db_data->mutex == 1);
   assert(pos < DB_NUMPOS);
   int index, circ_index;
   index = ++db_data->lastIndex[pos];
   circ_index = index % DB_MAXRECS;

   db_rec_t *rec = &db_data->historyBuf[pos][circ_index];
   rec->timerAddr = t;
   if(t != 0)
      rec->timerVal = *t;
   rec->op = op_;
}

void db_showTraceB(int pos) {
   int index = db_data->lastIndex[pos];   
   int rctr = 1;
   fprintf(stderr,"  ----- showTrace, pos = %d  ---------------------\n", pos);
   int rnum;
   for(rnum = index % DB_MAXRECS; rnum >= 0; rnum--, rctr++) {
      db_rec_t *rec = &db_data->historyBuf[pos][rnum];
      const tTimer *tm = &rec->timerVal;
      fprintf(stderr, "    %d-  %s, tAddr: 0x%p", rctr, 
              (rec->op==op_start?"START":"STOP"), rec->timerAddr);
      if(rec->timerAddr != 0)
         fprintf(stderr, ", ctr: %d, p1: %d, p2: %d",
                 tm->counter, tm->protector1, tm->protector2);
      fprintf(stderr, "\n");
   }

   if(index > DB_MAXRECS) {
      int circ_index = index % DB_MAXRECS;
      for(rnum = DB_MAXRECS-1; rnum>circ_index; rnum--, rctr++) {
         db_rec_t *rec = &db_data->historyBuf[pos][rnum];      
         const tTimer *tm = &rec->timerVal;
         fprintf(stderr, "    %d-  %s, tAddr: 0x%p", rctr, 
                 (rec->op==op_start?"START":"STOP"), rec->timerAddr);
         if(rec->timerAddr != 0)
            fprintf(stderr, ", ctr: %d, p1: %d, p2: %d",
                    tm->counter, tm->protector1, tm->protector2);
         fprintf(stderr, "\n");
      }
   }
}

void db_showTrace(char *msg) {
   while(db_data->mutex == 1);
   db_data->mutex = 1;
   fprintf(stderr, "======================================================\n");
   fprintf(stderr, "   %s\n", msg);
   int curPos;
   for(curPos=0; curPos<DB_NUMPOS; curPos++) {
      int index = db_data->lastIndex[curPos];
      if(index == -1)  continue;
      db_showTraceB(curPos);
   }
   fprintf(stderr,"=======================================================\n");
   fflush(stderr);
   db_data->mutex = 0;
}




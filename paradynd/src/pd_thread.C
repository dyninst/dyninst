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


#include "paradynd/src/pd_thread.h"
#include "paradynd/src/pd_process.h"


/*
// Useful for debugging.  Keeps a history of last VT_MAXRECS of virtual
// timers.
#define VT_MAXRECS 20
#define VT_NUMPOS 4

typedef struct {
   rawTime64 daemon_now;
   virtualTimer timerVal;
   unsigned daemon_lwp;
} rec_t;

rec_t lastVT[VT_NUMPOS][VT_MAXRECS];
int vt_index[VT_NUMPOS] = { -1, -1, -1, -1 };

void vt_record(unsigned pos, virtualTimer *vt, rawTime64 now, unsigned dlwp) {
   assert(pos < VT_NUMPOS);
   int index, circ_index;
   index = ++vt_index[pos];
   circ_index = index % VT_MAXRECS;

   rec_t *rec = &lastVT[pos][circ_index];
   rec->timerVal = *vt;
   rec->daemon_now = now;
   rec->daemon_lwp = dlwp;
}

void vt_showTraceB(int pos) {
   int index = vt_index[pos];   
   int rctr = 1;
   fprintf(stderr,"  ----- showTrace, pos = %d  ---------------------\n", pos);
   int rnum;
   for(rnum = index % VT_MAXRECS; rnum >= 0; rnum--, rctr++) {
      rec_t *rec = &lastVT[pos][rnum];
      virtualTimer *tm = &rec->timerVal;
      fprintf(stderr, ", tot: %lld, start: %lld, ctr: %d, rt_prev: %lld, "
              "rt_lwp: %d, now: %lld, dmn_lwp: %d\n", tm->total, tm->start, 
              tm->counter, tm->rt_previous, tm->lwp, rec->daemon_now,
              rec->daemon_lwp);
   }

   if(index > VT_MAXRECS) {
      int circ_index = index % VT_MAXRECS;
      for(rnum = VT_MAXRECS-1; rnum>circ_index; rnum--, rctr++) {
         rec_t *rec = &lastVT[pos][rnum];
         virtualTimer *tm = &rec->timerVal;
         fprintf(stderr, ", tot: %lld, start: %lld, ctr: %d, rt_prev: %lld, "
                 "rt_lwp: %d, now: %lld, dmn_lwp: %d\n", tm->total, tm->start, 
                 tm->counter, tm->rt_previous, tm->lwp, rec->daemon_now,
                 rec->daemon_lwp);
      }
   }
}

void vt_showTrace(char *msg) {
   fprintf(stderr, "======================================================\n");
   fprintf(stderr, "   %s\n", msg);
   int curPos;
   for(curPos=0; curPos<VT_NUMPOS; curPos++) {
      int index = vt_index[curPos];
      if(index == -1)  continue;
      vt_showTraceB(curPos);
   }
   fprintf(stderr,"=======================================================\n");
   fflush(stderr);
}

unsigned pos_junk = 101;
*/

rawTime64 pd_thread::getInferiorVtime(virtualTimer *vTimer, bool& success) {
   rawTime64 ret ;
   success = true ;
   
   if (!vTimer) {
      success = false ;
      return 0 ;
   }
   
   dyninst_thread->updateLWP();
   
   volatile const int protector2 = vTimer->protector2;
   
   const int    count = vTimer->counter;
   rawTime64 total, start;
   total = vTimer->total ;
   start = vTimer->start ;
   volatile const int protector1 = vTimer->protector1;
   
   if (protector1 != protector2) {
      success = false ;
      return 0;
   }
   rawTime64 now = 0;
   if (count > 0) {
      // pos_junk = get_pos();
      now = pd_proc->getRawCpuTime(vTimer->lwp);
      if(now == -1) {// error code
         success = false;
         return 0;
      }
      // pos_junk = 101;
      ret = total + (now - start);    
   } else {
      ret = total ;
   }
   // vt_record(get_pos(), vTimer, now, vTimer->lwp);
   return ret ;
}

// Daemon-side resetting of the virtual timer. Easier and 
// safer than running an inferior RPC.
// Thread should be paused when running this!

bool pd_thread::resetInferiorVtime(virtualTimer *vTimer) {
   if (!vTimer) {
     return false;
   }
   fprintf(stderr, "UpdatingLWP\n");
   dyninst_thread->updateLWP();
   fprintf(stderr, "1\n");
   if (vTimer->protector1 != vTimer->protector2)
     return false;

   fprintf(stderr, "2\n");
   vTimer->protector1++;

   fprintf(stderr, "3\n");
   // Stop the timer...
   if (vTimer->counter == 1) {
     fprintf(stderr, "Current lwp: %d\n", vTimer->lwp);
     fprintf(stderr, "pd_proc is %p\n", pd_proc);
     rawTime64 now = pd_proc->getRawCpuTime(vTimer->lwp);
     fprintf(stderr, "Now is %lld\n", now);
     if (now >= vTimer->start) 
       vTimer->total += (now - vTimer->start);
     
     vTimer->lwp = 0;
     vTimer->rt_fd = 0;
   }
   fprintf(stderr, "4\n");
   vTimer->counter--;

   fprintf(stderr, "5\n");
   // Start the timer
   if (vTimer->counter == 0) {
     vTimer->lwp = dyninst_thread->get_lwp()->get_lwp_id();
     vTimer->rt_fd = 0;
     vTimer->start = pd_proc->getRawCpuTime(vTimer->lwp);
     vTimer->rt_previous = vTimer->start;
   }
   fprintf(stderr, "6\n");
   vTimer->counter++;
   fprintf(stderr, "7\n");

   vTimer->protector2++;

   return true;
}


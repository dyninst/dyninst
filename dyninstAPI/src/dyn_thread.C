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

#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"

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

#if defined(MT_THREAD)
rawTime64 dyn_thread::getInferiorVtime(virtualTimer *vTimer,
                                       bool& success) {
  rawTime64 ret ;
  success = true ;

  if (!vTimer) {
    success = false ;
    return 0 ;
  }

  updateLWP();

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
     now = proc->getRawCpuTime(vTimer->lwp);
     // pos_junk = 101;
     ret = total + (now - start);    
  } else {
    ret = total ;
  }
  // vt_record(get_pos(), vTimer, now, vTimer->lwp);
  return ret ;
}
#endif //MT_THREAD 

// We have an LWP handle. Make sure it's still the correct
// one by checking its ID against the one in shared memory

#if !defined(BPATCH_LIBRARY)

bool dyn_thread::updateLWP()
{
  // ST case
  if ((!proc->multithread_ready()) || 
      (pos == (unsigned) -1)) {
    lwp = proc->getDefaultLWP();
    return true;
  }
  
  int lwp_id;
  if (lwp) lwp_id = lwp->get_lwp();
  else lwp_id = 0;
  int vt_lwp = proc->shmMetaData->getVirtualTimer(pos).lwp;

  if (vt_lwp < 0) {
    lwp = NULL; // Not currently scheduled
    return false;
  }
  if (lwp_id == vt_lwp) return true;

  lwp = proc->getLWP(vt_lwp);

  if (!lwp) // Odd, not made yet?
    return false;
  return true;
}

#else

// No shared data, so we can't use the above since the reference
// won't link
bool dyn_thread::updateLWP()
{
  return true;
}
#endif
  

#if !defined(MT_THREAD)
// MT version lives in the <os>MT.C files, and can do things like
// get info for threads not currently scheduled on an LWP
Frame dyn_thread::getActiveFrame()
{
  updateLWP();
  Frame lwpFrame = lwp->getActiveFrame();  
  return Frame(lwpFrame.getPC(), lwpFrame.getFP(),
               lwpFrame.getSP(), lwpFrame.getPID(),
               this, lwpFrame.getLWP(),
               true);
  
  return lwp->getActiveFrame();
}
#endif

// stackWalk: return parameter.
bool dyn_thread::walkStack(vector<Frame> &stackWalk)
{
  // We cheat (a bit): this method is here for transparency, 
  // but the process class does the work in the walkStackFromFrame
  // method. We get the active frame and hand off.
  Frame active = getActiveFrame();
  
  return proc->walkStackFromFrame(active, stackWalk);
}

dyn_lwp *dyn_thread::get_lwp()
{
  if (proc->multithread_ready())
    updateLWP();
  return lwp;
}

void dyn_thread::scheduleIRPC(inferiorRPCtoDo todo)
{
  thrRPCsWaitingToStart += todo;
}

bool dyn_thread::readyIRPC()
{
  return !thrRPCsWaitingToStart.empty();
}

inferiorRPCtoDo dyn_thread::peekIRPC()
{
  return thrRPCsWaitingToStart[0];
}

inferiorRPCtoDo dyn_thread::popIRPC()
{
  inferiorRPCtoDo first;
  first = thrRPCsWaitingToStart[0];
  thrRPCsWaitingToStart.removeOne();
  return first;
}

void dyn_thread::runIRPC(inferiorRPCinProgress running)
{
  thrCurrRunningRPC = running;
}

void dyn_thread::setRunningIRPC()
{
  in_IRPC=true;
}

inferiorRPCinProgress dyn_thread::getIRPC()
{
  return thrCurrRunningRPC;
}

void dyn_thread::clearRunningIRPC()
{
  in_IRPC = false;
}

bool dyn_thread::isRunningIRPC()
{
  return in_IRPC;
}

void dyn_thread::setInSyscall()
{
  in_syscall = true;
}

void dyn_thread::clearInSyscall()
{
  in_syscall = false;
}

bool dyn_thread::isInSyscall()
{
  return in_syscall;
}

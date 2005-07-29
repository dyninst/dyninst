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

#ifndef _BPatch_libInfo_h_
#define _BPatch_libInfo_h_

#include <sys/types.h>
#include "dyninstAPI/h/BPatch_process.h"
#include "common/h/Dictionary.h"
#include "common/h/Types.h"
#include "util.h"

class BPatch_libInfo {
public:
   dictionary_hash<int, BPatch_process *> procsByPid;
   BPatch_libInfo(): procsByPid(intHash) {};
};

class BPatch_funcMap {
   dictionary_hash<const int_function*, BPatch_function*> chart;
   static unsigned hash_bp(const int_function * const &bp ) { 
      return(addrHash4((Address) bp)); 
   }
 public:
   BPatch_funcMap() : chart(hash_bp) {}
   ~BPatch_funcMap() { chart.clear(); }

   bool defines(const int_function *func) 
      { return chart.defines(func); }
   void add(const int_function *func, BPatch_function *bfunc) 
      { chart[func] = bfunc; }
   BPatch_function *get(const int_function *func) 
      { return chart[func]; }
   void map(bool (*f)(BPatch_function *, void *), void *data ) {
      dictionary_hash<const int_function *, BPatch_function *>::iterator iter = 
         chart.begin();
      dictionary_hash<const int_function *, BPatch_function *>::iterator end = 
         chart.end();
      for (; iter != end; ++iter)
         if (!f(*iter, data))
            break;
   }
      
};

class BPatch_instpMap {
  // Note: if we ever have multiple BPatch_points to a single
  // instPoint (frex, a BPatch_point representing a loop backedge and
  // a BPatch_point representing the same edge in the CFG), this will
  // be an instPoint -> vector for BPatch_point map
   dictionary_hash<const instPoint *, BPatch_point *> chart;
   static unsigned hash_ip(const instPoint *const &ip) {
     return (addrHash4((Address)ip));
   }
 public:
   BPatch_instpMap() : chart(hash_ip) {}
   ~BPatch_instpMap() { chart.clear(); }

   bool defines(instPoint *a) { return chart.defines(a); }
   void add(instPoint *a, BPatch_point *bp) { chart[a] = bp; }
   BPatch_point *get(const instPoint *a) { return chart[a]; }
};

#endif /* _BPatch_libInfo_h_ */

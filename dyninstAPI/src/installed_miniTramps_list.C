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

// $Id: installed_miniTramps_list.C,v 1.3 2004/03/23 01:12:05 eli Exp $

#include <iostream>
#include "dyninstAPI/src/miniTrampHandle.h"
#include "dyninstAPI/src/installed_miniTramps_list.h"


miniTrampHandle *miniTramps_list::getLastMT() {
  if(mt_list.isEmpty()) return NULL;

  List<miniTrampHandle*>::iterator cur = mt_list.begin();
  while(cur+1 != mt_list.end()) {
    cur++;
  }
  return *cur;
}

void out_of_store() {
  cerr << "operater new failed: out of store\n";
  assert(false);
}

// returns the address of the new miniTrampHandle (the one to use)
// the miniTrampHandle passed into this function will be copied
//     (ie. &inst doesn't exist in the miniTramps_list)
void miniTramps_list::addMiniTramp(callOrder order,
					     miniTrampHandle *inst) {
  //set_new_handler(out_of_store);
  int befSize = mt_list.count();
  switch(order) {
    case orderFirstAtPoint:
      mt_list.push_front(inst);
     break;
    case orderLastAtPoint:	
      mt_list.push_back(inst);
      break;
    default:
      assert(false);
  }
  assert(mt_list.count() == befSize+1);
}

void miniTramps_list::deleteMiniTramp(miniTrampHandle *inst) {
  assert(mt_list.remove(inst) == true);
}



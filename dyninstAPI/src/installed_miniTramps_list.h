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

// $Id: installed_miniTramps_list.h,v 1.3 2003/10/21 17:22:16 bernat Exp $

#ifndef MINITRAMPS_LIST_H
#define MINITRAMPS_LIST_H

#include "common/h/List.h"
#include "dyninstAPI/src/instP.h"

class miniTrampHandle;

class miniTramps_list {
  List<miniTrampHandle *> mt_list;

 public:
  miniTramps_list() { }
  explicit miniTramps_list(const miniTramps_list &fromList)
    : mt_list(fromList.mt_list) { 
  }
  
  unsigned numMiniTramps() { 
    return (mt_list.isEmpty() ? 0 : mt_list.count());
  }
  // be careful, because these any references to these objects could be
  // invalid if addMiniTramp is called between your getFirstMT/getLastMT call
  // and your use of the miniTrampHandle.  The installedMiniTramps... dictionary
  // of instPoint to installedMiniTramps_list may copy and delete this
  // miniTramps_list when it expands it's internal data.
  miniTrampHandle *getFirstMT() {
    if(mt_list.isEmpty()) { return NULL; }
    return *(mt_list.begin());
  }
  miniTrampHandle *getLastMT();

  List<miniTrampHandle*>::iterator get_begin_iter() {
    return mt_list.begin();
  }
  List<miniTrampHandle*>::const_iterator get_begin_iter() const {
    return mt_list.begin();
  }

  List<miniTrampHandle*>::iterator get_end_iter() {
    return mt_list.end();
  }
  List<miniTrampHandle*>::const_iterator get_end_iter() const {
    return mt_list.end();
  }

  void addMiniTramp(callOrder order, miniTrampHandle *inst);

  void deleteMiniTramp(miniTrampHandle *inst);
  void clear() {
    mt_list.clear();
  }
};



#endif


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

// $Id: installed_miniTramps_list.h,v 1.1 2002/06/26 21:14:12 schendel Exp $

#ifndef INSTALLED_MINITRAMPS_LIST_H
#define INSTALLED_MINITRAMPS_LIST_H

#include "common/h/list.h"
#include "dyninstAPI/src/instP.h"

class instInstance;

class installed_miniTramps_list {
  List<instInstance *> mt_list;

 public:
  installed_miniTramps_list() { }
  explicit installed_miniTramps_list(const installed_miniTramps_list &fromList)
    : mt_list(fromList.mt_list) { 
  }
  
  unsigned numMiniTramps() { 
    return (mt_list.isEmpty() ? 0 : mt_list.count());
  }
  // be careful, because these any references to these objects could be
  // invalid if addMiniTramp is called between your getFirstMT/getLastMT call
  // and your use of the instInstance.  The installedMiniTramps... dictionary
  // of instPoint to installedMiniTramps_list may copy and delete this
  // installed_miniTramps_list when it expands it's internal data.
  instInstance *getFirstMT() {
    if(mt_list.isEmpty()) { return NULL; }
    return *(mt_list.begin());
  }
  instInstance *getLastMT();

  List<instInstance*>::iterator get_begin_iter() {
    return mt_list.begin();
  }
  List<instInstance*>::const_iterator get_begin_iter() const {
    return mt_list.begin();
  }

  List<instInstance*>::iterator get_end_iter() {
    return mt_list.end();
  }
  List<instInstance*>::const_iterator get_end_iter() const {
    return mt_list.end();
  }

  void addMiniTramp(callOrder order, instInstance *inst);

  void deleteMiniTramp(instInstance *inst);
  void clear() {
    mt_list.clear();
  }
};



#endif


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

// graphicalPath.h
// Ariel Tamches

/* $Id: graphicalPath.h,v 1.5 2004/03/23 01:12:30 eli Exp $ */

#ifndef _GRAPHICAL_PATH_H_
#define _GRAPHICAL_PATH_H_

#include "simpSeq.h"
#include "where4treeConstants.h"
#include "where4tree.h"

template <class ROOTTYPE>
class whereNodeGraphicalPath {
 public:
   enum pathEndsIn {ExpandedNode, ListboxItem, ListboxScrollbarUpArrow,
		    ListboxScrollbarDownArrow, ListboxScrollbarPageup,
		    ListboxScrollbarPagedown, ListboxScrollbarSlider,
		    Nothing};
 private:
   simpSeq<unsigned> thePath;

   pathEndsIn whatThePathEndsIn;

   // If the path ends in a listbox item, then the following coords merely refer to
   // the (center,top) of the listbox's parent; otherwise, they refer to the (center,
   // top) of the pink node, as they should.  Such a determination is made at
   // constructor time; if a listbox node subsequently becomes expanded, these values
   // will become invalid.  Be careful.
   int endpath_centerx, endpath_topy;

 public:
   whereNodeGraphicalPath(int point_x, int point_y,
			  const where4TreeConstants &,
			  const where4tree<ROOTTYPE> *rootPtr,
			  int root_centerx, int root_topy);
   whereNodeGraphicalPath(const simpSeq<unsigned> &iPath,
			  const where4TreeConstants &,
			  const where4tree<ROOTTYPE> *rootPtr,
			  int root_centerx, int root_topy);
  ~whereNodeGraphicalPath() {}

   whereNodeGraphicalPath &operator=(const whereNodeGraphicalPath &other);

   bool operator==(const whereNodeGraphicalPath &other) const;
      // note: this routine does _not_ take into account endpath_centerx/endpath_topy;
      //       I felt that comparing the actual path elements was more useful;
      //       comparing x/y could return false "too easily" --ari

   unsigned getSize() const {return thePath.getSize();}

   pathEndsIn whatDoesPathEndIn() const {return whatThePathEndsIn;}

   int get_endpath_centerx() const {return endpath_centerx;}
   int get_endpath_topy()    const {return endpath_topy;}

   simpSeq<unsigned> &getPath() {return thePath;}
   const simpSeq<unsigned> &getPath() const {return thePath;}
   operator simpSeq<unsigned> &() {return getPath();}
   operator const simpSeq<unsigned> &() const {return getPath();}

   where4tree<ROOTTYPE> *
   getLastPathNode(where4tree<ROOTTYPE> *root) {
      return root->get_end_of_path(thePath);
   }
   const where4tree<ROOTTYPE> *
   getLastPathNode(const where4tree<ROOTTYPE> *root) const {
      return root->get_end_of_path(thePath);
   }

   where4tree<ROOTTYPE> *
   getParentOfLastPathNode(where4tree<ROOTTYPE> *root) {
      return root->get_parent_of_end_of_path(thePath);
   }
//   const where4tree<ROOTTYPE> *
//   getParentOfLastPathNode(const where4tree<ROOTTYPE> *root) const {
//      return root->get_parent_of_end_of_path(thePath);
//   }
};

#endif

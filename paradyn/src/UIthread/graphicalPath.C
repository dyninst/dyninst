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

/* $Id: graphicalPath.C,v 1.8 2004/03/23 01:12:30 eli Exp $ */

#include "common/h/headers.h"
#include "graphicalPath.h"

template <class ROOTTYPE>
whereNodeGraphicalPath<ROOTTYPE>::
whereNodeGraphicalPath(const whereNodePosRawPath &iPath,
		       const where4TreeConstants &tc,
		       const where4tree<ROOTTYPE> *rootPtr,
		       int root_centerx, int root_topy) : thePath(iPath) {
   if (rootPtr==NULL) {
      whatThePathEndsIn = Nothing;
      return;
   }

   if (iPath.getSize() == 0) {
      // special case when there exists no parent node (the root)
      whatThePathEndsIn = ExpandedNode;
      endpath_centerx = root_centerx; endpath_topy = root_topy;
      return;
   }

   const where4tree<ROOTTYPE> *ptr = rootPtr;
   for (unsigned i=0; i < iPath.getSize()-1; i++) {
      unsigned childindex = iPath[i];
      int child_centerx = root_centerx -
                          ptr->horiz_pix_everything_below_root(tc) / 2 +
                          ptr->horiz_offset_to_expanded_child(tc, childindex) +
                          ptr->getChildTree(childindex)->entire_width(tc) / 2;
      int child_topy = root_topy + ptr->getNodeData().getHeightAsRoot() +
                       tc.vertPixParent2ChildTop;

      ptr = ptr->getChildTree(childindex);

      root_centerx = child_centerx;
      root_topy = child_topy;
   }

   // Now we are located at the parent of the last item in the path.
   // This gives us an opportunity to see whether or not the last item is
   // in a listbox or not.
   unsigned childindex = iPath.getLastItem();
   bool allChildrenExpanded = (ptr->getListboxPixWidth() == 0);
   bool childIsExpanded = allChildrenExpanded || ptr->getChildIsExpandedFlag(childindex);
   if (childIsExpanded) {
      whatThePathEndsIn = ExpandedNode;
      endpath_centerx = root_centerx -
	                ptr->horiz_pix_everything_below_root(tc) / 2 +
	                ptr->horiz_offset_to_expanded_child(tc, childindex) +
			ptr->getChildTree(childindex)->entire_width(tc) / 2;
      endpath_topy = root_topy + ptr->getNodeData().getHeightAsRoot() +
	             tc.vertPixParent2ChildTop;
   }
   else {
      whatThePathEndsIn = ListboxItem;
      endpath_centerx = root_centerx;
      endpath_topy = root_topy;
   }
}

template <class ROOTTYPE>
whereNodeGraphicalPath<ROOTTYPE>::
whereNodeGraphicalPath(int point_x, int point_y,
		       const where4TreeConstants &tc,
		       const where4tree<ROOTTYPE> *rootPtr,
		       int root_centerx, int root_topy) {
   if (rootPtr==NULL) {
      whatThePathEndsIn = Nothing;
      return;
   }

   while (true) {
      int theItemLocation = rootPtr->point2ItemOneStep(tc, point_x, point_y,
						       root_centerx, root_topy);

      endpath_centerx = root_centerx; endpath_topy = root_topy;

      if (theItemLocation == -1)
         // point in the root
         whatThePathEndsIn = ExpandedNode;
      else if (theItemLocation >=0 && theItemLocation < (int)rootPtr->getNumChildren()) {
         unsigned childindex = theItemLocation;

         bool allChildrenExpanded = (rootPtr->getListboxPixWidth() == 0);
         bool itemIsExpanded = allChildrenExpanded || rootPtr->getChildIsExpandedFlag(childindex);

         thePath.append(childindex);

         if (!itemIsExpanded) {
            // listbox item.  no recursion needed; we've found our endpoint
            endpath_centerx = root_centerx; endpath_topy = root_topy;
               // yes, these are coords of the root of the listbox, but I can't
               // think of anything better to put here. (or should be use the coords
               // of the (left,top) or (center,top) of the listbox?)
            whatThePathEndsIn = ListboxItem;
            return;
         }

         // Time to "recurse".
         const where4tree<ROOTTYPE> *childptr = rootPtr->getChildTree(childindex);
      
         int child_centerx = root_centerx -
	                     rootPtr->horiz_pix_everything_below_root(tc) / 2 +
                             rootPtr->horiz_offset_to_expanded_child(tc, childindex) +
                             childptr->entire_width(tc) / 2;
         int child_topy = root_topy + rootPtr->getNodeData().getHeightAsRoot() +
                          tc.vertPixParent2ChildTop;

         // set up "recursion" parameters:
         root_centerx = child_centerx;
         root_topy = child_topy;
         rootPtr = childptr;

         continue;
      }
      else if (theItemLocation == -2)
         whatThePathEndsIn = Nothing;
      else if (theItemLocation == -3)
         whatThePathEndsIn = ListboxScrollbarUpArrow;
      else if (theItemLocation == -4)
         whatThePathEndsIn = ListboxScrollbarDownArrow;
      else if (theItemLocation == -5)
         whatThePathEndsIn = ListboxScrollbarPageup;
      else if (theItemLocation == -6)
         whatThePathEndsIn = ListboxScrollbarPagedown;
      else if (theItemLocation == -7)
         whatThePathEndsIn = ListboxScrollbarSlider;
      else
         assert(false);

      return;
   }
}

template <class ROOTTYPE>
whereNodeGraphicalPath<ROOTTYPE> &
whereNodeGraphicalPath<ROOTTYPE>::operator=
(const whereNodeGraphicalPath<ROOTTYPE> &other) {
   thePath = other.thePath; // operator=
   whatThePathEndsIn = other.whatThePathEndsIn;
   endpath_centerx = other.endpath_centerx;
   endpath_topy = other.endpath_topy;

   return *this;
}


template <class ROOTTYPE>
bool whereNodeGraphicalPath<ROOTTYPE>::
operator==(const whereNodeGraphicalPath<ROOTTYPE> &other) const {
   // note: this routine does _not_ take into account endpath_centerx/endpath_topy;
   //       I felt that comparing the actual path elements was more useful;
   //       comparing x/y could return false "too easily" --ari
   if (whatThePathEndsIn != other.whatThePathEndsIn)
      return false;

   return (thePath==other.thePath);
}

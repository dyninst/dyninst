// graphicalPath.h
// Ariel Tamches

#include "graphicalPath.h"

template <class USERNODEDATA>
whereNodeGraphicalPath<USERNODEDATA>::
whereNodeGraphicalPath(const whereNodePosRawPath &iPath,
		       const where4TreeConstants &tc,
		       const where4tree<USERNODEDATA> *rootPtr,
		       int root_centerx, int root_topy) : thePath(iPath) {
   if (iPath.getSize() == 0) {
      // special case when there exists no parent node (the root)
      whatThePathEndsIn = ExpandedNode;
      endpath_centerx = root_centerx; endpath_topy = root_topy;
      return;
   }

   const where4tree<USERNODEDATA> *ptr = rootPtr;
   for (unsigned i=0; i < iPath.getSize()-1; i++) {
      unsigned childindex = iPath[i];
      int child_centerx = root_centerx -
                          ptr->horiz_pix_everything_below_root(tc) / 2 +
                          ptr->horiz_offset_to_expanded_child(tc, childindex) +
                          ptr->getChildTree(childindex)->entire_width(tc) / 2;
      int child_topy = root_topy + ptr->getRootNode().getHeight() +
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
      endpath_topy = root_topy + ptr->getRootNode().getHeight() +
	             tc.vertPixParent2ChildTop;
   }
   else {
      whatThePathEndsIn = ListboxItem;
      endpath_centerx = root_centerx;
      endpath_topy = root_topy;
   }
}

template <class USERNODEDATA>
whereNodeGraphicalPath<USERNODEDATA>::
whereNodeGraphicalPath(int point_x, int point_y,
		       const where4TreeConstants &tc,
		       const where4tree<USERNODEDATA> *rootPtr,
		       int root_centerx, int root_topy) {
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
         const where4tree<USERNODEDATA> *childptr = rootPtr->getChildTree(childindex);
      
         int child_centerx = root_centerx -
	                     rootPtr->horiz_pix_everything_below_root(tc) / 2 +
                             rootPtr->horiz_offset_to_expanded_child(tc, childindex) +
                             childptr->entire_width(tc) / 2;
         int child_topy = root_topy + rootPtr->getRootNode().getHeight() +
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



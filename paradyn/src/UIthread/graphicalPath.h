// graphicalPath.h
// Ariel Tamches

#ifndef _GRAPHICAL_PATH_H_
#define _GRAPHICAL_PATH_H_

#include "simpSeq.h"
#include "where4treeConstants.h"
#include "where4tree.h"

template <class USERNODEDATA>
class whereNodeGraphicalPath {
 private:
   simpSeq<USERNODEDATA> thePath;

   // If the path ends in a listbox item, then the following coords merely refer to
   // the (center,top) of the listbox's parent; otherwise, they refer to the (center,
   // top) of the pink node, as they should.  Such a determination is made at
   // constructor time; if a listbox node subsequently becomes expanded, these values
   // will become invalid.  Be careful.
   int endpath_centerx, endpath_topy;

 public:
   enum pathEndsIn {ExpandedNode, ListboxItem, ListboxScrollbarUpArrow,
		    ListboxScrollbarDownArrow, ListboxScrollbarPageup,
		    ListboxScrollbarPagedown, ListboxScrollbarSlider,
		    Nothing};
 private:
   pathEndsIn whatThePathEndsIn;

 public:
   whereNodeGraphicalPath(int point_x, int point_y,
			  const where4TreeConstants &,
			  const where4tree<USERNODEDATA> *rootPtr,
			  int root_centerx, int root_topy);
   whereNodeGraphicalPath(const simpSeq<USERNODEDATA> &iPath,
			  const where4TreeConstants &,
			  const where4tree<USERNODEDATA> *rootPtr,
			  int root_centerx, int root_topy);
  ~whereNodeGraphicalPath() {}

   unsigned getSize() const {return thePath.getSize();}

   pathEndsIn whatDoesPathEndIn() const {return whatThePathEndsIn;}

   int get_endpath_centerx() const {return endpath_centerx;}
   int get_endpath_topy()    const {return endpath_topy;}

   simpSeq<USERNODEDATA> &getPath() {return thePath;}
   const simpSeq<USERNODEDATA> &getPath() const {return thePath;}
   operator simpSeq<USERNODEDATA> &() {return getPath();}
   operator const simpSeq<USERNODEDATA> &() const {return getPath();}

   where4tree<USERNODEDATA> *
   getLastPathNode(where4tree<USERNODEDATA> *root) {
      return root->get_end_of_path(thePath);
   }
   const where4tree<USERNODEDATA> *
   getLastPathNode(const where4tree<USERNODEDATA> *root) const {
      return root->get_end_of_path(thePath);
   }

   where4tree<USERNODEDATA> *
   getParentOfLastPathNode(where4tree<USERNODEDATA> *root) {
      return root->get_parent_of_end_of_path(thePath);
   }
//   const where4tree<USERNODEDATA> *
//   getParentOfLastPathNode(const where4tree<USERNODEDATA> *root) const {
//      return root->get_parent_of_end_of_path(thePath);
//   }
};

#endif

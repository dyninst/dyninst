// graphicalPath.h
// Ariel Tamches

/* $Log: graphicalPath.h,v $
/* Revision 1.2  1995/10/17 20:53:56  tamches
/* class where4tree is templated in a different way.
/* Added operator=().
/*
 */

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

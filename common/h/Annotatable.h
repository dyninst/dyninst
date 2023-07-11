/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __ANNOTATABLE_H__
#define __ANNOTATABLE_H__

#if defined (MSC_VER)
#define DYN_DETAIL_BOOST_NO_INTRINSIC_WCHAR_T 1
#endif
#include "dyntypes.h"
#include <stddef.h>
#include <vector>
#include <map>
#include <typeinfo>
#include <string>
#include <string.h> // for strrchr()
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "compiler_annotations.h"

namespace Dyninst
{

COMMON_EXPORT bool annotation_debug_flag();
COMMON_EXPORT int annotatable_printf(const char *format, ...)
	DYNINST_PRINTF_ANNOTATION(1, 2);

typedef unsigned short AnnotationClassID;
typedef bool (*anno_cmp_func_t)(void *, void*);

extern int newAnnotationClass();
extern bool void_ptr_cmp_func(void *, void *);

class COMMON_EXPORT AnnotationClassBase
{
   private:
      static std::vector<AnnotationClassBase *> *annotation_types;
      static dyn_hash_map<std::string, AnnotationClassID> *annotation_ids_by_name;
      anno_cmp_func_t cmp_func;
      AnnotationClassID id;
      std::string name;

   protected:

     AnnotationClassBase(std::string n, 
                                       anno_cmp_func_t cmp_func_ = NULL);

     virtual ~AnnotationClassBase(); 

   public:

       static AnnotationClassBase *findAnnotationClass(unsigned int id);
       static void dumpAnnotationClasses();

       AnnotationClassID getID() { return id; }
       std::string &getName() {return name;}
       anno_cmp_func_t getCmpFunc() {return cmp_func;}
	   virtual const char *getTypeName() = 0;
	   virtual void *allocate() = 0;
};

template <class T> 
class AnnotationClass : public AnnotationClassBase {
   public:

	  AnnotationClass(std::string n, 
			  anno_cmp_func_t cmp_func_ = NULL) :
		  AnnotationClassBase(n, cmp_func_)
	  {
	  }

	  const char *getTypeName() { return typeid(T).name();}
	  void *allocate() DYNINST_MALLOC_ANNOTATION
	  {
		  return (void *) new T();
	  }

	  size_t size() {return sizeof(T);}
#if 0
	  bool isSparselyAnnotatable(); 
	  bool isDenselyAnnotatable(); 
#endif
};


typedef enum {
	    sparse,
		dense
} sparse_or_dense_anno_t;

typedef struct {
	AnnotationClassBase *acb;
	void *data;
	void *parent_id;
	sparse_or_dense_anno_t sod;
} ser_rec_t;

typedef enum {
	sp_add_anno = 2,
	sp_rem_anno = 3,
	sp_add_cont_item = 4,
	sp_rem_cont_item = 5
} ser_post_op_t;

COMMON_EXPORT const char *serPostOp2Str(ser_post_op_t);

class COMMON_EXPORT AnnotatableDense
{
	typedef void *anno_list_t;

	/**
    * Inheriting from this class adds a pointer to each object.  Multiple
    * types of annotations are stored under this pointer in a 
    * annotation_type -> anno_list_t map.
    **/

   private:

      typedef anno_list_t anno_map_t;

      struct aInfo {
         anno_map_t *data;
         AnnotationClassID max;
      };

      aInfo *annotations;

      bool addAnnotation(const void *a, AnnotationClassID id) 
	  {
		  if (annotation_debug_flag())
		  {
			  fprintf(stderr, "%s[%d]:  Dense(%p) add %s-%d\n", FILE__, __LINE__, (void*)this, 
					  AnnotationClassBase::findAnnotationClass(id) 
					  ? AnnotationClassBase::findAnnotationClass(id)->getName().c_str() 
					  : "bad_anno_id", id);
		  }

         unsigned size = id + 1;
         if (!annotations)
         {
            annotations = (aInfo *) malloc(sizeof(aInfo));
			annotations->data = NULL;
		 }

		 //  can have case where we have allocated annotations struct but not the
		 //  actual annotations data array in case where we have performed serialization

		 if (annotations->data == NULL) 
		 {
			annotations->data = (anno_list_t *) calloc(sizeof(anno_list_t), (size));
			annotations->max = size;
			for (unsigned i=0; i<size; i++)
				annotations->data[i] = NULL;
		 } 
		 else if (id >= annotations->max) 
		 {
			 int old_max = annotations->max;
			 size = annotations->max * 2;
			 annotations->max = size;
			 annotations->data = (anno_list_t *) realloc(annotations->data, sizeof(anno_list_t) * size);
			 for (unsigned i=old_max; i<size; i++)
				 annotations->data[i] = NULL;
		 }

		 annotations->data[id] = const_cast<void *>(a);

         return true;
      }
   public:
      AnnotatableDense() : annotations(NULL)
      {
      }

	  ~AnnotatableDense()
	  {
		  if (annotations)
		  {
			  if (annotations->data)
				  free(annotations->data);
			  free(annotations);
		  }
	  }

	  AnnotatableDense(const AnnotatableDense&) = default;

	  AnnotatableDense& operator=(const AnnotatableDense& rhs)
	  {
		  if (this != &rhs)
		  {
			  if (annotations)
			  {
				  if (annotations->data)
					  free(annotations->data);
				  free(annotations);
			  }

			  if (rhs.annotations)
			  {
				  annotations = (aInfo *) malloc(sizeof(aInfo));
				  unsigned size = rhs.annotations->max;
				  annotations->max = size;
				  annotations->data = (anno_list_t *)calloc(sizeof(anno_list_t), (size));
				  memcpy(annotations->data, rhs.annotations->data, size * sizeof(anno_list_t));
			  }  else  {
				  annotations = NULL;
			  }
		  }

		  return *this;
	  }

      template<class T> 
      bool addAnnotation(const T *a, AnnotationClass<T> &a_id) 
      {
		  if (annotation_debug_flag())
		  {
			  fprintf(stderr, "%s[%d]:  Dense(%p):  Add %s-%d, %s\n", FILE__, __LINE__, 
					  (void*)this, a_id.getName().c_str(), a_id.getID(), typeid(T).name());
		  }

         int id = a_id.getID();
		 T *a_noconst = const_cast<T *>(a);
		 bool ret = addAnnotation((void *)a_noconst, id);
		 if (!ret)
		 {
			 fprintf(stderr, "%s[%d]:  failed to add annotation\n", FILE__, __LINE__);
			 return ret;
		 }
		 return true;
	  }


      template<class T> 
      inline bool getAnnotation(T *&a, AnnotationClass<T> &a_id) const
      {
         if (!annotations)
            return false;

         int id = a_id.getID();

         if (id > annotations->max) 
         {
            return false;
         }

         a = (T *) annotations->data[id];
         if (!a) return false;

         return true;
      }

      template<class T> 
      inline bool removeAnnotation(AnnotationClass<T> &a_id)
      {
		  if (annotation_debug_flag())
		  {
			  fprintf(stderr, "%s[%d]:  Dense(%p) remove %s-%d, %s\n", FILE__, __LINE__, 
					  (void*)this, a_id.getName().c_str(), a_id.getID(), a_id.getTypeName());
		  }

         if (!annotations) return false;

         int id = a_id.getID();
         if (id > annotations->max) 
         {
            return false;
         }

         if (!annotations->data[id]) 
            return false;

         annotations->data[id] = NULL;

         return true;
      }

	  void annotationsReport()
	  {
		  std::vector<AnnotationClassBase *> atypes;
		  if (annotations && annotations->data)
		  {
			  for (unsigned int i = 0; i < annotations->max; ++i)
			  {
				  if (NULL != annotations->data[i])
				  {
					  AnnotationClassBase *acb = AnnotationClassBase::findAnnotationClass(i);
					  if (!acb)
					  {
						  fprintf(stderr, "%s[%d]:  ERROR:  failed to find acb for %u\n", 
								  FILE__, __LINE__, i);
						  continue;
					  }
					  else
						  atypes.push_back(acb);
				  }
			  }

			  fprintf(stderr, "%s[%d]:  Dense(%p):  have %lu annotations\n", 
				  FILE__, __LINE__, (void*)this, (unsigned long) atypes.size());

			  for (unsigned int i = 0; i < atypes.size(); ++i)
			  {
				  fprintf(stderr, "\t%s-%d, %s\n", atypes[i]->getName().c_str(), 
						  atypes[i]->getID(), atypes[i]->getTypeName());
			  }
		  }
	  }
};

#define NON_STATIC_SPARSE_MAP 1
#define AN_INLINE inline

class COMMON_EXPORT AnnotatableSparse
{
   public:
      struct void_ptr_hasher
      {
         size_t operator()(const void* a) const noexcept
         {
            return (size_t) a;
         }
      };

#if defined (_MSC_VER)
      typedef dyn_hash_map<void *, void *> annos_by_type_t;
#else
      typedef dyn_hash_map<void *, void *, void_ptr_hasher> annos_by_type_t;
#endif

      typedef std::vector<annos_by_type_t *> annos_t;

	  AnnotatableSparse() = default;

	  ~AnnotatableSparse()
	  {
		  ClearAnnotations("dtor");
	  }

	  AnnotatableSparse(const AnnotatableSparse&) = default;

	  AnnotatableSparse& operator=(const AnnotatableSparse &rhs)
	  {
		  if (this != &rhs)
		  {
			  ClearAnnotations("operator=");
		  }

		  return *this;
	  }

   private:
	  void ClearAnnotations(const char *reason)
	  {
		  //  We need to remove annotations from the static map when objects
		  //  are destroyed:  (1)  memory may be reclaimed and reused at the same
		  //  place, and (2) regardless of 1, the map can possibly explode to 
		  //  unmanageable sizes, with a lot of unused junk in it if a lot of i
		  //  annotatable objects are created and destroyed.

		  //  Alas this is kinda expensive right now, but the data structure is
		  //  set up to minimize search time, not deletion time.  It could
		  //  be changed if this becomes a significant time drain.

		  for (unsigned int i = 0; i < getAnnos()->size(); ++i)
		  {
			  annos_by_type_t *abt = (*getAnnos())[i];
			  if (!abt) continue;

			  annos_by_type_t::iterator iter = abt->find(this);
			  if (iter != abt->end())
			  {
				  if (annotation_debug_flag())
				  {
					  fprintf(stderr, "%s[%d]:  Sparse(%p) %s remove %s-%u\n", FILE__, __LINE__,  
							  (void*)this, reason, AnnotationClassBase::findAnnotationClass(i) 
							  ? AnnotationClassBase::findAnnotationClass(i)->getName().c_str() 
							  : "bad_anno_id", i);
				  }

				  abt->erase(iter);

				  //  get rid of this check...  just making sure that erase is behaving as
				  //  expected...
				  annos_by_type_t::iterator iter2 = abt->find(this);
				  if (iter2 != abt->end())
					  fprintf(stderr, "%s[%d]:  FIXME:  REMOVE FAILED\n", FILE__, __LINE__);
			  }
		  }
	  }

#if defined (NON_STATIC_SPARSE_MAP)
      //COMMON_EXPORT static annos_t *annos;
#else
      static annos_t annos;
#endif
	  annos_t *getAnnos() const;
	  static dyn_hash_map<void *, unsigned short> ser_ndx_map;

      annos_by_type_t *getAnnosOfType(AnnotationClassID aid, bool do_create =false) const
	  {
		  annos_t &l_annos = *getAnnos();
         long nelems_to_create = aid - l_annos.size() + 1;

         if (nelems_to_create > 0)
         {
            if (!do_create)
            {
               return NULL;
            }

            while (nelems_to_create)
            {
               annos_by_type_t *newl = new annos_by_type_t();
               l_annos.push_back(newl);
               nelems_to_create--;
            }
         }

         annos_by_type_t *abt = l_annos[aid];

         return abt;
      }

      template <class T>
      AN_INLINE annos_by_type_t *getAnnosOfType(AnnotationClass<T> &a_id, bool do_create =false) const
      {
         AnnotationClassID aid = a_id.getID();

		 return getAnnosOfType(aid, do_create);
	  }


      void *getAnnosForObject(annos_by_type_t *abt,
            void *obj, bool do_create = false) const 
      {
         assert(abt);
         assert(obj);

         void  *target = NULL;

         annos_by_type_t::iterator iter = abt->find(obj);

         if (iter == abt->end())
         {
            if (!do_create)
            {
               return NULL;
            }

            (*abt)[obj] = target;
         }
         else
         {
            target = iter->second;
         }

         return target;
      }

	  AN_INLINE bool addAnnotation(const void *a, AnnotationClassID aid)
	  {
		  if (annotation_debug_flag())
		  {
			  fprintf(stderr, "%s[%d]:  Sparse(%p) add %s-%d void\n", FILE__, __LINE__, (void*)this, 
					  AnnotationClassBase::findAnnotationClass(aid) 
					  ? AnnotationClassBase::findAnnotationClass(aid)->getName().c_str() 
					  : "bad_anno_id", aid);
		  }

		  void *obj = this;
		  annos_by_type_t *abt = getAnnosOfType(aid, true /*do create if needed*/);
		  assert(abt);

		  annos_by_type_t::iterator iter = abt->find(obj);
		  if (iter == abt->end())
		  {
			  (*abt)[obj] = const_cast<void *>(a);
		  }
		  else
		  {
			  //  do silent replacement -- this case can arise if an annotatable object
			  //  is destroyed and then reallocated as a new object at the same address.
			  //  Given the sparse map data structure, it would be rather expensive to 
			  //  go through the map in the dtor to eliminate references to the 
			  //  destroyed object.

			  //  Added annotation removal in dtor...  so this case should _not_ arise,
			  //  do the replacement, but make some noise:

			  if (a != iter->second)
			  {
				  annotatable_printf("%s[%d]:  WEIRD:  already have annotation of this type: %p, replacing with %p\n", FILE__, __LINE__, (void*)iter->second, (const void*)a);
				  iter->second = const_cast<void *>(a);
			  }

			  return true;
		  }

		  return true;
	  }

   public:

	  bool operator==(AnnotatableSparse &cmp)
	  {
		  annos_t &l_annos = *getAnnos();
		  unsigned this_ntypes = l_annos.size();
         unsigned cmp_ntypes = cmp.getAnnos()->size();
         unsigned ntypes = (cmp_ntypes > this_ntypes) ? cmp_ntypes : this_ntypes;

         for (unsigned int i = 0; i < ntypes; ++i) 
         {
            if ((i >= cmp_ntypes) || (i >= this_ntypes)) 
            {
               //  compare is done since at least one set of annotations
               //  has been exhausted
               break;
            }

            annos_by_type_t *this_abt = l_annos[i];
            annos_by_type_t *cmp_abt = (*cmp.getAnnos())[i];

            if (!this_abt) 
            {
               fprintf(stderr, "%s[%d]:  WEIRD: FIXME\n", FILE__, __LINE__);
               continue;
            }

            if (!cmp_abt) 
            {
               fprintf(stderr, "%s[%d]:  WEIRD: FIXME\n", FILE__, __LINE__);
               continue;
            }

            annos_by_type_t::iterator this_abt_iter = this_abt->find(this);
            annos_by_type_t::iterator cmp_abt_iter = cmp_abt->find(&cmp);

            //  if one has annotations of this particular type and other other
            //  doesn't, then we are def. not equal, so fail:

            if (this_abt_iter == this_abt->end())
            {
               if (cmp_abt_iter != cmp_abt->end())
               {
                  return false;
               }

               //  both are at end()
               continue;
            }

            if (   (cmp_abt_iter == cmp_abt->end())
                  && (this_abt_iter != this_abt->end()))
            {
               return false;
            }

            AnnotationClassBase *acb = AnnotationClassBase::findAnnotationClass(i);

            if (!acb)
            {
               return false;
            }

            //  both have annotation -- do the compare
            anno_cmp_func_t cmpfunc = acb->getCmpFunc();

            if (!cmpfunc)
            {
               //  even if not explicitly specified, a default pointer-compare
               //  function should be returned here.

               fprintf(stderr, "%s[%d]:  no cmp func for anno id %u\n", 
                     FILE__, __LINE__, i);
               return false;
            }

            void *arg1 = cmp_abt_iter->second;
            void *arg2 = this_abt_iter->second;

            bool ret = (*cmpfunc)(arg1, arg2);

            return ret;
         }

         return true;
      }

      template<class T>
      AN_INLINE bool addAnnotation(const T *a, AnnotationClass<T> &a_id)
         {
		  annotatable_printf("%s[%d]:  Sparse(%p):  Add %s-%d, %s\n", FILE__, __LINE__, 
				  (void*)this, a_id.getName().c_str(), a_id.getID(), typeid(T).name());
            void *obj = this;
            annos_by_type_t *abt = getAnnosOfType(a_id, true /*do create if needed*/);
            assert(abt);

            annos_by_type_t::iterator iter = abt->find(obj);
            if (iter == abt->end())
            {
               (*abt)[obj] = (void *) const_cast<T *>(a);
            }
            else
            {
				//  do silent replacement -- this case can arise if an annotatable object
				//  is destroyed and then reallocated as a new object at the same address.
				//  Given the sparse map data structure, it would be rather expensive to 
				//  go through the map in the dtor to eliminate references to the 
				//  destroyed object.

				//  Added annotation removal in dtor...  so this case should _not_ arise,
				//  do the replacement, but make some noise:

				if (iter->second != a) 
				{
					//fprintf(stderr, "%s[%d]:  WEIRD:  already have annotation of type %s: %p, replacing with %p\n", FILE__, __LINE__, a_id.getName().c_str(), iter->second, a);
					iter->second = (void *)const_cast<T *>(a);
				}

				return true;
            }
				
            return true;
         }

      template<class T>
      AN_INLINE bool getAnnotation(T *&a, AnnotationClass<T> &a_id) const 
      {
         a = NULL;

         annos_by_type_t *abt = getAnnosOfType(a_id, false /*don't create if none*/);

         if (!abt)
         {
            return false;
         }

         AnnotatableSparse * this_noconst = const_cast<AnnotatableSparse *>(this);
         void *annos_for_object = getAnnosForObject(abt, (void *)this_noconst,
               false /*no create if none*/);

         if (!annos_for_object)
         {
            return false;
         }

         a = (T *)annos_for_object;
         return true;
      }

	  template<class T>
	  inline bool removeAnnotation(AnnotationClass<T> &a_id)
	  {
		  if (annotation_debug_flag())
		  {
			  fprintf(stderr, "%s[%d]:  Sparse(%p) remove %s-%d, %s\n", FILE__, __LINE__,
					  (void*)this, a_id.getName().c_str(), a_id.getID(), typeid(T).name());
		  }

		  void *obj = this;
		  annos_by_type_t *abt = getAnnosOfType(a_id, false /*do create if needed*/);
		  assert(abt);

		  annos_by_type_t::iterator iter = abt->find(obj);
		  if (iter == abt->end())
		  {
			  //  annotation does not exist, so we return false (remove failed)
			  return false;
		  }
		  else
		  {
			  abt->erase(iter);
			  return true;
		  }
	      return false;
	  }

	  void annotationsReport()
	  {
		  std::vector<AnnotationClassBase *> atypes;
		  annos_t &l_annos = *getAnnos();

		  for (AnnotationClassID id = 0; id < l_annos.size(); ++id)
		  {
			  annos_by_type_t *abt = getAnnosOfType(id, false /*don't do create */);
			  if (NULL == abt) continue;

			  annos_by_type_t::iterator iter = abt->find(this);

			  if (iter == abt->end()) 
			  {
				  //	fprintf(stderr, "%s[%d]:  nothing for this obj\n", FILE__, __LINE__);
				  continue;
			  }

			  AnnotationClassBase *acb =  AnnotationClassBase::findAnnotationClass(id);
			  if (!acb)
			  {
				  fprintf(stderr, "%s[%d]:  FIXME, cant find anno class base for id %d\n", 
						  FILE__, __LINE__, id);
				  continue;
			  }
			  atypes.push_back(acb);
		  }

		  fprintf(stderr, "%s[%d]:  Sparse(%p):  have %lu annos:\n", FILE__, __LINE__, 
			  (void*)this, (unsigned long) atypes.size());
		  for (unsigned int i = 0; i < atypes.size(); ++i)
		  {
			  fprintf(stderr, "\t%s-%d, %s\n", atypes[i]->getName().c_str(), 
					  atypes[i]->getID(), atypes[i]->getTypeName());
		  }
	  }

};

} // namespace

#endif

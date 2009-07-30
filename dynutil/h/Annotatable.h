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

#ifndef __ANNOTATABLE_H__
#define __ANNOTATABLE_H__

#include <vector>
#include <map>
#include <typeinfo>
#include <string>
#include <string.h> // for strrchr()
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "dyntypes.h"
#include "util.h"
#include "Serialization.h"

namespace Dyninst
{

typedef unsigned short AnnotationClassID;
typedef bool (*anno_cmp_func_t)(void *, void*);

extern int newAnnotationClass();
extern bool void_ptr_cmp_func(void *, void *);

typedef void (*ser_func_t) (void *, SerializerBase *, const char *);
void ser_func_wrapper(void *it, SerializerBase *sb,  const char *tag);

class AnnotationClassBase
{
   private:
      static std::vector<AnnotationClassBase *> *annotation_types;
      static dyn_hash_map<std::string, AnnotationClassID> *annotation_ids_by_name;
      anno_cmp_func_t cmp_func;
      AnnotationClassID id;
      std::string name;

   protected:

	  ser_func_t serialize_func;

      COMMON_EXPORT AnnotationClassBase(std::string n, 
            anno_cmp_func_t cmp_func_ = NULL);

	  COMMON_EXPORT virtual ~AnnotationClassBase() {}
   public:

      static AnnotationClassBase *findAnnotationClass(unsigned int id);

      anno_cmp_func_t getCmpFunc()
      {
         return cmp_func;
      }

      AnnotationClassID getID() { return id; }
      std::string &getName() {return name;}

	  static ser_func_t getSerFuncForID(AnnotationClassID id)
	  {
		  if (id >= (AnnotationClassID) annotation_types->size())
		  {
			  fprintf(stderr, "%s[%d];  requested out-of-range annotation id!!\n", 
					  FILE__, __LINE__);
			  return NULL;
		  }

		  AnnotationClassBase *acb = (*annotation_types)[id];

		  if (acb)
		  {
			  return acb->getSerializeFunc();
		  }

		  return NULL;
	  }

	  virtual const char *getTypeName() = 0;

	  ser_func_t getSerializeFunc()
	  {
		  return serialize_func;
	  }

};

template <class T>
struct ser_func_for_serializable {
	ser_func_t operator()(void) {return NULL;}
};

template <>
struct ser_func_for_serializable<Serializable> {
	ser_func_t operator()(void) {return ser_func_wrapper;}
};

template <class T> 
class AnnotationClass : public AnnotationClassBase {
   public:

	  AnnotationClass(std::string n, 
			  anno_cmp_func_t cmp_func_ = NULL, 
			  ser_func_t s = NULL) :
		  AnnotationClassBase(n, cmp_func_)
	  {
		if (NULL == s)
		{
			//  if the type is Serializable, use its serialization function
			//  otherwise, leave it NULL so we don't accidentally dereference
			//  a random pointer as if it were automaitcally descended from
			//  Serializable

			ser_func_for_serializable<T> sfselector;
			serialize_func = sfselector();
		}
	  }

	  const char *getTypeName() { return typeid(T).name();}
};

typedef void *anno_list_t;

class AnnotatableDense
{
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

   public:
      AnnotatableDense() : annotations(NULL)
      {
      }

      template<class T> 
      bool addAnnotation(const T *a, AnnotationClass<T> &a_id) 
      {
         unsigned size;
         int id = a_id.getID();

         if (!annotations) 
         {
			 size = id+1;
            annotations = (aInfo *) malloc(sizeof(aInfo));

			annotations->data = (anno_list_t *) calloc(sizeof(anno_list_t *), (size));
			annotations->max = size;
			for (unsigned i=0; i<size; i++)
				annotations->data[i] = NULL;
		 } 
		 else if (id >= annotations->max) 
		 {
			 int old_max = annotations->max;
			 size = annotations->max * 2;
			 annotations->max = size;
			 annotations->data = (anno_list_t *) realloc(annotations->data, sizeof(anno_list_t *) * size);
			 for (unsigned i=old_max; i<size; i++)
				 annotations->data[i] = NULL;
		 }

		 annotations->data[id] = (void *) a;

         return true;
      }

      template<class T> 
      bool getAnnotation(T *&a, AnnotationClass<T> &a_id) const
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
      bool removeAnnotation(AnnotationClass<T> &a_id)
      {
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

	  void serializeAnnotations(SerializerBase *sb, const char *tag)
	  {
		  //  iterator over possible annotation types
		  //  if we have any, lookup the serialization function and call it
		  for (AnnotationClassID id = 0; id < annotations->max; ++id)
		  {
			  void *anno = annotations->data[id];
			  if (anno)
			  {
				  ser_func_t sf = AnnotationClassBase::getSerFuncForID(id);
				  if (NULL != sf)
				  {
					  (*sf)(anno, sb, tag);
				  }
			  }
		  }
	  }
};

class AnnotatableSparse
{
   public:
      struct void_ptr_hasher
      {
         size_t operator()(const void* a) const
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

   private:

      COMMON_EXPORT static annos_t annos;

      annos_by_type_t *getAnnosOfType(AnnotationClassID aid, bool do_create =false) const
	  {
         long nelems_to_create = aid - annos.size() + 1;

         if (nelems_to_create > 0)
         {
            if (!do_create)
            {
               return NULL;
            }

            while (nelems_to_create)
            {
               annos_by_type_t *newl = new annos_by_type_t();
               annos.push_back(newl);
               nelems_to_create--;
            }
         }

         annos_by_type_t *abt = annos[aid];

         return abt;
      }

      template <class T>
      annos_by_type_t *getAnnosOfType(AnnotationClass<T> &a_id, bool do_create =false) const
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

   public:
      bool operator==(AnnotatableSparse &cmp)
      {
         unsigned this_ntypes = annos.size();
         unsigned cmp_ntypes = cmp.annos.size();
         unsigned ntypes = (cmp_ntypes > this_ntypes) ? cmp_ntypes : this_ntypes;

         for (unsigned int i = 0; i < ntypes; ++i) 
         {
            if ((i >= cmp_ntypes) || (i >= this_ntypes)) 
            {
               //  compare is done since at least one set of annotations
               //  has been exhausted
               break;
            }

            annos_by_type_t *this_abt = annos[i];
            annos_by_type_t *cmp_abt = cmp.annos[i];

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
               fprintf(stderr, "%s[%d]:  cannot find annotation class for id %d\n", 
                     FILE__, __LINE__, i);
               return false;
            }

            //  both have annotation -- do the compare
            anno_cmp_func_t cmpfunc = acb->getCmpFunc();

            if (!cmpfunc)
            {
               //  even if not explicitly specified, a default pointer-compare
               //  function should be returned here.

               fprintf(stderr, "%s[%d]:  no cmp func for anno id %d\n", 
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
      bool addAnnotation(const T *a, AnnotationClass<T> &a_id)
         {
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
               //  if the annotation already exists and is identical (by pointer, of course)
               //  what is the best solution?  Replacement makes no sense, since it is the
               //  same.  The question is -- do we fail and report this situation as a logic
               //  error?  This is probably the most conservative and hence safest approach,
               //  but, since the pointer is identical, let's play nice and just report 
               //  success?

               return true;

               //  maybe want to do (silent) replace here?
            }

            return true;
         }

      template<class T>
      bool getAnnotation(T *&a, AnnotationClass<T> &a_id) const 
      {
         a = NULL;

         annos_by_type_t *abt = getAnnosOfType(a_id, false /*don't create if none*/);

         if (!abt)
         {
            //fprintf(stderr, "%s[%d]:  no annotations of type %s\n",
            //      FILE__, __LINE__, a_id.getName().c_str());
            return false;
         }

         AnnotatableSparse * this_noconst = const_cast<AnnotatableSparse *>(this);
         void *annos_for_object = getAnnosForObject(abt, (void *)this_noconst,
               false /*no create if none*/);

         if (!annos_for_object)
         {
            //fprintf(stderr, "%s[%d]:  no annotations of type %s\n", 
            //      FILE__, __LINE__, a_id->getName().c_str());
            return false;
         }

         a = (T *)annos_for_object;
         return true;
      }

	  template<class T>
	  bool removeAnnotation(AnnotationClass<T> &a_id)
	  {
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

	  void serializeAnnotations(SerializerBase *sb, const char *tag)
	  {
            void *obj = this;
			for (AnnotationClassID id = 0; id < annos.size(); ++id)
			{
				annos_by_type_t *abt = getAnnosOfType(id, false /*don't do create */);
				if (NULL == abt) continue;

				annos_by_type_t::iterator iter = abt->find(obj);
				if (iter == abt->end()) continue;

				//  we have an annotation of this type for this object, find the serialization
				//  function and call it (if it exists)

				ser_func_t sf = AnnotationClassBase::getSerFuncForID(id);

				if (NULL != sf) 
				{
					(*sf)(iter->second, sb, tag);
				}
			}
	  }
};

} // namespace

#endif

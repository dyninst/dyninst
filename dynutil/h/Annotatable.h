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
#include "boost/type_traits/is_base_of.hpp"

namespace Dyninst
{

typedef unsigned short AnnotationClassID;
typedef bool (*anno_cmp_func_t)(void *, void*);

extern int newAnnotationClass();
extern bool void_ptr_cmp_func(void *, void *);

class SerializerBase;
class Serializable;
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

      COMMON_EXPORT static AnnotationClassBase *findAnnotationClass(unsigned int id);
      COMMON_EXPORT static void dumpAnnotationClasses();

      COMMON_EXPORT AnnotationClassID getID() { return id; }
      COMMON_EXPORT std::string &getName() {return name;}
      COMMON_EXPORT anno_cmp_func_t getCmpFunc() {return cmp_func;}
	  COMMON_EXPORT ser_func_t getSerializeFunc() {return serialize_func;}
	  COMMON_EXPORT virtual const char *getTypeName() = 0;
	  COMMON_EXPORT virtual void *allocate() = 0;

#if 0
	  //  Probably can delete this func?
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
#endif

};

#if 0
template <class T>
struct ser_func_for_serializable {
	ser_func_t operator()(void) 
	{
		fprintf(stderr, "%s[%d]:  NULL func for serializable, type = %s\n", FILE__, __LINE__, typeid(T).name());
		return NULL;
	}
};

template <>
struct ser_func_for_serializable<Serializable> {
	ser_func_t operator()(void) 
	{
		fprintf(stderr, "%s[%d]:  got func for Serializable\n", FILE__, __LINE__ );
		return ser_func_wrapper;
	}
};
#endif

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
			//  a random pointer as if it were automatcally descended from
			//  Serializable

			if (boost::is_base_of<Serializable, T>::value)
			{
				//fprintf(stderr, "%s[%d]:  got func for Serializable\n", FILE__, __LINE__ );
				serialize_func = ser_func_wrapper;
			}
#if 0
			else
			{
				fprintf(stderr, "%s[%d]:  NULL func for serializable, type = %s\n", 
						FILE__, __LINE__, typeid(T).name());
			}
#endif
#if 0
			T *my_test_ptr;
			void *my_test_void_ptr = (void *) my_test_ptr;
			if (NULL == dynamic_cast<Serializable *>(my_test_void_ptr))
			{
				fprintf(stderr, "%s[%d]:  NULL func for serializable, type = %s\n", FILE__, __LINE__, typeid(T).name());
			}
			else
			{
				fprintf(stderr, "%s[%d]:  got func for Serializable\n", FILE__, __LINE__ );
				serialize_func = ser_func_wrapper;
			}
#endif
#if 0
			ser_func_for_serializable<T> sfselector;
			serialize_func = sfselector();
#endif
#if 0
			if (NULL == serialize_func)
			{
				fprintf(stderr, "%s[%d]:  no serialize func for annotation type %s\n", FILE__, __LINE__, n.c_str());
			}
#endif
		}
	  }

	  const char *getTypeName() { return typeid(T).name();}
	  void *allocate() {return (void *) new T();}
	  void *size() {return sizeof(T);}
};

typedef void *anno_list_t;

typedef struct {
	AnnotationClassBase *acb;
	void *data;
} ser_rec_t;

COMMON_EXPORT bool is_input(SerializerBase *sb);
COMMON_EXPORT bool is_output(SerializerBase *sb);
class AnnotatableDense;
class AnnotatableSparse;
COMMON_EXPORT bool add_annotations(SerializerBase *, AnnotatableDense *, std::vector<ser_rec_t> &);
COMMON_EXPORT bool add_annotations(SerializerBase *, AnnotatableSparse *, std::vector<ser_rec_t> &);
bool serialize_annotation_list(void *, std::vector<ser_rec_t> &, SerializerBase *sb, const char *);

class AnnotatableDense
{
	friend bool add_annotations(SerializerBase *, AnnotatableDense *, std::vector<ser_rec_t> &);
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

	  //  private version of addAnnotation exists for deserialize functions
	  //  to reconstruct annotations without explicit type info
      bool addAnnotation(const void *a, AnnotationClassID id) 
	  {
         unsigned size;
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

		 annotations->data[id] = const_cast<void *>(a);

         return true;
      }
   public:
      AnnotatableDense() : annotations(NULL)
      {
      }

      template<class T> 
      bool addAnnotation(const T *a, AnnotationClass<T> &a_id) 
      {
         int id = a_id.getID();
		 return addAnnotation((void *)a, id);
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
		  fprintf(stderr, "%s[%d]:  welcome to serializeAnotations:\n", FILE__, __LINE__);
		  //  iterator over possible annotation types
		  //  if we have any, lookup the serialization function and call it

		  int nelem = 0;
		  std::vector<ser_rec_t> my_sers;
		  if (is_output(sb))
		  {
			  //  need to figure out how many annotations will be serialized apriori
			  //  so we can output the size of the list as a header.
			  //  To avoid iterating over the full list twice, make a local copy
			  //  of all serializations/deserializations that need to be performed
			  //  as we go, and do them in bulk afterwards.
			  for (AnnotationClassID id = 0; id < annotations->max; ++id)
			  {
				  void *anno = annotations->data[id];
				  if (anno)
				  {
					  AnnotationClassBase *acb = AnnotationClassBase::findAnnotationClass(id);
					  if (!acb)
					  {
						  fprintf(stderr, "%s[%d]:  FIXME:  no annotation class for id %d\n", 
								  FILE__, __LINE__);
						  continue;
					  }
					  ser_func_t sf = acb->getSerializeFunc();
					  if (NULL != sf)
					  {
						  ser_rec_t sr;
						  sr.data = anno;
						  sr.acb = acb;
						  my_sers.push_back(sr);
					  }
				  }
			  }
			  nelem = my_sers.size();
		  }

		  if (!serialize_annotation_list(this, my_sers, sb, tag))
		  {
			  fprintf(stderr, "%s[%d]:  FIXME:  failed to serialize annotation list\n", 
					  FILE__, __LINE__);
		  }
	        if (!add_annotations(sb, this, my_sers))
			{
				fprintf(stderr, "%s[%d]:  failed to update annotation list after deserialize\n", FILE__, __LINE__);
			}
#if 0
		  for (AnnotationClassID id = 0; id < annotations->max; ++id)
		  {
			  void *anno = annotations->data[id];
			  if (anno)
			  {
				  AnnotationClassBase *acb = AnnotationClassBase::findAnnotationClass(id);
				  if (!acb)
				  {
					  fprintf(stderr, "%s[%d]:  FIXME:  no annotation class for id %d\n", FILE__, __LINE__);
					  continue;
				  }
				  //ser_func_t sf = AnnotationClassBase::getSerFuncForID(id);
				  ser_func_t sf = acb->getSerializeFunc();
				  if (NULL != sf)
				  {
					  (*sf)(anno, sb, tag);
				  }
			  }
		  }
#endif
	  }
};

class AnnotatableSparse
{
	friend bool add_annotations(SerializerBase *, AnnotatableSparse *, std::vector<ser_rec_t> &);
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
      inline annos_by_type_t *getAnnosOfType(AnnotationClass<T> &a_id, bool do_create =false) const
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

	  //  private version of addAnnotation used by deserialize function to restore
	  //  annotation set without explicitly specifying types
      inline bool addAnnotation(const void *a, AnnotationClassID aid)
         {
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
      inline bool addAnnotation(const T *a, AnnotationClass<T> &a_id)
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
      inline bool getAnnotation(T *&a, AnnotationClass<T> &a_id) const 
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
	  inline bool removeAnnotation(AnnotationClass<T> &a_id)
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
		  fprintf(stderr, "%s[%d]:  welcome to serializeAnotations:\n", FILE__, __LINE__);

		  std::vector<ser_rec_t> my_sers;
            void *obj = this;
			if (is_output(sb))
			{
				for (AnnotationClassID id = 0; id < annos.size(); ++id)
				{
					annos_by_type_t *abt = getAnnosOfType(id, false /*don't do create */);
					if (NULL == abt) continue;

					//fprintf(stderr, "%s[%d]:  serializeAnotations: %d annos for id %d\n", FILE__, __LINE__, abt->size(), id);
					annos_by_type_t::iterator iter = abt->find(obj);
					if (iter == abt->end()) {
						//	fprintf(stderr, "%s[%d]:  nothing for this obj\n", FILE__, __LINE__);
						continue;
					}

					//  we have an annotation of this type for this object, find the serialization
					//  function and call it (if it exists)

					AnnotationClassBase *acb =  AnnotationClassBase::findAnnotationClass(id);
					if (!acb)
					{
						fprintf(stderr, "%s[%d]:  FIXME, cannot find annotation class base for id %d, mode = %s\n", FILE__, __LINE__, id, is_input(sb) ? "deserialize" : "serialize");
						continue;
					}
					//ser_func_t sf = AnnotationClassBase::getSerFuncForID(id);
					ser_func_t sf = acb->getSerializeFunc();

					if (NULL == sf)
					{
						fprintf(stderr, "%s[%d]:  no serialization function for this anno\n", FILE__, __LINE__);
						continue;
					}
					//fprintf(stderr, "%s[%d]:  serializing annotation %s\n", FILE__, __LINE__, acb->getName().c_str());
					ser_rec_t sr;
					sr.acb = acb;
					sr.data = iter->second;
					my_sers.push_back(sr);
					//(*sf)(iter->second, sb, tag);
				}
				fprintf(stderr, "%s[%d]:  found %d req'd serializations for obj %p\n", 
						FILE__, __LINE__, my_sers.size(), this);
			}

			fprintf(stderr, "%s[%d]:  before serialize_annotation_list: %d annos\n", FILE__, __LINE__, my_sers.size());
			if (!serialize_annotation_list(this, my_sers, sb, tag))
			{
				fprintf(stderr, "%s[%d]:  FIXME:  failed to serialize annotation list\n", 
						FILE__, __LINE__);
			}
			//friend bool add_annotations(SerializerBase *, AnnotatableSparse *, std::vector<ser_rec_t> &);
			if (!add_annotations(sb, this, my_sers))
			{
				fprintf(stderr, "%s[%d]:  failed to update annotation list after deserialize\n", FILE__, __LINE__);
			}
	  }
};

} // namespace

#endif

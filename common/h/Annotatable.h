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

// $Id: Annotatable.h,v 1.6 2008/01/16 22:00:55 legendre Exp $

#ifndef _ANNOTATABLE_
#define _ANNOTATABLE_

#include <vector>
#include <string>
#include <typeinfo>
#include "headers.h"
#include "util.h"

#if 0
extern unsigned addrHashCommon(Address addr);
#endif
class AnnotatableBase;
#if defined(os_windows)
#else
namespace __gnu_cxx {
   template<> struct hash<AnnotatableBase *> {
      hash<char*> h;
      unsigned operator()(const AnnotatableBase *b) const {
         return addrHashCommon((Address)b);
      };
   };
};
#endif

#if 0
#include "serialize.h"
#endif

using std::vector;

class AnnotatableBase;

template<class T>
class AnnotationSet {
   static hash_map<AnnotatableBase *, T*> sets_by_obj;

   public:
   AnnotationSet() {}
   ~AnnotationSet() {}

   static T *findAnnotationSet(AnnotatableBase *b) 
   {
      if (sets_by_obj.find(b) == sets_by_obj.end())
         return NULL;
      return sets_by_obj[b];
   }

   static T *getAnnotationSet(AnnotatableBase *b)
   {
      T *it = NULL;

      if (NULL == (it = findAnnotationSet(b))) {
         it = new T();
         sets_by_obj[b] = it;
      }

      return it;
   }

   static bool removeAnnotationSet(AnnotatableBase *b)
   {
      typename hash_map<AnnotatableBase *, T *>::iterator iter;
      iter = sets_by_obj.find(b);
      if (iter == sets_by_obj.end())
         return false;
      delete iter->second;
      sets_by_obj.erase(iter);

      return true;
   }
};

template< class T > hash_map<AnnotatableBase*, T* >
AnnotationSet< T >::sets_by_obj;


class AnnotatableBase
{
   protected:
      DLLEXPORT AnnotatableBase();
      ~AnnotatableBase() {
      }
      static int number;
      static hash_map<std::string, int> annotationTypes;
      static hash_map<std::string, int> metadataTypes;
      static int metadataNum;

   public:

      DLLEXPORT int createAnnotationType(std::string &name);
      DLLEXPORT int getAnnotationType(std::string &name);
#if 0
      virtual int createMetadata(std::string &name);
      virtual int getMetadata(std::string &name);
#endif

};

template <class T, class ANNOTATION_NAME_T>
class Annotatable : public AnnotatableBase
{
   public:
   Annotatable() :
      AnnotatableBase()
   {
   }
   ~Annotatable() 
   {
   }

   Annotatable(const Annotatable<T, ANNOTATION_NAME_T> &/*src*/) :
      AnnotatableBase()
   {/*hrm deep copy here or no?*/}

   typedef typename std::vector<T> Container_t;
   Container_t *initAnnotations()
   {
      Container_t *v = NULL;
      v = AnnotationSet<Container_t>::getAnnotationSet(this);
      if (v) {
#if 0
         fprintf(stderr, "%s[%d]:  annotation set already existsfor %p\n", FILE__, __LINE__, this);
#endif
         return v;
      }

      if (!v) {
         fprintf(stderr, "%s[%d]:  malloc problem\n", FILE__, __LINE__);
         abort();
         return NULL;
      }

      ANNOTATION_NAME_T name_t;
      std::string anno_name = typeid(name_t).name();
      int anno_type = getAnnotationType(anno_name);
      if (anno_type == -1) {
         fprintf(stderr, "%s[%d]:  creating annotation type %s\n", 
               FILE__, __LINE__, anno_name.c_str());
         anno_type = createAnnotationType(anno_name);
      }

      if (anno_type == -1) {
         AnnotationSet<Container_t>::removeAnnotationSet(this);
         delete v;
         fprintf(stderr, "%s[%d]:  failing to create annotation for type %s\n", 
               FILE__, __LINE__, anno_name.c_str());
         return NULL;
      }
      fprintf(stderr, "%s[%d]:  created annotation type %s\n", 
            FILE__, __LINE__, anno_name.c_str());
      return v;
   }

   bool addAnnotation(T it)
   {
      Container_t *v = NULL;
      v = AnnotationSet<std::vector<T> >::findAnnotationSet(this);
      if (!v)
         if (NULL == ( v = initAnnotations())) {
            fprintf(stderr, "%s[%d]:  bad annotation type\n", FILE__, __LINE__);
            return false;
         }

      if (!v) {
         fprintf (stderr, "%s[%d]:  initAnnotations failed\n", FILE__, __LINE__);
         return false;
      }
      v->push_back(it);

      return true;
   }

   unsigned size() const {
      Container_t *v = NULL;
      
      const Annotatable<T, ANNOTATION_NAME_T> *thc = this; 
      Annotatable<T, ANNOTATION_NAME_T> *th  
         = const_cast<Annotatable<T, ANNOTATION_NAME_T> *> (thc);
      v = AnnotationSet<Container_t>::findAnnotationSet(th);
      if (!v) return 0;
      return v->size();
   }

   //  so called getDataStructure in case we generalize beyond vectors for annotations
   std::vector<T> &getDataStructure() {
      Container_t *v = NULL;
      v = AnnotationSet<Container_t>::findAnnotationSet(this);
      if (!v)
         if (NULL == (v = initAnnotations())) {
            fprintf(stderr, "%s[%d]:  failed to init annotations here\n", 
                  FILE__, __LINE__);
            assert(0);
         }
      return *v;
   }

   T &getAnnotation(unsigned index) const
   {
      Container_t *v = NULL;
      const Annotatable<T, ANNOTATION_NAME_T> *thc = this; 
      Annotatable<T, ANNOTATION_NAME_T> *th  
         = const_cast<Annotatable<T, ANNOTATION_NAME_T> *> (thc);
      v = AnnotationSet<Container_t>::findAnnotationSet(th);
      assert(v);
      assert(index < v->size());
      return (*v)[index];
   }

   T &operator[](unsigned index) const {return getAnnotation(index);}

};

#endif

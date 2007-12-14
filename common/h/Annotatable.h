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

// $Id: Annotatable.h,v 1.3 2007/12/14 04:16:47 jaw Exp $

#ifndef _ANNOTATABLE_
#define _ANNOTATABLE_

#include <vector>
#include <string>
#include <typeinfo>
#include "headers.h"

#if 0
#include "serialize.h"
#endif

#if 0
#if defined (os_windows)
#include <hash_map>
using stdext::hash_map;
#else
#include <ext/hash_map>
#include <ext/hash_set>
using namespace __gnu_cxx;
namespace __gnu_cxx {
   template<> struct hash<std::string> {
      hash<char*> h; 
      unsigned operator()(const std::string &s) const {
         return h(s.c_str());
      };
   };
};

#endif
#endif


using std::vector;

#if 0
class AnnotationBase {
  public:
     AnnotationBase() {}
     virtual ~AnnotationBase() {}
};

template <class T>
class Annotation : public AnnotationBase{
  T* item;
 public:
  Annotation(T* it) : item(it) {}
  void setItem(T* newItem) { item=newItem; }
  T* getItem() {return item; }
};
#endif

class AnnotatableBase
{
   protected:
      DLLEXPORT AnnotatableBase();
      virtual ~AnnotatableBase() {}
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
   vector<T> *annotations;
   public:
   Annotatable() :
      AnnotatableBase(),
     annotations(NULL) 
   {
   }
   virtual ~Annotatable() 
   {
      if (annotations) {
         annotations->clear();
         delete annotations;
      }
   }

   Annotatable(const Annotatable<T, ANNOTATION_NAME_T> &src) :
      AnnotatableBase(),
      annotations(src.annotations)
   {/*hrm deep copy here or no?*/}

   bool initAnnotations()
   {
      if (annotations) return true;
      annotations = new std::vector<T>();
      if (!annotations) {
         fprintf(stderr, "%s[%d]:  malloc problem\n", FILE__, __LINE__);
         abort();
         return false;
      }

      ANNOTATION_NAME_T name_t;
      std::string anno_name = typeid(name_t).name();
      int anno_type = getAnnotationType(anno_name);
      if (anno_type == -1) {
         anno_type = createAnnotationType(anno_name);
      }

      if (anno_type == -1) {
         delete annotations;
         annotations = NULL;
         fprintf(stderr, "%s[%d]:  failing to create annotation for type %s\n", 
               FILE__, __LINE__, anno_name.c_str());
         return false;
      }
      return true;
   }

#if 0
   bool serialize_annotation(typename ANNOTATION_NAME_T::SerializerType *s, T &it)
   {
      void ANNOTATION_NAME_T::SerializerType::annotation_start(const char *string_id, const char *tag);
      void ANNOTATION_NAME_T::SerializerType::annotation_end(const char *tag);
      void ANNOTATION_NAME_T::SerializerType::translate_base(T &param, const char *tag);
      ANNOTATION_NAME_T name_t;
      std::string anno_name = typeid(name_t).name();
      translate_annotation(s, it, anno_name.c_str());
     return true;
   }
   bool addAnnotation(T it, std::vector<typename ANNOTATION_NAME_T::SerializerType *> *svp = NULL)
#endif

   bool addAnnotation(T it)
   {
      if (!annotations)
         if (!initAnnotations()) {
            fprintf(stderr, "%s[%d]:  bad annotation type\n", FILE__, __LINE__);
            return false;
         }

      annotations->push_back(it);

#if 0
      if (svp) {
         std::vector<typename ANNOTATION_NAME_T::SerializerType *> &sv = *svp;
         for (unsigned int i = 0; i < sv.size(); ++i) {
            typename ANNOTATION_NAME_T::SerializerType *s = sv[i];
            assert(s);
            serialize_annotation(s, it);
         }
      }
      else {
         fprintf(stderr, "%s[%d]:  no serializers for annotation\n", FILE__, __LINE__);
      }
#endif
      return true;
   }

#if 0
   bool serializeAll(std::vector<typename ANNOTATION_NAME_T::SerializerType *> *svp) 
   {
      if (!annotations) {
         //  Do we really want to do nothing here, or do we want to write out a null set?
         return true; //  Nothing should be fine.
      }

      if (!svp) {
         fprintf(stderr, "%s[%d]:  FIXME:  serializeAll called without any serializers\n", 
               FILE__, __LINE__);
      }

      std::vector<typename ANNOTATION_NAME_T::SerializerType *> &sv = *svp;
      if (sv.size()) {
         fprintf(stderr, "%s[%d]:  FIXME:  serializeAll called without any serializers\n", 
               FILE__, __LINE__);
      }
      for (unsigned int i = 0; i < annotations->size(); ++i) {
         T &it = (*annotations)[i];
        for (unsigned int j = 0; j < sv.size(); ++j) {
            typename ANNOTATION_NAME_T::SerializerType *s = sv[j];
            assert(s);
            translate_annotation(s, it, NULL, NULL);
        }
      }
      return true;
   }
#endif

   unsigned size() const {
      if (!annotations) return 0;
      return annotations->size();
   }

   //  so called getDataStructure in case we generalize beyond vectors for annotations
   std::vector<T> &getDataStructure() {
      if (!annotations)
         if (!initAnnotations()) {
            fprintf(stderr, "%s[%d]:  failed to init annotations here\n", FILE__, __LINE__);
            assert(0);
         }
      return *annotations;
   }

   T &getAnnotation(unsigned index) const
   {
      assert(annotations);
      assert(index < annotations->size());
      return (*annotations)[index];
   }

   T &operator[](unsigned index) const {return getAnnotation(index);}

};


#if 0
template <class SERIALIZER, class T>
void translate_annotation(SERIALIZER *ser, T *it, const char *anno_str, const char *tag = NULL)
{
   ser->annotation_start(anno_str, tag);
   if (ser->iomode() == sd_serialize) {
      ser->translate_base(*it, tag);
   }
   ser->annotation_end();
}

template <class SERIALIZER, class T>
void translate_annotation(SERIALIZER *ser, T &it, const char *anno_str, const char *tag = NULL)
{
   ser->annotation_start(anno_str, tag);
   if (ser->iomode() == sd_serialize) {
      ser->translate_base(it, tag);
   }
   ser->annotation_end();
}
#endif

#endif

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

// $Id: Annotatable.C,v 1.9 2008/09/03 06:08:44 jaw Exp $

#include "common/h/headers.h"
#include "common/h/serialize.h"
#include "dynutil/h/Annotatable.h"
#include "dynutil/h/dyntypes.h"

using namespace Dyninst;

int AnnotatableBase::number;
int AnnotatableBase::metadataNum;
dyn_hash_map<std::string, AnnotatableBase::anno_info_t> AnnotatableBase::annotationTypes;
dyn_hash_map<std::string, int> AnnotatableBase::metadataTypes;
dyn_hash_map<std::string, std::string> AnnotatableBase::annotypes_to_typenames;
std::string AnnotatableBase::emptyString = std::string("");

DLLEXPORT AnnotatableBase::AnnotatableBase() 
{
   number = -1;
}

SerializerBase *getSDAnno(std::string &anno_name)
{
   SerializerBase *ret =  AnnotatableBase::getAnnotationSerializer(anno_name);

   if (!ret) {
      fprintf(stderr, "%s[%d]:  getAnnotationSerializer (%s) failed\n", 
            FILE__, __LINE__, anno_name.c_str());
   }

   return ret;
}

void dumpSDAnno()
{
   AnnotatableBase::dumpAnnotationSerializers();
}

#if defined (cap_serialization)

int AnnotatableBase::createAnnotationType(std::string &name, 
      const char *tname, SerializerBase *sb) 

#else

int AnnotatableBase::createAnnotationType(std::string &name, 
      const char *tname, SerializerBase *) 

#endif
{
   std::string n(name);
   int num = getAnnotationType(name, tname);

   if (num != -1) {
      //  also check to see if we have set the tname and serializer fields

#if defined (cap_serialization)
      if (sb) 
      {
         dyn_hash_map<std::string, anno_info_t>::iterator iter = annotationTypes.find(n); 

         if (iter != annotationTypes.end()) {
            AnnotatableBase::anno_info_t &anno_rec = iter->second;

            if (!anno_rec.anno_serializer) {
               fprintf(stderr, "%s[%d]:  assigning serializer for type %s to %p\n",
                     FILE__, __LINE__, name.c_str(), sb);
               anno_rec.anno_serializer = sb;
               fprintf(stderr, "%s[%d]:  assigning serializer for type %s to %s\n",
                     FILE__, __LINE__, name.c_str(), sb->name().c_str());
            }

            assert(num == anno_rec.anno_id);
         }
      }
#endif

      return num;
   }

   number++;
   AnnotatableBase::anno_info_t anno_rec;
   anno_rec.anno_id = number;

#if defined (cap_serialization)
   fprintf(stderr, "%s[%d]:  assigning serializer for type %s to %p\n", 
         FILE__, __LINE__, name.c_str(), sb);
   anno_rec.anno_serializer = sb;
#endif

   annotationTypes[n] = anno_rec;

#if defined (cap_serialization)
   assert(annotationTypes[n].anno_serializer == sb);
#endif
   
   if (!tname) {
      //  This should be OK, so long as we init the typename later on when we
      //  call getOrCreate...  if we get an annotation with an empty type string
      //  we should just fill it in.
      fprintf(stderr, "%s[%d]:  FIXME:  NULL typename here\n", FILE__, __LINE__);
      annotypes_to_typenames[name] = emptyString;
   }
   else
      annotypes_to_typenames[name] = std::string(tname);

   return number;
}

std::string &AnnotatableBase::findTypeName(std::string tname)
{
   dyn_hash_map<std::string, std::string>::iterator iter;
   iter = annotypes_to_typenames.find(tname);

   if (iter != annotypes_to_typenames.end()) {
      return iter->second;
   }

   return emptyString;
}

int AnnotatableBase::getAnnotationType(std::string &name, const char *tname) 
{
   std::string str(name);

   if (annotationTypes.find(name) == annotationTypes.end()) 
      return -1;

   AnnotatableBase::anno_info_t anno_rec = annotationTypes[name];

   if (tname) {
      //  check if we have typename, if not, add it
      dyn_hash_map<std::string, std::string>::iterator iter;
      iter  = annotypes_to_typenames.find(std::string(tname));

      if (iter == annotypes_to_typenames.end()) {
         // add this typename (it is possible to create an annotation type without a typename
         //  and patch it up later)
         annotypes_to_typenames[std::string(tname)] = name;
      }
      else {
         if (iter->second == emptyString) {
            iter->second = std::string(tname);
         }
      }
   }

   return anno_rec.anno_id;
}

int AnnotatableBase::getOrCreateAnnotationType(std::string &anno_name, const char *tname) {
   int anno_type = AnnotatableBase::getAnnotationType(anno_name, tname);
   if (anno_type == -1) {
      //  for serializable annotation types, we need to explicitly create 
      //  these during init time to provide the proper serializer object for
      //  serializing the annotation later
      anno_type = AnnotatableBase::createAnnotationType(anno_name, tname, NULL);
      //  fprintf(stderr, "%s[%d]:  created annotation type %s/%d\n", 
      //       FILE__, __LINE__, anno_name.c_str(), anno_type);
   }
   return anno_type;
}

#if !defined (cap_serialization)

SerializerBase * AnnotatableBase::getAnnotationSerializer(std::string &) 
{
   fprintf(stderr, "%s[%d]:  WARNING:  asking for annotation serializer is not defined\n", 
         FILE__, __LINE__);
   return NULL;
}

#else

SerializerBase * AnnotatableBase::getAnnotationSerializer(std::string &name) 
{
  std::string str(name);

  if (annotationTypes.find(name) == annotationTypes.end()) {
     fprintf(stderr, "%s[%d]:  No serializer for type %s\n", FILE__, __LINE__, name.c_str());
     return NULL;
  }

  AnnotatableBase::anno_info_t anno_rec = annotationTypes[name];

  if (!anno_rec.anno_serializer) {
     fprintf(stderr, "%s[%d]:  WARNING:  anno_serializer not initialized\n", FILE__, __LINE__);
  }

  SerializerBase *ret = NULL;
  ret = anno_rec.anno_serializer;

  if (!ret) {
     fprintf(stderr, "%s[%d]:  WARNING:  annotation serializer is NULL here?\n",
           FILE__, __LINE__);
  }

  fprintf(stderr, "%s[%d]:  annotation serializer for %s is %p\n", 
        FILE__, __LINE__, name.c_str(), ret );
  fprintf(stderr, "%s[%d]:  annotation serializer for %s is %s\n", 
        FILE__, __LINE__, name.c_str(), ret->name().c_str() );
  fprintf(stderr, "%s[%d]:  annotation serializer for %s is %p\n", 
        FILE__, __LINE__, name.c_str(), ret );

  return ret;
}
#endif

DLLEXPORT void AnnotatableBase::dumpAnnotationSerializers() 
{
   fprintf(stderr, "%s[%d]:  dump of serializers for annotations\n", FILE__, __LINE__);

#if defined (cap_serialization)

   dyn_hash_map<std::string, anno_info_t>::iterator iter;

   for (iter = annotationTypes.begin(); iter != annotationTypes.end(); iter++) 
   {
      std::string name = iter->first;
      anno_info_t anno_rec = iter->second;
      fprintf(stderr, "\t%s -- id: %d -- serializer %p\n", 
            name.c_str(), anno_rec.anno_id, anno_rec.anno_serializer);
   }

#else

  fprintf(stderr, "%s[%d]:  WARNING:  asking for annotation serializer dump is not defined\n", 
        FILE__, __LINE__);

#endif
}

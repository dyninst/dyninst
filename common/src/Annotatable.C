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

// $Id: Annotatable.C,v 1.12 2008/11/03 15:19:23 jaw Exp $

#include "common/h/headers.h"
#include "dynutil/h/dyntypes.h"
#include "dynutil/h/Annotatable.h"
#include "dynutil/h/Serialization.h"
#include "common/h/serialize.h"

using namespace Dyninst;

COMMON_EXPORT AnnotatableSparse::annos_t AnnotatableSparse::annos;
COMMON_EXPORT dyn_hash_map<void *, unsigned short> AnnotatableSparse::ser_ndx_map;

namespace Dyninst {

COMMON_EXPORT int AnnotationClass_nextId;

bool void_ptr_cmp_func(void *v1, void *v2)
{
   return v1 == v2;
}
};

std::vector<AnnotationClassBase *> *AnnotationClassBase::annotation_types = NULL;
dyn_hash_map<std::string, AnnotationClassID> *AnnotationClassBase::annotation_ids_by_name = NULL;

AnnotationClassBase::AnnotationClassBase(std::string n, 
		anno_cmp_func_t cmp_func_, 
		ser_func_t sf_) :
   name(n),
   serialize_func(sf_)
{
    // Using a static vector led to the following pattern on AIX:
    //   dyninstAPI static initialization
    //     ... add annotation types
    //   common static initialization
    //     ... vector constructor called, resetting size to 0.

    if (annotation_types == NULL)
        annotation_types = new std::vector<AnnotationClassBase *>;
    if (annotation_ids_by_name == NULL)
        annotation_ids_by_name = new dyn_hash_map<std::string, AnnotationClassID>;

   if (NULL == cmp_func_)
      cmp_func = void_ptr_cmp_func;
   else
      cmp_func = cmp_func_;

   dyn_hash_map<std::string, AnnotationClassID>::iterator iter;
   iter = annotation_ids_by_name->find(n);
   if (iter == annotation_ids_by_name->end()) 
   {
      id = (AnnotationClassID) annotation_types->size();
      annotation_types->push_back(this);
      (*annotation_ids_by_name)[name] = id;
   }
   else
   {
      id = iter->second;
   }

   if (id >= annotation_types->size())
	   assert(0 && "bad anno id");
}

Dyninst::AnnotationClassBase* AnnotationClassBase::findAnnotationClass(unsigned int id)
{
	if(id > annotation_types->size())
	{
		fprintf(stderr, "%s[%d]:  cannot find annotation class base for id %d, max is %ld\n", FILE__, __LINE__, id, annotation_types->size());
		return NULL;
	}
	if (NULL == (*annotation_types)[id])
	{
		fprintf(stderr, "%s[%d]:  FIXME:  have NULL slot\n", FILE__, __LINE__);
	}
	if ((*annotation_types)[id]->getID() != id)
	{
		fprintf(stderr, "%s[%d]:  FIXME:  have bad id in annotation class: %d, not %d\n", FILE__, __LINE__, (*annotation_types)[id]->getID(), id);
	}
	return (*annotation_types)[id];
}
void AnnotationClassBase::dumpAnnotationClasses()
{
	fprintf(stderr, "%s[%d]: have the following annotation classes:\n", FILE__, __LINE__);
	for (unsigned int i = 0; i < annotation_types->size(); ++i)
	{
		AnnotationClassBase *acb = (*annotation_types)[i];
		if (!acb)
		{
			fprintf(stderr, "\t<NULL>\n");
			continue;
		}
		fprintf(stderr, "\tid-%d\t%s, type %s\n", acb->getID(), acb->getName().c_str(), acb->getTypeName());
	}
}
#if 0
int Dyninst::AnnotationClass_nextId;
int newAnnotationClass(void *ptr)
{
   annotation_classes.push_back(ptr);
   assert(annotation_classes.size() == AnnotationClass_nextID);
   return AnnotationClass_nextID++;
}
#endif

#if 0


#if defined (cap_serialization)
#include "common/h/serialize.h"
#endif

using namespace Dyninst;

int AnnotatableBase::number;
int AnnotatableBase::metadataNum;
dyn_hash_map<std::string, AnnotatableBase::anno_info_t> AnnotatableBase::annotationTypes;
dyn_hash_map<std::string, int> AnnotatableBase::metadataTypes;
dyn_hash_map<std::string, std::string> AnnotatableBase::annotypes_to_typenames;
std::string AnnotatableBase::emptyString = std::string("");

AnnotatableBase::AnnotatableBase() 
{
   number = -1;
}

SerializerBase *getSDAnno(std::string &anno_name)
{
   SerializerBase *ret =  AnnotatableBase::getAnnotationSerializer(anno_name);

   if (!ret) 
   {
      fprintf(stderr, "%s[%d]:  getAnnotationSerializer (%s) failed\n", 
            FILE__, __LINE__, anno_name.c_str());
   }

   return ret;
}

void dumpSDAnno()
{
   AnnotatableBase::dumpAnnotationSerializers();
}


int AnnotatableBase::createAnnotationType(std::string &name, 
      const char *tname, const char *serializer_name) 
{
   std::string n(name);
   int num = getAnnotationType(name, tname);

   if (num != -1) 
   {
      //  also check to see if we have set the tname and serializer fields

#if defined (cap_serialization)
      SerializerBase *sb = NULL;

      if (serializer_name) 
      {
         sb = SerializerBin::findSerializerByName(serializer_name);
         if (!sb) 
         {
            fprintf(stderr, "%s[%d]:  WARNING:  cannot find serializer for name %s\n", 
                  FILE__, __LINE__, serializer_name);
         }
      }

      if (sb) 
      {
         dyn_hash_map<std::string, anno_info_t>::iterator iter = annotationTypes.find(n); 

         if (iter != annotationTypes.end()) 
         {
            AnnotatableBase::anno_info_t &anno_rec = iter->second;

            if (!anno_rec.anno_serializer) 
            {
               fprintf(stderr, "%s[%d]:  assigning serializer for type %s to %p\n",
                     FILE__, __LINE__, name.c_str(), sb);
               anno_rec.anno_serializer = sb;
               anno_rec.anno_serializer_name = serializer_name;
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
   SerializerBase *sb = NULL;

   if (serializer_name) 
   {
      sb = SerializerBin::findSerializerByName(serializer_name);

      if (!sb) 
      {
         fprintf(stderr, "%s[%d]:  WARNING:  cannot find serializer for name %s\n", 
               FILE__, __LINE__, serializer_name);
      }
   }

   fprintf(stderr, "%s[%d]:  assigning serializer for type %s to %p\n", 
         FILE__, __LINE__, name.c_str(), sb);

   anno_rec.anno_serializer = sb;
   anno_rec.anno_serializer_name = serializer_name;
#endif

   annotationTypes[n] = anno_rec;

#if defined (cap_serialization)
   assert(annotationTypes[n].anno_serializer == sb);
#endif
   
   if (!tname) 
   {
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

   if (iter != annotypes_to_typenames.end()) 
   {
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

   if (tname) 
   {
      //  check if we have typename, if not, add it
      dyn_hash_map<std::string, std::string>::iterator iter;
      iter  = annotypes_to_typenames.find(std::string(tname));

      
      if (iter == annotypes_to_typenames.end()) 
      {
         // add this typename (it is possible to create an annotation type without a typename
         //  and patch it up later)
         annotypes_to_typenames[std::string(tname)] = name;
      }
      else 
      {
         if (iter->second == emptyString) 
         {
            iter->second = std::string(tname);
         }
      }
   }

   return anno_rec.anno_id;
}

int AnnotatableBase::getOrCreateAnnotationType(std::string &anno_name, const char *tname,
      const char *serializer_name = NULL) 
{
   int anno_type = AnnotatableBase::getAnnotationType(anno_name, tname);

   if (anno_type == -1) 
   {
      //  for serializable annotation types, we need to explicitly create 
      //  these during init time to provide the proper serializer object for
      //  serializing the annotation later

      anno_type = AnnotatableBase::createAnnotationType(anno_name, tname, serializer_name);

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

  if (!anno_rec.anno_serializer) 
  {
     if (NULL != anno_rec.anno_serializer_name)
     {
        //  try to look it up by name:
        SerializerBin *sb = SerializerBin::findSerializerByName(anno_rec.anno_serializer_name);
        
        if (!sb)
        {
           fprintf(stderr, "%s[%d]:  WARNING:  no serializers for anno_serializer_name %s\n", 
                 FILE__, __LINE__, anno_rec.anno_serializer_name);
        }
        else
        {
           anno_rec.anno_serializer = sb;
        }
     }
  }

  if (!anno_rec.anno_serializer) 
  {
     fprintf(stderr, "%s[%d]:  WARNING:  anno_serializer not initialized\n", FILE__, __LINE__);
     dyn_hash_map<std::string, anno_info_t>::iterator iter;

     fprintf(stderr, "%s[%d]:  have serializers for:\n", FILE__, __LINE__);

     for (iter = annotationTypes.begin(); iter != annotationTypes.end(); iter++)
     {
        std::string an = iter->first;
        anno_info_t &at = iter->second;
        fprintf(stderr, "\t%s[%d]: %s--%p\n", an.c_str(), at.anno_id, 
              at.anno_serializer_name ? at.anno_serializer_name : "no_name_provided", 
              at.anno_serializer);
     }
  }

  SerializerBase *ret = NULL;
  ret = anno_rec.anno_serializer;

  if (!ret) 
  {
     fprintf(stderr, "%s[%d]:  WARNING:  annotation serializer is NULL here?\n",
           FILE__, __LINE__);
     return ret;
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

void AnnotatableBase::dumpAnnotationSerializers() 
{
   fprintf(stderr, "%s[%d]:  dump of serializers for annotations\n", FILE__, __LINE__);

#if defined (cap_serialization)

   dyn_hash_map<std::string, anno_info_t>::iterator iter;

   for (iter = annotationTypes.begin(); iter != annotationTypes.end(); iter++) 
   {
      std::string name = iter->first;
      anno_info_t anno_rec = iter->second;
      fprintf(stderr, "\t%s -- id: %d -- serializer %s--%p\n", 
            name.c_str(), anno_rec.anno_id, 
            anno_rec.anno_serializer_name ? anno_rec.anno_serializer_name: "no-name-provided",
            anno_rec.anno_serializer);
   }

#else

  fprintf(stderr, "%s[%d]:  WARNING:  asking for annotation serializer dump is not defined\n", 
        FILE__, __LINE__);

#endif
}

//  this is just a helper function to get a reference to gtranslate out of the
//  header files
bool serialize_int_param(SerializerBase *sb, int &param, const char *tag)
{
   if (!sb) 
   {
      fprintf(stderr, "%s[%d]: FIXME\n", FILE__, __LINE__);
      return false;
   }

   try 
   {
      gtranslate(sb, param, tag);
   }
   catch (const SerializerError &err)
   {
      fprintf(stderr, "%s[%d]:  %sserialization error here\n", FILE__, __LINE__,
            sb->iomode() == sd_serialize ? "" : "de");
      printSerErr(err);
      return false;
   }

   return true;

}

bool getAndExecuteSerFunc(AnnotatableBase *ab, SerializerBin *ser, int anno_id, 
      std::string anno_name, unsigned /*nelem*/)
{
   SerDesBin &sdbin = ser->getSD_bin(); 

#if 0
   if (!sdbin)
   {
      fprintf(stderr, "%s[%d]: Bulk serialize annotations broken serializer\n",
            FILE__, __LINE__);
      return false;
   }
#endif

   AnnoFunc *afp = sdbin.findAnnoFunc(anno_id, anno_name);

   if (NULL == afp)
   {
      fprintf(stderr, "%s[%d]:  failed to findAnnoFunc for id %d\n",
            FILE__, __LINE__, anno_id);
      return false;
   }

   AnnoFunc &af = *afp;

#if 0
   fprintf(stderr, "%s[%d]:  executing func %d times\n", FILE__, __LINE__, nelem);
   for (unsigned int i = 0; i < nelem; ++i) 
   {
      bool res = af(ser, ab);

      if (false == res)
      {
         fprintf(stderr, "%s[%d]:  failed execute function: %d out of %d\n", 
               FILE__, __LINE__, i, nelem);
         return false;
      }
   }
#endif

   bool res = af(ser, ab);

   if (false == res)
   {
      fprintf(stderr, "%s[%d]:  failed execute function\n", 
            FILE__, __LINE__);
      return false;
      }

   return true;
}

#endif

namespace Dyninst {
bool is_input(SerializerBase *sb)
{
	return sb->isInput();
}
bool is_output(SerializerBase *sb)
{
	return sb->isOutput();
}
bool serialize_annotation_list(void *id, std::vector<ser_rec_t> &sers, SerializerBase *sb, const char *tag)
{
	if (sers.size())
		fprintf(stderr, "%s[%d]:  welcome to serialize_annotation_list, size %lu, id = %p\n", FILE__, __LINE__, sers.size(), id);
	assert(sb);
	assert(id);
	try {
		sb->serialize_annotations(id, sers, tag);
	} 
	catch (const SerializerError &err)
	{
		fprintf(stderr, "%s[%d]:  serializer error translating annotations\n", FILE__, __LINE__);
		return false;
	}
	return true;
}

bool serialize_post_annotation(void *parent, void *anno, SerializerBase *sb, AnnotationClassBase *acb, sparse_or_dense_anno_t sod, const char *tag)
{
	fprintf(stderr, "%s[%d]:  welcome to serialize_post_annotation_list, id = %p\n", 
			FILE__, __LINE__, parent);
	assert(parent);
	assert(anno);
	if (!sb)
	{
		fprintf(stderr, "%s[%d]:  no existing output serializer\n", FILE__, __LINE__);
		return true;
	}
	try {
		sb->serialize_post_annotation(parent, anno, acb, sod, tag);
	}
	catch (const SerializerError &err)
	{
		fprintf(stderr, "%s[%d]:  serializer error translating annotations\n", FILE__, __LINE__);
		return false;
	}
	return true;
}

bool add_annotations(SerializerBase *sb, AnnotatableSparse *an, std::vector<ser_rec_t> &sers)
{
	if (sers.size())
		fprintf(stderr, "%s[%d]:  welcome to addAnnotations: got %lu\n", FILE__, __LINE__, sers.size());
	//  if we are not doing deserialization, there is nothing to do here, just return true
	//  to keep from triggering error handling.
	if (sb->isOutput())
		return true;
	bool err = false;
	for (unsigned int i = 0; i < sers.size(); ++i)
	{
		ser_rec_t &sr = sers[i];
		if (!sr.data)
		{
			fprintf(stderr, "%s[%d]:  bad deserialize annotation record\n", FILE__, __LINE__);
			err = true;
			continue;
		}
		if (!sr.acb)
		{
			fprintf(stderr, "%s[%d]:  bad deserialize annotation record\n", FILE__, __LINE__);
			err = true;
			continue;
		}
		fprintf(stderr, "%s[%d]:  adding pre annotation: %p, parent = %p\n", FILE__, __LINE__, sr.data, an);

		if (!an->addAnnotation(sr.data, sr.acb->getID()))
		{
			fprintf(stderr, "%s[%d]:  failed to add deserialized annotation here\n", FILE__, __LINE__);
			err = true;
		}
	}
	return (err == false);
}
bool add_annotations(SerializerBase *sb, AnnotatableDense *an, std::vector<ser_rec_t> &sers)
{
	fprintf(stderr, "%s[%d]:  welcome to addAnnotations: got %lu\n", FILE__, __LINE__, sers.size());
	//  if we are not doing deserialization, there is nothing to do here, just return true
	//  to keep from triggering error handling.
	if (sb->isOutput())
		return true;
	bool err = false;
	for (unsigned int i = 0; i < sers.size(); ++i)
	{
		ser_rec_t &sr = sers[i];
		if (!sr.data)
		{
			fprintf(stderr, "%s[%d]:  bad deserialize annotation record\n", FILE__, __LINE__);
			err = true;
			continue;
		}
		if (!sr.acb)
		{
			fprintf(stderr, "%s[%d]:  bad deserialize annotation record\n", FILE__, __LINE__);
			err = true;
			continue;
		}
		fprintf(stderr, "%s[%d]:  adding pre annotation\n", FILE__, __LINE__);
		if (!an->addAnnotation(sr.data, sr.acb->getID()))
		{
			fprintf(stderr, "%s[%d]:  failed to add deserialized annotation here\n", FILE__, __LINE__);
			err = true;
		}
	}
	return (err == false);
}
}
bool dummy_bs()
{
   fprintf(stderr, "%s[%d]:  \n", FILE__, __LINE__);
   return true;
}

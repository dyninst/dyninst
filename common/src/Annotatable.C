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
#include "dyntypes.h"
#include "Annotatable.h"
#include "Serialization.h"
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
}

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
		serialize_printf("%s[%d]:  welcome to serialize_annotation_list, size %lu, id = %p\n", FILE__, __LINE__, sers.size(), id);
	assert(sb);
	assert(id);
	try {
		sb->serialize_annotations(id, sers, tag);
	} 
	catch (const SerializerError &err)
	{
		fprintf(stderr, "%s[%d]:  serializer error translating annotations\n", FILE__, __LINE__);
		printSerErr(err);
		return false;
	}
	return true;
}

bool serialize_post_annotation(void *parent, void *anno, SerializerBase *sb, AnnotationClassBase *acb, sparse_or_dense_anno_t sod, const char *tag)
{
	serialize_printf("%s[%d]:  welcome to serialize_post_annotation_list, id = %p\n", 
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
		printSerErr(err);
		return false;
	}
	return true;
}

bool add_annotations(SerializerBase *sb, AnnotatableSparse *an, std::vector<ser_rec_t> &sers)
{
	if (sers.size())
		serialize_printf("%s[%d]:  welcome to addAnnotations: got %lu\n", FILE__, __LINE__, sers.size());
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
	serialize_printf("%s[%d]:  welcome to addAnnotations: got %lu\n", FILE__, __LINE__, sers.size());
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
		serialize_printf("%s[%d]:  adding pre annotation\n", FILE__, __LINE__);
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

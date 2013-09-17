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

// $Id: Annotatable.C,v 1.12 2008/11/03 15:19:23 jaw Exp $

#include "common/src/headers.h"
#include "dyntypes.h"
#include "Annotatable.h"
#include "Serialization.h"
#include "common/src/serialize.h"

using namespace Dyninst;

#if defined (NON_STATIC_SPARSE_MAP)
char buffer[1024];
//AnnotatableSparse::annos_t *AnnotatableSparse::annos = NULL;
AnnotatableSparse::annos_t *annos = NULL;
AnnotatableSparse::annos_t *AnnotatableSparse::getAnnos() const
{
	if (!annos)
	{
		sprintf(buffer, "booga_booga");
		annos = new annos_t();
	}
	return annos;
}
#else
AnnotatableSparse::annos_t AnnotatableSparse::annos;
AnnotatableSparse::annos_t *AnnotatableSparse::getAnnos() const
{
	return &annos;
}
#endif

dyn_hash_map<void *, unsigned short> AnnotatableSparse::ser_ndx_map;

namespace Dyninst 
{

bool dyn_debug_annotations = false;
bool annotation_debug_flag()
{
	return dyn_debug_annotations;
}

void annotations_debug_init()
{
	if (dyn_debug_annotations) return;

	if (getenv("DYNINST_DEBUG_ANNOTATIONS")) {
		fprintf(stderr, "Enabling DyninstAPI annotations debug\n");
		dyn_debug_annotations = true;
	}
	else if (getenv("DYNINST_DEBUG_ANNOTATION")) {
		fprintf(stderr, "Enabling DyninstAPI annotations debug\n");
		dyn_debug_annotations = true;
	}
	else if (getenv("DYNINST_DEBUG_ANNOTATABLE")) {
		fprintf(stderr, "Enabling DyninstAPI annotations debug\n");
		dyn_debug_annotations = true;
	}
}

int annotatable_printf(const char *format, ...)
{
	if (!dyn_debug_annotations) return 0;
	if (NULL == format) return -1;

	//debugPrintLock->_Lock(FILE__, __LINE__);

	//  probably want to have basic thread-id routines in libcommon...
	//  uh...  later....

	//fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
	va_list va;
	va_start(va, format);
	int ret = vfprintf(stderr, format, va);
	va_end(va);

	//debugPrintLock->_Unlock(FILE__, __LINE__);

	return ret;
}

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
	annotations_debug_init();
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
	  annotatable_printf("%s[%d]:  New AnnotationClass %d: %s\n", 
			  FILE__, __LINE__, id, n.c_str());
      annotation_types->push_back(this);
      (*annotation_ids_by_name)[name] = id;
   }
   else
   {
      id = iter->second;
	  annotatable_printf("%s[%d]:  Existing AnnotationClass %d\n", FILE__, __LINE__, id);
   }

   if (id >= annotation_types->size())
	   assert(0 && "bad anno id");
}

AnnotationClassBase::~AnnotationClassBase()
{
	//  Still waffling...  maybe a bad idea
#if 0 
	//  The general rule in dyninst/symtab etc is to use global/static
	//  Annotation classes, so they never go away.  This is good.
	//  But in the testsuite, we have a bunch of transient fly-by-night
	//  AnnotationClasses for the purposes of testing.  
	//
	//  This may be a bit dangerous and might require a bit more thought,
	//  but for now, if this AnnotationClass was the last one allocated
	//  remove it from the static mapping so it can be reused.

	if (!annotation_types)  return; //  should never happen
	if (id >= annotation_types->size()) return; //  should never happen
	if (id == (annotation_types->size() -1))
	{
		annotatable_printf("%s[%d]:  removing annotation class %d: %s\n", 
				FILE__, __LINE__, id, name.c_str());
		//  this is the special case where we can "undo" the existence of
		// the annotation type
		annotation_types->pop_back();
		assert((*annotation_types)[id] == this);
		dyn_hash_map<std::string, AnnotationClassID>::iterator iter;
		iter = annotation_ids_by_name->find(name);
		if (iter != annotation_ids_by_name->end()) 
		{
			annotation_ids_by_name->erase(iter);
		}
	}
#endif
}

#if 0
void AnnotationClassBase::clearAnnotationIDMap()
{
	if (!annotation_ids_by_name) return;
	annotation_ids_by_name->clear();
	delete annotation_ids_by_name;
	annotation_ids_by_name = NULL;
}
#endif

Dyninst::AnnotationClassBase* AnnotationClassBase::findAnnotationClass(unsigned int id)
{
	if(id > annotation_types->size())
	{
		fprintf(stderr, "%s[%d]:  cannot find annotation class base for id %d, max is %ld\n", FILE__, __LINE__, id, (long int) annotation_types->size());
		return NULL;
	}
	if (NULL == (*annotation_types)[id])
	{
		fprintf(stderr, "%s[%d]:  FIXME:  have NULL slot\n", FILE__, __LINE__);
	}
	if ((*annotation_types)[id]->getID() != id)
	{
		fprintf(stderr, "%s[%d]:  FIXME:  have bad id in annotation class: %d, not %d\n", FILE__, __LINE__, (*annotation_types)[id]->getID(), id);
		return NULL;
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

#if !defined(SERIALIZATION_DISABLED)
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

#else
bool is_input(SerializerBase *)
{
   return false;
}

bool is_output(SerializerBase *) {
   return false;
}

bool serialize_annotation_list(void *, std::vector<ser_rec_t> &, SerializerBase *, const char *) {
   return false;
}

bool serialize_post_annotation(void *, void *, SerializerBase *, AnnotationClassBase *, sparse_or_dense_anno_t, const char *) {
   return false;
}

bool add_annotations(SerializerBase *, AnnotatableSparse *, std::vector<ser_rec_t> &) {
   return false;
}

bool add_annotations(SerializerBase *, AnnotatableDense *, std::vector<ser_rec_t> &) {
   return false;
}
#endif

}
bool dummy_bs()
{
   fprintf(stderr, "%s[%d]:  \n", FILE__, __LINE__);
   return true;
}

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

#include <stdio.h>
#include <vector>

#include "common/h/dyntypes.h"
#include "common/h/Annotatable.h"
#include "common/src/serialize.h"
#include "common/src/Types.h"
#include "common/src/headers.h"

#if defined(SERIALIZATION_DISABLED)
unsigned short Dyninst::get_serializer_index(Dyninst::SerializerBase *) {
  return 0;
}


#else

using namespace Dyninst;

//COMMON_EXPORT dyn_hash_map<Address, AnnotatableBase *> SerDesBin::annotatable_id_map;


COMMON_EXPORT dyn_hash_map<std::string, SerializerBase::subsystem_serializers_t> SerializerBase::all_serializers;


dyn_hash_map<void *, AnnotationContainerBase *> AnnotationContainerBase::containers_by_id;
void AnnotationContainerBase::clearContainersByID()
{
	containers_by_id.clear();
}
void Serializable::clearContainersByID()
{
	AnnotationContainerBase::clearContainersByID();
}

namespace Dyninst 
{
bool dyn_debug_serializer = false;
bool &serializer_debug_flag()
{
   //  This function exists to get around problems with exporting the variable
   //  across library boundaries on windows...   there's probably a better way to do this...
   return dyn_debug_serializer;
}

Serializable * ser_func_wrapper(void *it, SerializerBase *sb, 
		const char *tag)
{   
	assert(it);
	assert(sb);
	Serializable *s = (Serializable *) (it);
	return s->serialize(sb, tag);
}   


void serialize_debug_init()
{
   char *p;
   if ( (p=getenv("DYNINST_DEBUG_SERIALIZER"))) {
      fprintf(stderr, "Enabling DyninstAPI serializer debug\n");
      dyn_debug_serializer = true;
   }
}


int serializer_printf(const char *format, ...) 
{   
   if (!dyn_debug_serializer) return 0;
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

#ifndef CASE_RETURN_STR 
#define CASE_RETURN_STR(x) case x: return #x;
#endif
const char *serPostOp2Str(ser_post_op_t s)
{
	switch (s)
	{
		CASE_RETURN_STR(sp_add_anno);
		CASE_RETURN_STR(sp_rem_anno);
		CASE_RETURN_STR(sp_add_cont_item);
		CASE_RETURN_STR(sp_rem_cont_item);
	}
	return "bad_op";
}

std::vector<std::pair<std::string, dyn_hash_map<std::string, short>*> >  SerContextBase::ser_control_map;
dyn_hash_map<std::string, short> *SerContextBase::getMapForType(std::string tname)
{
	for (unsigned int i = 0; i < ser_control_map.size(); ++i)
	{
		if (std::string(tname) == ser_control_map[i].first)
			return ser_control_map[i].second;
	}
	return NULL;
}

SerContextBase::SerContextBase(std::string tname, std::string fname_) :
	fname(fname_)
{
	dyn_hash_map<std::string, short> *map_p = getMapForType(tname);
	if (!map_p)
	{
		map_p = new dyn_hash_map<std::string, short>();
		ser_control_map.push_back(std::make_pair(std::string(tname), map_p));
		(*map_p)[fname] = 0;
	}
	//  If we already have a serialize control specification for this file
	//  do not reset it here.  This allows us to specify serializer behavior
	//  in the testsuite apriori.
}

void SerContextBase::enableSerialize(std::string tname, std::string filename, bool val)
{
	dyn_hash_map<std::string, short> *map_p = getMapForType(tname);
	if (!map_p)
	{
		//  This case should not be able to happen, but, anyways....
		map_p = new dyn_hash_map<std::string, short>();
		ser_control_map.push_back(std::make_pair(tname, map_p));
	}

	dyn_hash_map<std::string, short>::iterator iter = map_p->find(filename);
	if (iter == map_p->end())
	{
		if (val)
			(*map_p)[filename] = SERIALIZE_ENABLE_FLAG;
		else
			(*map_p)[filename] = 0;
	}
	else
	{
		short &flags = iter->second;
		if (val)
			flags |= SERIALIZE_ENABLE_FLAG;
		else
			flags &= ~SERIALIZE_ENABLE_FLAG;
	}
}

void SerContextBase::enableSerialize(bool val)
{
	SerContextBase::enableDeserialize(std::string(getRootTypename()), fname, val);
}

void SerContextBase::enableDeserialize(std::string tname, std::string filename, bool val)
{
	dyn_hash_map<std::string, short> *map_p = getMapForType(tname);
	if (!map_p)
	{
		//  This case should not be able to happen, but, anyways....
		map_p = new dyn_hash_map<std::string, short>();
		ser_control_map.push_back(std::make_pair(tname, map_p));
	}

	dyn_hash_map<std::string, short>::iterator iter = map_p->find(filename);
	if (iter == map_p->end())
	{
		if (val)
			(*map_p)[filename] = DESERIALIZE_ENABLE_FLAG;
		else
			(*map_p)[filename] = 0;
	}
	else
	{
		short &flags = iter->second;
		if (val)
			flags |= DESERIALIZE_ENABLE_FLAG;
		else
			flags &= ~DESERIALIZE_ENABLE_FLAG;
	}
}

void SerContextBase::enableDeserialize(bool val)
{
	SerContextBase::enableDeserialize(std::string(getRootTypename()), fname, val);
}

void SerContextBase::enforceDeserialize(std::string tname, std::string filename, bool val)
{
	dyn_hash_map<std::string, short> *map_p = getMapForType(tname);
	if (!map_p)
	{
		//  This case should not be able to happen, but, anyways....
		map_p = new dyn_hash_map<std::string, short>();
		ser_control_map.push_back(std::make_pair(tname, map_p));
	}

	dyn_hash_map<std::string, short>::iterator iter = map_p->find(filename);
	if (iter == map_p->end())
	{
		if (val)
			(*map_p)[filename] = DESERIALIZE_ENFORCE_FLAG;
		else
			(*map_p)[filename] = 0;
	}
	else
	{
		short &flags = iter->second;
		if (val)
			flags |= DESERIALIZE_ENFORCE_FLAG;
		else
			flags &= ~DESERIALIZE_ENFORCE_FLAG;
	}
}

void SerContextBase::enforceDeserialize(bool val)
{
	SerContextBase::enforceDeserialize(std::string(getRootTypename()), fname, val);
}

void SerContextBase::enableSerDes(std::string tname, std::string filename, bool val)
{
	dyn_hash_map<std::string, short> *map_p = getMapForType(tname);
	if (!map_p)
	{
		//  This case should not be able to happen, but, anyways....
		map_p = new dyn_hash_map<std::string, short>();
		ser_control_map.push_back(std::make_pair(tname, map_p));
	}

	dyn_hash_map<std::string, short>::iterator iter = map_p->find(filename);
	if (iter == map_p->end())
	{
		if (val)
			(*map_p)[filename] = DESERIALIZE_ENABLE_FLAG | SERIALIZE_ENABLE_FLAG;
		else
			(*map_p)[filename] = 0;
	}
	else
	{
		short &flags = iter->second;
		if (val)
		{
			flags |= SERIALIZE_ENABLE_FLAG;
			flags |= DESERIALIZE_ENABLE_FLAG;
		}
		else
			flags = 0;
	}
}

void SerContextBase::enableSerDes(bool val)
{
	SerContextBase::enableSerDes(std::string(getRootTypename()), fname, val);
}

bool SerContextBase::serializeEnabled()
{
	dyn_hash_map<std::string, short> *map_p = getMapForType(std::string(getRootTypename()));
	if (!map_p)
	{
		//  This case should not be able to happen, but, anyways....
		return false;
	}

	dyn_hash_map<std::string, short>::iterator iter = map_p->find(fname);
	if (iter == map_p->end())
	{
		//  This case should not be able to happen, but, anyways....
		return false;
	}

	short &flags = iter->second;
	return (0 != (flags & SERIALIZE_ENABLE_FLAG));
}

bool SerContextBase::deserializeEnabled()
{
	dyn_hash_map<std::string, short> *map_p = getMapForType(std::string(getRootTypename()));
	if (!map_p)
	{
		//  This case should not be able to happen, but, anyways....
		return false;
	}

	dyn_hash_map<std::string, short>::iterator iter = map_p->find(fname);
	if (iter == map_p->end())
	{
		//  This case should not be able to happen, but, anyways....
		return false;
	}

	short &flags = iter->second;
	return (0 != (flags & DESERIALIZE_ENABLE_FLAG));
}

bool SerContextBase::deserializeEnforced(std::string tname, std::string filename)
{
	dyn_hash_map<std::string, short> *map_p = getMapForType(tname);
	if (!map_p)
	{
		//  This case should not be able to happen, but, anyways....
		return false;
	}

	dyn_hash_map<std::string, short>::iterator iter = map_p->find(filename);
	if (iter == map_p->end())
	{
		//  This case should not be able to happen, but, anyways....
		return false;
	}

	short &flags = iter->second;
	return (0 != (flags & DESERIALIZE_ENFORCE_FLAG));
}

bool SerContextBase::deserializeEnforced()
{
	return SerContextBase::deserializeEnforced(std::string(getRootTypename()), fname);
}

std::string SerContextBase::getSerFileName()
{
	return serfilename;
}

SerializerBase *createSerializer(SerContextBase *scs, 
		std::string ser_name, std::string file, ser_type_t ser_type, 
		iomode_t mode, bool verbose)

{
	if (!scs->serializeEnabled() && (mode == sd_serialize))
	{
		serialize_printf("%s[%d]:  serialization not enabled\n", FILE__, __LINE__);
		return NULL;
	}

	if (!scs->deserializeEnabled() && (mode == sd_deserialize))
	{
		serialize_printf("%s[%d]:  deserialization not enabled\n", FILE__, __LINE__);
		return NULL;
	}

	SerializerBase *ret = NULL;
	try {
		if (!scs)
		{
			fprintf(stderr, "%s[%d]:  ERROR context object not given for serialiezr\n", 
					FILE__, __LINE__);
			return NULL;
		}

		if (ser_type == ser_bin)
		{
			serialize_printf("%s[%d]:  creating bin serializer %s/%s, %p\n", 
					FILE__, __LINE__, ser_name.c_str(), file.c_str(), scs);
			ret = new SerializerBin(scs, ser_name, file, mode, verbose);
		}
		else if (ser_type == ser_xml)
		{
			ret = new SerializerXML(scs, ser_name, file, mode, verbose);
		}
		else
		{
			serialize_printf("%s[%d]:  bad serializer type provided\n", 
					FILE__, __LINE__);
			return NULL;
		}
	}
	catch(const SerializerError &err) 
	{
		fprintf(stderr, "%s[%d]:  %sserialize failed init...  \n\t%s[%d]: %s\n",
				FILE__, __LINE__, mode == sd_serialize ? "" : "de", 
				err.file().c_str(), err.line(), err.what());
		return NULL;
	}
	return ret;
}

SerDes *SerFile::getSD()
{
	return sd;
}

SerFile::SerFile(std::string fname, iomode_t mode, bool verbose) :
	writer (NULL), 
	f(NULL), 
	iomode_(mode), 
	noisy(verbose) 
{
        std::string file_path = resolve_file_path(fname.c_str());
        if( file_path.empty() )
	{
		char msg[1024];
		snprintf(msg, 1024, "failed to resolve path for '%s'\n", fname.c_str());
		SER_ERR(msg);
	}

	filename = std::string(file_path);
	serialize_debug_init();

	if (strstr(filename.c_str(), "xml")) 
	{
		fprintf(stderr, "%s[%d]:  opening xml file %s for %s\n", FILE__, __LINE__, 
				filename.c_str(), mode == sd_serialize ? "output" : "input");

		if (mode == sd_deserialize) 
		{
			fprintf(stderr, "%s[%d]:  ERROR:  deserializing xml not supported\n", 
					FILE__, __LINE__);
			assert(0);
		}

#if defined (cap_have_libxml)
		writer = SerDesXML::init(fname, mode, verbose);

		if (!writer) 
		{
			fprintf(stderr, "%s[%d]:  ERROR:  failed to init xml writer\n", 
					FILE__, __LINE__);
			assert(0);
		}
#else

		writer = NULL;
#endif

		sd = new SerDesXML(writer, mode);

	}
	else 
	{
		serialize_printf("%s[%d]:  opening %s file for %s\n", FILE__, __LINE__, 
				filename.c_str(), mode == sd_serialize ? "output" : "input");

		f = SerDesBin::init(std::string(fname), mode, verbose);

		if (!f) 
		{
			fprintf(stderr, "%s[%d]:  failed to init file i/o\n", FILE__, __LINE__);
			assert(0);
		}

		sd = new SerDesBin(f,mode, verbose);
	}
}

iomode_t SerFile::iomode()
{
	return iomode_;
}

std::string SerFile::getCacheFileName()
{
	std::string cache_name;
	SerDes *sd = getSD();
	if (!sd) return std::string();
	SerDesBin *sdb = dynamic_cast<SerDesBin *>(sd);
	if (!sdb) return std::string();
	sdb->resolveCachePath(filename, cache_name); 
	return cache_name;
}

bool Serializable::serialize(std::string file, SerContextBase *scb, ser_type_t mode)
{
	std::string sername = file;
	SerializerBase *serializer = createSerializer(scb, sername, file, mode, sd_serialize, false);

	if (!serializer)
	{
		serialize_printf("%s[%d]:  failed to create serializer\n", FILE__, __LINE__);
		return false;
	}

	try 
	{
		//  Do the serialization
		serialize(serializer, NULL);
		void *barrier_magic = (void *)0xdeadbeef;
		serialize_annotatable_id(serializer, barrier_magic, NULL);
		if (barrier_magic != (void *)0xdeadbeef)
		{
			fprintf(stderr, "%s[%d]:  FIXME:  failed to read magic barrier val\n", 
					FILE__, __LINE__);
		}

	}
	catch (const SerializerError &err_)
	{
		fprintf(stderr, "%s[%d]:  serialize failed\n", FILE__, __LINE__);
		printSerErr(err_);
		return false;
	}

	return true;

}

AnnotatableSparse *find_sparse_annotatable(SerializerBase *serializer, void *id)
{
	assert(serializer);
	return serializer->findSparseAnnotatable(id);
}

AnnotatableDense *find_dense_annotatable(SerializerBase *serializer,void *id)
{
	assert(serializer);
	return serializer->findDenseAnnotatable(id);
}

bool isEOF(SerializerBase *sb)
{
	if (!sb)
	{
		fprintf(stderr, "%s[%d]:  FIXME\n", FILE__, __LINE__);
		return false;
	}
	return sb->isEOF();
}

bool SerializerBase::isEOF()
{
	return sd->isEOF();
}

void SerializerBase::clearAnnotationMaps()
{
	if (sparse_annotatable_map) 
	{
		sparse_annotatable_map->clear();
		//delete sparse_annotatable_map;
		//sparse_annotatable_map = NULL;
	}
	if (dense_annotatable_map) 
	{
		dense_annotatable_map->clear();
		//delete dense_annotatable_map;
		//dense_annotatable_map = NULL;
	}
}

AnnotatableSparse *SerializerBase::findSparseAnnotatable(void *id)
{
	dyn_hash_map<void *, AnnotatableSparse *>::iterator iter;
	iter = sparse_annotatable_map->find(id);
	if (iter == sparse_annotatable_map->end())
	{
		serialize_printf("%s[%d]:  ERROR:  cannot find parent to assign annotation to\n",
				__FILE__, __LINE__);
		return NULL;
	}
	return iter->second;
}

AnnotatableDense *SerializerBase::findDenseAnnotatable(void *id)
{
	dyn_hash_map<void *, AnnotatableDense *>::iterator iter;
	iter = dense_annotatable_map->find(id);
	if (iter == dense_annotatable_map->end())
	{
		serialize_printf("%s[%d]:  ERROR:  cannot find parent to assign annotation to\n",
				__FILE__, __LINE__);
		return NULL;
	}
	return iter->second;
}

void printSerErr(const SerializerError &err) 
{
	fprintf(stderr, "\tserializer exception %s from \n\t%s[%d]\n", 
			err.what(), err.file().c_str(), err.line());
}


bool isOutput(SerializerBase *ser)
{
	return (ser->iomode() == sd_serialize);
}

bool isBinary(SerializerBase *ser)
{
	return ser->isBin();
}

#if 0
void trans_adapt(SerializerBase *ser, Serializable &it, const char *tag)
{
	it.serialize(ser, tag);
}

void trans_adapt(SerializerBase *ser, Serializable  *itp, const char *tag)
{
   assert(itp);
   trans_adapt(ser, *itp, tag);
}

void trans_adapt(SerializerBase *ser, bool &it, const char *tag)
{
   assert(ser);
   ser->translate_base(it, tag);
}

void trans_adapt(SerializerBase *ser, int &it, const char *tag)
{
   assert(ser);
   ser->translate_base(it, tag);
}

void trans_adapt(SerializerBase *ser, unsigned int &it, const char *tag)
{
   assert(ser);
   ser->translate_base(it, tag);
}

void trans_adapt(SerializerBase *ser, long &it, const char *tag)
{
   assert(ser);
   ser->translate_base(it, tag);
}

void trans_adapt(SerializerBase *ser, unsigned long &it, const char *tag)
{
   assert(ser);
   ser->translate_base(it, tag);
}

void trans_adapt(SerializerBase *ser, char &it, const char *tag)
{
   assert(ser);
   ser->translate_base(it, tag);
}

void trans_adapt(SerializerBase *ser, char *&it, const char *tag)
{
   assert(ser);
   assert(it);
   int s_len = strlen(it);
   ser->translate_base(const_cast<const char *&>(it), s_len, tag);
}

void trans_adapt(SerializerBase *ser, std::string &it, const char *tag)
{
   assert(ser);
   ser->translate_base(it, tag);
}

void trans_adapt(SerializerBase *ser, float &it, const char *tag)
{
   assert(ser);
   ser->translate_base(it, tag);
}

void trans_adapt(SerializerBase *ser, double  &it, const char *tag)
{
   assert(ser);
   ser->translate_base(it, tag);
}
#endif

bool sb_is_input(SerializerBase *sb) 
{
   return  (sb->iomode() == sd_deserialize);
}

bool sb_is_output(SerializerBase *sb) 
{
   return  (sb->iomode() == sd_serialize);
}


SerializerBase *getExistingOutputSB(Serializable *s)
{
	return s->lookupExistingSerializer();
}

SerializerBase *getExistingOutputSB(unsigned short ndx)
{
	if (SerializerBase::active_serializers.size() <= ndx)
		return NULL;
	return SerializerBase::active_serializers[ndx];
}

} /* namespace Dyninst */

bool SerializerBase::removeSerializer(unsigned short ndx)
{
	if (SerializerBase::active_serializers.size() <= ndx)
		return false;
	SerializerBase::active_serializers[ndx] = NULL;
	if (ndx == (unsigned short) (active_serializers.size() - 1))
		active_serializers.pop_back();
	return true;
}
bool SerializerBase::global_disable = false;

#if 0
COMMON_EXPORT dyn_hash_map<std::string, SerializerBase *> SerializerBase::active_bin_serializers;
#endif
COMMON_EXPORT std::vector<SerializerBase *> SerializerBase::active_serializers;

SerDesBin &SerializerBin::getSD_bin()
{
	SerializerBase *sb = this;
   SerDes &sd = sb->getSD();
   SerDesBin *sdbin = dynamic_cast<SerDesBin *> (&sd);
   assert(sdbin);
   return *sdbin;
}

SerializerBin::SerializerBin(SerContextBase *s, std::string name_, std::string filename,
		iomode_t dir, bool verbose) :
	SerializerBase(s, std::string(name_), std::string(filename), dir, verbose)
{ 
	SerializerBase *sb = this;
	if (sb->serializationDisabled())
	{
		fprintf(stderr, "%s[%d]:  Failing to construct Bin Translator:  global disable set\n"
				,       
				FILE__, __LINE__);

		throw SerializerError(FILE__, __LINE__,
				std::string("serialization disabled"),
				SerializerError::ser_err_disabled);
	}
}

SerializerBin::~SerializerBin()
{
	serialize_printf("%s[%d]:  WELCOME TO SERIALIZER_BIN dtor\n", FILE__, __LINE__);
	//dyn_hash_map<std::string, SerializerBase *>::iterator iter;
}

void SerializerBase::dumpActiveBinSerializers()
{
	fprintf(stderr, "%s[%d]:  have serializers: FIXME\n", FILE__, __LINE__);
}

FILE *SerDesBin::init(std::string filename, iomode_t mode, bool /*verbose*/) 
{
   if (SerializerBase::serializationDisabled()) 
   {
      fprintf(stderr, "%s[%d]:  Failing BinTranslator:  global disable set\n", 
			  FILE__, __LINE__);
      throw SerializerError(FILE__, __LINE__, 
            std::string("serialization disabled"), 
            SerializerError::ser_err_disabled);
   }

   //  NOTE:  fname is path-resolved and turned into "filename" by the SerDes ctor
   FILE *f = NULL;
   std::string cache_name;

   if (! SerDesBin::resolveCachePath(filename, cache_name)) 
   {
      serialize_printf("%s[%d]:  no cache file exists for %s\n", 
            FILE__, __LINE__, filename.c_str());

      if (mode == sd_deserialize) 
	  {
         //  can't deserialize from a file that does not exist
         char msg[1024];
         snprintf(msg, 1024, "%s[%d]:  no cache file exists for %s/%s\n", 
               FILE__, __LINE__, filename.c_str(), cache_name.c_str());
		 fprintf(stderr, "%s\n", msg);
         SER_ERR(msg);
      }
   }

   errno = 0;
   serialize_printf("%s[%d]:  opening cache file %s for %s\n", FILE__, __LINE__, cache_name.c_str(), (mode == sd_serialize) ? "serialize" : "deserialize");

   f = fopen(cache_name.c_str(), (mode == sd_serialize) ? "w+" : "r");

   if (!f) 
   {
      char msg[1024];
      serialize_printf("%s[%d]: fopen(%s, %s): %s\n", FILE__, __LINE__, 
            cache_name.c_str(), (mode == sd_serialize) ? "w+" : "r", strerror(errno));
      snprintf(msg, 1024, "fopen(%s, %s): %s", cache_name.c_str(), 
              (mode == sd_serialize) ? "w+" : "r", strerror(errno));
      fprintf(stderr, "%s\n", msg);
      SER_ERR(msg);
   }

   serialize_printf("%s[%d]:  opened cache file %s: %s\n", 
		   FILE__, __LINE__, cache_name.c_str(), strerror(errno));

   try 
   {
      if (mode == sd_serialize)
	  {
         writeHeaderPreamble(f, filename, cache_name);
      }
      else 
	  {
         readHeaderAndVerify(filename, cache_name, f);
      }
   }
   catch(const SerializerError &err) 
   {
      fclose(f);
      serialize_printf("%s[%d]:  %sserialize failed init.\n\t%s[%d]: %s\n\trethrowing.\n",
            FILE__, __LINE__, mode == sd_serialize ? "" : "de", 
            err.file().c_str(), err.line(), err.what());
      throw(err);
   }

   return f;
}

SerDesBin::~SerDesBin()
{
}

bool SerDesBin::isEOF()
{
	return (0 != feof(f));
}

bool SerDesBin::getDefaultCacheDir(std::string &path)
{
   char *home_dir = getenv("HOME");

   if (!home_dir) 
   {
      fprintf(stderr, "%s[%d]:  weird, no $HOME dir\n", FILE__, __LINE__);
      return false;
   }

   std::string dot_dyninst_dir = std::string(home_dir) + std::string("/")
      + std::string(DEFAULT_DYNINST_DIR);

   struct stat statbuf;

   if (0 != stat(dot_dyninst_dir.c_str(), &statbuf)) 
   {
      if (errno == ENOENT) 
	  {
#if defined (os_windows)
         if (0 != P_mkdir(dot_dyninst_dir.c_str(), 0)) 
		 {
            fprintf(stderr, "%s[%d]:  failed to make %s\n", FILE__, __LINE__, 
                  dot_dyninst_dir.c_str(), strerror(errno));
            return false;
         } 
#else
         if (0 != mkdir(dot_dyninst_dir.c_str(), S_IRWXU)) 
		 {
            fprintf(stderr, "%s[%d]:  failed to make %s: %s\n", FILE__, __LINE__, 
                  dot_dyninst_dir.c_str(), strerror(errno));
            return false;
         } 
#endif
      }
      else 
	  {
         fprintf(stderr, "%s[%d]:  stat(%s) failed: %s\n", FILE__, __LINE__, 
               dot_dyninst_dir.c_str(), strerror(errno));
         return false;
      }
   }
   else 
   {
#if !defined (os_windows)
      //  sanity check that its a dir
      if (!S_ISDIR(statbuf.st_mode)) 
	  {
         fprintf(stderr, "%s[%d]:  ERROR:  %s is not a dir\n", FILE__, __LINE__, 
               dot_dyninst_dir.c_str());
         return false;
      }
#else
      //  windows equiv to S_ISDIR??
#endif
   }

   std::string cpath = dot_dyninst_dir + std::string("/") + std::string(DEFAULT_CACHE_DIR);

   if (0 != stat(cpath.c_str(), &statbuf)) 
   {
      if (errno == ENOENT) 
	  {
#if defined (os_windows)
         if (0 != P_mkdir(cpath.c_str(), 0)) 
		 {
            fprintf(stderr, "%s[%d]:  failed to make %s\n", FILE__, __LINE__, 
                  cpath.c_str(), strerror(errno));
            return false;
         } 
#else
         if (0 != mkdir(cpath.c_str(), S_IRWXU)) 
		 {
            fprintf(stderr, "%s[%d]:  failed to make %s: %s\n", FILE__, __LINE__, 
                  cpath.c_str(), strerror(errno));
            return false;
         } 
#endif
      }
      else 
	  {
         fprintf(stderr, "%s[%d]:  stat(%s) failed: %s\n", FILE__, __LINE__, 
               cpath.c_str(), strerror(errno));
         return false;
      }
   }
   else 
   {
#if !defined (os_windows)
      //  sanity check that its a dir
      if (!S_ISDIR(statbuf.st_mode)) 
	  {
         fprintf(stderr, "%s[%d]:  ERROR:  %s is not a dir\n", FILE__, __LINE__, 
               cpath.c_str());
         return false;
      }
#else
      //  windows equiv to S_ISDIR??
#endif
   }

   //  Finally qualify path with platform to avoid collisions with cachenames
   //  when the home directory is shared across multiple machines.

   path = cpath + std::string("/") + std::string(platform_string()); 

   if (0 != stat(path.c_str(), &statbuf)) 
   {
      if (errno == ENOENT) 
	  {
#if defined (os_windows)
         if (0 != P_mkdir(path.c_str(), 0)) 
		 {
            fprintf(stderr, "%s[%d]:  failed to make %s\n", FILE__, __LINE__, 
                  path.c_str(), strerror(errno));
            return false;
         } 
#else
         if (0 != mkdir(path.c_str(), S_IRWXU)) 
		 {
            fprintf(stderr, "%s[%d]:  failed to make %s: %s\n", FILE__, __LINE__, 
                  path.c_str(), strerror(errno));
            return false;
         } 
#endif
      }
      else 
	  {
         fprintf(stderr, "%s[%d]:  stat(%s) failed: %s\n", FILE__, __LINE__, 
               path.c_str(), strerror(errno));
         return false;
      }
   }
   else 
   {
#if !defined (os_windows)
      //  sanity check that its a dir
      if (!S_ISDIR(statbuf.st_mode)) 
	  {
         fprintf(stderr, "%s[%d]:  ERROR:  %s is not a dir\n", FILE__, __LINE__, 
               path.c_str());
         return false;
      }
#else
      //  windows equiv to S_ISDIR??
#endif
   }

   serialize_printf("%s[%d]:  using default cache dir: %s\n", 
		   FILE__, __LINE__, path.c_str());

   return true;
}

bool SerDesBin::resolveCachePath(std::string full_file_path, std::string &cache_name)
{
   std::string path;
   char *path_dir = getenv(CACHE_DIR_VAR); 

   if (!path_dir) 
   {
      if (!getDefaultCacheDir(path)) 
	  {
         fprintf(stderr, "%s[%d]:  weird, failed to make $HOME/.dyninst/caches\n",
               FILE__, __LINE__);
         return false;
      }
   }

   // get size of file (this is encoded into cache name)
   struct stat statbuf;

   if (0 != stat(full_file_path.c_str(), &statbuf)) 
   {
      fprintf(stderr, "%s[%d]:  stat %s failed: %s\n", FILE__, __LINE__, 
            full_file_path.c_str(), strerror(errno));
      return false;
   }

   std::string short_name = extract_pathname_tail(full_file_path);
   serialize_printf("%s[%d]:  file %s short name: %s\n", FILE__, __LINE__, 
         full_file_path.c_str(), short_name.c_str());

   // construct cache name from cache path, cache prefix, short name, and size
   //  and now also platform, to prevent cache name collisions for configurations
   //  where the users home dir is shared.

   char sizestr[16];
   snprintf(sizestr, 16, "%d", (int)statbuf.st_size);
   cache_name = path + std::string("/") + std::string(CACHE_PREFIX) 
	   + short_name 
      + std::string("_") 
      + std::string(sizestr);

   serialize_printf("%s[%d]:  constructed cache name: %s\n", FILE__, __LINE__, cache_name.c_str());

   if (0 != stat(cache_name.c_str(), &statbuf)) 
   {
      if (errno != ENOENT) 
	  {
         //  Its OK if the file doesn't exist, but complain if we get a different
         // error
         fprintf(stderr, "%s[%d]:  stat %s failed: %s\n", FILE__, __LINE__, 
               cache_name.c_str(), strerror(errno));
      }
      serialize_printf("%s[%d]:  cache file %s does not exist\n", FILE__, __LINE__, 
            cache_name.c_str());
      return false;
   }

   serialize_printf("%s[%d]:  cache file %s exists\n", FILE__, __LINE__, cache_name.c_str());
   return true;
}

bool SerDesBin::cacheFileExists(std::string fname)
{
   std::string cache_name;
   return resolveCachePath(fname, cache_name); 
}

void SerDesBin::readHeaderAndVerify(std::string full_file_path, std::string cache_name, FILE *fptr)
{
   struct stat statbuf;

   if (0 != stat(full_file_path.c_str(), &statbuf)) 
   {
      char msg[1024];
      if (errno != ENOENT) 
	  {
         //  Its OK if the file doesn't exist, but complain if we get a different
         // error
         snprintf(msg, 1024, "%s[%d]:  stat %s failed: %s\n", FILE__, __LINE__, 
               full_file_path.c_str(), strerror(errno));
      }
      SER_ERR(msg);
   }

   FILE *f = NULL;
   if (fptr) 
      f = fptr;
   else 
   {
	   serialize_printf("%s[%d]:  trying to open %s\n", FILE__, __LINE__, 
			   cache_name.c_str());

      f = fopen(cache_name.c_str(), "r");

      if (!f) 
	  {
         char msg[1024];
         snprintf(msg, 1024, "%s[%d]:  failed to open file %s: %s\n", 
               FILE__, __LINE__, full_file_path.c_str(), strerror(errno));
		 fprintf(stderr, "%s\n", msg);
         SER_ERR(msg);
      }
   }

   size_t source_file_size = statbuf.st_size;

   cache_header_t header;

   int rc = fread(&header, sizeof(cache_header_t), 1, f);

   if (1 != rc) 
   {
      char msg[1024];
      snprintf(msg, 1024, "%s[%d]:  failed to read header struct for %s: %s, rc = %d\n", 
            FILE__, __LINE__, cache_name.c_str(), strerror(errno), rc);
      SER_ERR(msg);
   }

   if (header.cache_magic != (unsigned) CACHE_MAGIC) 
   {
      char msg[1024];
      snprintf(msg, 1024, "%s[%d]:  magic number check failure for %s--%s: got %d, expected %d\n", 
            FILE__, __LINE__, full_file_path.c_str(), cache_name.c_str(), 
			header.cache_magic, CACHE_MAGIC);
      SER_ERR(msg);
   }

   if (header.source_file_size != source_file_size) 
   {
      char msg[1024];
      snprintf(msg, 1024, "%s[%d]:  size discrepancy found for %s/%s\n", 
            FILE__, __LINE__, full_file_path.c_str(), cache_name.c_str());
      SER_ERR(msg);
   }

   if (!verifyChecksum(full_file_path, header.sha1)) 
   {
      char msg[1024];
      snprintf(msg, 1024, "%s[%d]:  checksum discrepancy found for %s/%s\n", 
            FILE__, __LINE__, full_file_path.c_str(), cache_name.c_str());

      if (!invalidateCache(cache_name)) 
      {
         fprintf(stderr, "%s[%d]:  failed to invalidate cache for file %s/%s\n", 
                 FILE__, __LINE__, full_file_path.c_str(), cache_name.c_str());
      }

      SER_ERR(msg);
   }

   if (!fptr)
   {
	   fprintf(stderr, "%s[%d]:  closing file pointer here\n", FILE__, __LINE__);
      fclose (f);
   }
}


void SerDesBin::writeHeaderPreamble(FILE *f, std::string full_file_path, 
		std::string /*cache_name*/)
{
   serialize_printf("%s[%d]:  welcome to write header preamble for %s\n", 
		   FILE__, __LINE__, full_file_path.c_str());

   //  get a few bits of info on this file to construct the header of the cache
   //  file...  checksum, size, ...  not mtime, since we don't care if someone 
   //  copies the file around

   struct stat statbuf;

   if (0 != stat(full_file_path.c_str(), &statbuf)) 
   {
      char msg[1024];
      snprintf(msg, 1024, "%s[%d]:  stat %s failed: %s\n", FILE__, __LINE__, 
            full_file_path.c_str(), strerror(errno));
      SER_ERR(msg);
   }

   cache_header_t header;
   header.cache_magic = CACHE_MAGIC;
   header.source_file_size = statbuf.st_size;

   if (NULL == sha1_file(full_file_path.c_str(), header.sha1)) 
   {
      char msg[1024];
      snprintf(msg, 1024, "sha1_file failed\n");
      SER_ERR(msg);
   }

   int rc = fwrite(&header, sizeof(cache_header_t), 1, f);

   if (1 != rc) 
      SER_ERR("fwrite");

}

bool SerDesBin::verifyChecksum(std::string &full_file_path, 
      const char comp_checksum[SHA1_DIGEST_LEN*2])
{
   char new_checksum[SHA1_DIGEST_LEN*2]; 

   if (NULL == sha1_file(full_file_path.c_str(), new_checksum)) 
   {
      fprintf(stderr, "%s[%d]:  sha1_file(%s) failed \n", 
            FILE__, __LINE__, full_file_path.c_str());
      return false;
   }

   if (strncmp(comp_checksum, new_checksum, SHA1_DIGEST_LEN*2)) 
   {
      fprintf(stderr, "%s[%d]:  sha1_file(%s): checksum mismatch: \n\told:%s\n\tnew:%s\n", 
            FILE__, __LINE__, full_file_path.c_str(), comp_checksum, new_checksum);
      return false;
   }

   return true;
}

bool SerDesBin::invalidateCache(std::string cache_name) 
{
   if (-1 == P_unlink(cache_name.c_str())) 
   {
      fprintf(stderr, "%s[%d]:  unlink(%s): %s\n", FILE__, __LINE__, 
            cache_name.c_str(), strerror(errno));
      return false;
   }

   return true;
}

void SerDesBin::file_start(std::string &/*full_file_path*/)
{
}

void SerDesBin::vector_start(unsigned long &size, const char *) DECLTHROW (SerializerError)
{
	if (iomode_ == sd_deserialize) 
		size = 0UL;

   //  before reading/writing a vector, we need to read its size
   //  (so we know how many elements to read/alloc on deserialize

	magic_check(FILE__, __LINE__);
   translate(size);
	magic_check(FILE__, __LINE__);
	if (iomode_ == sd_deserialize) 
	{
		serialize_printf("%s[%d]:  DESERIALIZE VECTOR START:  size = %lu\n", 
				FILE__, __LINE__, size);
	}
	else
		serialize_printf("%s[%d]:  SERIALIZE VECTOR START:  size = %lu\n", 
				FILE__, __LINE__, size);
}

void SerDesBin::vector_end()
{
   //  don't need to do anything
}

void SerDesBin::multimap_start(unsigned long &size, const char *) DECLTHROW (SerializerError)
{
   //  before reading/writing a multimap, we need to read its size
   //  (so we know how many elements to read/alloc on deserialize
   translate(size);
}

void SerDesBin::multimap_end()
{
   //  don't need to do anything
}

void SerDesBin::pair_start(const char *) DECLTHROW (SerializerError)
{
}

void SerDesBin::pair_end()
{
   //  don't need to do anything
}

void SerDesBin::hash_map_start(unsigned long &size, const char *) DECLTHROW (SerializerError)
{
   //  before reading/writing a hash map, we need to read its size
   //  (so we know how many elements to read/alloc on deserialize
   translate(size);
}

void SerDesBin::hash_map_end() 
{
}

void SerDesBin::annotation_start(AnnotationClassID &a_id, void *&parent_id, 
		sparse_or_dense_anno_t &sod, const char *, const char *)
{
	//serialize_printf("%s[%d]:  welcome to  annotation_start: aid = %d\n", FILE__, __LINE__, a_id);

	unsigned int lsod = 0;
	lsod = (int)sod;
	magic_check(FILE__, __LINE__);
	translate(a_id);
	assert(sizeof(Address) == sizeof(void *));
	translate((Address &)parent_id);
	//translate((unsigned short &)sod);
	translate(lsod);
	magic_check(FILE__, __LINE__);

	if (iomode() == sd_deserialize)
		sod = (sparse_or_dense_anno_t)lsod;

	serialize_printf("%s[%d]:  leaving %s annotation_start: aid = %d, id = %p\n", 
			FILE__, __LINE__, 
			(iomode_ == sd_serialize) ? "serialize" : "deserialize", a_id, parent_id);
}

void SerDesBin::annotation_end()
{
	//magic_check(FILE__, __LINE__);
   //  don't need to do anything
}

void SerDesBin::annotation_container_start(void *&id)
{
	magic_check(FILE__, __LINE__);
	assert(sizeof(Address) == sizeof(void *));
	translate((Address &)id);
	magic_check(FILE__, __LINE__);
}

void SerDesBin::annotation_container_end()
{
	magic_check(FILE__, __LINE__);
   //  don't need to do anything
}
void SerDesBin::annotation_container_item_start(void *&id)
{
	magic_check(FILE__, __LINE__);
	assert(sizeof(Address) == sizeof(void *));
	translate((Address &)id);
	magic_check(FILE__, __LINE__);
}

void SerDesBin::annotation_container_item_end()
{
	magic_check(FILE__, __LINE__);
   //  don't need to do anything
}

void SerDesBin::magic_check(const char *file__, unsigned int line__)
{
	unsigned short magic = 33;
	translate(magic);
	if (iomode_ == sd_deserialize)
	{
		if (magic != 33)
		{
			fprintf(stderr, "%s[%d]:  OUT OF SYNC HERE\n", file__, line__);
			if (isEOF())
				fprintf(stderr, "%s[%d]:  GOT EOF\n", file__, line__);
			abort();
		}
	}
}

void SerDesBin::annotation_list_start(Address &id, unsigned long &nelem, const char *)
{
   if (iomode_ == sd_deserialize) 
   {
	   nelem = 0UL;
   }

	if (nelem)
	{
		serialize_printf("%s[%d]: enter annotation_list_start id = %p, nelem = %ld\n", 
				FILE__, __LINE__, (void *)id, nelem);
	}

	magic_check(FILE__, __LINE__);
	translate(id);
	translate(nelem);

	if (nelem)
		serialize_printf("%s[%d]: leave annotation_list_start id = %p, nelem = %ld\n", FILE__, __LINE__, (void *)id, nelem);
}

void SerDesBin::annotation_list_end()
{
   //  don't need to do anything
}

void SerDesBin::translate(bool &param, const char *tag)
{
   int rc;
   if (iomode_ == sd_serialize) 
   {
      rc = fwrite(&param, sizeof(bool), 1, f);

      if (1 != rc) 
         SER_ERR("fwrite");
   }
   else 
   {
      rc = fread(&param, sizeof(bool), 1, f);

      if (1 != rc) 
         SER_ERR("fread");
   }

   if (noisy)
      serialize_printf("%s[%d]:  %sserialize %s=%s\n", FILE__, __LINE__,
            iomode_ == sd_serialize ? "" : "de", 
            tag ? tag : "no-tag",
            param ? "true": "false");
}

void SerDesBin::translate(char &param, const char *tag)
{
   int rc;

   if (iomode_ == sd_serialize) 
   {
      rc = fwrite(&param, sizeof(char), 1, f);

      if (1 != rc) 
         SER_ERR("fwrite");
   }
   else 
   {
      rc = fread(&param, sizeof(char), 1, f);

      if (1 != rc) 
         SER_ERR("fread");
   }

   if (noisy)
      serialize_printf("%s[%d]:  %sserialize %s=%c\n", FILE__, __LINE__,
            iomode_ == sd_serialize ? "" : "de", 
            tag ? tag : "no-tag", param);
}

void SerDesBin::translate(int &param, const char *tag)
{
   int rc;
   if (iomode_ == sd_serialize) 
   {
      rc = fwrite(&param, sizeof(int), 1, f);

      if (1 != rc) 
         SER_ERR("fwrite");
   }
   else 
   {
	   errno = 0;
	   rc = fread(&param, sizeof(int), 1, f);

      if (1 != rc) 
	  {
		  //fprintf(stderr, "%s[%d]:  failed to deserialize int-'%s', rc = %d:%s, noisy = %d\n", 
		//		  FILE__, __LINE__, tag ? tag : "no_tag", rc, strerror(errno), noisy);
		 // fprintf(stderr, "isEOF = %s\n", isEOF() ? "true" : "false");
         SER_ERR("fread");
	  }
   }

   if (noisy)
      serialize_printf("%s[%d]:  %sserialize %s=%d\n", FILE__, __LINE__,
            iomode_ == sd_serialize ? "" : "de", 
            tag ? tag : "no-tag", param);
}

void SerDesBin::translate(long &param, const char *tag)
{
   int rc;
   if (iomode_ == sd_serialize) 
   {
      rc = fwrite(&param, sizeof(long), 1, f);

      if (1 != rc) 
         SER_ERR("fwrite");
   }
   else 
   {
      rc = fread(&param, sizeof(long), 1, f);

      if (1 != rc) 
         SER_ERR("fread");
   }

   if (noisy)
      serialize_printf("%s[%d]:  %sserialize %s=%lu\n", FILE__, __LINE__,
            iomode_ == sd_serialize ? "" : "de", 
            tag ? tag : "no-tag", param);
}

void SerDesBin::translate(unsigned short &param, const char *tag)
{
	short lshort = static_cast<short>(param);
	translate (lshort, tag);
	param = static_cast<unsigned short>(lshort);
}

void SerDesBin::translate(short &param, const char *tag)
{
   int rc;
   if (iomode_ == sd_serialize) 
   {
      rc = fwrite(&param, sizeof(short), 1, f);

      if (1 != rc) 
         SER_ERR("fwrite");
   }
   else 
   {
      rc = fread(&param, sizeof(short), 1, f);

      if (1 != rc) 
         SER_ERR("fread");
   }

   if (noisy)
      serialize_printf("%s[%d]:  %sserialize %s=%d\n", FILE__, __LINE__,
            iomode_ == sd_serialize ? "" : "de", 
            tag ? tag : "no-tag", param);
}

void SerDesBin::translate(unsigned int &param, const char * tag)
{
   //  overkill for a typecast??
   translate( (int &) param, tag);
}

void SerDesBin::translate(float &param, const char *tag)
{
   int rc;
   if (iomode_ == sd_serialize) 
   {
      rc = fwrite(&param, sizeof(float), 1, f);

      if (1 != rc) 
         SER_ERR("fwrite");
   }
   else 
   {
      rc = fread(&param, sizeof(float), 1, f);

      if (1 != rc) 
         SER_ERR("fread");
   }

   if (noisy)
      serialize_printf("%s[%d]:  %sserialize %s=%e\n", FILE__, __LINE__,
            iomode_ == sd_serialize ? "" : "de", 
            tag ? tag : "no-tag", param);
}

void SerDesBin::translate(double &param, const char *tag)
{
   int rc;
   if (iomode_ == sd_serialize) 
   {
      rc = fwrite(&param, sizeof(double), 1, f);

      if (1 != rc) 
         SER_ERR("fwrite");
   }
   else 
   {
      rc = fread(&param, sizeof(double), 1, f);

      if (1 != rc) 
         SER_ERR("fread");
   }

   if (noisy)
      serialize_printf("%s[%d]:  %sserialize %s=%g\n", FILE__, __LINE__,
            iomode_ == sd_serialize ? "" : "de", 
            tag ? tag : "no-tag", param);
}

void SerDesBin::translate(Address &param, const char *tag)
{
   int rc;
   if (iomode_ == sd_serialize) 
   {
      rc = fwrite(&param, sizeof(Address), 1, f);

      if (1 != rc) 
         SER_ERR("fwrite");
   }
   else 
   {
	   memset(&param, 0, sizeof(Address));
      rc = fread(&param, sizeof(Address), 1, f);

      if (1 != rc) 
         SER_ERR("fread");
   }

   if (noisy)
      serialize_printf("%s[%d]:  %sserialize %s=%lx\n", FILE__, __LINE__,
            iomode_ == sd_serialize ? "" : "de", 
            tag ? tag : "no-tag", param);
}

void SerDesBin::translate(void * &param, const char *tag)
{
   int rc;
   if (iomode_ == sd_serialize) 
   {
      rc = fwrite(&param, sizeof(void *), 1, f);

      if (1 != rc) 
         SER_ERR("fwrite");
   }
   else 
   {
      rc = fread(&param, sizeof(void *), 1, f);

      if (1 != rc) 
         SER_ERR("fread");
   }

   if (noisy)
      serialize_printf("%s[%d]:  %sserialize %s=%p\n", FILE__, __LINE__,
            iomode_ == sd_serialize ? "" : "de", 
            tag ? tag : "no-tag", param);
}

void SerDesBin::translate(const char * &param, int bufsize, const char *tag)
{
   //  string output format is
   //  [1]  length of string
   //  [2]  string data
   int rc, len;
   if (iomode_ == sd_serialize) 
   {
      len = strlen(param);

      rc = fwrite( &len, sizeof(int), 1, f);

      if (1 != rc) 
         SER_ERR("fwrite");

      rc = fwrite(param, sizeof(char), len, f);

      if (len != rc) 
         SER_ERR("fwrite");
   }
   else 
   {
      rc = fread(&len, sizeof(int), 1, f);

      if (1 != rc)  
      {
         fprintf(stderr, "%s[%d]:  fread, got %d not 1: %s\n", FILE__, __LINE__, rc,  strerror(errno));
         SER_ERR("fread");
      }

      if (len > bufsize) 
      {
         fprintf(stderr, "%s[%d]:  insufficient buffer: %d, need %d...  truncation....\n", FILE__, __LINE__, bufsize, len);
		 len = bufsize;

         //char msg[1024];
         //snprintf(msg, 1024, "not enough space in string buffer, %d needed", len);
         //SER_ERR("msg");
      }

      if (len < 0) 
      {
         fprintf(stderr, "%s[%d]:  bad bufsize %d for %s\n", FILE__, __LINE__, len, tag ? tag : "no_tag");
         char msg[1024];
         snprintf(msg, 1024, "bad bufsize, %d ", len);
         SER_ERR("msg");
      }

	  char *l_ptr = const_cast<char *> (param);
	  if (len != 0)
	  {
		  rc = fread(l_ptr, sizeof(char), len, f);

		  if (len != rc) 
		  {
			  fprintf(stderr, "%s[%d]:  fread, got %d not %d: %s\n", 
					  FILE__, __LINE__, rc, len, strerror(errno));
			  SER_ERR("fread");
		  }
	  }
	  else 
	  {
		  //  Zero length strings are allowed
		  //fprintf(stderr, "%s[%d]:  WARN:  zero length string for %s\n", FILE__, __LINE__, tag ? tag : "no_tag_provided");
	  }

	  l_ptr[len] = '\0';

   }

   if (noisy)
      serialize_printf("%s[%d]:  %sserialize %s=%s\n", FILE__, __LINE__,
            iomode_ == sd_serialize ? "" : "de", 
            tag ? tag : "no-tag", param);
}

void SerDesBin::translate(char * &param, int bufsize, const char *tag)
{
   const char * p = const_cast<const char *>(param);
   translate(p, bufsize, tag);
   param = const_cast<char *>(p);
}

void SerDesBin::translate(std::string &param, const char *tag)
{
   if (iomode_ == sd_serialize) 
   {
      const char *cstr = param.c_str();
      translate(cstr, 0, tag);
   }
   else 
   {
      char buf[4096];
      const char *buf2 = buf;
      translate(buf2, 4096, tag);
      param = std::string(buf2);
   }
   if ((iomode_ == sd_deserialize) ||strstr(param.c_str(), "file")) 
   {
      serialize_printf("%s[%d]:  %sserializing string %s--%s, len = %lu\n", 
            FILE__, __LINE__, (iomode_ == sd_serialize) ? "" : "de", tag ? tag : "unnamed",
              param.c_str(), (unsigned long) param.length());
   }
}

void SerDesBin::translate(std::vector<std::string> &param, const char *tag, const char * elem_tag)
{
   //  string list output format is
   //  [1]  length of list, n
   //  [2]  <n> strings

   unsigned long nelem = param.size();
   translate(nelem, tag);

   if (iomode_ == sd_serialize) 
   {
      for (unsigned long i = 0; i < nelem; ++i) 
      {
         translate(param[i], elem_tag);
      }
   }
   else 
   {
      param.resize(nelem);
      for (unsigned long i = 0; i < nelem; ++i) 
      {
         param[i] = "";
         translate(param[i], elem_tag);
      }
   }
}

SerializerBase *Serializable::lookupExistingSerializer()
{
	//fprintf(stderr, "%s[%d]:  lookupExistingSerializer: index = %d, serializers.size() = %lu\n", FILE__, __LINE__, active_serializer_index, SerializerBase::active_serializers.size());

	if (active_serializer_index == (unsigned short) -1)
		return NULL;

	if (SerializerBase::active_serializers.size() <= active_serializer_index)
		return NULL;

	return SerializerBase::active_serializers[active_serializer_index];
}

namespace Dyninst {

bool ser_operation(SerializerBase *sb, ser_post_op_t &op, const char *tag)
{
	try 
	{
		gtranslate(sb, op, serPostOp2Str, tag);
	}
	catch (const Dyninst::SerializerError &)
	{   
		if (isEOF(sb))
		{   
			serialize_printf("%s[%d]:  got EOF\n", FILE__, __LINE__);
		}   
		else
		{
			fprintf(stderr, "%s[%d]:  SerializerError thrown\n", FILE__, __LINE__);
		}
		return false;
	}
	catch (...)
	{
		fprintf(stderr, "%s[%d]:  Unknown error thrown\n", FILE__, __LINE__);
		return false;
	}

	return true;

}

void throw_ser_err(const char *file__, unsigned int line, const char *msg)
{
	throw SerializerError(file__, line, std::string(msg));
}
void serialize_annotatable_id(SerializerBase *sb, void *&id, const char *tag)
{
	Address l_id = (Address &)id;
	gtranslate(sb, l_id, tag);
	id = (void *)l_id;
}

bool set_sb_annotatable_sparse_map(SerializerBase *sb, AnnotatableSparse *as, void *id)
{
	sb->set_annotatable_sparse_map(as, id);
	return true;
}

bool set_sb_annotatable_dense_map(SerializerBase *sb, AnnotatableDense *ad, void *id)
{
	sb->set_annotatable_dense_map(ad, id);
	return true;
}

void SerializerBase::set_annotatable_sparse_map(AnnotatableSparse *as, void *id)
{
	(*sparse_annotatable_map)[id] = as;
}

void SerializerBase::set_annotatable_dense_map(AnnotatableDense *ad, void *id)
{
	(*dense_annotatable_map)[id] = ad;
}

unsigned short get_serializer_index(SerializerBase *sb)
{
	if (!sb) return (unsigned short) -1;
	return sb->getIndex();
}
} // namespace dyninst

SerializerBase::SerializerBase(SerContextBase *scb, std::string name_, 
		std::string filename, 
		iomode_t dir, 
		bool verbose):
	sparse_annotatable_map(NULL),
	dense_annotatable_map(NULL)
{
	if (!scb)
	{
		serializer_printf("%s[%d]:  ERROR:  no context for serializer\n", FILE__, __LINE__);
		return;
	}

	sparse_annotatable_map = new dyn_hash_map<void *, AnnotatableSparse *>();
	dense_annotatable_map = new dyn_hash_map<void *, AnnotatableDense *>();
	scon = scb;
	serializer_name = std::string(name_);

	serialize_printf("%s[%d]:  before new SerFile, scb = %p, name = %s/%s\n", 
			FILE__, __LINE__, scb, serializer_name.c_str(), filename.c_str());

	sf = new SerFile(std::string(filename), dir, verbose);
	assert(sf);

	sd = sf->getSD();
	if (!sd) 
	{
		fprintf(stderr, "%s[%d]:  failed to get sd here\n", FILE__, __LINE__);
	}
	assert(scb);

	scb->serfilename = sf->getCacheFileName();
	if (dir == sd_serialize)
	{
		ser_index = (unsigned short) active_serializers.size();
		active_serializers.push_back(this);
	}
}

SerializerBase::SerializerBase() : 
	ser_index((unsigned short) -1),
	sparse_annotatable_map(NULL),
	dense_annotatable_map(NULL)
{
	fprintf(stderr, "%s[%d]:  WARN:  inside default ctor\n", FILE__, __LINE__);
	sparse_annotatable_map = new dyn_hash_map<void *, AnnotatableSparse *>();
	dense_annotatable_map = new dyn_hash_map<void *, AnnotatableDense *>();
}

SerializerBase::~SerializerBase()
{
	serialize_printf("%s[%d]:  serializer %p-%sdtor\n", FILE__, __LINE__,
			this, serializer_name.c_str());
}

unsigned short get_serializer_index(SerializerBase *sb)
{
	assert(sb);
	return sb->getIndex();
}

unsigned short SerializerBase::getIndex()
{
	return ser_index;
}

void SerializerBase::globalDisable()
{
	global_disable = true;
}
bool SerializerBase::serializationDisabled()
{
	return global_disable;
}

void SerializerBase::globalEnable()
{
	global_disable = false;
}

SerContextBase *SerializerBase::getContext() 
{
	return scon;
}

bool SerializerBase::isInput () 
{
	return iomode() == sd_deserialize;
}

bool SerializerBase::isOutput () 
{
	return iomode() == sd_serialize;
}

SerializerBase *SerializerBase::getSerializer(std::string subsystem, std::string fname)
{
	dyn_hash_map<std::string, subsystem_serializers_t>::iterator ssiter; 
	ssiter = all_serializers.find(subsystem);

	if (ssiter == all_serializers.end()) 
	{
		fprintf(stderr, "%s[%d]:  no serializer for subsystem %s\n", 
				FILE__, __LINE__, subsystem.c_str());
		return NULL;
	}

	subsystem_serializers_t &subsys_map = ssiter->second;

	dyn_hash_map<std::string, SerializerBase *>::iterator sbiter;
	sbiter = subsys_map.find(fname);
	if (sbiter == subsys_map.end()) 
	{
		fprintf(stderr, "%s[%d]:  no serializer for filename %s\n", 
				FILE__, __LINE__, fname.c_str());
		return NULL;
	}

	SerializerBase *sb =  sbiter->second;
	if (!sb) 
	{
		fprintf(stderr, "%s[%d]:  ERROR:  NULL serializer\n", FILE__, __LINE__);
		return NULL;
	}

	return sb;
}

bool SerializerBase::addSerializer(std::string subsystem, 
		std::string fname, SerializerBase *sb)
{
   subsystem_serializers_t ss_serializers;
   dyn_hash_map<std::string, subsystem_serializers_t>::iterator ssiter; 
   ssiter =  all_serializers.find(subsystem);

   if (ssiter == all_serializers.end()) 
   {
      //  make an entry 
      all_serializers[subsystem] = ss_serializers;
   }

   ss_serializers = all_serializers[subsystem];

   dyn_hash_map<std::string, SerializerBase *>::iterator sbiter;
   sbiter = ss_serializers.find(fname);
   if (sbiter != ss_serializers.end()) 
   {
      serializer_printf("%s[%d]:  already have serializer for filename %s\n", 
            FILE__, __LINE__, fname.c_str());
      return false;
   }

   //  add serializer to map since it does not exist there already

   ss_serializers[fname] = sb;
   return true;
}

iomode_t SerializerBase::iomode()
{
   if (sd) 
   {
      return sd->iomode();
   }

   fprintf(stderr, "%s[%d]:  no sd for iomode query\n", FILE__, __LINE__);
   return sd_serialize;
}

void SerializerBase::serialize_annotations(void *id, 
		std::vector<ser_rec_t> &sers, const char *tag)
{
	Address id_add = (Address) id;

	unsigned long nelem = 0UL;
	if (isOutput())
		nelem =	sers.size();

	getSD().annotation_list_start(id_add, nelem);

	if (nelem)
		serialize_printf("%s[%d]:  need to %s %lu annos\n", 
				FILE__, __LINE__, isInput() ? "deserialize" : "serialize", nelem);

	if (sers.size())
		serialize_printf( "%s[%d]: serialize_annotations:  %s, id = %p, nelem = %lu\n", 
				FILE__, __LINE__, isInput() ? "deserialize" : "serialize", 
				(void *)id_add, nelem);

	for (unsigned long i = 0; i < nelem; ++i)
	{
		AnnotationClassBase *acb = NULL;
		void *my_anno = NULL;
		AnnotationClassID a_id = (AnnotationClassID) -1;
		void *lparent_id = NULL;
		sparse_or_dense_anno_t lsod = sparse;

		if (isOutput())
		{
			acb = sers[i].acb;
			assert(acb);
			a_id = acb->getID();
			my_anno = sers[i].data;
			lparent_id = sers[i].parent_id;
			lsod = sers[i].sod;
		}

		getSD().annotation_start(a_id, lparent_id, lsod, acb ? acb->getName().c_str() : NULL);

		if (isInput())
		{
			acb = AnnotationClassBase::findAnnotationClass(a_id);

			if (!acb)
			{
				AnnotationClassBase::dumpAnnotationClasses();
				fprintf(stderr, "%s[%d]:  FIXME:  cannot find annotation class for id %d\n", 
						FILE__, __LINE__, a_id);
				return;
			}
			else
			{
				serialize_printf("%s[%d]:  got annotation type id=%d\n", 
						FILE__, __LINE__, acb->getID());
			}

			//  when deserializing, we need to allocate an object
			//  of the type of the annotation before deserializing into it.
			serialize_printf("%s[%d]:  before allocation\n", FILE__, __LINE__);
			my_anno = acb->allocate();
			assert(my_anno);

			ser_rec_t sr;
			sr.data = my_anno;
			sr.acb = acb;
			sers.push_back(sr);
			serialize_printf("%s[%d]:  created deserialize rec: %p/%p\n", 
					FILE__, __LINE__, my_anno, acb);
		}

		ser_func_t sf = acb->getSerializeFunc();

		assert(sf);
		assert(my_anno);

		//  execute the serialization function for this annotation
		serialize_printf("%s[%d]:  calling serialize func for type %s\n", 
				FILE__, __LINE__, acb->getTypeName());
		(*sf)(my_anno, this, tag);

		getSD().annotation_end();
	}
	getSD().annotation_list_end();
}

namespace Dyninst {

void annotation_start(SerializerBase *sb, AnnotationClassID &a_id, 
		void *&lparent_id, sparse_or_dense_anno_t &lsod, const char *)
{
	sb->annotation_start(a_id, lparent_id, lsod, NULL);
	serialize_printf("%s[%d]:  leaving to annotation_start:  id = %d\n", 
			FILE__, __LINE__, a_id);
}

void annotation_end(SerializerBase *sb)
{
	sb->annotation_end();
}

void annotation_container_start(SerializerBase *sb, void *&id)
{
	sb->annotation_container_start(id);
}

void annotation_container_end(SerializerBase *sb)
{
	sb->annotation_container_end();
}

void annotation_container_item_start(SerializerBase *sb, void *&id)
{
	sb->annotation_container_item_start(id);
}

void annotation_container_item_end(SerializerBase *sb)
{
	sb->annotation_container_item_end();
}

void vector_start(SerializerBase *sb, unsigned long &nelem, const char *tag)
{	
	sb->vector_start(nelem, tag);
}

void vector_end(SerializerBase *sb)
{
	sb->vector_end();
}

AnnotationContainerBase * get_container(void *parent_id)
{
	AnnotationContainerBase *cont =  AnnotationContainerBase::getContainer(parent_id);
	if (!cont)
	{
		fprintf(stderr, "%s[%d]:  failed to find container with id %p\n", 
				FILE__, __LINE__, parent_id);
	}
	return cont;
}

bool deserialize_container_item(AnnotationContainerBase *acb, SerializerBase *sb)
{
	if (!acb->deserialize_item(sb))
	{
		fprintf(stderr, "%s[%d]:  failed to deserialize container item\n", FILE__, __LINE__);
		return false;
	}
	return true;
}

bool deserialize_container_item(SerializerBase *sb, void *parent_id)
{
	AnnotationContainerBase *cont =  AnnotationContainerBase::getContainer(parent_id);
	if (!cont)
	{
		fprintf(stderr, "%s[%d]:  failed to find container with id %p\n", 
				FILE__, __LINE__, parent_id);
		return false;
	}

	if (!cont->deserialize_item(sb))
	{
		fprintf(stderr, "%s[%d]:  failed to deserialize container item\n", 
				FILE__, __LINE__);
		return false;
	}
	return true;

}

}

void SerializerBase::annotation_start(AnnotationClassID &a_id, 
		void *&lparent_id, sparse_or_dense_anno_t &lsod, const char *)
{
	getSD().annotation_start(a_id, lparent_id, lsod, NULL);

	serialize_printf("%s[%d]:  leaving to annotation_start:  id = %d, ser_id = %d\n",
		   	FILE__, __LINE__, a_id, ser_index);
}

void SerializerBase::annotation_end()
{
	getSD().annotation_end();
}

void SerializerBase::annotation_container_start(void *&id)
{
	getSD().annotation_container_start(id);
}

void SerializerBase::annotation_container_end()
{
	getSD().annotation_container_end();
}

void SerializerBase::annotation_container_item_start(void *&id)
{
	getSD().annotation_container_item_start(id);
}

void SerializerBase::annotation_container_item_end()
{
	getSD().annotation_container_item_end();
}

void SerializerBase::magic_check(const char *file__, unsigned int line__)
{
	getSD().magic_check(file__, line__);
}

bool SerializerBase::serialize_post_annotation(void *parent_id, void *anno, 
		AnnotationClassBase *acb, sparse_or_dense_anno_t sod, const char *tag)
{
	serialize_printf("%s[%d]: serialize_post_annotation:  ser_id = %d, parent_id = %p\n", 
			FILE__, __LINE__, ser_index, parent_id);

	void *my_anno = NULL;
	AnnotationClassID a_id;
	sparse_or_dense_anno_t lsod = sod;
	void *lparent_id = parent_id;

	if (isOutput())
	{
		assert(acb);
		a_id = acb->getID();
		assert(anno);
		my_anno = anno;
	}

	getSD().annotation_start(a_id, lparent_id, lsod, acb ? acb->getName().c_str() : NULL);

	if (isInput())
	{
		acb = AnnotationClassBase::findAnnotationClass(a_id);

		if (!acb)
		{
			AnnotationClassBase::dumpAnnotationClasses();
			fprintf(stderr, "%s[%d]:  FIXME:  cannot find annotation class for id %d\n", 
					FILE__, __LINE__, a_id);
			return false;
		}
		else
		{
			fprintf(stderr, "%s[%d]:  got annotation type id=%d\n", 
					FILE__, __LINE__, acb->getID());
		}

		//  when deserializing, we need to allocate an object
		//  of the type of the annotation before deserializing into it.
		my_anno = acb->allocate();
		assert(my_anno);
		//parent_id = lparent_id;
		//sod = lsod;
	}

	ser_func_t sf = acb->getSerializeFunc();

	assert(sf);
	assert(my_anno);

	//  execute the serialization function for this annotation
	serialize_printf("%s[%d]:  calling serialize func for type %s\n", 
			FILE__, __LINE__, acb->getTypeName());

	(*sf)(my_anno, this, tag);

		getSD().annotation_end();

		if (isInput())
		{
			//  we have the (void *) annotation and the annotation type
			//  now lookup the object to which it belonged in the map of annotatable objects
			//  and add it as an annotation.

			if (lsod == sparse)
			{
				dyn_hash_map<void *, AnnotatableSparse *>::iterator iter;
				iter = sparse_annotatable_map->find(lparent_id);
				if (iter == sparse_annotatable_map->end())
				{
					fprintf(stderr, "%s[%d]:  ERROR:  cant find parent to assign anno to\n", 
							FILE__, __LINE__);
				}
				else
				{
					AnnotatableSparse *as = iter->second;
					assert(as);
					assert(acb);
					fprintf(stderr, "%s[%d]:  readding post annotation\n", FILE__, __LINE__);
					if (!as->addAnnotation(my_anno, acb->getID()))
					{
						fprintf(stderr, "%s[%d]:  ERROR:  failed to add annotation here\n", 
								FILE__, __LINE__);
					}
				}
			}
			else if (lsod == dense)
			{
				dyn_hash_map<void *, AnnotatableDense *>::iterator iter;
				iter = dense_annotatable_map->find(lparent_id);
				if (iter == dense_annotatable_map->end())
				{
					fprintf(stderr, "%s[%d]:  ERROR:  cant find parent to assign anno to\n", 
							FILE__, __LINE__);
				}
				else
				{
					AnnotatableDense *ad = iter->second;
					assert(ad);
					assert(acb);
					fprintf(stderr, "%s[%d]:  readding post annotation\n", FILE__, __LINE__);
					if (!ad->addAnnotation(my_anno, acb->getID()))
					{
						fprintf(stderr, "%s[%d]:  ERROR:  failed to add annotation here\n", 
								FILE__, __LINE__);
					}
				}
			}
			else
			{
				fprintf(stderr, "%s[%d]:  ERROR:  sparse/dense not set properly\n", 
						FILE__, __LINE__);
				return false;
			}
		}

		return true;
}

void SerializerBase::vector_start(unsigned long &size, const char *tag) 
{
   getSD().vector_start(size, tag);
}


void SerializerBase::vector_end() 
{
   getSD().vector_end();
}

void SerializerBase::hash_map_start(unsigned long &size, const char *tag) 
{
   getSD().hash_map_start(size, tag);
}

void SerializerBase::hash_map_end() 
{
   getSD().hash_map_end();
}

void SerializerBase::multimap_start(unsigned long &size, const char *tag) 
{
   getSD().multimap_start(size, tag);
}

void SerializerBase::multimap_end() 
{
   getSD().multimap_end();
}

void SerializerBase::pair_start(const char *tag) 
{
   getSD().pair_start(tag);
}

SerDes &SerializerBase::getSD()
{
	assert(sd);
	return *sd;
}

SerFile &SerializerBase::getSF()
{
	assert(sf);
	return *sf;
}

void SerializerBase::pair_end() 
{
   getSD().pair_end();
}

void SerializerBase::translate_base(short &v, const char *&t)
{
   getSD().translate(v, t);
}

void SerializerBase::translate_base(unsigned short &v, const char *&t)
{
   getSD().translate(v, t);
}

void SerializerBase::translate_base(bool &v, const char *&t)
{
   getSD().translate(v, t);
}

void SerializerBase::translate_base(char &v, const char *&t)
{
   getSD().translate(v, t);
}

void SerializerBase::translate_base(int &v, const char *&t)
{
   getSD().translate(v, t);
}

void SerializerBase::translate_base(unsigned int &v, const char *&t)
{
   getSD().translate(v, t);
}

void SerializerBase::translate_base(long &v, const char *&t)
{
   getSD().translate(v, t);
}

void SerializerBase::translate_base(unsigned long &v, const char *&t)
{
   getSD().translate(v, t);
}

void SerializerBase::translate_base(float &v, const char *&t)
{
   getSD().translate(v, t);
}

void SerializerBase::translate_base(double &v, const char *&t)
{
   getSD().translate(v, t);
}

void SerializerBase::translate_base(const char *&v, int bufsize, const char *&t)
{
   getSD().translate(v, bufsize, t);
}

void SerializerBase::translate_base(char *&v, int bufsize, const char *&t)
{
   getSD().translate(v, bufsize, t);
}

void SerializerBase::translate_base(void *&v, const char *&t)
{
   getSD().translate(v, t);
}

void SerializerBase::translate_base(std::string &v, const char *t)
{
   getSD().translate(v, t);
}

#endif

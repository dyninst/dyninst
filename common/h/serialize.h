/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __SERDES_H__
#define __SERDES_H__
#include "common/h/headers.h"

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <stdio.h>

#if defined(os_windows)
#if defined (cap_have_libxml)
#include <libxml/xmlversion.h>
#undef LIBXML_ICONV_ENABLED
#endif
#endif

#if defined (cap_have_libxml)
#include <libxml/xmlwriter.h>
#endif

#include "dynutil/h/util.h"
//#include "dynutil/h/Annotatable.h"
#include "dynutil/h/Serialization.h"
#include "common/h/Types.h"
#include "common/h/sha1.h"
#include "common/h/pathName.h"


namespace Dyninst {

#define CACHE_DIR_VAR "DYNINST_CACHE_DIR"
#define DEFAULT_DYNINST_DIR ".dyninstAPI"
#define DEFAULT_CACHE_DIR "caches"
#define CACHE_MAGIC 0x555
#define CACHE_PREFIX "cache_"

#ifndef PATH_MAX
#define PATH_MAX 512
#endif

//  SER_CATCH("string") is mostly for debugging...  it is just a default catch-block
//  that prints out a message and then throws another exception.  The idea is that,
//  when an exception is thrown, even though it comes with an informative message,
//  it is even more informative to know that call-path that produced it.
//  SER_CATCH provides a fairly non-intrusive way to add this functionality

#define SER_CATCH(x) catch (const SerializerError &err) { \
   fprintf(stderr, "%s[%d]: %s from %s[%d]\n", FILE__, __LINE__, \
         err.what(), err.file().c_str(), err.line()); \
   SER_ERR(x); }

void COMMON_EXPORT serialize_debug_init();

class SerDes;
class SerFile;

#if 0
class SerializerBase {

	public:
		//  TODO:  make these private or protected
   COMMON_EXPORT static dyn_hash_map<std::string, SerializerBase *> active_bin_serializers;
   static bool global_disable;
	private:

   SerFile *sf;
   SerDes *sd;

   std::string serializer_name;

   typedef dyn_hash_map<std::string, SerializerBase *> subsystem_serializers_t;
   COMMON_EXPORT static dyn_hash_map<std::string, subsystem_serializers_t> all_serializers;

   public:
   COMMON_EXPORT static void globalDisable()
   {
	   global_disable = true;
   }
   COMMON_EXPORT static bool serializationDisabled()
   {
	   return global_disable; 
   }

   COMMON_EXPORT static void globalEnable()
   {
	   global_disable = false;
   }
   COMMON_EXPORT virtual bool isXML() = 0;
   COMMON_EXPORT virtual bool isBin ()= 0;
   COMMON_EXPORT bool isInput () {return iomode() == sd_deserialize;}
   COMMON_EXPORT bool isOutput () {return iomode() == sd_serialize;}

   COMMON_EXPORT static void dumpActiveBinSerializers();

   COMMON_EXPORT SerializerBase(const char *name_, std::string filename, 
         iomode_t dir, bool verbose); 

   COMMON_EXPORT SerializerBase();
   
   COMMON_EXPORT virtual ~SerializerBase() 
   {
      serialize_printf("%s[%d]:  serializer %p-%sdtor\n", FILE__, __LINE__, 
            this, serializer_name.c_str());
   }

   COMMON_EXPORT virtual SerDes &getSD()  { assert(sd); return *sd;}
   COMMON_EXPORT SerFile &getSF() {assert(sf); return *sf;}
   COMMON_EXPORT std::string &name() {return serializer_name;}
   COMMON_EXPORT static SerializerBase *getSerializer(std::string subsystem, std::string fname);
   COMMON_EXPORT static bool addSerializer(std::string subsystem, std::string fname, SerializerBase *sb);

   COMMON_EXPORT virtual void vector_start(unsigned int &, const char * = NULL);
   COMMON_EXPORT virtual void vector_end();
   COMMON_EXPORT virtual void hash_map_start(unsigned int &size, const char *tag = NULL); 
   COMMON_EXPORT virtual void hash_map_end();
   COMMON_EXPORT void translate_base(bool &v, const char *&t);
   COMMON_EXPORT void translate_base(short &v, const char *&t);
   COMMON_EXPORT void translate_base(char &v, const char *&t);
   COMMON_EXPORT void translate_base(int &v, const char *&t);
   COMMON_EXPORT void translate_base(unsigned int &v, const char *&t);
   COMMON_EXPORT void translate_base(unsigned long &v, const char *&t);
   COMMON_EXPORT void translate_base(long &v, const char *&t);
   COMMON_EXPORT void translate_base(float &v, const char *&t);
   COMMON_EXPORT void translate_base(double &v, const char *&t);
   COMMON_EXPORT void translate_base(const char * &v, int bufsize, const char *&t);
   COMMON_EXPORT void translate_base(char * &v, int bufsize, const char *&t);
   COMMON_EXPORT void translate_base(std::string &v, const char *t);

   COMMON_EXPORT virtual iomode_t iomode(); 

   protected:


};


template <class T>
class ScopedSerializerBase : public SerializerBase
{
	T *scope;
	public:
	ScopedSerializerBase(T *scope_, const char *name_, std::string filename, 
			iomode_t dir, bool verbose) :
		SerializerBase(name_, filename, dir, verbose), scope(scope_) {}
	ScopedSerializerBase(T *scope_) :
		SerializerBase(), scope(scope_) {}
	virtual ~ScopedSerializerBase() {}
	T *getScope() {return scope;}
};

#endif

class SerDes {

	//  SerDes is a base class that provides generic serialization/deserialization
	//  access primitives and a common interface, (a toolbox, if you will).
	//  It is specialized (currently) by SerDesBin and SerDesXML, which implement the 
	//  actual low-level ser-des routines 

	//  anno_funcs is a mapping of annotation type
	//  onto functions used to deserialize that type of annotation
	//  NOTE:  annotation types identifiers might not be consistent between different
	//  runs of dyninst, since annotation name->type mapping is determined dynamically
	//  at runtime.  Thus, when deserializing annotations, a new mapping will have to be 
   //  constructed.

   public:

#if 0
      COMMON_EXPORT static dyn_hash_map<std::string, AnnoFunc > anno_funcs;

      //  old_anno_name_to_id_map keeps a running mapping of 
      //  annotation names onto annotation ids that was used when building
      //  the file that is being deserialized.  This info is used to 
      //  rebuild annotations information, the name<->type mapping may change
      //  between different runs of dyninst.
      dyn_hash_map<unsigned, std::string> old_anno_name_to_id_map;
#endif

   protected:

      iomode_t iomode_;

   public:

#if 0
      COMMON_EXPORT AnnoFunc *findAnnoFunc(unsigned anno_type, 
            std::string anno_name = AnnotatableBase::emptyString);

      COMMON_EXPORT static bool addAnnoFunc(std::string type_name, AnnoFunc sf);
#endif

      COMMON_EXPORT SerDes() {assert(0);}
      COMMON_EXPORT SerDes(iomode_t mode) : iomode_(mode){}
      COMMON_EXPORT virtual ~SerDes() {}

      COMMON_EXPORT virtual void file_start(std::string &/*full_file_path*/) {}
      COMMON_EXPORT virtual void vector_start(unsigned int &size, 
            const char *tag = NULL) DECLTHROW(SerializerError) = 0;
      COMMON_EXPORT virtual void vector_end() = 0;
      COMMON_EXPORT virtual void multimap_start(unsigned int &size, 
            const char *tag = NULL) DECLTHROW(SerializerError) = 0;
      COMMON_EXPORT virtual void hash_map_start(unsigned int &size, 
            const char *tag = NULL) DECLTHROW(SerializerError) = 0;
      COMMON_EXPORT virtual void hash_map_end() = 0;
      COMMON_EXPORT virtual void annotation_start(const char *string_id, 
            const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void annotation_end() = 0;

      COMMON_EXPORT virtual void multimap_end() = 0;

      COMMON_EXPORT virtual void translate(bool &param, const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void translate(char &param, const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void translate(int &param, const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void translate(long &param, const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void translate(short &param, const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void translate(unsigned int &param, const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void translate(float &param, const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void translate(double &param, const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void translate(Address &param, const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void translate(const char * &param, int bufsize = 0, 
            const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void translate(char * &param, int bufsize = 0, 
            const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void translate(std::string &param, const char *tag = NULL) = 0;
      COMMON_EXPORT virtual void translate(std::vector<std::string> &param, const char *tag = NULL,
            const char *elem_tag = NULL) = 0;

      COMMON_EXPORT virtual iomode_t iomode() {return iomode_;} 
};

class SerDesXML : public SerDes {
   friend class SerFile;
   friend bool COMMON_EXPORT ifxml_start_element(SerializerBase *, const char *);
   friend bool COMMON_EXPORT ifxml_end_element(SerializerBase *, const char *);
   friend bool COMMON_EXPORT start_xml_elem(SerDesXML &, const char *);
   friend bool COMMON_EXPORT end_xml_elem(SerDesXML &);



#if defined (cap_have_libxml)
      xmlTextWriterPtr writer;
      COMMON_EXPORT SerDesXML(xmlTextWriterPtr w, iomode_t mode)  : SerDes(mode), writer(w) { }
      COMMON_EXPORT static xmlTextWriterPtr init(std::string fname, iomode_t mode, bool verbose);
#else
      void *writer;
      COMMON_EXPORT SerDesXML(void * w, iomode_t mode)  : SerDes(mode), writer(w) { }
#endif

   public:
      COMMON_EXPORT SerDesXML() { assert(0);}
      COMMON_EXPORT virtual ~SerDesXML();

      COMMON_EXPORT virtual void vector_start(unsigned int &size, 
            const char *tag = NULL) DECLTHROW(SerializerError);
      COMMON_EXPORT virtual void vector_end();
      COMMON_EXPORT virtual void multimap_start(unsigned int &size, 
            const char *tag = NULL) DECLTHROW(SerializerError);
      COMMON_EXPORT virtual void multimap_end();
      COMMON_EXPORT virtual void hash_map_start(unsigned int &size, 
            const char *tag = NULL) DECLTHROW(SerializerError);
      COMMON_EXPORT virtual void hash_map_end();
      COMMON_EXPORT virtual void annotation_start(const char *string_id, const char *tag = NULL);
      COMMON_EXPORT virtual void annotation_end();
      COMMON_EXPORT virtual void translate(bool &param, const char *tag = NULL);
      COMMON_EXPORT virtual void translate(char &param, const char *tag = NULL);
      COMMON_EXPORT virtual void translate(int &param, const char *tag = NULL);
      COMMON_EXPORT virtual void translate(long &param, const char *tag = NULL);
      COMMON_EXPORT virtual void translate(short &param, const char *tag = NULL);
      COMMON_EXPORT virtual void translate(unsigned int &param, const char *tag = NULL);
      COMMON_EXPORT virtual void translate(float &param, const char *tag = NULL);
      COMMON_EXPORT virtual void translate(double &param, const char *tag = NULL);
      COMMON_EXPORT virtual void translate(Address &param, const char *tag = NULL);
      COMMON_EXPORT virtual void translate(const char * &param, int bufsize = 0, 
            const char *tag = NULL);
      COMMON_EXPORT virtual void translate(char * &param, int bufsize = 0, const char *tag = NULL);
      COMMON_EXPORT virtual void translate(std::string &param, const char *tag = NULL);
      COMMON_EXPORT virtual void translate(std::vector<std::string> &param, const char *tag = NULL,
            const char *elem_tag = NULL);

#if 0
      COMMON_EXPORT void start_element(const char *tag);
      COMMON_EXPORT void end_element();
      COMMON_EXPORT void xml_value(const char *val, const char *tag);
#endif
};

//class AnnotatableBase;

class SerDesBin : public SerDes {

   typedef struct {
      unsigned int cache_magic;
      unsigned int source_file_size; //  if size is different, don't bother with checksum
      char sha1[SHA1_DIGEST_LEN*2];
   } cache_header_t;

   FILE *f;

   bool noisy;

   public:

   //COMMON_EXPORT static dyn_hash_map<Address, AnnotatableBase *> annotatable_id_map;
   COMMON_EXPORT static FILE *init(std::string fname, iomode_t mode, bool verbose);

   COMMON_EXPORT SerDesBin() {assert(0);}

   COMMON_EXPORT SerDesBin(FILE *ff, iomode_t mode, bool verbose) : 
      SerDes(mode), 
      f(ff),  
      noisy(verbose) {}

   COMMON_EXPORT virtual ~SerDesBin();

   //COMMON_EXPORT static AnnotatableBase *findAnnotatee(void *id); 

   COMMON_EXPORT virtual void file_start(std::string &full_file_path);
   COMMON_EXPORT virtual void vector_start(unsigned int &size, 
         const char *tag = NULL) DECLTHROW(SerializerError);
   COMMON_EXPORT virtual void vector_end();
   COMMON_EXPORT virtual void multimap_start(unsigned int &size, 
         const char *tag = NULL) DECLTHROW(SerializerError);
   COMMON_EXPORT virtual void multimap_end();
   COMMON_EXPORT virtual void hash_map_start(unsigned int &size, 
         const char *tag = NULL) DECLTHROW(SerializerError);
   COMMON_EXPORT virtual void hash_map_end();
   COMMON_EXPORT virtual void annotation_start(const char *string_id, const char *tag = NULL);
   COMMON_EXPORT virtual void annotation_end();
   COMMON_EXPORT virtual void translate(bool &param, const char *tag = NULL);
   COMMON_EXPORT virtual void translate(char &param, const char *tag = NULL);
   COMMON_EXPORT virtual void translate(int &param, const char *tag = NULL);
   COMMON_EXPORT virtual void translate(long &param, const char *tag = NULL);
   COMMON_EXPORT virtual void translate(short &param, const char *tag = NULL);
   COMMON_EXPORT virtual void translate(unsigned int &param, const char *tag = NULL);
   COMMON_EXPORT virtual void translate(float &param, const char *tag = NULL);
   COMMON_EXPORT virtual void translate(double &param, const char *tag = NULL);
   COMMON_EXPORT virtual void translate(Address &param, const char *tag = NULL);
   COMMON_EXPORT virtual void translate(const char * &param, 
         int bufsize = 0, const char *tag = NULL);
   COMMON_EXPORT virtual void translate(char * &param, int bufsize = 0, const char *tag = NULL);
   COMMON_EXPORT virtual void translate(std::string &param, const char *tag = NULL);
   COMMON_EXPORT virtual void translate(std::vector<std::string> &param, const char *tag = NULL,
         const char *elem_tag = NULL);

   // readHeaderAndVerify just opens, verifies (checksum, magic compare), and closes
   // cache file, unless the FILE * is provided, in which case the file pointer is
   // advanced past the preamble and is not closed;

   COMMON_EXPORT static void readHeaderAndVerify(std::string full_file_path, 
         std::string cache_name, FILE *f = NULL);

   COMMON_EXPORT static void writeHeaderPreamble(FILE *f, std::string full_file_path, 
         std::string cache_name);

   COMMON_EXPORT static bool getDefaultCacheDir(std::string &cache_dir);
   COMMON_EXPORT static bool resolveCachePath(std::string fname, std::string &cache_name);
   COMMON_EXPORT static bool verifyChecksum(std::string &filename, 
         const char comp_checksum[SHA1_DIGEST_LEN]);
   COMMON_EXPORT static bool cacheFileExists(std::string fname);
   COMMON_EXPORT static bool invalidateCache(std::string cache_name);

};

bool start_xml_elem(void *writer, const char *tag);
bool end_xml_elem(void *);

#if 0
template <class T>
class SerializerXML : public ScopedSerializerBase<T> 
{
   public:
   COMMON_EXPORT virtual bool isXML() {return true;}
   COMMON_EXPORT virtual bool isBin () {return false;}

      COMMON_EXPORT SerializerXML(T *t, const char *name_, std::string filename, 
            iomode_t dir, bool verbose) :
         ScopedSerializerBase<T>(t, name_, filename, dir, verbose) {}

      COMMON_EXPORT virtual ~SerializerXML() {}

      COMMON_EXPORT SerDesXML &getSD_xml()
	  {
		  SerializerBase *sb = this;
		  SerDes &sd = sb->getSD();
		  SerDesXML *sdxml = dynamic_cast<SerDesXML *> (&sd);
		  assert(sdxml);
		  return *sdxml;
	  }

      COMMON_EXPORT static bool start_xml_element(SerializerBase *sb, const char *tag)
	  {
		  SerializerXML<T> *sxml = dynamic_cast<SerializerXML<T> *>(sb);

		  if (!sxml)
		  {
			  fprintf(stderr, "%s[%d]:  FIXME:  called xml function with non xml serializer\n",
					  FILE__, __LINE__);
			  return false;
		  }

		  SerDesXML sdxml = sxml->getSD_xml();
		  start_xml_elem(sdxml.writer, tag);
		  return true;
	  }

      COMMON_EXPORT static bool end_xml_element(SerializerBase *sb, const char *)
	  {
		  SerializerXML<T> *sxml = dynamic_cast<SerializerXML<T> *>(sb);

		  if (!sxml)
		  {
			  fprintf(stderr, "%s[%d]:  FIXME:  called xml function with non xml serializer\n",
					  FILE__, __LINE__);
			  return false;
		  }

		  SerDesXML sdxml = sxml->getSD_xml();
		  end_xml_elem(sdxml.writer);

		  return true;
	  }
};
#endif


#if 0
template <class T>
class SerializerBin : public ScopedSerializerBase<T> {
   friend class SerDesBin;

   public:
   virtual bool isXML() {return false;}
   virtual bool isBin () {return true;}

   SerializerBin(T *t)  :
	   ScopedSerializerBase<T>(t) {}

   SerializerBin(T *t, const char *name_, std::string filename, 
         iomode_t dir, bool verbose) :
	   ScopedSerializerBase<T>(t, name_, filename, dir, verbose)
   {
	   SerializerBase *sb = this;
	   if (sb->serializationDisabled())
	   {
		   fprintf(stderr, "%s[%d]:  Failing to construct Bin Translator:  global disable set\n",
				   FILE__, __LINE__);

		   throw SerializerError(FILE__, __LINE__,
				   std::string("serialization disabled"),
				   SerializerError::ser_err_disabled);
	   }

	   dyn_hash_map<std::string, SerializerBase *>::iterator iter;

	   iter = sb->active_bin_serializers.find(std::string(name_));

	   if (iter == sb->active_bin_serializers.end())
	   {
		   serialize_printf("%s[%d]:  Adding Active serializer for name %s\n",
				   FILE__, __LINE__, name_);

		   sb->active_bin_serializers[std::string(name_)] = this;
	   }
	   else
	   {
		   fprintf(stderr, "%s[%d]:  Weird, already have active serializer for name %s\n",
				   FILE__, __LINE__, name_);
	   }


   }

   virtual ~SerializerBin()
   {
	   serialize_printf("%s[%d]:  WELCOME TO SERIALIZER_BIN dtor\n", FILE__, __LINE__);
	   dyn_hash_map<std::string, SerializerBase *>::iterator iter;

	   SerializerBase *sb = this;
	   iter = sb->active_bin_serializers.find(sb->name());

	   if (iter == sb->active_bin_serializers.end())
	   {
		   fprintf(stderr, "%s[%d]:  Weird, no static ptr for name %s\n",
				   FILE__, __LINE__, sb->name().c_str());
	   }
	   else
	   {
		   serialize_printf("%s[%d]:  Removing active serializer for name %s\n",
				   FILE__, __LINE__, sb->name().c_str());
		   sb->active_bin_serializers.erase(iter);
	   }

   }

   SerDesBin &getSD_bin()
   {
	   SerializerBase *sb = this;
	   SerDes &sd = sb->getSD();
	   SerDesBin *sdbin = dynamic_cast<SerDesBin *> (&sd);
	   assert(sdbin);
	   return *sdbin;
   }


   static SerializerBin *findSerializerByName(const char *name_)
   {
	   dyn_hash_map<std::string, SerializerBase *>::iterator iter;

	   iter = SerializerBase::active_bin_serializers.find(std::string(name_));

	   if (iter == SerializerBase::active_bin_serializers.end())
	   {
		   fprintf(stderr, "%s[%d]:  No static ptr for name %s\n",
				   FILE__, __LINE__, name_);
		   SerializerBase::dumpActiveBinSerializers();
	   }
	   else
	   {
		   fprintf(stderr, "%s[%d]:  Found active serializer for name %s\n",
				   FILE__, __LINE__, name_);

		   return iter->second;
	   }

	   return NULL;
   }


};

#endif
//COMMON_EXPORT SerializationFunctionBase *findSerDesFuncForAnno(unsigned anno_type);
//COMMON_EXPORT SerFunc *findSerFuncForAnno(unsigned anno_type);



class SerFile {

   SerDes *sd;
#if defined (cap_have_libxml)
   xmlTextWriterPtr writer;
#else
   void * writer;
#endif
   FILE *f;

   public:

   COMMON_EXPORT SerDes *getSD() 
   {
      return sd;
   }

   COMMON_EXPORT SerFile(std::string fname, iomode_t mode, bool verbose = false) :
      writer (NULL), 
      f(NULL), 
      iomode_(mode), 
      noisy(verbose) 
   {
      char file_path[PATH_MAX];

      if (!resolve_file_path(fname.c_str(), file_path)) 
      {
         char msg[128];
         sprintf(msg, "failed to resolve path for '%s'\n", fname.c_str());
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
            fprintf(stderr, "%s[%d]:  ERROR:  failed to init xml writer\n", FILE__, __LINE__);
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

         f = SerDesBin::init(fname, mode, verbose);

         if (!f) 
         {
            fprintf(stderr, "%s[%d]:  failed to init file i/o\n", FILE__, __LINE__);
            assert(0);
         }

         sd = new SerDesBin(f,mode, verbose);
      }
   }

   COMMON_EXPORT iomode_t iomode() 
   {
      return iomode_;
   }

   static bool validCacheExistsFor(std::string full_file_path);

   protected:

   std::string filename;
   iomode_t iomode_;

   public:

   bool noisy;

};

template <class S, class T>
class SpecAdaptor {

   public:

      COMMON_EXPORT SpecAdaptor() {}

      COMMON_EXPORT T *operator()(S *s, T &t, const char *tag) 
      {
         s->translate_base(t, tag);
         return &t;
      }
};

template <class S, class T>
class SpecAdaptor<S, T *> {

   public:

      COMMON_EXPORT SpecAdaptor() {}

      COMMON_EXPORT T* operator()(S *s, T *t, const char *tag) 
      {
         assert(t);
         assert(s);
         s->translate_base(*t, tag);
         return t;
      }
};

template <class S, class T> 
void sd_translate(S *sd, T &it, const char * tag) 
{
   fprintf(stderr, "%s[%d]:  welcome to sd_translate<%s, %s>(%p)\n", 
         FILE__, __LINE__, 
         typeid(S).name(), 
         typeid(T).name(), &it);

   SpecAdaptor<S,T> saf;

   if (NULL == saf(sd, it, tag)) 
   {
      fprintf(stderr, "%s[%d]:  ERROR here\n", FILE__, __LINE__);
   }

   return;
}


template<class S, class T, class TT2> 
class trans_adaptor<S, dyn_hash_map<T, TT2> > {

   public:

      COMMON_EXPORT trans_adaptor() 
      {
         fprintf(stderr, "%s[%d]:  welcome to trans_adaptor<%s, hash<%s, %s> >()\n",
               FILE__, __LINE__,
               typeid(S).name(),
               typeid(T).name(), typeid(TT2).name() );
      }

      COMMON_EXPORT dyn_hash_map<T, TT2> * operator()(S *ser, dyn_hash_map<T, TT2> &m, 
            const char *tag = NULL, const char *tag2 = NULL) 
      {
         fprintf(stderr, "%s[%d]:  hash_size = %d\n", FILE__, __LINE__, m.size());
         translate_hash_map(ser, m, tag, tag2);

         //  maybe catch errors here?
         return &m;
      }
};



#if 0
class SerTest : public Serializable {

   int my_int;

   public:

   SerTest() 
   { 
      my_int = 777;
   }

   ~SerTest() {}

   void serialize(SerializerBase *s, const char * = NULL) 
   {
      try 
      {
         gtranslate(s, my_int);
      }  SER_CATCH("SerTest");
   }

   void testit() 
   {
      SerializerBase sb("SerTest", std::string("boogabooga"), sd_serialize, true);
      serialize( &sb);
   }
};
#endif
} /*namespace Dyninst*/
#endif

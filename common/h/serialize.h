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
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <stdio.h>

#if defined(os_windows)
#include <libxml/xmlversion.h>
#undef LIBXML_ICONV_ENABLED
#endif

#include <libxml/xmlwriter.h>

#include "dynutil/h/util.h"
#include "dynutil/h/Annotatable.h"
#include "dynutil/h/Serialization.h"
#include "common/h/headers.h"
#include "common/h/Types.h"
#include "common/h/sha1.h"
#include "common/h/pathName.h"

#define CACHE_DIR_VAR "DYNINST_CACHE_DIR"
#define DEFAULT_DYNINST_DIR ".dyninstAPI"
#define DEFAULT_CACHE_DIR "caches"
#define CACHE_MAGIC 0x555
#define CACHE_PREFIX "cache_"

#ifndef PATH_MAX
#define PATH_MAX 512
#endif


#if 0
DLLEXPORT bool &serializer_debug_flag();
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

void DLLEXPORT serialize_debug_init();

class SerDes;
class SerFile;
void printSerErr(const SerializerError &err);

class SerializerBase {
   SerFile *sf;
   SerDes *sd;
   std::string serializer_name;
   typedef dyn_hash_map<std::string, SerializerBase *> subsystem_serializers_t;
   DLLEXPORT static dyn_hash_map<std::string, subsystem_serializers_t> all_serializers;

   public:
      //SerializerBase(SerDes *sd_) : sd(sd_) { assert(sd);}
      DLLEXPORT SerializerBase(const char *name_, std::string filename, iomode_t dir, bool verbose); 
      DLLEXPORT virtual ~SerializerBase() {fprintf(stderr, "%s[%d]:  serializer %p-%sdtor\n", FILE__, __LINE__, this, serializer_name.c_str());}
      DLLEXPORT virtual SerDes &getSD()  { assert(sd); return *sd;}
      DLLEXPORT SerFile &getSF() {assert(sf); return *sf;}
      DLLEXPORT std::string &name() {return serializer_name;}
      DLLEXPORT static SerializerBase *getSerializer(std::string subsystem, std::string fname);
      DLLEXPORT static bool addSerializer(std::string subsystem, std::string fname, SerializerBase *sb);

      DLLEXPORT virtual void vector_start(unsigned int &, const char * = NULL);
      DLLEXPORT virtual void vector_end();
      DLLEXPORT virtual void hash_map_start(unsigned int &size, const char *tag = NULL); 
      DLLEXPORT virtual void hash_map_end();
      DLLEXPORT void translate_base(bool &v, const char *&t);
      DLLEXPORT void translate_base(short &v, const char *&t);
      DLLEXPORT void translate_base(char &v, const char *&t);
      DLLEXPORT void translate_base(int &v, const char *&t);
      DLLEXPORT void translate_base(unsigned int &v, const char *&t);
      DLLEXPORT void translate_base(unsigned long &v, const char *&t);
      DLLEXPORT void translate_base(long &v, const char *&t);
      DLLEXPORT void translate_base(float &v, const char *&t);
      DLLEXPORT void translate_base(double &v, const char *&t);
      DLLEXPORT void translate_base(const char * &v, int bufsize, const char *&t);
      DLLEXPORT void translate_base(char * &v, int bufsize, const char *&t);
      DLLEXPORT void translate_base(std::string &v, const char *t);

      DLLEXPORT virtual iomode_t iomode(); 

   protected:
      DLLEXPORT bool read_annotations();
   private:
      // SerDes *sd;
};


class SerDesXML;
//  TODO:  make these more specific
class SerializerXML : public SerializerBase {
   public:
      DLLEXPORT SerializerXML(const char *name_, std::string filename, iomode_t dir, bool verbose) :
         SerializerBase(name_, filename, dir, verbose) {}
      DLLEXPORT ~SerializerXML() {}

      DLLEXPORT SerDesXML &getSD_xml();
      DLLEXPORT static bool start_xml_element(SerializerBase *, const char *);
      DLLEXPORT static bool end_xml_element(SerializerBase *, const char *);
#if 0
      {
         SerDes &sd = getSD();
         SerDesXML *sdxml = dynamic_cast<SerDesXML *> (&sd);
         assert(sdxml);
         return *sdxml;
      }
#endif
};

class SerializerBin : public SerializerBase {
   friend class SerDesBin;
   public:
      DLLEXPORT SerializerBin(const char *name_, std::string filename, iomode_t dir, bool verbose); 
      DLLEXPORT ~SerializerBin() {}
      
      DLLEXPORT SerDesBin &getSD_bin();
#if 0
      {
         SerDes &sd = getSD();
         SerDesBin *sdbin = dynamic_cast<SerDesBin *> (&sd);
         assert(sdbin);
         return *sdbin;
      }
#endif
      static void globalDisable();
      static void globalEnable();

   private:
      static bool global_disable;
};

class AnnotationBase;


template <class T, class ANNO_NAME_T>
bool realloc_and_annotate(SerializerBase *sb, AnnotatableBase *pb)
{
   Annotatable<T, ANNO_NAME_T> *parent = (Annotatable<T, ANNO_NAME_T> *)pb;
   if (!parent) {
      fprintf(stderr, "%s[%d]:  failed to cast annotation parent here\n", FILE__, __LINE__);
      return false;
   }

   T it; 

   try {
      //  Maybe just need to do try/catch here since the template mapping may 
      //  change the type of return value thru template specialization
      trans_adaptor<SerializerBase, T> ta;
      fprintf(stderr, "%s[%d]: gtranslate: before operation\n", FILE__, __LINE__);
      T *itp = ta(sb, it, NULL, NULL);
      if (!itp) {
         fprintf(stderr, "%s[%d]: translate adaptor failed to de/serialize\n", FILE__, __LINE__);
      }

      //gtranslate(sb, it_ser, NULL);
   }
   catch (const SerializerError &err) {
      fprintf(stderr, "%s[%d]:  deserialization error here\n", FILE__, __LINE__);
      printSerErr(err);
      return false;
   }

   if (!parent->addAnnotation(it)) {
      fprintf(stderr, "%s[%d]:  failed to add annotation here\n", FILE__, __LINE__);
      return false;
   }

   return true;
}

class AnnoFunc {
   typedef bool (*func_t) (SerializerBase *, AnnotatableBase *);
   std::vector<func_t> funcbits;

   public:
      AnnoFunc() {}
      AnnoFunc(func_t f) {funcbits.push_back(f);}
      void add(func_t f) {funcbits.push_back(f);}
      AnnoFunc(const AnnoFunc &src) {
         funcbits = src.funcbits;
      }
      AnnoFunc &operator=(const AnnoFunc &src) {
         funcbits = src.funcbits;
         return *this;
      }

      bool operator==(const AnnoFunc &src) {
         if (funcbits.size() != src.funcbits.size()) 
            return false;
         for (unsigned int i = 0; i < funcbits.size(); ++i) {
            if (funcbits[i] != src.funcbits[i]) {
               return false;
            }
         }
         return true;
      }
      ~AnnoFunc() {funcbits.clear();}

      bool operator()(SerializerBase *sd, AnnotatableBase *parent) {
         if (!funcbits.size()) {
            fprintf(stderr, "%s[%d]:  WARNING:  empty serialization function\n", FILE__, __LINE__);
            return false;
         }
         for (unsigned int i = 0; i < funcbits.size(); ++i) {
            if (! (funcbits[i])(sd, parent)) {
               fprintf(stderr, "%s[%d]:  function sequence failing\n", FILE__, __LINE__);
               return false;
            }
         }
         return true;
      }
};

class SerializationFunctionBase {
   public:
      static dyn_hash_map<unsigned, SerializationFunctionBase *> func_map;
      SerializationFunctionBase() {}
      virtual ~SerializationFunctionBase() {}
      //virtual AnnotationBase *operator()(SerDes *, void *, const char *tag) {return NULL;}
      //virtual AnnotationBase *operator()(SerDes *, const char *tag) {return NULL;}
};

#if 0
template <class T> 
class SerFunc : public SerializationFunctionBase {
   typedef void (*ser_func_t)(SerDes *, T *, const char *);
   ser_func_t func;

   public:
      SerFunc(ser_func_t f) : func (f) {}
      virtual void operator()(SerDes *sd, T *it, const char *tag) 
      {
         try {
            func(sd, it, tag);
         } SER_CATCH("annotation")
      }
};
#endif



class SerializationFunctionMap {
   public:
      DLLEXPORT SerializationFunctionMap();
      DLLEXPORT ~SerializationFunctionMap();
   private:
      DLLEXPORT static dyn_hash_map<unsigned, SerializationFunctionBase *> func_map;
};


//DLLEXPORT SerializationFunctionBase *findSerDesFuncForAnno(unsigned anno_type);
//DLLEXPORT SerFunc *findSerFuncForAnno(unsigned anno_type);

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
    DLLEXPORT static dyn_hash_map<std::string, AnnoFunc > anno_funcs;

#if 0
   //  ser_funcs is a mapping of type name (c++ type name, not annotation type name)
   //  onto functions used to deserialize that type of object
    DLLEXPORT static dyn_hash_map<std::string, std::vector<SerializationFunctionBase *> > ser_funcs;
#endif

#if 0
    DLLEXPORT static std::vector<SerializationFunctionBase *> emptyFuncList;
#endif

    //  old_anno_name_to_id_map keeps a running mapping of 
    //  annotation names onto annotation ids that was used when building
    //  the file that is being deserialized.  This info is used to 
    //  rebuild annotations information, the name<->type mapping may change
    //  between different runs of dyninst.
    dyn_hash_map<unsigned, std::string> old_anno_name_to_id_map;
   protected:
    iomode_t iomode_;
  public:

  DLLEXPORT AnnoFunc *findAnnoFunc(unsigned anno_type);
  DLLEXPORT static bool addAnnoFunc(std::string type_name, AnnoFunc sf);

    DLLEXPORT SerDes() {assert(0);}
    DLLEXPORT SerDes(iomode_t mode) : iomode_(mode){}
    DLLEXPORT virtual ~SerDes() {}

#if 0
    DLLEXPORT static std::vector<SerializationFunctionBase *> &findType(std::string typestr) 
    {
      dyn_hash_map<std::string, std::vector<SerializationFunctionBase *> >::iterator iter;
      iter = ser_funcs.find(typestr); 
       if (iter == ser_funcs.end()) {
          fprintf(stderr, "%s[%d]:  no serialization functions found for type %s\n", 
                FILE__, __LINE__, typestr.c_str());
          return emptyFuncList;
       }
       return iter->second;
    }
#endif

#if 0
    DLLEXPORT static bool addFuncForType(std::string typestr, SerializationFunctionBase *sfb) 
    {
       std::vector<SerializationFunctionBase *> &funcs = ser_funcs[typestr];
       funcs.push_back(sfb);
       return true;
    }
#endif

    DLLEXPORT virtual void file_start(std::string &/*full_file_path*/) {}
    DLLEXPORT virtual void vector_start(unsigned int &size, const char *tag = NULL) DECLTHROW(SerializerError) = 0;
    DLLEXPORT virtual void vector_end() = 0;
    DLLEXPORT virtual void multimap_start(unsigned int &size, const char *tag = NULL) DECLTHROW(SerializerError) = 0;
    DLLEXPORT virtual void hash_map_start(unsigned int &size, const char *tag = NULL) DECLTHROW(SerializerError) = 0;
    DLLEXPORT virtual void hash_map_end() = 0;
    DLLEXPORT virtual void annotation_start(const char *string_id, const char *tag = NULL) = 0;
    DLLEXPORT virtual void annotation_end() = 0;

    DLLEXPORT virtual void multimap_end() = 0;

    DLLEXPORT virtual void translate(bool &param, const char *tag = NULL) = 0;
    DLLEXPORT virtual void translate(char &param, const char *tag = NULL) = 0;
    DLLEXPORT virtual void translate(int &param, const char *tag = NULL) = 0;
    DLLEXPORT virtual void translate(long &param, const char *tag = NULL) = 0;
    DLLEXPORT virtual void translate(short &param, const char *tag = NULL) = 0;
    DLLEXPORT virtual void translate(unsigned int &param, const char *tag = NULL) = 0;
#if 0
    virtual void translate(OFFSET &param, const char *tag = NULL) = 0;
    virtual void translate(pdstring &param, const char *tag = NULL) = 0;
#endif
    DLLEXPORT virtual void translate(float &param, const char *tag = NULL) = 0;
    DLLEXPORT virtual void translate(double &param, const char *tag = NULL) = 0;
    DLLEXPORT virtual void translate(Address &param, const char *tag = NULL) = 0;
    DLLEXPORT virtual void translate(const char * &param, int bufsize = 0, const char *tag = NULL) = 0;
    DLLEXPORT virtual void translate(char * &param, int bufsize = 0, const char *tag = NULL) = 0;
    DLLEXPORT virtual void translate(std::string &param, const char *tag = NULL) = 0;
    DLLEXPORT virtual void translate(std::vector<std::string> &param, const char *tag = NULL,
                           const char *elem_tag = NULL) = 0;


    //virtual AnnotationBase * read_annotation(void *&id, unsigned anno_type) {return NULL;}
    //virtual void read_annotations() {}

    DLLEXPORT virtual iomode_t iomode() {return iomode_;} 
};

class SerDesXML : public SerDes {
  public:
     xmlTextWriterPtr writer;

    DLLEXPORT static xmlTextWriterPtr init(std::string fname, iomode_t mode, bool verbose);

    DLLEXPORT SerDesXML() { assert(0);}
    DLLEXPORT SerDesXML(xmlTextWriterPtr w, iomode_t mode)  : SerDes(mode), writer(w) { }
    DLLEXPORT virtual ~SerDesXML();

    DLLEXPORT virtual void vector_start(unsigned int &size, const char *tag = NULL) DECLTHROW(SerializerError);
    DLLEXPORT virtual void vector_end();
    DLLEXPORT virtual void multimap_start(unsigned int &size, const char *tag = NULL) DECLTHROW(SerializerError);
    DLLEXPORT virtual void multimap_end();
    DLLEXPORT virtual void hash_map_start(unsigned int &size, const char *tag = NULL) DECLTHROW(SerializerError);
    DLLEXPORT virtual void hash_map_end();
    DLLEXPORT virtual void annotation_start(const char *string_id, const char *tag = NULL);
    DLLEXPORT virtual void annotation_end();
    DLLEXPORT virtual void translate(bool &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(char &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(int &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(long &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(short &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(unsigned int &param, const char *tag = NULL);
#if 0
    virtual void translate(OFFSET &param, const char *tag = NULL);
    virtual void translate(pdstring &param, const char *tag = NULL);
#endif
    DLLEXPORT virtual void translate(float &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(double &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(Address &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(const char * &param, int bufsize = 0, const char *tag = NULL);
    DLLEXPORT virtual void translate(char * &param, int bufsize = 0, const char *tag = NULL);
    DLLEXPORT virtual void translate(std::string &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(std::vector<std::string> &param, const char *tag = NULL,
                           const char *elem_tag = NULL);
    DLLEXPORT void start_element(const char *tag);
    DLLEXPORT void end_element();
    DLLEXPORT void xml_value(const char *val, const char *tag);

  protected:
    //xmlTextWriterPtr writer;
};

class AnnotatableBase;

class SerDesBin : public SerDes {

  typedef struct {
     unsigned int cache_magic;
     unsigned int source_file_size; //  if size is different, don't bother with checksum
     char sha1[SHA1_DIGEST_LEN];
  } cache_header_t;

    FILE *f;

    bool noisy;
  public:
    DLLEXPORT static dyn_hash_map<Address, AnnotatableBase *> annotatable_id_map;

    DLLEXPORT static FILE *init(std::string fname, iomode_t mode, bool verbose);
    DLLEXPORT SerDesBin() {assert(0);}
    DLLEXPORT SerDesBin(FILE *ff, iomode_t mode, bool verbose) : SerDes(mode), f(ff),  noisy(verbose) {}
    //SerDesBin(std::string fname, iomode_t mode, bool verbose = false);
    DLLEXPORT virtual ~SerDesBin();

    DLLEXPORT static AnnotatableBase *findAnnotatee(void *id); 

    DLLEXPORT virtual void file_start(std::string &full_file_path);
    DLLEXPORT virtual void vector_start(unsigned int &size, const char *tag = NULL) DECLTHROW(SerializerError);
    DLLEXPORT virtual void vector_end();
    DLLEXPORT virtual void multimap_start(unsigned int &size, const char *tag = NULL) DECLTHROW(SerializerError);
    DLLEXPORT virtual void multimap_end();
    DLLEXPORT virtual void hash_map_start(unsigned int &size, const char *tag = NULL) DECLTHROW(SerializerError);
    DLLEXPORT virtual void hash_map_end();
    DLLEXPORT virtual void annotation_start(const char *string_id, const char *tag = NULL);
    DLLEXPORT virtual void annotation_end();
    DLLEXPORT virtual void translate(bool &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(char &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(int &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(long &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(short &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(unsigned int &param, const char *tag = NULL);
#if 0
    virtual void translate(OFFSET &param, const char *tag = NULL);
    virtual void translate(pdstring &param, const char *tag = NULL);
#endif
    DLLEXPORT virtual void translate(float &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(double &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(Address &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(const char * &param, int bufsize = 0, const char *tag = NULL);
    DLLEXPORT virtual void translate(char * &param, int bufsize = 0, const char *tag = NULL);
    DLLEXPORT virtual void translate(std::string &param, const char *tag = NULL);
    DLLEXPORT virtual void translate(std::vector<std::string> &param, const char *tag = NULL,
                           const char *elem_tag = NULL);

    //virtual void read_annotations(); 
    //virtual AnnotationBase *read_annotation(void *&id, unsigned anno_type); 
    
    // readHeaderAndVerify just opens, verifies (checksum, magic compare), and closes
    // cache file, unless the FILE * is provided, in which case the file pointer is
    // advanced past the preamble and is not closed;
    DLLEXPORT static void readHeaderAndVerify(std::string full_file_path, std::string cache_name, FILE *f = NULL);
    DLLEXPORT static void writeHeaderPreamble(FILE *f, std::string full_file_path, std::string cache_name);

    DLLEXPORT static bool getDefaultCacheDir(std::string &cache_dir);
    DLLEXPORT static bool resolveCachePath(std::string fname, std::string &cache_name);
    DLLEXPORT static bool verifyChecksum(std::string &filename, const char comp_checksum[SHA1_DIGEST_LEN]);
    DLLEXPORT static bool cacheFileExists(std::string fname);
    DLLEXPORT static bool invalidateCache(std::string cache_name);
  private:

};


class DLLEXPORT SerFile {
   SerDes *sd;
    xmlTextWriterPtr writer;
    FILE *f;

   public:
    SerDes *getSD() {return sd;}
    SerFile(std::string fname, iomode_t mode, bool verbose = false) :
       writer (NULL), f(NULL), iomode_(mode), noisy(verbose) {
          char file_path[PATH_MAX];
          if (!resolve_file_path(fname.c_str(), file_path)) {
             char msg[128];
             sprintf(msg, "failed to resolve path for '%s'\n", fname.c_str());
             SER_ERR(msg);
          }
          filename = std::string(file_path);
          serialize_debug_init();
          if (strstr(filename.c_str(), "xml")) {
             fprintf(stderr, "%s[%d]:  opening xml file %s for %s\n", FILE__, __LINE__, filename.c_str(), mode == sd_serialize ? "output" : "input");
             if (mode == sd_deserialize) {
                fprintf(stderr, "%s[%d]:  ERROR:  deserializing xml not supported\n", FILE__, __LINE__);
                assert(0);
             }
             writer = SerDesXML::init(fname, mode, verbose);
             if (!writer) {
                fprintf(stderr, "%s[%d]:  ERROR:  failed to init xml writer\n", FILE__, __LINE__);
                assert(0);
             }
             sd = new SerDesXML(writer, mode);
          }
          else {
             fprintf(stderr, "%s[%d]:  opening %s file for %s\n", FILE__, __LINE__, filename.c_str(), mode == sd_serialize ? "output" : "input");
             f = SerDesBin::init(fname, mode, verbose);
             if (!f) {
                fprintf(stderr, "%s[%d]:  failed to init file i/o\n", FILE__, __LINE__);
                assert(0);
             }
             sd = new SerDesBin(f,mode, verbose);
          }
       }

    iomode_t iomode() {return iomode_;}

    static bool validCacheExistsFor(std::string full_file_path);

  protected:
    std::string filename;
    iomode_t iomode_;
  public:
    bool noisy;

  private:
};

#if 0
template <class T>
bool read_and_reannotate()
{
   unsigned anno_type = 0;
   void *id = NULL;

   if (iomode_ == sd_serialize) {
      fprintf(stderr, "%s[%d]:  this code should only be used in a deserialization sequence!\n",
            FILE__, __LINE__);
      return false;
   }

   translate((Address &)id, "Annotation Parent");

   if (!id) {
      fprintf(stderr, "%s[%d]:  weird:  zero id for annotation\n", FILE__, __LINE__);
   }

   translate(anno_type, NULL);

   //  look up the function that we need to call for this type of annotation
   SerializationFunctionBase *funcb_ptr = findSerDesFuncForAnno(anno_type);
   if (!funcb_ptr) {
      fprintf(stderr, "%s[%d]:  WARNING:  cannot find function to read anno type %d\n",
            FILE__, __LINE__);
      return false;
   }

   SerializationFunction<T> *func_ptr = dynamic_cast<SerializationFunction<T> *> (funcb_ptr);

   if (!func_ptr) {
      fprintf(stderr, "%s[%d]:  WARNING:  cannot properly cast function ptr for anno type %d\n",
            FILE__, __LINE__);
      return false;
   }
   
   T *newobj = new T();
   T *anno = func(newobj, NULL, "AnnotationBody");

   if (!anno) {
      fprintf(stderr, "%s[%d]:  translation function failed here\n", FILE__, __LINE__);
   }

   //  now look up the parent and re-annotate it with the child
   AnnotatableBase *parent = SerDesBin::findAnnotatee(id);
   if (!parent) {
      fprintf(stderr, "%s[%d]:  ERROR:  cannot find parent for annotation\n",
            FILE__, __LINE__);
      return false;
   }

   return true;
}
#endif
#if 0 // SERIALIZE
class SerializeCommonBase {
  public:
     virtual bool translate_annotation(void *anno, const char *name) = 0;

  protected:
     SerializeCommonBase() {};
     virtual ~SerializeCommonBase() {};
};
#endif

#if 0
template <class S, class T>
void translate_vector(S *ser, std::vector<T> &vec, 
                      const char *tag = NULL, const char *elem_tag = NULL) 
{
   unsigned int nelem = vec.size();
   ser->vector_start(nelem, tag);
   if (ser->iomode() == sd_deserialize) {
      if (vec.size()) 
         SER_ERR("nonempty vector used to create");
      //  zero size vectors are allowed
      //  what it T is a complex type (with inheritance info)??
      //  does resize() call default ctors, or should we do that
      //  manually here? look this up.
      if (nelem)
         vec.resize(nelem);
   }
      
   for (unsigned int i = 0; i < vec.size(); ++i) {
    T &t = vec[i];
    ser->translate_base(t, elem_tag);
   }
   ser->vector_end();
}

template <class S, class T>
void translate_vector(S *ser, std::vector<T *> &vec, 
                      const char *tag = NULL, const char *elem_tag = NULL) 
{
   unsigned int nelem = vec.size();
   ser->vector_start(nelem, tag);
   if (ser->iomode() == sd_deserialize) {
      if (vec.size()) 
         SER_ERR("nonempty vector used to create");
      //  zero size vectors are allowed
      if (nelem) {
         //  block-allocate array of underlying type, then assign to our vector
         //  What happens if an individual elem is later deleted??
         T *chunk_alloc = new T[nelem];
         vec.resize(nelem);
         for (unsigned int i = 0; i < nelem; ++i) 
             vec[i] = &(chunk_alloc[i]);
      }
   }

   for (unsigned int i = 0; i < vec.size(); ++i) {
    T &t = *(vec[i]);
    ser->translate_base(t, elem_tag);
   }
   ser->vector_end();
}

template <class S, class T>
void translate_vector(S *ser, std::vector<std::vector<T> > &vec, 
                      const char *tag = NULL, const char *elem_tag = NULL) 
{
   fprintf(stderr, "%s[%d]:  welcome to translate vector of vectors\n", FILE__, __LINE__);
   unsigned int nelem = vec.size();
   ser->vector_start(nelem, tag);
   if (ser->iomode() == sd_deserialize) {
      if (vec.size()) 
         SER_ERR("nonempty vector used to create");
      //  zero size vectors are allowed
      //  what it T is a complex type (with inheritance info)??
      //  does resize() call default ctors, or should we do that
      //  manually here? look this up.
      if (nelem)
         vec.resize(nelem);
   }
      
   for (unsigned int i = 0; i < vec.size(); ++i) {
      std::vector<T> &tv = vec[i];
      translate_vector(ser,tv, tag, elem_tag);
   }
   ser->vector_end();
}
#endif

#if 0
template <class S, class K, class V, class CMP>
void translate_multimap(S *ser, std::multimap<K, V, CMP> &mm, 
      const char *tag = NULL, const char *key_tag = NULL, const char *value_tag = NULL)
{
   unsigned int nelem = mm.size();
   ser->multimap_start(nelem, tag);
   if (ser->iomode() == sd_serialize) {
      typename std::multimap<K,V,CMP>::iterator iter = mm.begin();
      while (iter != mm.end()) {
         K k = iter->first;
         V v = iter->second;
         ser->translate_base(k, key_tag);
         ser->translate_base(v, value_tag);
      }
   }
   else {
      for (unsigned int i = 0; i < nelem; ++i) {
         K k;
         V v;
         ser->translate_base(k, key_tag);
         ser->translate_base(v, value_tag);
         mm[k] = v;
      }
   }
   ser->multimap_end();
}
#endif

#if 0
//  Moved to public header
template <class S, class K, class V>
void translate_hash_map(S *ser, hash_map<K, V> &hash, 
      const char *tag = NULL, const char *key_tag = NULL, const char *value_tag = NULL)
{
   fprintf(stderr, "%s[%d]:  welcome to translate_hash_map<%s, %s>()\n", FILE__, __LINE__,
         typeid(K).name(), typeid(V).name());

   unsigned int nelem = hash.size();
   ser->hash_map_start(nelem, tag);
   fprintf(stderr, "%s[%d]:  after hash_map start, mode = %sserialize\n", FILE__, __LINE__, ser->iomode() == sd_serialize ? "" : "de");

   if (ser->iomode() == sd_serialize) {
      typename hash_map<K,V>::iterator iter = hash.begin();
      fprintf(stderr, "%s[%d]:  about to serialize hash with %d elements\n", FILE__, __LINE__, hash.size());
      while (iter != hash.end()) {
         K k = iter->first;
         V v = iter->second;
         ser->translate_base(k, key_tag);
         ser->translate_base(v, value_tag);
         iter++;
      }
   }
   else {
      //  can we do some kind of preallocation here?
      for (unsigned int i = 0; i < nelem; ++i) {
         K k;
         V v;
         ser->translate_base(k, key_tag);
         ser->translate_base(v, value_tag);
         hash[k] = v;
      }
   }
   ser->hash_map_end();
}

template <class S, class K, class V>
void translate_hash_map(S *ser, hash_map<K, V *> &hash, 
      const char *tag = NULL, const char *key_tag = NULL, const char *value_tag = NULL)
{
   fprintf(stderr, "%s[%d]:  welcome to translate_hash_map<%s, %s*>()\n", FILE__, __LINE__,
         typeid(K).name(), typeid(V).name());

   unsigned int nelem = hash.size();
   ser->hash_map_start(nelem, tag);
   fprintf(stderr, "%s[%d]:  after hash_map start, mode = %sserialize\n", FILE__, __LINE__, ser->iomode() == sd_serialize ? "" : "de");

   if (ser->iomode() == sd_serialize) {
      typename hash_map<K,V *>::iterator iter = hash.begin();
      while (iter != hash.end()) {
         K k = iter->first;
         V *v = iter->second;
         ser->translate_base(k, key_tag);
         ser->translate_base(*v, value_tag);
         iter++;
      }
   }
   else {
      //  can we do some kind of preallocation here?
      for (unsigned int i = 0; i < nelem; ++i) {
         K k;
         V *v = new V();
         ser->translate_base(k, key_tag);
         ser->translate_base(*v, value_tag);
         hash[k] = v;
      }
   }
   ser->hash_map_end();
}

template <class S, class K, class V>
void translate_hash_map(S *ser, hash_map<K, char *> &hash, 
      const char *tag = NULL, const char *key_tag = NULL, const char *value_tag = NULL)
{
   //  THIS SPECIALIZATION DOES NOT WORK CORRECTLY (YET)
   fprintf(stderr, "%s[%d]:  welcome to translate_hash_map<%s, %s*>()\n", FILE__, __LINE__,
         typeid(K).name(), typeid(V).name());

   unsigned int nelem = hash.size();
   ser->hash_map_start(nelem, tag);
   fprintf(stderr, "%s[%d]:  after hash_map start, mode = %sserialize\n", FILE__, __LINE__, ser->iomode() == sd_serialize ? "" : "de");

   if (ser->iomode() == sd_serialize) {
      typename hash_map<K,V *>::iterator iter = hash.begin();
      while (iter != hash.end()) {
         K k = iter->first;
         V v = iter->second;
         ser->translate_base(k, key_tag);
         ser->translate_base(v, value_tag);
         iter++;
      }
   }
   else {
      //  can we do some kind of preallocation here?
      for (unsigned int i = 0; i < nelem; ++i) {
         K k;
         V v;
         ser->translate_base(k, key_tag);
         ser->translate_base(*v, value_tag);
         hash[k] = v;
      }
   }
   ser->hash_map_end();
}
#endif
#if 0
template <class S, class T>
void translate_annotation(S *ser, T &it, const char *anno_str, const char *tag = NULL)
{
   ser->annotation_start(anno_str, tag);
   if (ser->iomode() == sd_serialize) {
      ser->translate_base(it, tag);
   }
   ser->annotation_end();
}

template <class S, class T>
void translate_annotation(S *ser, T *it, const char *anno_str, const char *tag = NULL)
{
   ser->annotation_start(anno_str, tag);
   if (ser->iomode() == sd_serialize) {
      ser->translate_base(*it, tag);
   }
   ser->annotation_end();
}
#endif



template <class S, class T>
class SpecAdaptor {
   public:
      SpecAdaptor() {}
      T *operator()(S *s, T &t, const char *tag) {
         s->translate_base(t, tag);
         return &t;
      }
};

template <class S, class T>
class SpecAdaptor<S, T *> {
   public:
      SpecAdaptor() {}
      T* operator()(S *s, T *t, const char *tag) {
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
   if (NULL == saf(sd, it, tag)) {
      fprintf(stderr, "%s[%d]:  ERROR here\n", FILE__, __LINE__);
   }
   return;
}

#if 0
template <class S, class T> 
T *sd_translate(S *sd, T &it, const char * tag) 
{

   assert(it);
   sd->translate_base(*it, tag);

   return it;
}

template <class S, class T> 
T *sd_translate(S *sd, T &it, const char * tag) 
{
#if 0
   const std::string &typestr = typeid(T).name();
   
   std::vector<SerializationFunctionBase *> &funcs = sd->findType(typestr); 
   if (!funcs.size()) {
      fprintf(stderr, "%s[%d]:  ERROR:  no serialization functions registered for type %s\n", 
            FILE__, __LINE__, typestr.c_str());
      return NULL;
   }

   if (!it) {
      fprintf(stderr, "%s[%d]:  deserialization:  allocating new %s\n", 
            FILE__, __LINE__, typestr.c_str());
      it = new T();
   }

   for (unsigned int i = 0; i < funcs.size(); ++i) {
      SerializationFunctionBase *sfb = funcs[i];
      SerFunc<T> *sf = dynamic_cast<SerFunc<T>*>(sfb);
      if (!sf) {
         fprintf(stderr, "%s[%d]:  FIXME\n", FILE__, __LINE__);
         continue;
      }
      SerFunc<T> &func = *sf;
      func(sd, it, NULL);
   }
#endif
   if (typeid(T).__is_pointer_p()) {
      fprintf(stderr, "%s[%d]:  WARNINIG:  this should not happen\n", FILE__, __LINE__);
   }
   else
      sd->translate_base(it, tag);

      

   return &it;
}
#endif

template <class T, class ANNO_NAME_T>
bool init_anno_serialization(SerializerBase * /*serializer_*/)
{

   fprintf(stderr, "%s[%d]:  initializing annotation function for annotation type %s\n",
         FILE__, __LINE__, typeid(ANNO_NAME_T).name());

   AnnoFunc sf(realloc_and_annotate<T, ANNO_NAME_T>);
  //static bool addSerFuncForAnno(unsigned anno_type, SerFunc sf);
   std::string anno_name = typeid(ANNO_NAME_T).name();
   int anno_id = AnnotatableBase::getOrCreateAnnotationType(anno_name, typeid(T).name());
   if (anno_id == -1) {
      fprintf(stderr, "%s[%d]:  failed to get annotation type here\n", FILE__, __LINE__);
      return false;
   }

   if (!SerDesBin::addAnnoFunc(anno_name, sf)) {
      fprintf(stderr, "%s[%d]:  failed to add serialization function\n", FILE__, __LINE__);
      return false;
   }
   return true;
}

#if 0
//  Moved to public header file
typedef void NOTYPE_T;
template<class S, class T, class T2 = NOTYPE_T> 
class trans_adaptor {
   public:
      trans_adaptor() {
         fprintf(stderr, "%s[%d]:  trans_adaptor  -- general\n", FILE__, __LINE__);
      }
      T * operator()(S *ser, T & it, const char *tag = NULL, const char *tag2 = NULL) {
         ser->translate_base(it, tag);
         return &it;
      }
};

template<class S, class T, class TT2> 
class trans_adaptor<S, std::vector<T>, TT2 > {
   public:
      trans_adaptor() {
         fprintf(stderr, "%s[%d]:  trans_adaptor  -- vectorl\n", FILE__, __LINE__);
      }
      std::vector<T> * operator()(S *ser, std::vector<T> &v, const char *tag = NULL, const char *tag2 = NULL) {
         translate_vector(ser, v, tag, tag2);
         //  maybe catch errors here?
         return &v;
      }
};

template<class S, class T, class TT2> 
class trans_adaptor<S, std::vector<T *>, TT2> {
   public:
      trans_adaptor() {
         fprintf(stderr, "%s[%d]:  trans_adaptor  -- vector of ptrs\n", FILE__, __LINE__);
      }
      std::vector<T*> * operator()(S *ser, std::vector<T *> &v, const char *tag = NULL, const char *tag2 = NULL) {
         translate_vector(ser, v, tag, tag2);
         //  maybe catch errors here?
         return &v;
      }
};
#endif

template<class S, class T, class TT2> 
class trans_adaptor<S, dyn_hash_map<T, TT2> > {
   public:
      trans_adaptor() {
         fprintf(stderr, "%s[%d]:  welcome to trans_adaptor<%s, hash<%s, %s> >()\n",
               FILE__, __LINE__,
               typeid(S).name(),
               typeid(T).name(), typeid(TT2).name() );
      }
      dyn_hash_map<T, TT2> * operator()(S *ser, dyn_hash_map<T, TT2> &m, const char *tag = NULL, const char *tag2 = NULL) {
         fprintf(stderr, "%s[%d]:  hash_size = %d\n", FILE__, __LINE__, m.size());
         translate_hash_map(ser, m, tag, tag2);
         //  maybe catch errors here?
         return &m;
      }
};




#if 0
   template <class S, class K, class V>
void gtranslate(S *ser, hash_map<K, V> &it, const char *tag, const char *tag2)
{
   fprintf(stderr, "%s[%d]:  welcome to gtranslate<%s, hash<%s, %s> >(%p)\n",
         FILE__, __LINE__,
         typeid(S).name(),
         typeid(K).name(), typeid(V).name(), &it );
   fprintf(stderr, "%s[%d]:  hash_size = %d\n", FILE__, __LINE__, it.size());

   //  Maybe just need to do try/catch here since the template mapping may 
   //  change the type of return value thru template specialization
   trans_adaptor<S, K, V> ta;
   fprintf(stderr, "%s[%d]:  gtranslate<%s, %s, %s>(%p), before trans, nelem = %d\n",
         FILE__, __LINE__,
         typeid(S).name(),
         typeid(K).name(), 
         typeid(V).name(), 
         &it, it.size());
   hash_map<K, V> *itp = ta(ser, it, tag, tag2);
   if (!itp) {
      fprintf(stderr, "%s[%d]: translate adaptor failed to de/serialize\n", FILE__, __LINE__);
   }
}
#endif

#if 0
   template <class S, class TT>
void gtranslate(S *ser, TT&it, const char *tag = NULL)
{
   fprintf(stderr, "%s[%d]:  welcome to gtranslate<%s, %s>(%p)\n",
         FILE__, __LINE__,
         typeid(S).name(),
         typeid(TT).name(), &it);

   //  Maybe just need to do try/catch here since the template mapping may 
   //  change the type of return value thru template specialization
   trans_adaptor<S, TT> ta;
   fprintf(stderr, "%s[%d]: gtranslate: before operation\n", FILE__, __LINE__);
   TT *itp = ta(ser, it, tag);
   if (!itp) {
      fprintf(stderr, "%s[%d]: translate adaptor failed to de/serialize\n", FILE__, __LINE__);
   }
}
#endif

#if 0
template <class T, class S>
bool serialize(T &it, S &trans)
{
   try 
   {
      trans.translate_base(it);
   } 
   catch(const SerializerError &err)
   {
      int line = err.line();
      std::string file = err.file();
      fprintf(stderr, "%s[%d]: serialization error: %s\n", __FILE__, __LINE__, err.what());
      fprintf(stderr, "\t original location is -- %s[%d]\n", file.c_str(), line);
      return false;
   }
   return true;
}

template <class T, class S>
bool deserialize(T &it, S &trans)
{
   return serialize<T, S>(it, trans);
}
#endif

class SerTest : public Serializable {
   int my_int;
   public:
      SerTest() { my_int = 777;}
      ~SerTest() {}
      void serialize(SerializerBase *s, const char * = NULL) {
         try {
            gtranslate(s, my_int);
         }  SER_CATCH("SerTest");
      }
      void testit() {
         SerializerBase sb("SerTest", std::string("boogabooga"), sd_serialize, true);
         serialize( &sb);
      }
};
#endif

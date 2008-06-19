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


DLLEXPORT bool &serializer_debug_flag();
//  SER_ERR("msg") -- an attempt at "graceful" failure.  If debug flag is set
//  it will assert, otherwise it throws...  leaving the "graceful" aspect
//  to the next (hopefully top-level) exception handler.

#define SER_ERR(cmsg) \
   do { \
      if (serializer_debug_flag()) { \
        fprintf(stderr, "%s", cmsg); \
        assert (0); \
      } else { \
        throw SerializerError(FILE__, __LINE__, std::string(cmsg)); \
      } \
   } while (0)

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

typedef enum {sd_serialize, sd_deserialize} iomode_t;

class SerializerError : public std::runtime_error {
   //  SerializerError:  a small class that is thrown by serialization/deserialization
   //  routines.  This exists as an attempt to standardize and simplify error handling
   //  for ser-des routines that are possibly deeply nested.
   //  Here's the rub:  we don't want stray, unhandled exceptions finding their way into
   //  the larger system...  thus all entry points to serialization/deserialization need
   //  to catch this exception to render it transparent to the rest of the system.

    std::string file__;
    int line__;

  public:

  SerializerError(const std::string &__file__, const int &__line__, const std::string &msg) :
     runtime_error(msg),
     file__(__file__),
     line__(__line__) {}
  virtual ~SerializerError() throw() {}

  std::string file() const {return file__;}
  int line() const {return line__;}
};

class DLLEXPORT SerDes {
   //  SerDes is a base class that provides generic serialization/deserialization
   //  access primitives and a common interface, (a toolbox, if you will).
   //  It is specialized (currently) by SerDesBin and SerDesXML, which implement the 
   //  actual low-level ser-des routines 

  public:

    SerDes(std::string fname, iomode_t mode, bool verbose = false) :
       iomode_(mode), noisy(verbose) {
          char file_path[PATH_MAX];
          if (!resolve_file_path(fname.c_str(), file_path)) {
             char msg[128];
             sprintf(msg, "failed to resolve path for '%s'\n", fname.c_str());
             SER_ERR(msg);
          }
          filename = std::string(file_path);
          serialize_debug_init();
       }
    virtual ~SerDes() {}

    virtual void file_start(std::string &/*full_file_path*/) {}
    virtual void vector_start(unsigned int &size, const char *tag = NULL) throw(SerializerError) = 0;
    virtual void vector_end() = 0;
    virtual void multimap_start(unsigned int &size, const char *tag = NULL) throw(SerializerError) = 0;
    virtual void annotation_start(const char *string_id, const char *tag = NULL) = 0;
    virtual void annotation_end() = 0;

    virtual void multimap_end() = 0;
    virtual void translate(bool &param, const char *tag = NULL) = 0;
    virtual void translate(char &param, const char *tag = NULL) = 0;
    virtual void translate(int &param, const char *tag = NULL) = 0;
    virtual void translate(unsigned int &param, const char *tag = NULL) = 0;
#if 0
    virtual void translate(OFFSET &param, const char *tag = NULL) = 0;
    virtual void translate(pdstring &param, const char *tag = NULL) = 0;
#endif
    virtual void translate(Address &param, const char *tag = NULL) = 0;
    virtual void translate(const char * &param, int bufsize = 0, const char *tag = NULL) = 0;
    virtual void translate(std::string &param, const char *tag = NULL) = 0;
    virtual void translate(std::vector<std::string> &param, const char *tag = NULL,
                           const char *elem_tag = NULL) = 0;

    iomode_t iomode() {return iomode_;}
  protected:
    std::string filename;
    iomode_t iomode_;
  public:
    bool noisy;
};

class DLLEXPORT SerDesXML : public SerDes {
  public:

    SerDesXML(std::string fname, iomode_t mode, bool verbose = false);
    virtual ~SerDesXML();

    virtual void vector_start(unsigned int &size, const char *tag = NULL) throw(SerializerError);
    virtual void vector_end();
    virtual void multimap_start(unsigned int &size, const char *tag = NULL) throw(SerializerError);
    virtual void multimap_end();
    virtual void annotation_start(const char *string_id, const char *tag = NULL);
    virtual void annotation_end();
    virtual void translate(bool &param, const char *tag = NULL);
    virtual void translate(char &param, const char *tag = NULL);
    virtual void translate(int &param, const char *tag = NULL);
    virtual void translate(unsigned int &param, const char *tag = NULL);
#if 0
    virtual void translate(OFFSET &param, const char *tag = NULL);
    virtual void translate(pdstring &param, const char *tag = NULL);
#endif
    virtual void translate(Address &param, const char *tag = NULL);
    virtual void translate(const char * &param, int bufsize = 0, const char *tag = NULL);
    virtual void translate(std::string &param, const char *tag = NULL);
    virtual void translate(std::vector<std::string> &param, const char *tag = NULL,
                           const char *elem_tag = NULL);
    void start_element(const char *tag);
    void end_element();
    void xml_value(const char *val, const char *tag);

  protected:
    xmlTextWriterPtr writer;
};


class DLLEXPORT SerDesBin : public SerDes {

  typedef struct {
     unsigned int cache_magic;
     unsigned int source_file_size; //  if size is different, don't bother with checksum
     char sha1[SHA1_DIGEST_LEN];
  } cache_header_t;

    FILE *f;

  public:

    SerDesBin(std::string fname, iomode_t mode, bool verbose = false);
    virtual ~SerDesBin();

    static bool validCacheExistsFor(std::string full_file_path);
    void readHeaderAndVerify(std::string full_file_path, std::string cache_name);
    void writeHeaderPreamble(std::string full_file_path, std::string cache_name);

    virtual void file_start(std::string &full_file_path);
    virtual void vector_start(unsigned int &size, const char *tag = NULL) throw(SerializerError);
    virtual void vector_end();
    virtual void multimap_start(unsigned int &size, const char *tag = NULL) throw(SerializerError);
    virtual void multimap_end();
    virtual void annotation_start(const char *string_id, const char *tag = NULL);
    virtual void annotation_end();
    virtual void translate(bool &param, const char *tag = NULL);
    virtual void translate(char &param, const char *tag = NULL);
    virtual void translate(int &param, const char *tag = NULL);
    virtual void translate(unsigned int &param, const char *tag = NULL);
#if 0
    virtual void translate(OFFSET &param, const char *tag = NULL);
    virtual void translate(pdstring &param, const char *tag = NULL);
#endif
    virtual void translate(Address &param, const char *tag = NULL);
    virtual void translate(const char * &param, int bufsize = 0, const char *tag = NULL);
    virtual void translate(std::string &param, const char *tag = NULL);
    virtual void translate(std::vector<std::string> &param, const char *tag = NULL,
                           const char *elem_tag = NULL);

  private:

    static bool getDefaultCacheDir(std::string &cache_dir);
    static bool resolveCachePath(std::string fname, std::string &cache_name);
    static bool verifyChecksum(std::string &filename, const char comp_checksum[SHA1_DIGEST_LEN]);
    static bool cacheFileExists(std::string fname);
    static bool invalidateCache(std::string cache_name);
};

#if 0 // SERIALIZE
class SerializeCommonBase {
  public:
     virtual bool translate_annotation(void *anno, const char *name) = 0;

  protected:
     SerializeCommonBase() {};
     virtual ~SerializeCommonBase() {};
};
#endif

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
#endif

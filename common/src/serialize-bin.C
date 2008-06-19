
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

#include <stdio.h>
#include "common/h/serialize.h"

bool dyn_debug_serializer = false;
bool &serializer_debug_flag()
{
   //  This function exists to get around problems with exporting the variable
   //  across library boundaries on windows...   there's probably a better way to do this...
   return dyn_debug_serializer;
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

SerDesBin::SerDesBin(std::string fname, iomode_t mode, bool verbose) :
  SerDes(fname, mode, verbose)
{
   //  NOTE:  fname is path-resolved and turned into "filename" by the SerDes ctor
   std::string cache_name;
   if (! resolveCachePath(filename, cache_name)) {
     fprintf(stderr, "%s[%d]:  no cache file exists for %s\n", 
           FILE__, __LINE__, filename.c_str());
     if (mode == sd_deserialize) {
        //  can't deserialize from a file that does not exist
       char msg[128];
       sprintf(msg, "%s[%d]:  no cache file exists for %s\n", 
             FILE__, __LINE__, filename.c_str());
       SER_ERR(msg);
     }
   }

   fprintf(stderr, "%s[%d]:  opening cache file %s\n", FILE__, __LINE__, cache_name.c_str());
   f = fopen(cache_name.c_str(), (mode == sd_serialize) ? "w+" : "r");
   if (!f) {
      char msg[128];
      fprintf(stderr, "%s[%d]: fopen(%s, %s): %s\n", FILE__, __LINE__, 
            cache_name.c_str(), (mode == sd_serialize) ? "w+" : "r", strerror(errno));
      sprintf(msg, "fopen(%s, %s): %s", cache_name.c_str(), 
            (mode == sd_serialize) ? "w+" : "r", strerror(errno));
      SER_ERR(msg);
   }

   fprintf(stderr, "%s[%d]:  opened cache file %s\n", FILE__, __LINE__, cache_name.c_str());

   try {
     if (mode == sd_serialize){
       writeHeaderPreamble(filename, cache_name);
     }
     else {
       readHeaderAndVerify(filename, cache_name);
     }
   }
   catch(const SerializerError &err) {
     fclose(f);
     fprintf(stderr, "%s[%d]:  %sserialize failed init...  \n\t%s[%d]: %s\n\trethrowing...\n",
             FILE__, __LINE__, mode == sd_serialize ? "" : "de", 
             err.file().c_str(), err.line(), err.what());
     throw(err);
   }
}

SerDesBin::~SerDesBin()
{
  if (f)
    fclose(f);
}

bool SerDesBin::getDefaultCacheDir(std::string &path)
{
    char *home_dir = getenv("HOME");
    if (!home_dir) {
       fprintf(stderr, "%s[%d]:  weird, no $HOME dir\n", FILE__, __LINE__);
       return false;
    }

    std::string dot_dyninst_dir = std::string(home_dir) + std::string("/")
       + std::string(DEFAULT_DYNINST_DIR);
    
    struct stat statbuf;
    if (0 != stat(dot_dyninst_dir.c_str(), &statbuf)) {
       if (errno == ENOENT) {
#if defined (os_windows)
         if (0 != mkdir(dot_dyninst_dir.c_str())) {
            fprintf(stderr, "%s[%d]:  failed to make %s\n", FILE__, __LINE__, 
                  dot_dyninst_dir.c_str(), strerror(errno));
            return false;
         } 
#else
         if (0 != mkdir(dot_dyninst_dir.c_str(), S_IRWXU)) {
            fprintf(stderr, "%s[%d]:  failed to make %s: %s\n", FILE__, __LINE__, 
                  dot_dyninst_dir.c_str(), strerror(errno));
            return false;
         } 
#endif
       }
       else {
          fprintf(stderr, "%s[%d]:  stat(%s) failed: %s\n", FILE__, __LINE__, 
                dot_dyninst_dir.c_str(), strerror(errno));
          return false;
       }
    }
    else {
#if !defined (os_windows)
       //  sanity check that its a dir
       if (!S_ISDIR(statbuf.st_mode)) {
          fprintf(stderr, "%s[%d]:  ERROR:  %s is not a dir\n", FILE__, __LINE__, 
                dot_dyninst_dir.c_str());
          return false;
       }
#else
       //  windows equiv to S_ISDIR??
#endif
    }

    path = dot_dyninst_dir + std::string("/") + std::string(DEFAULT_CACHE_DIR);

    if (0 != stat(path.c_str(), &statbuf)) {
       if (errno == ENOENT) {
#if defined (os_windows)
         if (0 != mkdir(path.c_str())) {
            fprintf(stderr, "%s[%d]:  failed to make %s\n", FILE__, __LINE__, 
                  path.c_str(), strerror(errno));
            return false;
         } 
#else
         if (0 != mkdir(path.c_str(), S_IRWXU)) {
            fprintf(stderr, "%s[%d]:  failed to make %s: %s\n", FILE__, __LINE__, 
                  path.c_str(), strerror(errno));
            return false;
         } 
#endif
       }
       else {
          fprintf(stderr, "%s[%d]:  stat(%s) failed: %s\n", FILE__, __LINE__, 
                path.c_str(), strerror(errno));
          return false;
       }
    }
    else {
#if !defined (os_windows)
       //  sanity check that its a dir
       if (!S_ISDIR(statbuf.st_mode)) {
          fprintf(stderr, "%s[%d]:  ERROR:  %s is not a dir\n", FILE__, __LINE__, 
                path.c_str());
          return false;
       }
#else
       //  windows equiv to S_ISDIR??
#endif
    }
    fprintf(stderr, "%s[%d]:  using default cache dir: %s\n", FILE__, __LINE__, path.c_str());
  return true;
}

bool SerDesBin::resolveCachePath(std::string full_file_path, std::string &cache_name)
{
   std::string path;
   char *path_dir = getenv(CACHE_DIR_VAR); 
   if (!path_dir) {
      if (!getDefaultCacheDir(path)) {
         fprintf(stderr, "%s[%d]:  weird, failed to make $HOME/.dyninst/caches\n",
                 FILE__, __LINE__);
         return false;
      }
   }

   // get size of file (this is encoded into cache name)
   struct stat statbuf;
   if (0 != stat(full_file_path.c_str(), &statbuf)) {
     fprintf(stderr, "%s[%d]:  stat %s failed: %s\n", FILE__, __LINE__, 
           full_file_path.c_str(), strerror(errno));
     return false;
   }
   
#if 0
   //  extract file short name from the full path
   size_t last_slash = full_file_path.find_last_of("/\\");
   if (last_slash == std::string::npos) {
     fprintf(stderr, "%s[%d]:  %s is not a full path??\n", FILE__, __LINE__, 
           full_file_path.c_str());
     return false;
   }
#endif

   std::string short_name = extract_pathname_tail(full_file_path);
   fprintf(stderr, "%s[%d]:  file %s short name: %s\n", FILE__, __LINE__, 
         full_file_path.c_str(), short_name.c_str());

   // construct cache name from cache path, cache prefix, short name, and size
   char sizestr[16];
   sprintf(sizestr, "%d", (int)statbuf.st_size);
   cache_name = path + std::string("/") + std::string(CACHE_PREFIX) + short_name 
      + std::string("_") 
      + std::string(sizestr);

   fprintf(stderr, "%s[%d]:  constructed cache name: %s\n", FILE__, __LINE__, cache_name.c_str());
   if (0 != stat(cache_name.c_str(), &statbuf)) {
      if (errno != ENOENT) {
         //  Its OK if the file doesn't exist, but complain if we get a different
         // error
         fprintf(stderr, "%s[%d]:  stat %s failed: %s\n", FILE__, __LINE__, 
               cache_name.c_str(), strerror(errno));
      }
      fprintf(stderr, "%s[%d]:  cache file %s does not exist\n", FILE__, __LINE__, 
            cache_name.c_str());
      return false;
   }

   fprintf(stderr, "%s[%d]:  cache file %s exists\n", FILE__, __LINE__, cache_name.c_str());
   return true;
}

bool SerDesBin::cacheFileExists(std::string fname)
{
   std::string cache_name;
   return resolveCachePath(fname, cache_name); 
}

void SerDesBin::readHeaderAndVerify(std::string full_file_path, std::string cache_name)
{
   struct stat statbuf;

   if (0 != stat(full_file_path.c_str(), &statbuf)) {
      char msg[128];
      if (errno != ENOENT) {
         //  Its OK if the file doesn't exist, but complain if we get a different
         // error
         sprintf(msg, "%s[%d]:  stat %s failed: %s\n", FILE__, __LINE__, 
               full_file_path.c_str(), strerror(errno));
      }
      SER_ERR(msg);
   }

   size_t source_file_size = statbuf.st_size;

   cache_header_t header;
   int rc = fread(&header, sizeof(cache_header_t), 1, f);
   if (1 != rc) {
      char msg[128];
      sprintf(msg, "%s[%d]:  failed to read header struct for %s: %s\n", 
              FILE__, __LINE__, full_file_path.c_str(), strerror(errno));
      SER_ERR(msg);
   }
   
  if (header.cache_magic != CACHE_MAGIC) {
      char msg[128];
      sprintf(msg, "%s[%d]:  magic number check failure for %s/%s\n", 
              FILE__, __LINE__, full_file_path.c_str(), cache_name.c_str());
      SER_ERR(msg);
  }

  if (header.source_file_size != source_file_size) {
      char msg[128];
      sprintf(msg, "%s[%d]:  size discrepancy found for %s/%s\n", 
              FILE__, __LINE__, full_file_path.c_str(), cache_name.c_str());
      SER_ERR(msg);
  }

  if (!verifyChecksum(full_file_path, header.sha1)) {
      char msg[128];
      sprintf(msg, "%s[%d]:  checksum discrepancy found for %s/%s\n", 
              FILE__, __LINE__, full_file_path.c_str(), cache_name.c_str());

      if (!invalidateCache(cache_name)) {
        fprintf(stderr, "%s[%d]:  failed to invalidate cache for file %s/%s\n", 
                FILE__, __LINE__, full_file_path.c_str(), cache_name.c_str());
      }

      SER_ERR(msg);
  }
}
 
void SerDesBin::writeHeaderPreamble(std::string full_file_path, std::string /*cache_name*/)
{
  //  get a few bits of info on this file to construct the header of the cache
  //  file...  checksum, size, ...  not mtime, since we don't care if someone 
  //  copies the file around

   struct stat statbuf;
   if (0 != stat(full_file_path.c_str(), &statbuf)) {
     char msg[128];
     sprintf(msg, "%s[%d]:  stat %s failed: %s\n", FILE__, __LINE__, 
           full_file_path.c_str(), strerror(errno));
     SER_ERR(msg);
   }

  cache_header_t header;
  header.cache_magic = CACHE_MAGIC;
  header.source_file_size = statbuf.st_size;

  if (NULL == sha1_file(full_file_path.c_str(), header.sha1)) {
     char msg[128];
     sprintf(msg, "sha1_file failed\n");
     SER_ERR(msg);
  }

  int rc = fwrite(&header, sizeof(cache_header_t), 1, f);

  if (1 != rc) 
     SER_ERR("fwrite");
  
}

bool SerDesBin::verifyChecksum(std::string &full_file_path, 
                               const char comp_checksum[SHA1_DIGEST_LEN])
{
  char new_checksum[SHA1_DIGEST_LEN]; 
  if (NULL == sha1_file(full_file_path.c_str(), new_checksum)) {
      fprintf(stderr, "%s[%d]:  sha1_file(%s) failed \n", 
              FILE__, __LINE__, full_file_path.c_str());
      return false;
  }

  if (strncmp(comp_checksum, new_checksum, SHA1_DIGEST_LEN)) {
      fprintf(stderr, "%s[%d]:  sha1_file(%s): checksum mismatch: \n\told: %s\n\tnew:%s\n", 
              FILE__, __LINE__, full_file_path.c_str(), comp_checksum, new_checksum);
      return false;
  }

  return true;
}

bool SerDesBin::invalidateCache(std::string cache_name) 
{
  if (-1 == unlink(cache_name.c_str())) {
     fprintf(stderr, "%s[%d]:  unlink(%s): %s\n", FILE__, __LINE__, 
             cache_name.c_str(), strerror(errno));
     return false;
  }

  return true;
}

void SerDesBin::file_start(std::string &/*full_file_path*/)
{
  if (iomode_ == sd_serialize) {
  }
  else {

  }
}

void SerDesBin::vector_start(unsigned int &size, const char *) throw (SerializerError)
{
  //  before reading/writing a vector, we need to read its size
  //  (so we know how many elements to read/alloc on deserialize
  translate(size);
}

void SerDesBin::vector_end()
{
  //  don't need to do anything
}

void SerDesBin::multimap_start(unsigned int &size, const char *) throw (SerializerError)
{
  //  before reading/writing a multimap, we need to read its size
  //  (so we know how many elements to read/alloc on deserialize
  translate(size);
}

void SerDesBin::multimap_end()
{
  //  don't need to do anything
}

void SerDesBin::annotation_start(const char *, const char *)
{
}

void SerDesBin::annotation_end()
{
  //  don't need to do anything
}

void SerDesBin::translate(bool &param, const char *tag)
{
  int rc;
  if (iomode_ == sd_serialize) {
    rc = fwrite(&param, sizeof(bool), 1, f);

    if (1 != rc) 
         SER_ERR("fwrite");
  }
  else {
    rc = fread(&param, sizeof(bool), 1, f);

    if (1 != rc) 
         SER_ERR("fread");
  }

  if (noisy)
     fprintf(stderr, "%s[%d]:  %sserialize %s=%s\n", FILE__, __LINE__,
           iomode_ == sd_serialize ? "" : "de", 
           tag ? tag : "no-tag",
           param ? "true": "false");
}

void SerDesBin::translate(char &param, const char *tag)
{
  int rc;
  if (iomode_ == sd_serialize) {
    rc = fwrite(&param, sizeof(char), 1, f);

    if (1 != rc) 
         SER_ERR("fwrite");
  }
  else {
    rc = fread(&param, sizeof(char), 1, f);

    if (1 != rc) 
         SER_ERR("fread");
  }

  if (noisy)
     fprintf(stderr, "%s[%d]:  %sserialize %s=%c\n", FILE__, __LINE__,
           iomode_ == sd_serialize ? "" : "de", 
           tag ? tag : "no-tag", param);
}

void SerDesBin::translate(int &param, const char *tag)
{
  int rc;
  if (iomode_ == sd_serialize) {
    rc = fwrite(&param, sizeof(int), 1, f);

    if (1 != rc) 
         SER_ERR("fwrite");
  }
  else {
    rc = fread(&param, sizeof(int), 1, f);

    if (1 != rc) 
         SER_ERR("fread");
  }

  if (noisy)
     fprintf(stderr, "%s[%d]:  %sserialize %s=%d\n", FILE__, __LINE__,
           iomode_ == sd_serialize ? "" : "de", 
           tag ? tag : "no-tag", param);
}

void SerDesBin::translate(unsigned int &param, const char * tag)
{
  //  overkill for a typecast??
  translate( (int &) param, tag);
}

void SerDesBin::translate(Address &param, const char *tag)
{
  int rc;
  if (iomode_ == sd_serialize) {
    rc = fwrite(&param, sizeof(Address), 1, f);

    if (1 != rc) 
         SER_ERR("fwrite");
  }
  else {
    rc = fread(&param, sizeof(Address), 1, f);

    if (1 != rc) 
         SER_ERR("fread");
  }

  if (noisy)
     fprintf(stderr, "%s[%d]:  %sserialize %s=%lx\n", FILE__, __LINE__,
           iomode_ == sd_serialize ? "" : "de", 
           tag ? tag : "no-tag", param);
}

#if 0
void SerDesBin::translate(OFFSET &param, const char *tag)
{
  int rc;
  if (iomode_ == sd_serialize) {
    rc = fwrite(&param, sizeof(OFFSET), 1, f);

    if (1 != rc) 
         SER_ERR("fwrite");
  }
  else {
    rc = fread(&param, sizeof(OFFSET), 1, f);

    if (1 != rc) 
         SER_ERR("fread");
  }

  if (noisy)
     fprintf(stderr, "%s[%d]:  %sserialize %s=%p\n", FILE__, __LINE__,
           iomode_ == sd_serialize ? "" : "de", 
           tag ? tag : "no-tag", param);
}
#endif

void SerDesBin::translate(const char * &param, int bufsize, const char *tag)
{
  //  string output format is
  //  [1]  length of string
  //  [2]  string data
  int rc, len;
  if (iomode_ == sd_serialize) {
      len = strlen(param);

      rc = fwrite( &len, sizeof(int), 1, f);

      if (1 != rc) 
         SER_ERR("fwrite");

      rc = fwrite(param, sizeof(char), len, f);

      if (len != rc) 
         SER_ERR("fwrite");
  }
  else {
      rc = fread(&len, sizeof(int), 1, f);

      if (1 != rc) 
         SER_ERR("fread");

      if (len > bufsize) {
        char msg[128];
        sprintf(msg, "not enough space in string buffer, %d needed", len);
        SER_ERR("msg");
      }

      char *l_ptr = const_cast<char *> (param);
      rc = fread(l_ptr, sizeof(char), len, f);

      if (len != rc) 
         SER_ERR("fread");

      l_ptr[len] = '\0';
  }

  if (noisy)
     fprintf(stderr, "%s[%d]:  %sserialize %s=%s\n", FILE__, __LINE__,
           iomode_ == sd_serialize ? "" : "de", 
           tag ? tag : "no-tag", param);
}

void SerDesBin::translate(std::string &param, const char *tag)
{
  if (iomode_ == sd_serialize) {
    const char *cstr = param.c_str();
    translate(cstr, 0, tag);
  }
  else {
     char buf[2048];
     const char *buf2 = buf;
     translate(buf2, 2048, tag);
     param = std::string(buf2);
  }
}

#if 0
void SerDesBin::translate(pdstring &param, const char *tag)
{
  if (iomode_ == sd_serialize) {
    const char *cstr = param.c_str();
    translate(cstr, 0, tag);
  }
  else {
     char buf[2048];
     const char *buf2 = buf;
     translate(buf2, 2048, tag);
     param = pdstring(buf2);
  }
}
#endif

void SerDesBin::translate(std::vector<std::string> &param, const char *tag, const char * elem_tag)
{
  //  string list output format is
  //  [1]  length of list, n
  //  [2]  <n> strings

  unsigned int nelem = param.size();
  translate(nelem, tag);

  if (iomode_ == sd_serialize) {
    for (unsigned int i = 0; i < nelem; ++i) {
      translate(param[i], elem_tag);
    }
  }
  else {
    param.resize(nelem);
    for (unsigned int i = 0; i < nelem; ++i) {
      param[i] = "";
      translate(param[i], elem_tag);
    }
  }
}

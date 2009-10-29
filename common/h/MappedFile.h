#ifndef __MAPPEDFILE_H__
#define __MAPPEDFILE_H__

#include "headers.h"

#include <string>
#include "Types.h"

class MappedFile {
     static dyn_hash_map<std::string, MappedFile *> mapped_files;

   public:
      COMMON_EXPORT static MappedFile *createMappedFile(std::string fullpath_);
      COMMON_EXPORT static MappedFile *createMappedFile(void *map_loc, unsigned long size_);
      COMMON_EXPORT static void closeMappedFile(MappedFile *&mf);

      COMMON_EXPORT std::string pathname();
      COMMON_EXPORT std::string filename();
      COMMON_EXPORT void *base_addr() {return map_addr;}
#if defined(os_windows)
      COMMON_EXPORT HANDLE getFileHandle() {return hFile;}
#else
      COMMON_EXPORT int getFD() {return fd;}
#endif
      COMMON_EXPORT unsigned long size() {return file_size;}
      COMMON_EXPORT MappedFile *clone() { refCount++; return this; }

      COMMON_EXPORT void setSharing(bool s);
      COMMON_EXPORT bool canBeShared();

   private:

      MappedFile(std::string fullpath_, bool &ok);
      MappedFile(void *loc, unsigned long size_, bool &ok);
      ~MappedFile();
      bool clean_up();
      std::string fullpath;
      void *map_addr;

      bool check_path(std::string &);
      bool open_file();
      bool open_file(void *, unsigned long size_ = 0);
      bool map_file();
      bool unmap_file();
      bool close_file();
#if defined (os_windows)
      HANDLE hMap; 
      HANDLE hFile; 
#else
      int fd;
#endif
      bool did_mmap;
      bool did_open;
      bool can_share;
      unsigned long file_size;
      int refCount;
};

#endif

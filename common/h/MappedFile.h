#ifndef __MAPPEDFILE_H__
#define __MAPPEDFILE_H__
#include <string>

#include "headers.h"
#include "Types.h"
#include "pathName.h"

class MappedFile {

   public:
      static DLLEXPORT MappedFile *createMappedFile(std::string fullpath_);
      static DLLEXPORT MappedFile *createMappedFile(void *map_loc);
      DLLEXPORT std::string &pathname() {return fullpath;}
      DLLEXPORT std::string filename();
      DLLEXPORT void *base_addr() {return map_addr;}
      DLLEXPORT unsigned size() {return file_size;}
#if defined(os_windows)
      DLLEXPORT HANDLE getFileHandle() {return hFile;}
#else
      DLLEXPORT int getFD() {return fd;}
#endif

   private:
      MappedFile(std::string fullpath, bool &ok);
      MappedFile(void *loc, bool &ok);
      ~MappedFile();
      bool clean_up();
      std::string fullpath;
      void *map_addr;

      bool check_path(std::string &);
      bool open_file();
      bool open_file(void *);
      bool map_file();
      bool unmap_file();
      bool close_file();
#if defined (os_windows)
      HANDLE hMap; 
      HANDLE mapAddr; 
      HANDLE hFile; 
#else
      int fd;
#endif
      bool did_mmap;
      bool did_open;
      unsigned file_size;
};

#endif

#include "common/h/MappedFile.h"
#include "common/h/pathName.h"

dyn_hash_map<std::string, MappedFile *> MappedFile::mapped_files;

MappedFile *MappedFile::createMappedFile(std::string fullpath_)
{
   if (mapped_files.find(fullpath_) != mapped_files.end()) {
      //fprintf(stderr, "%s[%d]:  mapped file exists for %s\n", FILE__, __LINE__, fullpath_.c_str());
      MappedFile  *ret = mapped_files[fullpath_];
      ret->refCount++;
      return ret;
   }


   bool ok = false;
   MappedFile *mf = new MappedFile(fullpath_, ok);
   if (!mf || !ok) {
      if (mf)
         delete mf;
      return NULL;
   }
   
   mapped_files[fullpath_] = mf;
   
//   fprintf(stderr, "%s[%d]:  MMAPFILE %s: mapped_files.size() =  %d\n", FILE__, __LINE__, fullpath_.c_str(), mapped_files.size());
   return mf;
}

MappedFile::MappedFile(std::string fullpath_, bool &ok) :
   fullpath(fullpath_),
   did_mmap(false),
   did_open(false),
   refCount(1)
{
  ok = check_path(fullpath);
  if (!ok) return;
  ok = open_file();
  if (!ok) return;
  ok = map_file();

  //  I think on unixes we can close the fd after mapping the file, 
  //  but is this really somehow better?
}

MappedFile *MappedFile::createMappedFile(void *loc, unsigned long size_)
{
   bool ok = false;
   MappedFile *mf = new MappedFile(loc, size_, ok);
   if (!mf || !ok) {
      if (mf)
         delete mf;
      return NULL;
  }

  return mf;
}

MappedFile::MappedFile(void *loc, unsigned long size_, bool &ok) :
   fullpath("in_memory_file"),
   did_mmap(false),
   did_open(false),
   refCount(1)
{
  ok = open_file(loc, size_);
#if defined(os_windows)  
  if (!ok) return;
  ok = map_file();
#endif
}

void MappedFile::closeMappedFile(MappedFile *&mf)
{
   if (!mf) 
   {
      fprintf(stderr, "%s[%d]:  BAD NEWS:  called closeMappedFile(NULL)\n", FILE__, __LINE__);
      return;
   }

   //fprintf(stderr, "%s[%d]:  welcome to closeMappedFile(%s) refCount = %d\n", FILE__, __LINE__, mf->pathname().c_str(), mf->refCount);

   mf->refCount--;

   if (mf->refCount <= 0) 
   {
      dyn_hash_map<std::string, MappedFile *>::iterator iter;
      iter = mapped_files.find(mf->pathname());

      if (iter != mapped_files.end()) 
      {
         mapped_files.erase(iter);
      }

      //fprintf(stderr, "%s[%d]:  DELETING mapped file\n", FILE__, __LINE__);
      //  dtor handles unmap and close

      delete mf;
      mf = NULL;
   }
}

bool MappedFile::clean_up()
{
   if (did_mmap) {
      if (!unmap_file()) goto err;
   }
   if (did_open) {
      if (!close_file()) goto err;
   }
   return true;

err:
   fprintf(stderr, "%s[%d]:  error unmapping file %s\n", 
         FILE__, __LINE__, fullpath.c_str() );
   return false;
}

MappedFile::~MappedFile()
{
  //  warning, destructor should not allowed to throw exceptions
   if (did_mmap)  {
      //fprintf(stderr, "%s[%d]: unmapping %s\n", FILE__, __LINE__, fullpath.c_str());
      unmap_file();
   }
   if (did_open) 
      close_file();
}

bool MappedFile::check_path(std::string &filename)
{
   struct stat statbuf;
   if (0 != stat(filename.c_str(), &statbuf)) {
      char ebuf[1024];
#if defined(os_windows)
      LPVOID lpMsgBuf;
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
            |     FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
            GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)
            &lpMsgBuf,    0,    NULL );

      sprintf(ebuf, "stat: %s", (char *) lpMsgBuf);
      LocalFree(lpMsgBuf);
#else
      sprintf(ebuf, "stat: %s", strerror(errno));
#endif
      goto err;
   }

   file_size = statbuf.st_size;

   return true;

err:
   return false;
}

bool MappedFile::open_file(void *loc, unsigned long size_)
{
#if defined(os_windows)
   hFile = LocalHandle( loc );  //For a mem image
   if (!hFile) {
      LPVOID lpMsgBuf;
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                    GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                    (LPTSTR) &lpMsgBuf, 0, NULL);

      char ebuf[1024];
      sprintf(ebuf, "CreateFileMapping failed: %s", (char *) lpMsgBuf);
      LocalFree(lpMsgBuf);
      goto err;
   }
   did_open = true;

   return true;
err:
   fprintf(stderr, "%s[%d]: failed to open file\n", FILE__, __LINE__);
   return false;
#else
   map_addr = loc;
   file_size = size_;
   did_open = false;
   fd = -1;
   return true;
#endif
}

bool MappedFile::open_file()
{
#if defined(os_windows)
   hFile = CreateFile(fullpath.c_str(), GENERIC_READ, FILE_SHARE_READ,
         NULL,OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
      LPVOID lpMsgBuf;
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
            |     FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
            GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)
            &lpMsgBuf,    0,    NULL );

      char ebuf[1024];
      sprintf(ebuf, "CreateFileMapping failed: %s", (char *) lpMsgBuf);
      LocalFree(lpMsgBuf);
      goto err;
   }
#else
   fd = open(fullpath.c_str(), O_RDONLY);
   if (-1 == fd) {
      char ebuf[1024];
      sprintf(ebuf, "open(%s) failed: %s", fullpath.c_str(), strerror(errno));
      goto err;
   }
#endif

   did_open = true;
   return true;
err:
   fprintf(stderr, "%s[%d]: failed to open file\n", FILE__, __LINE__);
   return false;
}

bool MappedFile::map_file()
{
   char ebuf[1024];
#if defined(os_windows)

   // map the file to our address space
   // first, create a file mapping object
   
   hMap = CreateFileMapping( hFile,
         NULL,           // security attrs
         PAGE_READONLY,  // protection flags
         0,              // max size - high DWORD
         0,              // max size - low DWORD
         NULL );         // mapping name - not used

   if (!hMap) {
      LPVOID lpMsgBuf;
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
            |     FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
            GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)
            &lpMsgBuf,    0,    NULL );

      sprintf(ebuf, "CreateFileMapping failed: %s", (char *) lpMsgBuf);
      LocalFree(lpMsgBuf);
      goto err;
   }

   // next, map the file to our address space

   map_addr = MapViewOfFileEx( hMap,             // mapping object
         FILE_MAP_READ,  // desired access
         0,              // loc to map - hi DWORD
         0,              // loc to map - lo DWORD
         0,              // #bytes to map - 0=all
         NULL );         // suggested map addr

   if (!map_addr) {
      LPVOID lpMsgBuf;
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
            |     FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
            GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)
            &lpMsgBuf,    0,    NULL );

      sprintf(ebuf, "MapViewOfFileEx failed: %s", (char *) lpMsgBuf);
      LocalFree(lpMsgBuf);
      goto err;
   }

#else

   map_addr = mmap(0, file_size, PROT_READ, MAP_SHARED, fd, 0);
   if (MAP_FAILED == map_addr) {
      sprintf(ebuf, "mmap(0, %d, PROT_READ, MAP_SHARED, %d, 0): %s", 
            file_size, fd, strerror(errno));
      goto err;
   }

#endif

   did_mmap = true;
   return true;
err:
   return false;
}

bool MappedFile::unmap_file()
{
#if defined(os_windows)

   UnmapViewOfFile(map_addr);
   CloseHandle(hMap);

#else

   if ( 0 != munmap(map_addr, file_size))  {
      fprintf(stderr, "%s[%d]: failed to unmap file\n", FILE__, __LINE__);
      return false;
   }
   
   map_addr = NULL;
#endif
   return true;
}

bool MappedFile::close_file()
{
#if defined (os_windows)

   CloseHandle(hFile);

#else

   if (-1 == close(fd)) {
      fprintf(stderr, "%s[%d]: failed to close file\n", FILE__, __LINE__);
      return false;
   }

#endif
   return true;
}

std::string MappedFile::pathname() 
{
	return fullpath;
}

std::string MappedFile::filename() 
{
	return extract_pathname_tail(fullpath);
}

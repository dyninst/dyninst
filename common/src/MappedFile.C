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
#include "common/src/MappedFile.h"
#include "common/src/pathName.h"
#include <iostream>
using namespace std;

dyn_hash_map<std::string, MappedFile *> MappedFile::mapped_files;

MappedFile *MappedFile::createMappedFile(std::string fullpath_)
{
   //fprintf(stderr, "%s[%d]:  createMappedFile %s\n", FILE__, __LINE__, fullpath_.c_str());
   if (mapped_files.find(fullpath_) != mapped_files.end()) {
      //fprintf(stderr, "%s[%d]:  mapped file exists for %s\n", FILE__, __LINE__, fullpath_.c_str());
      MappedFile  *ret = mapped_files[fullpath_];
      if (ret->can_share) {
         ret->refCount++;
         return ret;
      }
   }

   bool ok = false;
   MappedFile *mf = new MappedFile(fullpath_, ok);
   if (!mf) {
       return NULL;
   }

   if (!ok) {

#if defined(os_windows)
      if (std::string::npos != fullpath_.find(".dll") &&
          std::string::npos == fullpath_.find("\\"))
      {
          size_t envLen = 64;
          char *buf = (char*) malloc(envLen);
          bool repeat = false;
          do {
              repeat = false;
              if (getenv_s(&envLen, buf, envLen, "windir")) {
                  if (envLen > 64) { // error due to size problem
                      repeat = true;
                      free(buf);
                      buf = (char*) malloc(envLen);
                  }
              }
          } while(repeat); // repeat once if needed
          fullpath_ = buf + ("\\system32\\" + fullpath_);
          free(buf);
          return MappedFile::createMappedFile(fullpath_);
      }
	  else {
		  delete mf;
		  return NULL;
	  }
#else
      delete mf;
      return NULL;
#endif
   }

   mapped_files[fullpath_] = mf;

   //fprintf(stderr, "%s[%d]:  MMAPFILE %s: mapped_files.size() =  %d\n", FILE__, __LINE__, fullpath_.c_str(), mapped_files.size());
   return mf;
}

MappedFile::MappedFile(std::string fullpath_, bool &ok) :
   fullpath(fullpath_),
	   map_addr(NULL),
#if defined(os_windows)
	   hMap(NULL),
	   hFile(NULL),
#else
	   fd(-1),
#endif
   remote_file(false),
   did_mmap(false),
   did_open(false),
   can_share(true),
   refCount(1)
{
  ok = check_path(fullpath);
  if (!ok) {
	  return;
  }
  ok = open_file();
  if (!ok) return;
  ok = map_file();

  //  I think on unixes we can close the fd after mapping the file, 
  //  but is this really somehow better?
}

MappedFile *MappedFile::createMappedFile(void *loc, unsigned long size_, const std::string &name)
{
   bool ok = false;
   MappedFile *mf = new MappedFile(loc, size_, name, ok);
   if (!mf || !ok) {
      if (mf)
         delete mf;
      return NULL;
  }

  return mf;
}

MappedFile::MappedFile(void *loc, unsigned long size_, const std::string &name, bool &ok) :
   fullpath(name),
	map_addr(NULL),
#if defined(os_windows)
	hMap(NULL),
	hFile(NULL),
#endif
	   remote_file(false),
   did_mmap(false),
   did_open(false),
   can_share(true),
   refCount(1)
{
  ok = open_file(loc, size_);
#if defined(os_windows)  
  if (!ok) {
	  return;
  }
  //ok = map_file();
  map_addr = loc;
  this->file_size = size_;
#endif
}

void MappedFile::closeMappedFile(MappedFile *&mf)
{
   if (!mf) 
   {
      fprintf(stderr, "%s[%d]:  BAD NEWS:  called closeMappedFile(NULL)\n", FILE__, __LINE__);
      return;
   }

  //fprintf(stderr, "%s[%d]:  welcome to closeMappedFile() refCount = %d\n", FILE__, __LINE__, mf->refCount);
   mf->refCount--;

   if (mf->refCount <= 0) 
   {
      dyn_hash_map<std::string, MappedFile *>::iterator iter;
      iter = mapped_files.find(mf->filename());

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
         FILE_MAP_COPY,  // desired access
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

   int mmap_prot  = PROT_READ | PROT_WRITE;
   int mmap_flags = MAP_PRIVATE;

   map_addr = mmap(0, file_size, mmap_prot, mmap_flags, fd, 0);
   if (MAP_FAILED == map_addr) {
      sprintf(ebuf, "mmap(0, %lu, prot=0x%x, flags=0x%x, %d, 0): %s", 
            file_size, (unsigned int)mmap_prot, (unsigned int)mmap_flags, fd, strerror(errno));
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
   if (remote_file) {
      return true;
   }

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
   if (remote_file) {
      return true;
   }

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

std::string MappedFile::filename()
{
	return fullpath;
}

void MappedFile::setSharing(bool s)
{
   can_share = s;
}

bool MappedFile::canBeShared()
{
   return can_share;
}

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
#ifndef __MAPPEDFILE_H__
#define __MAPPEDFILE_H__

#include "headers.h"

#include <string>
#include "util.h"

class MappedFile {
     static dyn_hash_map<std::string, MappedFile *> mapped_files;

   public:
      COMMON_EXPORT static MappedFile *createMappedFile(std::string fullpath_);
      COMMON_EXPORT static MappedFile *createMappedFile(void *map_loc, unsigned long size_, const std::string &name);
      COMMON_EXPORT static void closeMappedFile(MappedFile *&mf);

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
      MappedFile(void *loc, unsigned long size_, const std::string & name, bool &ok);
      ~MappedFile();
      bool clean_up();

      bool check_path(std::string &);
      bool open_file();
      bool open_file(void *, unsigned long size_ = 0);
      bool map_file();
      bool unmap_file();
      bool close_file();

	  std::string fullpath;
      void *map_addr;

#if defined (os_windows)
      HANDLE hMap; 
      HANDLE hFile; 
#else
      int fd;
#endif

      bool remote_file;
      bool did_mmap;
      bool did_open;
      bool can_share;
      unsigned long file_size;
      int refCount;
};

#endif

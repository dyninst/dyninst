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

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>

#include <boost/crc.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/set.hpp>
#include <boost/assign/std/vector.hpp>

#include "common/src/headers.h"
#include "unaligned_memory_access.h"
#include "Elf_X.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <libelf.h>

#if DEBUGINFOD_LIB
#include <elfutils/debuginfod.h>
#endif

using namespace std;
using boost::crc_32_type;
using namespace boost::assign;

using namespace Dyninst;

#define DEBUGLINK_NAME ".gnu_debuglink"
#define BUILD_ID_NAME ".note.gnu.build-id"

template class std::map<std::pair<std::string, int> , Elf_X*>;
template class std::map<std::pair<std::string, char*> , Elf_X*>;
template class std::vector<Elf_X_Shdr>;
template class std::vector<Elf_X_Phdr>;

map<pair<string, int>, Elf_X *> Elf_X::elf_x_by_fd;
map<pair<string, char *>, Elf_X *> Elf_X::elf_x_by_ptr;

#define APPEND(X) X ## 1
#define APPEND2(X) APPEND(X)
#define LIBELF_TEST APPEND2(_LIBELF_H)
#if (LIBELF_TEST == 11)
#define USES_ELFUTILS
#endif

Elf_X *Elf_X::newElf_X(int input, Elf_Cmd cmd, Elf_X *ref, string name)
{
#if defined(USES_ELFUTILS)
   //If using libelf via elfutils from RedHat
   if (cmd == ELF_C_READ) {
      cmd = ELF_C_READ_MMAP_PRIVATE;
   }
#endif
   if (name.empty()) {
      return new Elf_X(input, cmd, ref);
   }
   auto i = elf_x_by_fd.find(make_pair(name, input));
   if (i != elf_x_by_fd.end()) {
     Elf_X *ret = i->second;
     ret->ref_count++;
     return ret;
   }
   Elf_X *ret = new Elf_X(input, cmd, ref);
   ret->filename = name;
   elf_x_by_fd.insert(make_pair(make_pair(name, input), ret));
   return ret;
}

Elf_X *Elf_X::newElf_X(char *mem_image, size_t mem_size, string name)
{
   if (name.empty()) {
      return new Elf_X(mem_image, mem_size);
   }
   auto i = elf_x_by_ptr.find(make_pair(name, mem_image));
   if (i != elf_x_by_ptr.end()) {
     Elf_X *ret = i->second;
     
     ret->ref_count++;
     return ret;
   }
   Elf_X *ret = new Elf_X(mem_image, mem_size);
   ret->filename = name;
   elf_x_by_ptr.insert(make_pair(make_pair(name, mem_image), ret));
   return ret;
}

// ------------------------------------------------------------------------
// Class Elf_X simulates the Elf(32|64)_Ehdr structure.
// Also works for ELF archives. 
Elf_X::Elf_X()
    : elf(NULL), ehdr32(NULL), ehdr64(NULL), phdr32(NULL), phdr64(NULL),
      filedes(-1), is64(false), isArchive(false), ref_count(1),
      cached_debug_buffer(NULL), cached_debug_size(0), cached_debug(false)
{ }

Elf_X::Elf_X(int input, Elf_Cmd cmd, Elf_X *ref)
    : elf(NULL), ehdr32(NULL), ehdr64(NULL), phdr32(NULL), phdr64(NULL),
      filedes(input), is64(false), isArchive(false), ref_count(1),
      cached_debug_buffer(NULL), cached_debug_size(0), cached_debug(false)
{
    if (elf_version(EV_CURRENT) == EV_NONE) {
       return;
    }
    elf_errno(); // Reset elf_errno to zero.
    if (ref)
       elf = elf_begin(input, cmd, ref->e_elfp());
    else {
       elf = elf_begin(input, cmd, NULL);
    }
    if (elf) {
       char *identp = elf_getident(elf, NULL);
       is64 = (identp && identp[EI_CLASS] == ELFCLASS64);
       isBigEndian = (identp && identp[EI_DATA] == ELFDATA2MSB);
       isArchive = (elf_kind(elf) == ELF_K_AR);
       
       if (!is64)  ehdr32 = elf32_getehdr(elf);
       else       ehdr64 = elf64_getehdr(elf);
       
       if (!is64) phdr32 = elf32_getphdr(elf);
       else       phdr64 = elf64_getphdr(elf);
    }

    if (elf_kind(elf) == ELF_K_ELF) {
       size_t phdrnum = e_phnum();
       size_t shdrnum = e_shnum();
       shdrs.resize(shdrnum);
       phdrs.resize(phdrnum);
    }
}

unsigned short Elf_X::e_endian() const {
    return isBigEndian;
}


Elf_X::Elf_X(char *mem_image, size_t mem_size)
    : elf(NULL), ehdr32(NULL), ehdr64(NULL), phdr32(NULL), phdr64(NULL),
      filedes(-1), is64(false), isArchive(false), ref_count(1),
      cached_debug_buffer(NULL), cached_debug_size(0), cached_debug(false)
{
    if (elf_version(EV_CURRENT) == EV_NONE) {
       return;
    }

    elf_errno(); // Reset elf_errno to zero.
    elf = elf_memory(mem_image, mem_size);
    
    int err;
    if ( (err = elf_errno()) != 0) {
       //const char *msg = elf_errmsg(err);
    }
    
    if (elf) {
       if (elf_kind(elf) == ELF_K_ELF) {
          char *identp = elf_getident(elf, NULL);
          is64 = (identp && identp[EI_CLASS] == ELFCLASS64);
          isBigEndian = (identp && identp[EI_DATA] == ELFDATA2MSB);
          isArchive = (elf_kind(elf) == ELF_K_AR);
       }
       
       if (!is64) ehdr32 = elf32_getehdr(elf);
       else       ehdr64 = elf64_getehdr(elf);
       
       if (!is64) phdr32 = elf32_getphdr(elf);
       else       phdr64 = elf64_getphdr(elf);
    }

    if (elf_kind(elf) == ELF_K_ELF) {
       size_t phdrnum = e_phnum();
       size_t shdrnum = e_shnum();
       shdrs.resize(shdrnum);
       phdrs.resize(phdrnum);
    }
}

void Elf_X::end()
{
   if (ref_count > 1) {
      ref_count--;
      return;
   }
   /*
     Stop cleaning Elf_X.  Leads to constant remapping of file.
   if (elf) {
      elf_end(elf);
      elf = NULL;
      ehdr32 = NULL;
      ehdr64 = NULL;
      phdr32 = NULL;
      phdr64 = NULL;
   }
   delete this;*/
}

Elf_X::~Elf_X()
{
  // Unfortunately, we have to be slow here
  for (auto iter = elf_x_by_fd.begin(); iter != elf_x_by_fd.end(); ++iter) {
    if (iter->second == this) {
      elf_x_by_fd.erase(iter);
      return;
    }
  }

  for (auto iter = elf_x_by_ptr.begin(); iter != elf_x_by_ptr.end(); ++iter) {
    if (iter->second == this) {
      elf_x_by_ptr.erase(iter);
      return;
    }
  }
}

// Read Interface
Elf *Elf_X::e_elfp() const
{
    return elf;
}

unsigned char *Elf_X::e_ident() const
{
    return (!is64 ?
            static_cast<unsigned char*>(ehdr32->e_ident) :
            static_cast<unsigned char*>(ehdr64->e_ident));
}

unsigned short Elf_X::e_type() const
{
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_type) :
            static_cast<unsigned short>(ehdr64->e_type));
}

unsigned short Elf_X::e_machine() const
{
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_machine) :
            static_cast<unsigned short>(ehdr64->e_machine));
}

unsigned long Elf_X::e_version() const
{
    return (!is64 ?
            static_cast<unsigned long>(ehdr32->e_version) :
            static_cast<unsigned long>(ehdr64->e_version));
}

unsigned long Elf_X::e_entry() const
{
    return (!is64 ?
            static_cast<unsigned long>(ehdr32->e_entry) :
            static_cast<unsigned long>(ehdr64->e_entry));
}

unsigned long Elf_X::e_phoff() const
{
    return (!is64 ?
            static_cast<unsigned long>(ehdr32->e_phoff) :
            static_cast<unsigned long>(ehdr64->e_phoff));
}

unsigned long Elf_X::e_shoff() const
{
    return (!is64 ?
            static_cast<unsigned long>(ehdr32->e_shoff) :
            static_cast<unsigned long>(ehdr64->e_shoff));
}

unsigned long Elf_X::e_flags() const
{
    return (!is64 ?
            static_cast<unsigned long>(ehdr32->e_flags) :
            static_cast<unsigned long>(ehdr64->e_flags));
}

unsigned short Elf_X::e_ehsize() const
{
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_ehsize) :
            static_cast<unsigned short>(ehdr64->e_ehsize));
}

unsigned short Elf_X::e_phentsize() const {
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_phentsize) :
            static_cast<unsigned short>(ehdr64->e_phentsize));
}

unsigned long Elf_X::e_phnum()
{
    size_t ret;
    elf_getphdrnum(elf, &ret);
    return (unsigned long)ret;
}

unsigned short Elf_X::e_shentsize() const
{
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_shentsize) :
            static_cast<unsigned short>(ehdr64->e_shentsize));
}

unsigned long Elf_X::e_shnum()
{
    size_t ret;
    elf_getshdrnum(elf, &ret);
    return (unsigned long)ret;
}

unsigned long Elf_X::e_shstrndx()
{
    size_t ret;
    elf_getshdrstrndx(elf, &ret);
    return (unsigned long)ret;
}

const char *Elf_X::e_rawfile(size_t &nbytes) const
{
    return elf_rawfile(elf, &nbytes);
}

Elf_X *Elf_X::e_next(Elf_X *ref)
{
    if (!isArchive)
        return NULL;
    Elf_Cmd cmd = elf_next(ref->e_elfp());
    return Elf_X::newElf_X(filedes, cmd, this);
}

Elf_X *Elf_X::e_rand(unsigned offset)
{
    if (!isArchive)
        return NULL;
    elf_rand(elf, offset);
    return Elf_X::newElf_X(filedes, ELF_C_READ, this);
}

// Write Interface
void Elf_X::e_ident(unsigned char *input)
{
    if (!is64) P_memcpy(ehdr32->e_ident, input, EI_NIDENT);
    else       P_memcpy(ehdr64->e_ident, input, EI_NIDENT);
}

void Elf_X::e_type(unsigned short input)
{
    if (!is64) ehdr32->e_type = input;
    else       ehdr64->e_type = input;
}

void Elf_X::e_machine(unsigned short input)
{
    if (!is64) ehdr32->e_machine = input;
    else       ehdr64->e_machine = input;
}

void Elf_X::e_version(unsigned long input)
{
    if (!is64) ehdr32->e_version = input;
    else       ehdr64->e_version = input;
}

void Elf_X::e_entry(unsigned long input)
{
    if (!is64) ehdr32->e_entry = input;
    else       ehdr64->e_entry = input;
}

void Elf_X::e_phoff(unsigned long input)
{
    if (!is64) ehdr32->e_phoff = input;
    else       ehdr64->e_phoff = input;
}

void Elf_X::e_shoff(unsigned long input)
{
    if (!is64) ehdr32->e_shoff = input;
    else       ehdr64->e_shoff = input;
}

void Elf_X::e_flags(unsigned long input)
{
    if (!is64) ehdr32->e_flags = input;
    else       ehdr64->e_flags = input;
}

void Elf_X::e_ehsize(unsigned short input)
{
    if (!is64) ehdr32->e_ehsize = input;
    else       ehdr64->e_ehsize = input;
}

void Elf_X::e_phentsize(unsigned short input)
{
    if (!is64) ehdr32->e_phentsize = input;
    else       ehdr64->e_phentsize = input;
}

void Elf_X::e_phnum(unsigned short input)
{
    if (!is64) ehdr32->e_phnum = input;
    else       ehdr64->e_phnum = input;
}

void Elf_X::e_shentsize(unsigned short input)
{
    if (!is64) ehdr32->e_shentsize = input;
    else       ehdr64->e_shentsize = input;
}

void Elf_X::e_shnum(unsigned short input)
{
    if (!is64) ehdr32->e_shnum = input;
    else       ehdr64->e_shnum = input;
}

void Elf_X::e_shstrndx(unsigned short input)
{
    if (!is64) ehdr32->e_shstrndx = input;
    else       ehdr64->e_shstrndx = input;
}

// Data Interface
bool Elf_X::isValid() const
{
    return (ehdr32 || ehdr64);
}

int Elf_X::wordSize() const
{
    return (!is64 ? 4 : 8);
}

Elf_X_Phdr &Elf_X::get_phdr(unsigned int i)
{
   if (is64 && !phdrs[i].phdr64) {
      phdrs[i] = Elf_X_Phdr(is64, phdr64 + i);
   }
   else if (!is64 && !phdrs[i].phdr32) {
      phdrs[i] = Elf_X_Phdr(is64, phdr32 + i);
   }
   return phdrs[i];
}

Elf_X_Shdr &Elf_X::get_shdr(unsigned int i)
{
   if (!shdrs[i]._elf) {
      Elf_Scn *scn = elf_getscn(elf, i);
      shdrs[i] = Elf_X_Shdr(is64, scn);
      shdrs[i]._elf = this;
   }
   return shdrs[i];
}

// ------------------------------------------------------------------------
// Class Elf_X_Phdr simulates the Elf(32|64)_Phdr structure.
Elf_X_Phdr::Elf_X_Phdr()
    : phdr32(NULL), phdr64(NULL), is64(false)
{ }

Elf_X_Phdr::Elf_X_Phdr(bool is64_, void *input)
    : phdr32(NULL), phdr64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) phdr32 = (Elf32_Phdr *)input;
        else       phdr64 = (Elf64_Phdr *)input;
    }
}

// Read Interface
unsigned long Elf_X_Phdr::p_type() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_type) :
            static_cast<unsigned long>(phdr64->p_type));
}

unsigned long Elf_X_Phdr::p_offset() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_offset) :
            static_cast<unsigned long>(phdr64->p_offset));
}

unsigned long Elf_X_Phdr::p_vaddr() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_vaddr) :
            static_cast<unsigned long>(phdr64->p_vaddr));
}

unsigned long Elf_X_Phdr::p_paddr() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_paddr) :
            static_cast<unsigned long>(phdr64->p_paddr));
}

unsigned long Elf_X_Phdr::p_filesz() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_filesz) :
            static_cast<unsigned long>(phdr64->p_filesz));
}

unsigned long Elf_X_Phdr::p_memsz() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_memsz) :
            static_cast<unsigned long>(phdr64->p_memsz));
}

unsigned long Elf_X_Phdr::p_flags() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_flags) :
            static_cast<unsigned long>(phdr64->p_flags));
}

unsigned long Elf_X_Phdr::p_align() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_align) :
            static_cast<unsigned long>(phdr64->p_align));
}

// Write Interface
void Elf_X_Phdr::p_type(unsigned long input)
{
    if (!is64) phdr32->p_type = input;
    else       phdr64->p_type = input;
}

void Elf_X_Phdr::p_offset(unsigned long input)
{
    if (!is64) phdr32->p_offset = input;
    else       phdr64->p_offset = input;
}

void Elf_X_Phdr::p_vaddr(unsigned long input)
{
    if (!is64) phdr32->p_vaddr = input;
    else       phdr64->p_vaddr = input;
}

void Elf_X_Phdr::p_paddr(unsigned long input)
{
    if (!is64) phdr32->p_paddr = input;
    else       phdr64->p_paddr = input;
}

void Elf_X_Phdr::p_filesz(unsigned long input)
{
    if (!is64) phdr32->p_filesz = input;
    else       phdr64->p_filesz = input;
}

void Elf_X_Phdr::p_memsz(unsigned long input)
{
    if (!is64) phdr32->p_memsz = input;
    else       phdr64->p_memsz = input;
}

void Elf_X_Phdr::p_flags(unsigned long input)
{
    if (!is64) phdr32->p_flags = input;
    else       phdr64->p_flags = input;
}

void Elf_X_Phdr::p_align(unsigned long input)
{
    if (!is64) phdr32->p_align = input;
    else       phdr64->p_align = input;
}

bool Elf_X_Phdr::isValid() const
{
    return (phdr32 || phdr64);
}

// ------------------------------------------------------------------------
// Class Elf_X_Shdr simulates the Elf(32|64)_Shdr structure.
Elf_X_Shdr::Elf_X_Shdr()
    : scn(NULL), data(NULL), shdr32(NULL), shdr64(NULL), is64(false),
      fromDebugFile(false), _elf(NULL)
{ }

Elf_X_Shdr::Elf_X_Shdr(bool is64_, Elf_Scn *input)
    : scn(input), data(NULL), shdr32(NULL), shdr64(NULL), is64(is64_),
      fromDebugFile(false), _elf(NULL)
{
    if (input) {
        first_data();
        if (!is64) shdr32 = elf32_getshdr(scn);
        else       shdr64 = elf64_getshdr(scn);
    }
}

// Read Interface
unsigned long Elf_X_Shdr::sh_name() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_name) :
            static_cast<unsigned long>(shdr64->sh_name));
}

unsigned long Elf_X_Shdr::sh_type() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_type) :
            static_cast<unsigned long>(shdr64->sh_type));
}

unsigned long Elf_X_Shdr::sh_flags() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_flags) :
            static_cast<unsigned long>(shdr64->sh_flags));
}

unsigned long Elf_X_Shdr::sh_addr() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_addr) :
            static_cast<unsigned long>(shdr64->sh_addr));
}

unsigned long Elf_X_Shdr::sh_offset() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_offset) :
            static_cast<unsigned long>(shdr64->sh_offset));
}

unsigned long Elf_X_Shdr::sh_size() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_size) :
            static_cast<unsigned long>(shdr64->sh_size));
}

unsigned long Elf_X_Shdr::sh_link() const
{
    return (!is64 ?
            shdr32->sh_link :
            shdr64->sh_link);
}

unsigned long Elf_X_Shdr::sh_info() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_info) :
            static_cast<unsigned long>(shdr64->sh_info));
}

unsigned long Elf_X_Shdr::sh_addralign() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_addralign) :
            static_cast<unsigned long>(shdr64->sh_addralign));
}

unsigned long Elf_X_Shdr::sh_entsize() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_entsize) :
            static_cast<unsigned long>(shdr64->sh_entsize));
}

bool Elf_X_Shdr::isFromDebugFile() const
{
    return fromDebugFile;
}

// Write Interface
void Elf_X_Shdr::sh_name(unsigned long input)
{
    if (!is64) shdr32->sh_name = input;
    else       shdr64->sh_name = input;
}

void Elf_X_Shdr::sh_type(unsigned long input)
{
    if (!is64) shdr32->sh_type = input;
    else       shdr64->sh_type = input;
}

void Elf_X_Shdr::sh_flags(unsigned long input)
{
    if (!is64) shdr32->sh_flags = input;
    else       shdr64->sh_flags = input;
}

void Elf_X_Shdr::sh_addr(unsigned long input)
{
    if (!is64) shdr32->sh_addr = input;
    else       shdr64->sh_addr = input;
}

void Elf_X_Shdr::sh_offset(unsigned long input)
{
    if (!is64) shdr32->sh_offset = input;
    else       shdr64->sh_offset = input;
}

void Elf_X_Shdr::sh_size(unsigned long input)
{
    if (!is64) shdr32->sh_size = input;
    else       shdr64->sh_size = input;
}

void Elf_X_Shdr::sh_link(unsigned long input)
{
    if (!is64) shdr32->sh_link = input;
    else       shdr64->sh_link = input;
}

void Elf_X_Shdr::sh_info(unsigned long input)
{
    if (!is64) shdr32->sh_info = input;
    else       shdr64->sh_info = input;
}

void Elf_X_Shdr::sh_addralign(unsigned long input)
{
    if (!is64) shdr32->sh_addralign = input;
    else       shdr64->sh_addralign = input;
}

void Elf_X_Shdr::sh_entsize(unsigned long input)
{
    if (!is64) shdr32->sh_entsize = input;
    else       shdr64->sh_entsize = input;
}

void Elf_X_Shdr::setDebugFile(bool b)
{
    fromDebugFile = b;
}

// Section Data Interface
Elf_X_Data Elf_X_Shdr::get_data() const
{
    return Elf_X_Data(is64, data);
}

// For Sections with Multiple Data Sections
void Elf_X_Shdr::first_data()
{
    data = elf_getdata(scn, NULL);
}

bool Elf_X_Shdr::next_data()
{
    Elf_Data *nextData = elf_getdata(scn, data);
    if (nextData) data = nextData;
    return nextData;
}

bool Elf_X_Shdr::isValid() const
{
    return (shdr32 || shdr64) && data;
}

unsigned Elf_X_Shdr::wordSize() const
{
    return is64 ? 8 : 4;
}

Elf_Scn *Elf_X_Shdr::getScn() const
{
    return scn;
}

Elf_X_Nhdr Elf_X_Shdr::get_note() const
{
    if (sh_type() != SHT_NOTE)
        return Elf_X_Nhdr();
    return Elf_X_Nhdr(data, 0);
}

// ------------------------------------------------------------------------
// Class Elf_X_Data simulates the Elf_Data structure.
Elf_X_Data::Elf_X_Data()
    : data(NULL), is64(false)
{ }

Elf_X_Data::Elf_X_Data(bool is64_, Elf_Data *input)
    : data(input), is64(is64_)
{ }

// Read Interface
void *Elf_X_Data::d_buf() const
{
    return data->d_buf;
}

Elf_Data * Elf_X_Data::elf_data() const
{
    return data;
}

Elf_Type Elf_X_Data::d_type() const
{
    return data->d_type;
}

unsigned int Elf_X_Data::d_version() const
{
    return data->d_version;
}

size_t Elf_X_Data::d_size() const
{
    return data->d_size;
}

off_t Elf_X_Data::d_off() const
{
    return (off_t) data->d_off;
}

size_t Elf_X_Data::d_align() const
{
    return data->d_align;
}

// Write Interface
void Elf_X_Data::d_buf(void *input)
{
    data->d_buf = input;
}

void Elf_X_Data::d_type(Elf_Type input)
{
    data->d_type = input;
}

void Elf_X_Data::d_version(unsigned int input)
{
    data->d_version = input;
}

void Elf_X_Data::d_size(unsigned int input)
{
    data->d_size = input;
}

void Elf_X_Data::d_off(signed int input)
{
    data->d_off = input;
}

void Elf_X_Data::d_align(unsigned int input)
{
    data->d_align = input;
}
void Elf_X_Data::xlatetom(unsigned int encode)
{
    if(is64)
    {
        elf64_xlatetom(data, data, encode);
    } else {
        elf32_xlatetom(data, data, encode);
    }
}
void Elf_X_Data::xlatetof(unsigned int encode)
{
    Elf_Data tmp;
    memcpy(&tmp, data, sizeof(Elf_Data));
    tmp.d_buf = malloc(tmp.d_size);
    if(is64)
    {
        elf64_xlatetof(data, data, encode);
    } else {
        elf32_xlatetof(data, data, encode);
    }
    memcpy(data->d_buf, tmp.d_buf, tmp.d_size);
    free(tmp.d_buf);
}

// Data Interface
const char *Elf_X_Data::get_string() const
{
    return (const char *)data->d_buf;
}

Elf_X_Dyn Elf_X_Data::get_dyn()
{
    return Elf_X_Dyn(is64, data);
}

Elf_X_Versym Elf_X_Data::get_versyms()
{
    return Elf_X_Versym(is64, data);
}

Elf_X_Verneed *Elf_X_Data::get_verNeedSym()
{
    return new Elf_X_Verneed(is64, data->d_buf);
}

Elf_X_Verdef *Elf_X_Data::get_verDefSym()
{
    return new Elf_X_Verdef(is64, data->d_buf);
}

Elf_X_Rel Elf_X_Data::get_rel()
{
    return Elf_X_Rel(is64, data);
}

Elf_X_Rela Elf_X_Data::get_rela()
{
    return Elf_X_Rela(is64, data);
}

Elf_X_Sym Elf_X_Data::get_sym()
{
    return Elf_X_Sym(is64, data);
}

bool Elf_X_Data::isValid() const
{
    return data != NULL;
}

// ------------------------------------------------------------------------
// Class Elf_X_Versym simulates the SHT_GNU_versym structure.
Elf_X_Versym::Elf_X_Versym()
    : data(NULL), versym32(NULL), versym64(NULL), is64(false)
{ }

Elf_X_Versym::Elf_X_Versym(bool is64_, Elf_Data *input)
    : data(input), versym32(NULL), versym64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) versym32 = (Elf32_Half *)data->d_buf;
        else       versym64 = (Elf64_Half *)data->d_buf;
    }
}

// Read Interface
unsigned long Elf_X_Versym::get(int i) const
{
    return (!is64 ? versym32[i]
                  : versym64[i]);
}

// Meta-Info Interface
unsigned long Elf_X_Versym::count() const
{
    return (data->d_size / (!is64 ? sizeof(Elf32_Half)
                                  : sizeof(Elf64_Half) ));
}

bool Elf_X_Versym::isValid() const
{
    return (versym32 || versym64);
}

// ------------------------------------------------------------------------
// Class Elf_X_Verdaux simulates the Elf(32|64)_Verdaux structure.
Elf_X_Verdaux::Elf_X_Verdaux()
    : data(NULL), verdaux32(NULL), verdaux64(NULL), is64(false)
{ }

Elf_X_Verdaux::Elf_X_Verdaux(bool is64_, void *input)
    : data(input), verdaux32(NULL), verdaux64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) verdaux32 = (Elf32_Verdaux *)data;
        else       verdaux64 = (Elf64_Verdaux *)data;
    }
}

// Read Interface
unsigned long Elf_X_Verdaux::vda_name() const
{
    return (!is64 ? verdaux32->vda_name
                  : verdaux64->vda_name);
}

unsigned long Elf_X_Verdaux::vda_next() const
{
    return (!is64 ? verdaux32->vda_next
                  : verdaux64->vda_next);
}

Elf_X_Verdaux *Elf_X_Verdaux::get_next() const
{
    if (vda_next() == 0)
        return NULL;
    return new Elf_X_Verdaux(is64, (char *)data+vda_next());
}

// Meta-Info Interface
bool Elf_X_Verdaux::isValid() const
{
    return (verdaux32 || verdaux64);
}

// ------------------------------------------------------------------------
// Class Elf_X_Verdef simulates the Elf(32|64)_Verdef structure.
Elf_X_Verdef::Elf_X_Verdef()
    : data(NULL), verdef32(NULL), verdef64(NULL), is64(false)
{ }

Elf_X_Verdef::Elf_X_Verdef(bool is64_, void *input)
    : data(input), verdef32(NULL), verdef64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) verdef32 = (Elf32_Verdef *)data;
        else       verdef64 = (Elf64_Verdef *)data;
    }
}

// Read Interface
unsigned long Elf_X_Verdef::vd_version() const
{
    return (!is64 ? verdef32->vd_version
                  : verdef64->vd_version);
}

unsigned long Elf_X_Verdef::vd_flags() const
{
    return (!is64 ? verdef32->vd_flags
                  : verdef64->vd_flags);
}

unsigned long Elf_X_Verdef::vd_ndx() const
{
    return (!is64 ? verdef32->vd_ndx
                  : verdef64->vd_ndx);
}

unsigned long Elf_X_Verdef::vd_cnt() const
{
    return (!is64 ? verdef32->vd_cnt
                  : verdef64->vd_cnt);
}

unsigned long Elf_X_Verdef::vd_hash() const
{
    return (!is64 ? verdef32->vd_hash
                  : verdef64->vd_hash);
}

unsigned long Elf_X_Verdef::vd_aux() const
{
    return (!is64 ? verdef32->vd_aux
                  : verdef64->vd_aux);
}

unsigned long Elf_X_Verdef::vd_next() const
{
    return (!is64 ? verdef32->vd_next
                  : verdef64->vd_next);
}

Elf_X_Verdaux *Elf_X_Verdef::get_aux() const
{
    if (vd_cnt() == 0)
        return NULL;
    return new Elf_X_Verdaux(is64, (char *)data+vd_aux());
}

Elf_X_Verdef *Elf_X_Verdef::get_next() const
{
    if (vd_next() == 0)
        return NULL;
    return new Elf_X_Verdef(is64, (char *)data+vd_next());
}

// Meta-Info Interface
bool Elf_X_Verdef::isValid() const
{
    return (verdef32 || verdef64);
}

// ------------------------------------------------------------------------
// Class Elf_X_Vernaux simulates the Elf(32|64)_Vernaux structure.
Elf_X_Vernaux::Elf_X_Vernaux()
    : data(NULL), vernaux32(NULL), vernaux64(NULL), is64(false)
{ }

Elf_X_Vernaux::Elf_X_Vernaux(bool is64_, void *input)
    : data(input), vernaux32(NULL), vernaux64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) vernaux32 = (Elf32_Vernaux *)data;
        else       vernaux64 = (Elf64_Vernaux *)data;
    }
}

// Read Interface
unsigned long Elf_X_Vernaux::vna_hash() const
{
    return (!is64 ? vernaux32->vna_hash
                  : vernaux64->vna_hash);
}

unsigned long Elf_X_Vernaux::vna_flags() const
{
    return (!is64 ? vernaux32->vna_flags
                  : vernaux64->vna_flags);
}

unsigned long Elf_X_Vernaux::vna_other() const
{
    return (!is64 ? vernaux32->vna_other
                  : vernaux64->vna_other);
}

unsigned long Elf_X_Vernaux::vna_name() const
{
    return (!is64 ? vernaux32->vna_name
                  : vernaux64->vna_name);
}

unsigned long Elf_X_Vernaux::vna_next() const
{
    return (!is64 ? vernaux32->vna_next
                  : vernaux64->vna_next);
}

Elf_X_Vernaux *Elf_X_Vernaux::get_next() const
{
    if (vna_next() == 0)
        return NULL;
    return new Elf_X_Vernaux(is64, (char *)data+vna_next());
}

// Meta-Info Interface
bool Elf_X_Vernaux::isValid() const
{
    return (vernaux32 || vernaux64);
}

// ------------------------------------------------------------------------
// Class Elf_X_Verneed simulates the Elf(32|64)_Verneed structure.
Elf_X_Verneed::Elf_X_Verneed()
    : data(NULL), verneed32(NULL), verneed64(NULL), is64(false)
{ }

Elf_X_Verneed::Elf_X_Verneed(bool is64_, void *input)
    : data(input), verneed32(NULL), verneed64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) verneed32 = (Elf32_Verneed *)data;
        else       verneed64 = (Elf64_Verneed *)data;
    }
}

// Read Interface
unsigned long Elf_X_Verneed::vn_version() const
{
    return (!is64 ? verneed32->vn_version
                  : verneed64->vn_version);
}

unsigned long Elf_X_Verneed::vn_cnt() const
{
    return (!is64 ? verneed32->vn_cnt
            : verneed64->vn_cnt);
}

unsigned long Elf_X_Verneed::vn_file() const
{
    return (!is64 ? verneed32->vn_file
                  : verneed64->vn_file);
}

unsigned long Elf_X_Verneed::vn_aux() const
{
    return (!is64 ? verneed32->vn_aux
                  : verneed64->vn_aux);
}

unsigned long Elf_X_Verneed::vn_next() const
{
    return (!is64 ? verneed32->vn_next
            : verneed64->vn_next);
}

Elf_X_Vernaux *Elf_X_Verneed::get_aux() const
{
    if (vn_cnt() == 0)
        return NULL;
    return new Elf_X_Vernaux(is64, (char *)data+vn_aux());
}

Elf_X_Verneed *Elf_X_Verneed::get_next() const
{
    if (vn_next() == 0)
        return NULL;
    return new Elf_X_Verneed(is64, (char *)data+vn_next());
}

// Meta-Info Interface
bool Elf_X_Verneed::isValid() const
{
    return (verneed32 || verneed64);
}


// ------------------------------------------------------------------------
// Class Elf_X_Sym simulates the Elf(32|64)_Sym structure.
Elf_X_Sym::Elf_X_Sym()
    : data(NULL), sym32(NULL), sym64(NULL), is64(false)
{ }

Elf_X_Sym::Elf_X_Sym(bool is64_, Elf_Data *input)
    : data(input), sym32(NULL), sym64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) sym32 = (Elf32_Sym *)data->d_buf;
        else       sym64 = (Elf64_Sym *)data->d_buf;
    }
}

// Read Interface
unsigned long Elf_X_Sym::st_name(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(sym32[i].st_name) :
            static_cast<unsigned long>(sym64[i].st_name));
}

unsigned long Elf_X_Sym::st_value(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(sym32[i].st_value) :
            static_cast<unsigned long>(sym64[i].st_value));
}

unsigned long Elf_X_Sym::st_size(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(sym32[i].st_size) :
            static_cast<unsigned long>(sym64[i].st_size));
}

unsigned char Elf_X_Sym::st_info(int i) const
{
    return (!is64 ?
            sym32[i].st_info :
            sym64[i].st_info);
}

unsigned char Elf_X_Sym::st_other(int i) const
{
    return (!is64 ?
            sym32[i].st_other :
            sym64[i].st_other);
}

unsigned short Elf_X_Sym::st_shndx(int i) const
{
    return (!is64 ?
            sym32[i].st_shndx :
            sym64[i].st_shndx);
}

unsigned char Elf_X_Sym::ST_BIND(int i) const
{
    return (!is64 ?
            static_cast<unsigned char>(ELF32_ST_BIND(sym32[i].st_info)) :
            static_cast<unsigned char>(ELF64_ST_BIND(sym64[i].st_info)));
}

unsigned char Elf_X_Sym::ST_TYPE(int i) const
{
    return (!is64 ?
            static_cast<unsigned char>(ELF32_ST_TYPE(sym32[i].st_info)) :
            static_cast<unsigned char>(ELF64_ST_TYPE(sym64[i].st_info)));
}

unsigned char Elf_X_Sym::ST_VISIBILITY(int i) const
{
    return (!is64 ?
            static_cast<unsigned char>(ELF32_ST_VISIBILITY(sym32[i].st_other)) :
            static_cast<unsigned char>(ELF64_ST_VISIBILITY(sym64[i].st_other)));
}

void *Elf_X_Sym::st_symptr(int i) const
{
    return (!is64 ?
            (void *)(sym32 + i) :
            (void *)(sym64 + i));
}

unsigned Elf_X_Sym::st_entsize() const
{
    return (is64 ?
            sizeof(Elf64_Sym) :
            sizeof(Elf32_Sym));
}

// Write Interface
void Elf_X_Sym::st_name(int i, unsigned long input)
{
    if (!is64) sym32[i].st_name = input;
    else       sym64[i].st_name = input;
}

void Elf_X_Sym::st_value(int i, unsigned long input)
{
    if (!is64) sym32[i].st_value = input;
    else       sym64[i].st_value = input;
}

void Elf_X_Sym::st_size(int i, unsigned long input)
{
    if (!is64) sym32[i].st_size = input;
    else       sym64[i].st_size = input;
}

void Elf_X_Sym::st_info(int i, unsigned char input)
{
    if (!is64) sym32[i].st_info = input;
    else       sym64[i].st_info = input;
}

void Elf_X_Sym::st_other(int i, unsigned char input)
{
    if (!is64) sym32[i].st_other = input;
    else       sym64[i].st_other = input;
}

void Elf_X_Sym::st_shndx(int i, unsigned short input)
{
    if (!is64) sym32[i].st_shndx = input;
    else       sym64[i].st_shndx = input;
}

// Meta-Info Interface
unsigned long Elf_X_Sym::count() const
{
    return (data->d_size / (!is64 ? sizeof(Elf32_Sym)
                                  : sizeof(Elf64_Sym)));
}

bool Elf_X_Sym::isValid() const
{
    return sym32 || sym64;
}

// ------------------------------------------------------------------------
// Class Elf_X_Rel simulates the Elf(32|64)_Rel structure.
Elf_X_Rel::Elf_X_Rel()
    : data(NULL), rel32(NULL), rel64(NULL), is64(false)
{ }

Elf_X_Rel::Elf_X_Rel(bool is64_, Elf_Data *input)
    : data(input), rel32(NULL), rel64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) 
            rel32 = (Elf32_Rel *)data->d_buf;
        else       
            rel64 = (Elf64_Rel *)data->d_buf;
    }
}

// Read Interface
unsigned long Elf_X_Rel::r_offset(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(rel32[i].r_offset) :
            static_cast<unsigned long>(rel64[i].r_offset));
}

unsigned long Elf_X_Rel::r_info(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(rel32[i].r_info) :
            static_cast<unsigned long>(rel64[i].r_info));
}

unsigned long Elf_X_Rel::R_SYM(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(ELF32_R_SYM(rel32[i].r_info)) :
            static_cast<unsigned long>(ELF64_R_SYM(rel64[i].r_info)));
}

unsigned long Elf_X_Rel::R_TYPE(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(ELF32_R_TYPE(rel32[i].r_info)) :
            static_cast<unsigned long>(ELF64_R_TYPE(rel64[i].r_info)));
}

// Write Interface
void Elf_X_Rel::r_offset(int i, unsigned long input)
{
    if (!is64)
        rel32[i].r_offset = input;
    else
        rel64[i].r_offset = input;
}

void Elf_X_Rel::r_info(int i, unsigned long input)
{
    if (!is64)
        rel32[i].r_info = input;
    else
        rel64[i].r_info = input;
}

// Meta-Info Interface
unsigned long Elf_X_Rel::count() const
{
    return (data->d_size / (!is64 ? sizeof(Elf32_Rel)
                                  : sizeof(Elf64_Rel)) );
}

bool Elf_X_Rel::isValid() const
{
    return (rel32 || rel64);
}

// ------------------------------------------------------------------------
// Class Elf_X_Rela simulates the Elf(32|64)_Rela structure.
Elf_X_Rela::Elf_X_Rela()
    : data(NULL), rela32(NULL), rela64(NULL), is64(false)
{ }

Elf_X_Rela::Elf_X_Rela(bool is64_, Elf_Data *input)
    : data(input), rela32(NULL), rela64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) 
            rela32 = (Elf32_Rela *)data->d_buf;
        else       
            rela64 = (Elf64_Rela *)data->d_buf;
    }
}

// Read Interface
unsigned long Elf_X_Rela::r_offset(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(rela32[i].r_offset) :
            static_cast<unsigned long>(rela64[i].r_offset));
}

unsigned long Elf_X_Rela::r_info(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(rela32[i].r_info) :
            static_cast<unsigned long>(rela64[i].r_info));
}

signed long Elf_X_Rela::r_addend(int i) const
{
    return (!is64 ?
            static_cast<signed long>(rela32[i].r_addend) :
            static_cast<signed long>(rela64[i].r_addend));
}

unsigned long Elf_X_Rela::R_SYM(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(ELF32_R_SYM(rela32[i].r_info)) :
            static_cast<unsigned long>(ELF64_R_SYM(rela64[i].r_info)));
}

unsigned long Elf_X_Rela::R_TYPE(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(ELF32_R_TYPE(rela32[i].r_info)) :
            static_cast<unsigned long>(ELF64_R_TYPE(rela64[i].r_info)));
}

// Write Interface
void Elf_X_Rela::r_offset(int i, unsigned long input)
{
    if (!is64)
        rela32[i].r_offset = input;
    else
        rela64[i].r_offset = input;
}

void Elf_X_Rela::r_info(int i, unsigned long input)
{
    if (!is64)
        rela32[i].r_info = input;
    else
        rela64[i].r_info = input;
}

void Elf_X_Rela::r_addend(int i, signed long input)
{
    if (!is64)
        rela32[i].r_addend = input;
    else
        rela64[i].r_addend = input;
}

// Meta-Info Interface
unsigned long Elf_X_Rela::count() const
{
    return (data->d_size / (!is64 ? sizeof(Elf32_Rela)
                                  : sizeof(Elf64_Rela)));
}

bool Elf_X_Rela::isValid() const
{
    return (rela32 || rela64);
}


// ------------------------------------------------------------------------
// Class Elf_X_Dyn simulates the Elf(32|64)_Dyn structure.
Elf_X_Dyn::Elf_X_Dyn()
    : data(NULL), dyn32(NULL), dyn64(NULL), is64(false)
{ }

Elf_X_Dyn::Elf_X_Dyn(bool is64_, Elf_Data *input)
    : data(input), dyn32(NULL), dyn64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) dyn32 = (Elf32_Dyn *)data->d_buf;
        else       dyn64 = (Elf64_Dyn *)data->d_buf;
    }
}

// Read Interface
signed long Elf_X_Dyn::d_tag(int i) const
{ 
    return (!is64 ?
            static_cast<signed long>(dyn32[i].d_tag) :
            static_cast<signed long>(dyn64[i].d_tag));
}

unsigned long Elf_X_Dyn::d_val(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(dyn32[i].d_un.d_val) :
            static_cast<unsigned long>(dyn64[i].d_un.d_val));
}

unsigned long Elf_X_Dyn::d_ptr(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(dyn32[i].d_un.d_ptr) :
            static_cast<unsigned long>(dyn64[i].d_un.d_ptr));
}

// Write Interface
void Elf_X_Dyn::d_tag(int i, signed long input)
{
    if (!is64) dyn32[i].d_tag = input;
    else       dyn64[i].d_tag = input;
}

void Elf_X_Dyn::d_val(int i, unsigned long input)
{
    if (!is64) dyn32[i].d_un.d_val = input;
    else       dyn64[i].d_un.d_val = input;
}

void Elf_X_Dyn::d_ptr(int i, unsigned long input)
{
    if (!is64) dyn32[i].d_un.d_ptr = input;
    else       dyn64[i].d_un.d_ptr = input;
}

// Meta-Info Interface
unsigned long Elf_X_Dyn::count() const
{
    return (data->d_size / (!is64 ? sizeof(Elf32_Dyn)
                                  : sizeof(Elf64_Dyn) ));
}

bool Elf_X_Dyn::isValid() const
{
    return (dyn32 || dyn64);
}

static bool loadDebugFileFromDisk(string name, char* &output_buffer, unsigned long &output_buffer_size)
{
   struct stat fileStat;
   int result = stat(name.c_str(), &fileStat);
   if (result == -1)
      return false;
   if (S_ISDIR(fileStat.st_mode))
      return false;
   int fd = open(name.c_str(), O_RDONLY);
   if (fd == -1)
      return false;

   char *buffer = (char *) mmap(NULL, fileStat.st_size,
                                PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
   close(fd);
   if (!buffer)
      return false;

   output_buffer = buffer;
   output_buffer_size = fileStat.st_size;

   return true;
}

// The standard procedure to look for a separate debug information file
// is as follows:
// 1. Lookup build_id from .note.gnu.build-id section and debug-file-name and
//    crc from .gnu_debuglink section of the original binary.
// 2. Look for the following files:
//        /usr/lib/debug/.build-id/<path-obtained-using-build-id>.debug
//        <debug-file-name> in <directory-of-executable>
//        <debug-file-name> in <directory-of-executable>/.debug
//        <debug-file-name> in /usr/lib/debug/<directory-of-executable>
// Reference: http://sourceware.org/gdb/current/onlinedocs/gdb_16.html#SEC157
bool Elf_X::findDebugFile(std::string origfilename, string &output_name, char* &output_buffer, unsigned long &output_buffer_size)
{
   if (cached_debug) {
      output_buffer = cached_debug_buffer;
      output_buffer_size = cached_debug_size;
      output_name = cached_debug_name;
      return (output_buffer != NULL);
   }
   cached_debug = true;

   uint16_t shnames_idx = e_shstrndx();
    // If we don't have names, bail.
    if(shnames_idx >= e_shnum()) return false;
   Elf_X_Shdr shnames_hdr = get_shdr(shnames_idx);
   if (!shnames_hdr.isValid())
      return false;
   const char *shnames = (const char *) shnames_hdr.get_data().d_buf();
   
  string debugFileFromDebugLink, debugFileFromBuildID;
  unsigned debugFileCrc = 0;

  for(auto i = 0UL; i < e_shnum(); i++) {
     Elf_X_Shdr scn = get_shdr(i);
     if (!scn.isValid()) { // section is malformed
        continue;
     }

     const char *name = &shnames[scn.sh_name()];
     if(strcmp(name, DEBUGLINK_NAME) == 0) {
        Elf_X_Data data = scn.get_data();
        debugFileFromDebugLink = (char *) data.d_buf();
        void *crcLocation = ((char *) data.d_buf() + data.d_size() - 4);
        debugFileCrc = *(unsigned *) crcLocation;
     }
     else if (scn.sh_type() == SHT_NOTE) {
        // Look for a build-id note in this section.  It is usually a note by
        // itself in section .note.gnu.build-id, but not necessarily so.
        for (Elf_X_Nhdr note = scn.get_note();
              note.isValid(); note = note.next()) {
           if (note.n_type() == 3 // NT_GNU_BUILD_ID
                 && note.n_namesz() == sizeof("GNU")
                 && strcmp(note.get_name(), "GNU") == 0
                 && note.n_descsz() >= 2) {
              // This is a raw build-id, now convert it to hex
              const unsigned char *desc = (const unsigned char *)note.get_desc();
              stringstream buildid_path;
              buildid_path << "/usr/lib/debug/.build-id/"
                 << hex << setfill('0') << setw(2) << (unsigned)desc[0] << '/';
              for (unsigned long j = 1; j < note.n_descsz(); ++j)
                 buildid_path << setw(2) << (unsigned)desc[j];
              buildid_path << ".debug";
              debugFileFromBuildID = buildid_path.str();
              break;
           }
        }
     }
  }

  if (!debugFileFromBuildID.empty()) {
     bool result = loadDebugFileFromDisk(debugFileFromBuildID, output_buffer, output_buffer_size);
     if (result) {
        output_name = debugFileFromBuildID;
        cached_debug_buffer = output_buffer;
        cached_debug_size = output_buffer_size;
        cached_debug_name = output_name;
        return true;
     }
  }

  if (!debugFileFromDebugLink.empty()) {
     char *mfPathNameCopy = strdup(origfilename.c_str());
     string objectFileDirName = dirname(mfPathNameCopy);

     vector<string> fnames = list_of
       (objectFileDirName + "/" + debugFileFromDebugLink)
       (objectFileDirName + "/.debug/" + debugFileFromDebugLink)
       ("/usr/lib/debug/" + objectFileDirName + "/" + debugFileFromDebugLink);

     free(mfPathNameCopy);

     for(unsigned i = 0; i < fnames.size(); i++) {
        bool result = loadDebugFileFromDisk(fnames[i], output_buffer, output_buffer_size);
        if (!result)
           continue;

        boost::crc_32_type crcComputer;
        crcComputer.process_bytes(output_buffer, output_buffer_size);
        if(crcComputer.checksum() != debugFileCrc) {
           munmap(output_buffer, output_buffer_size);
           continue;
        }

        output_name = fnames[i];
        cached_debug_buffer = output_buffer;
        cached_debug_size = output_buffer_size;
        cached_debug_name = output_name;
        return true;
     }
  }

#ifdef DEBUGINFOD_LIB
  if (!debugFileFromBuildID.empty()) {
     // Given /usr/lib/debug/.buildid/XX/YYYYYY.debug, isolate XXYYYYYY.
     size_t idx1 = debugFileFromBuildID.find_last_of("/");
     size_t idx2 = debugFileFromBuildID.find_last_of(".");

     if (idx1 == string::npos || idx2 == string::npos
         || idx1 < 2 || idx1 > idx2)
        return false;

     idx1 -= 2;
     string buildid(debugFileFromBuildID.substr(idx1, idx2 - idx1));
     buildid.erase(2, 1);

     debuginfod_client *client = debuginfod_begin();
     if (client == NULL)
        return false;

     char *path;
     int fd = debuginfod_find_debuginfo(client,
                                        (const unsigned char *)buildid.c_str(),
                                        0, &path);
     debuginfod_end(client);

     if (fd >= 0) {
        string fname = string(path);
        free(path);
        close(fd);

        bool result = loadDebugFileFromDisk(fname,
                                            output_buffer,
                                            output_buffer_size);
        if (result) {
           output_name = fname;
           cached_debug_buffer = output_buffer;
           cached_debug_size = output_buffer_size;
           cached_debug_name = output_name;
           return true;
        }
     }
  }
#endif

  return false;
}

// Add definitions that may not be in all elf.h files
#if !defined(EM_K10M)
#define EM_K10M 180
#endif
#if !defined(EM_L10M)
#define EM_L10M 181
#endif
#if !defined(EM_AARCH64)
#define EM_AARCH64 183
#endif
Dyninst::Architecture Elf_X::getArch() const
{
    switch(e_machine())
    {
        case EM_PPC:
            return Dyninst::Arch_ppc32;
        case EM_PPC64:
            return Dyninst::Arch_ppc64;
        case EM_386:
            return Dyninst::Arch_x86;
        case EM_X86_64:
        case EM_K10M:
        case EM_L10M:
            return Dyninst::Arch_x86_64;
        case EM_CUDA:
            return Dyninst::Arch_cuda;
        case EM_INTEL_GEN9:
            return Dyninst::Arch_intelGen9;
        case EM_INTELGT:
            return Dyninst::Arch_intelGen9;
        case EM_ARM:
            return Dyninst::Arch_aarch32;
        case EM_AARCH64:
            return Dyninst::Arch_aarch64;
        case EM_AMDGPU:
            {

                unsigned int ef_amdgpu_mach = 0x000000ff & e_flags();
                //cerr << " dealing with amd gpu , mach = "  << std::hex << ef_amdgpu_mach << endl;
                switch(ef_amdgpu_mach){
                    case 0x40:
                        return Dyninst::Arch_amdgpu_gfx940;
                    case 0x3f:
                        return Dyninst::Arch_amdgpu_gfx90a;
                    case 0x30:
                    case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: case 0x31:
                        return Dyninst::Arch_amdgpu_gfx908;
                    case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: case 0x18:
                    case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
                        assert(0 && "reserved for r600 architecture");
                    case 0x27: case 0x32 : case 0x39:
                        assert(0 && "reserved");
                    default:
                        //cerr  << "unsupported amdgpu architecture , value = " << ef_amdgpu_mach << endl;
                        assert(0 && "probabily won't be supported");

                }

                 
            }
        default:
            return Dyninst::Arch_none;
    }
}

// ------------------------------------------------------------------------
// Class Elf_X_Nhdr simulates the Elf(32|64)_Nhdr structure.
Elf_X_Nhdr::Elf_X_Nhdr()
    : data(NULL), nhdr(NULL)
{ }

Elf_X_Nhdr::Elf_X_Nhdr(Elf_Data *data_, size_t offset)
    : data(data_), nhdr(NULL)
{
    // 32|64 are actually the same, which simplifies things
    assert(sizeof(Elf32_Nhdr) == sizeof(Elf64_Nhdr));

    if (data && offset < data->d_size) {
        size_t size = data->d_size - offset;
        if (sizeof(*nhdr) <= size) {
            size -= sizeof(*nhdr);
            nhdr = alignas_cast<Elf32_Nhdr>((char *)data->d_buf + offset);
            if (n_namesz() > size || n_descsz() > size - n_namesz())
                nhdr = NULL;
        }
    }
    if (!nhdr)
        data = NULL;
}

// Read Interface
unsigned long Elf_X_Nhdr::n_namesz() const
{
    return isValid() ? nhdr->n_namesz : 0;
}

unsigned long Elf_X_Nhdr::n_descsz() const
{
    return isValid() ? nhdr->n_descsz : 0;
}

unsigned long Elf_X_Nhdr::n_type() const
{
    return isValid() ? nhdr->n_type : 0;
}

bool Elf_X_Nhdr::isValid() const
{
    return (data && nhdr);
}

const char* Elf_X_Nhdr::get_name() const
{
    return isValid() ? (char *)nhdr + sizeof(*nhdr) : NULL;
}

const void* Elf_X_Nhdr::get_desc() const
{
    return isValid() ? get_name() + n_namesz() : NULL;
}

Elf_X_Nhdr Elf_X_Nhdr::next() const
{
    if (!isValid())
        return Elf_X_Nhdr();

    size_t offset = (const char *)get_desc() + n_descsz() - (char *)data->d_buf;
    return Elf_X_Nhdr(data, offset);
}

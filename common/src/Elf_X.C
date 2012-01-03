/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "common/h/Elf_X.h"

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

using namespace std;
using boost::crc_32_type;
using namespace boost::assign;

#define DEBUGLINK_NAME ".gnu_debuglink"
#define BUILD_ID_NAME ".note.gnu.build-id"

#if defined(INLINE_ELF_X)
#define INLINE_DEF inline
#else
#define INLINE_DEF
#endif
// ------------------------------------------------------------------------
// Class Elf_X simulates the Elf(32|64)_Ehdr structure.
// Also works for ELF archives. 
INLINE_DEF Elf_X::Elf_X()
    : elf(NULL), ehdr32(NULL), ehdr64(NULL), phdr32(NULL), phdr64(NULL),
      filedes(-1), is64(false), isArchive(false)
{ }

INLINE_DEF Elf_X::Elf_X(int input, Elf_Cmd cmd, Elf_X *ref)
    : ehdr32(NULL), ehdr64(NULL), phdr32(NULL), phdr64(NULL),
      filedes(input), is64(false), isArchive(false)
{
    if (elf_version(EV_CURRENT) != EV_NONE) {
        elf_errno(); // Reset elf_errno to zero.
        if (ref)
            elf = elf_begin(input, cmd, ref->e_elfp());
        else
            elf = elf_begin(input, cmd, NULL);
        int errnum;
        if ((errnum = elf_errno()) != 0) {
            //const char *msg = elf_errmsg(errnum);
            //fprintf(stderr, "Elf error: %s\n", msg);
        }
        if (elf) {
            if (elf_kind(elf) == ELF_K_ELF) {
                char *identp = elf_getident(elf, NULL);
                is64 = (identp && identp[EI_CLASS] == ELFCLASS64);
            }
            else if(elf_kind(elf) == ELF_K_AR) {
                char *identp = elf_getident(elf, NULL);
                is64 = (identp && identp[EI_CLASS] == ELFCLASS64);
                isArchive = true;
            }

            if (!is64) ehdr32 = elf32_getehdr(elf);
            else       ehdr64 = elf64_getehdr(elf);

            if (!is64) phdr32 = elf32_getphdr(elf);
            else       phdr64 = elf64_getphdr(elf);
        }
    }
}

INLINE_DEF Elf_X::Elf_X(char *mem_image, size_t mem_size)
    : ehdr32(NULL), ehdr64(NULL), phdr32(NULL), phdr64(NULL),
      is64(false), isArchive(false)
{
    if (elf_version(EV_CURRENT) != EV_NONE) {
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
            }

            if (!is64) ehdr32 = elf32_getehdr(elf);
            else       ehdr64 = elf64_getehdr(elf);

            if (!is64) phdr32 = elf32_getphdr(elf);
            else       phdr64 = elf64_getphdr(elf);
        }
    }
}

INLINE_DEF void Elf_X::end()
{
    if (elf) {
        elf_end(elf);
        elf = NULL;
        ehdr32 = NULL;
        ehdr64 = NULL;
        phdr32 = NULL;
        phdr64 = NULL;
    }
}

// Read Interface
INLINE_DEF Elf *Elf_X::e_elfp() const
{
    return elf;
}

INLINE_DEF unsigned char *Elf_X::e_ident() const
{
    return (!is64 ?
            static_cast<unsigned char*>(ehdr32->e_ident) :
            static_cast<unsigned char*>(ehdr64->e_ident));
}

INLINE_DEF unsigned short Elf_X::e_type() const
{
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_type) :
            static_cast<unsigned short>(ehdr64->e_type));
}

INLINE_DEF unsigned short Elf_X::e_machine() const
{
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_machine) :
            static_cast<unsigned short>(ehdr64->e_machine));
}

INLINE_DEF unsigned long Elf_X::e_version() const
{
    return (!is64 ?
            static_cast<unsigned long>(ehdr32->e_version) :
            static_cast<unsigned long>(ehdr64->e_version));
}

INLINE_DEF unsigned long Elf_X::e_entry() const
{
    return (!is64 ?
            static_cast<unsigned long>(ehdr32->e_entry) :
            static_cast<unsigned long>(ehdr64->e_entry));
}

INLINE_DEF unsigned long Elf_X::e_phoff() const
{
    return (!is64 ?
            static_cast<unsigned long>(ehdr32->e_phoff) :
            static_cast<unsigned long>(ehdr64->e_phoff));
}

INLINE_DEF unsigned long Elf_X::e_shoff() const
{
    return (!is64 ?
            static_cast<unsigned long>(ehdr32->e_shoff) :
            static_cast<unsigned long>(ehdr64->e_shoff));
}

INLINE_DEF unsigned long Elf_X::e_flags() const
{
    return (!is64 ?
            static_cast<unsigned long>(ehdr32->e_flags) :
            static_cast<unsigned long>(ehdr64->e_flags));
}

INLINE_DEF unsigned short Elf_X::e_ehsize() const
{
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_ehsize) :
            static_cast<unsigned short>(ehdr64->e_ehsize));
}

INLINE_DEF unsigned short Elf_X::e_phentsize() const {
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_phentsize) :
            static_cast<unsigned short>(ehdr64->e_phentsize));
}

INLINE_DEF unsigned short Elf_X::e_phnum() const
{
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_phnum) :
            static_cast<unsigned short>(ehdr64->e_phnum));
}

INLINE_DEF unsigned short Elf_X::e_shentsize() const
{
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_shentsize) :
            static_cast<unsigned short>(ehdr64->e_shentsize));
}

INLINE_DEF unsigned short Elf_X::e_shnum() const
{
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_shnum) :
            static_cast<unsigned short>(ehdr64->e_shnum));
}

INLINE_DEF unsigned short Elf_X::e_shstrndx() const
{
    return (!is64 ?
            static_cast<unsigned short>(ehdr32->e_shstrndx) :
            static_cast<unsigned short>(ehdr64->e_shstrndx));
}

INLINE_DEF const char *Elf_X::e_rawfile(size_t &nbytes) const
{
    return elf_rawfile(elf, &nbytes);
}

INLINE_DEF Elf_X *Elf_X::e_next(Elf_X *ref)
{
    if (!isArchive)
        return NULL;
    Elf_Cmd cmd = elf_next(ref->e_elfp());
    return new Elf_X(filedes, cmd, this);
}

INLINE_DEF Elf_X *Elf_X::e_rand(unsigned offset)
{
    if (!isArchive)
        return NULL;
    elf_rand(elf, offset);
    return new Elf_X(filedes, ELF_C_READ, this);
}

// Write Interface
INLINE_DEF void Elf_X::e_ident(unsigned char *input)
{
    if (!is64) P_memcpy(ehdr32->e_ident, input, EI_NIDENT);
    else       P_memcpy(ehdr64->e_ident, input, EI_NIDENT);
}

INLINE_DEF void Elf_X::e_type(unsigned short input)
{
    if (!is64) ehdr32->e_type = input;
    else       ehdr64->e_type = input;
}

INLINE_DEF void Elf_X::e_machine(unsigned short input)
{
    if (!is64) ehdr32->e_machine = input;
    else       ehdr64->e_machine = input;
}

INLINE_DEF void Elf_X::e_version(unsigned long input)
{
    if (!is64) ehdr32->e_version = input;
    else       ehdr64->e_version = input;
}

INLINE_DEF void Elf_X::e_entry(unsigned long input)
{
    if (!is64) ehdr32->e_entry = input;
    else       ehdr64->e_entry = input;
}

INLINE_DEF void Elf_X::e_phoff(unsigned long input)
{
    if (!is64) ehdr32->e_phoff = input;
    else       ehdr64->e_phoff = input;
}

INLINE_DEF void Elf_X::e_shoff(unsigned long input)
{
    if (!is64) ehdr32->e_shoff = input;
    else       ehdr64->e_shoff = input;
}

INLINE_DEF void Elf_X::e_flags(unsigned long input)
{
    if (!is64) ehdr32->e_flags = input;
    else       ehdr64->e_flags = input;
}

INLINE_DEF void Elf_X::e_ehsize(unsigned short input)
{
    if (!is64) ehdr32->e_ehsize = input;
    else       ehdr64->e_ehsize = input;
}

INLINE_DEF void Elf_X::e_phentsize(unsigned short input)
{
    if (!is64) ehdr32->e_phentsize = input;
    else       ehdr64->e_phentsize = input;
}

INLINE_DEF void Elf_X::e_phnum(unsigned short input)
{
    if (!is64) ehdr32->e_phnum = input;
    else       ehdr64->e_phnum = input;
}

INLINE_DEF void Elf_X::e_shentsize(unsigned short input)
{
    if (!is64) ehdr32->e_shentsize = input;
    else       ehdr64->e_shentsize = input;
}

INLINE_DEF void Elf_X::e_shnum(unsigned short input)
{
    if (!is64) ehdr32->e_shnum = input;
    else       ehdr64->e_shnum = input;
}

INLINE_DEF void Elf_X::e_shstrndx(unsigned short input)
{
    if (!is64) ehdr32->e_shstrndx = input;
    else       ehdr64->e_shstrndx = input;
}

// Data Interface
INLINE_DEF bool Elf_X::isValid() const
{
    return (ehdr32 || ehdr64);
}

INLINE_DEF int Elf_X::wordSize() const
{
    return (!is64 ? 4 : 8);
}

INLINE_DEF Elf_X_Phdr Elf_X::get_phdr(unsigned int i) const
{
    if (!is64) return Elf_X_Phdr(is64, phdr32 + i);
    else       return Elf_X_Phdr(is64, phdr64 + i);
}

INLINE_DEF Elf_X_Shdr Elf_X::get_shdr(unsigned int i) const
{
    Elf_Scn *scn = elf_getscn(elf, i);
    Elf_X_Shdr result(is64, scn);
    result._elf = this;
    return result;
}

// ------------------------------------------------------------------------
// Class Elf_X_Phdr simulates the Elf(32|64)_Phdr structure.
INLINE_DEF Elf_X_Phdr::Elf_X_Phdr()
    : phdr32(NULL), phdr64(NULL), is64(false)
{ }

INLINE_DEF Elf_X_Phdr::Elf_X_Phdr(bool is64_, void *input)
    : phdr32(NULL), phdr64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) phdr32 = (Elf32_Phdr *)input;
        else       phdr64 = (Elf64_Phdr *)input;
    }
}

// Read Interface
INLINE_DEF unsigned long Elf_X_Phdr::p_type() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_type) :
            static_cast<unsigned long>(phdr64->p_type));
}

INLINE_DEF unsigned long Elf_X_Phdr::p_offset() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_offset) :
            static_cast<unsigned long>(phdr64->p_offset));
}

INLINE_DEF unsigned long Elf_X_Phdr::p_vaddr() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_vaddr) :
            static_cast<unsigned long>(phdr64->p_vaddr));
}

INLINE_DEF unsigned long Elf_X_Phdr::p_paddr() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_paddr) :
            static_cast<unsigned long>(phdr64->p_paddr));
}

INLINE_DEF unsigned long Elf_X_Phdr::p_filesz() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_filesz) :
            static_cast<unsigned long>(phdr64->p_filesz));
}

INLINE_DEF unsigned long Elf_X_Phdr::p_memsz() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_memsz) :
            static_cast<unsigned long>(phdr64->p_memsz));
}

INLINE_DEF unsigned long Elf_X_Phdr::p_flags() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_flags) :
            static_cast<unsigned long>(phdr64->p_flags));
}

INLINE_DEF unsigned long Elf_X_Phdr::p_align() const
{
    return (!is64 ?
            static_cast<unsigned long>(phdr32->p_align) :
            static_cast<unsigned long>(phdr64->p_align));
}

// Write Interface
INLINE_DEF void Elf_X_Phdr::p_type(unsigned long input)
{
    if (!is64) phdr32->p_type = input;
    else       phdr64->p_type = input;
}

INLINE_DEF void Elf_X_Phdr::p_offset(unsigned long input)
{
    if (!is64) phdr32->p_offset = input;
    else       phdr64->p_offset = input;
}

INLINE_DEF void Elf_X_Phdr::p_vaddr(unsigned long input)
{
    if (!is64) phdr32->p_vaddr = input;
    else       phdr64->p_vaddr = input;
}

INLINE_DEF void Elf_X_Phdr::p_paddr(unsigned long input)
{
    if (!is64) phdr32->p_paddr = input;
    else       phdr64->p_paddr = input;
}

INLINE_DEF void Elf_X_Phdr::p_filesz(unsigned long input)
{
    if (!is64) phdr32->p_filesz = input;
    else       phdr64->p_filesz = input;
}

INLINE_DEF void Elf_X_Phdr::p_memsz(unsigned long input)
{
    if (!is64) phdr32->p_memsz = input;
    else       phdr64->p_memsz = input;
}

INLINE_DEF void Elf_X_Phdr::p_flags(unsigned long input)
{
    if (!is64) phdr32->p_flags = input;
    else       phdr64->p_flags = input;
}

INLINE_DEF void Elf_X_Phdr::p_align(unsigned long input)
{
    if (!is64) phdr32->p_align = input;
    else       phdr64->p_align = input;
}

INLINE_DEF bool Elf_X_Phdr::isValid() const
{
    return (phdr32 || phdr64);
}

// ------------------------------------------------------------------------
// Class Elf_X_Shdr simulates the Elf(32|64)_Shdr structure.
INLINE_DEF Elf_X_Shdr::Elf_X_Shdr()
    : scn(NULL), data(NULL), shdr32(NULL), shdr64(NULL), is64(false),
      fromDebugFile(false), _elf(NULL)
{ }

INLINE_DEF Elf_X_Shdr::Elf_X_Shdr(bool is64_, Elf_Scn *input)
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
INLINE_DEF unsigned long Elf_X_Shdr::sh_name() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_name) :
            static_cast<unsigned long>(shdr64->sh_name));
}

INLINE_DEF unsigned long Elf_X_Shdr::sh_type() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_type) :
            static_cast<unsigned long>(shdr64->sh_type));
}

INLINE_DEF unsigned long Elf_X_Shdr::sh_flags() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_flags) :
            static_cast<unsigned long>(shdr64->sh_flags));
}

INLINE_DEF unsigned long Elf_X_Shdr::sh_addr() const
{
#if defined(os_vxworks)
    assert(_elf);
    if (_elf->e_type() == ET_REL) {
        // VxWorks relocatable object files (kernel modules) don't have
        // the address filled out.  Return the disk offset instead.
        return (!is64 ?
                static_cast<unsigned long>(shdr32->sh_offset) :
                static_cast<unsigned long>(shdr64->sh_offset));
    }
#endif

    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_addr) :
            static_cast<unsigned long>(shdr64->sh_addr));
}

INLINE_DEF unsigned long Elf_X_Shdr::sh_offset() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_offset) :
            static_cast<unsigned long>(shdr64->sh_offset));
}

INLINE_DEF unsigned long Elf_X_Shdr::sh_size() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_size) :
            static_cast<unsigned long>(shdr64->sh_size));
}

INLINE_DEF unsigned long Elf_X_Shdr::sh_link() const
{
    return (!is64 ?
            shdr32->sh_link :
            shdr64->sh_link);
}

INLINE_DEF unsigned long Elf_X_Shdr::sh_info() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_info) :
            static_cast<unsigned long>(shdr64->sh_info));
}

INLINE_DEF unsigned long Elf_X_Shdr::sh_addralign() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_addralign) :
            static_cast<unsigned long>(shdr64->sh_addralign));
}

INLINE_DEF unsigned long Elf_X_Shdr::sh_entsize() const
{
    return (!is64 ?
            static_cast<unsigned long>(shdr32->sh_entsize) :
            static_cast<unsigned long>(shdr64->sh_entsize));
}

INLINE_DEF bool Elf_X_Shdr::isFromDebugFile() const
{
    return fromDebugFile;
}

// Write Interface
INLINE_DEF void Elf_X_Shdr::sh_name(unsigned long input)
{
    if (!is64) shdr32->sh_name = input;
    else       shdr64->sh_name = input;
}

INLINE_DEF void Elf_X_Shdr::sh_type(unsigned long input)
{
    if (!is64) shdr32->sh_type = input;
    else       shdr64->sh_type = input;
}

INLINE_DEF void Elf_X_Shdr::sh_flags(unsigned long input)
{
    if (!is64) shdr32->sh_flags = input;
    else       shdr64->sh_flags = input;
}

INLINE_DEF void Elf_X_Shdr::sh_addr(unsigned long input)
{
    if (!is64) shdr32->sh_flags = input;
    else       shdr64->sh_flags = input;
}

INLINE_DEF void Elf_X_Shdr::sh_offset(unsigned long input)
{
    if (!is64) shdr32->sh_offset = input;
    else       shdr64->sh_offset = input;
}

INLINE_DEF void Elf_X_Shdr::sh_size(unsigned long input)
{
    if (!is64) shdr32->sh_size = input;
    else       shdr64->sh_size = input;
}

INLINE_DEF void Elf_X_Shdr::sh_link(unsigned long input)
{
    if (!is64) shdr32->sh_link = input;
    else       shdr64->sh_link = input;
}

INLINE_DEF void Elf_X_Shdr::sh_info(unsigned long input)
{
    if (!is64) shdr32->sh_info = input;
    else       shdr64->sh_info = input;
}

INLINE_DEF void Elf_X_Shdr::sh_addralign(unsigned long input)
{
    if (!is64) shdr32->sh_addralign = input;
    else       shdr64->sh_addralign = input;
}

INLINE_DEF void Elf_X_Shdr::sh_entsize(unsigned long input)
{
    if (!is64) shdr32->sh_entsize = input;
    else       shdr64->sh_entsize = input;
}

INLINE_DEF void Elf_X_Shdr::setDebugFile(bool b)
{
    fromDebugFile = b;
}

// Section Data Interface
INLINE_DEF Elf_X_Data Elf_X_Shdr::get_data() const
{
    return Elf_X_Data(is64, data);
}

// For Sections with Multiple Data Sections
INLINE_DEF void Elf_X_Shdr::first_data()
{
    data = elf_getdata(scn, NULL);
}

INLINE_DEF bool Elf_X_Shdr::next_data()
{
    Elf_Data *nextData = elf_getdata(scn, data);
    if (nextData) data = nextData;
    return nextData;
}

INLINE_DEF bool Elf_X_Shdr::isValid() const
{
    return (shdr32 || shdr64);
}

INLINE_DEF unsigned Elf_X_Shdr::wordSize() const
{
    return is64 ? 8 : 4;
}

INLINE_DEF Elf_Scn *Elf_X_Shdr::getScn() const
{
    return scn;
}

// ------------------------------------------------------------------------
// Class Elf_X_Data simulates the Elf_Data structure.
INLINE_DEF Elf_X_Data::Elf_X_Data()
    : data(NULL), is64(false)
{ }

INLINE_DEF Elf_X_Data::Elf_X_Data(bool is64_, Elf_Data *input)
    : data(input), is64(is64_)
{ }

// Read Interface
INLINE_DEF void *Elf_X_Data::d_buf() const
{
    return data->d_buf;
}

INLINE_DEF Elf_Type Elf_X_Data::d_type() const
{
    return data->d_type;
}

INLINE_DEF unsigned int Elf_X_Data::d_version() const
{
    return data->d_version;
}

INLINE_DEF size_t Elf_X_Data::d_size() const
{
    return data->d_size;
}

INLINE_DEF off_t Elf_X_Data::d_off() const
{
    return (off_t) data->d_off;
}

INLINE_DEF size_t Elf_X_Data::d_align() const
{
    return data->d_align;
}

// Write Interface
INLINE_DEF void Elf_X_Data::d_buf(void *input)
{
    data->d_buf = input;
}

INLINE_DEF void Elf_X_Data::d_type(Elf_Type input)
{
    data->d_type = input;
}

INLINE_DEF void Elf_X_Data::d_version(unsigned int input)
{
    data->d_version = input;
}

INLINE_DEF void Elf_X_Data::d_size(unsigned int input)
{
    data->d_size = input;
}

INLINE_DEF void Elf_X_Data::d_off(signed int input)
{
    data->d_off = input;
}

INLINE_DEF void Elf_X_Data::d_align(unsigned int input)
{
    data->d_align = input;
}

// Data Interface
INLINE_DEF const char *Elf_X_Data::get_string() const
{
    return (const char *)data->d_buf;
}

INLINE_DEF Elf_X_Dyn Elf_X_Data::get_dyn()
{
    return Elf_X_Dyn(is64, data);
}

INLINE_DEF Elf_X_Versym Elf_X_Data::get_versyms()
{
    return Elf_X_Versym(is64, data);
}

INLINE_DEF Elf_X_Verneed *Elf_X_Data::get_verNeedSym()
{
    return new Elf_X_Verneed(is64, data->d_buf);
}

INLINE_DEF Elf_X_Verdef *Elf_X_Data::get_verDefSym()
{
    return new Elf_X_Verdef(is64, data->d_buf);
}

INLINE_DEF Elf_X_Rel Elf_X_Data::get_rel()
{
    return Elf_X_Rel(is64, data);
}

INLINE_DEF Elf_X_Rela Elf_X_Data::get_rela()
{
    return Elf_X_Rela(is64, data);
}

INLINE_DEF Elf_X_Sym Elf_X_Data::get_sym()
{
    return Elf_X_Sym(is64, data);
}

#if defined(arch_mips)
INLINE_DEF Elf_X_Options Elf_X_Data::get_options()
{
    return Elf_X_Options(is64, data);
}
#endif

INLINE_DEF bool Elf_X_Data::isValid() const
{
    return data;
}

// ------------------------------------------------------------------------
// Class Elf_X_Versym simulates the SHT_GNU_versym structure.
INLINE_DEF Elf_X_Versym::Elf_X_Versym()
    : data(NULL), versym32(NULL), versym64(NULL), is64(false)
{ }

INLINE_DEF Elf_X_Versym::Elf_X_Versym(bool is64_, Elf_Data *input)
    : data(input), versym32(NULL), versym64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) versym32 = (Elf32_Half *)data->d_buf;
        else       versym64 = (Elf64_Half *)data->d_buf;
    }
}

// Read Interface
INLINE_DEF unsigned long Elf_X_Versym::get(int i) const
{
    return (!is64 ? versym32[i]
                  : versym64[i]);
}

// Meta-Info Interface
INLINE_DEF unsigned long Elf_X_Versym::count() const
{
    return (data->d_size / (!is64 ? sizeof(Elf32_Half)
                                  : sizeof(Elf64_Half) ));
}

INLINE_DEF bool Elf_X_Versym::isValid() const
{
    return (versym32 || versym64);
}

// ------------------------------------------------------------------------
// Class Elf_X_Verdaux simulates the Elf(32|64)_Verdaux structure.
INLINE_DEF Elf_X_Verdaux::Elf_X_Verdaux()
    : data(NULL), verdaux32(NULL), verdaux64(NULL), is64(false)
{ }

INLINE_DEF Elf_X_Verdaux::Elf_X_Verdaux(bool is64_, void *input)
    : data(input), verdaux32(NULL), verdaux64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) verdaux32 = (Elf32_Verdaux *)data;
        else       verdaux64 = (Elf64_Verdaux *)data;
    }
}

// Read Interface
INLINE_DEF unsigned long Elf_X_Verdaux::vda_name() const
{
    return (!is64 ? verdaux32->vda_name
                  : verdaux64->vda_name);
}

INLINE_DEF unsigned long Elf_X_Verdaux::vda_next() const
{
    return (!is64 ? verdaux32->vda_next
                  : verdaux64->vda_next);
}

INLINE_DEF Elf_X_Verdaux *Elf_X_Verdaux::get_next() const
{
    if (vda_next() == 0)
        return NULL;
    return new Elf_X_Verdaux(is64, (char *)data+vda_next());
}

// Meta-Info Interface
INLINE_DEF bool Elf_X_Verdaux::isValid() const
{
    return (verdaux32 || verdaux64);
}

// ------------------------------------------------------------------------
// Class Elf_X_Verdef simulates the Elf(32|64)_Verdef structure.
INLINE_DEF Elf_X_Verdef::Elf_X_Verdef()
    : data(NULL), verdef32(NULL), verdef64(NULL), is64(false)
{ }

INLINE_DEF Elf_X_Verdef::Elf_X_Verdef(bool is64_, void *input)
    : data(input), verdef32(NULL), verdef64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) verdef32 = (Elf32_Verdef *)data;
        else       verdef64 = (Elf64_Verdef *)data;
    }
}

// Read Interface
INLINE_DEF unsigned long Elf_X_Verdef::vd_version() const
{
    return (!is64 ? verdef32->vd_version
                  : verdef64->vd_version);
}

INLINE_DEF unsigned long Elf_X_Verdef::vd_flags() const
{
    return (!is64 ? verdef32->vd_flags
                  : verdef64->vd_flags);
}

INLINE_DEF unsigned long Elf_X_Verdef::vd_ndx() const
{
    return (!is64 ? verdef32->vd_ndx
                  : verdef64->vd_ndx);
}

INLINE_DEF unsigned long Elf_X_Verdef::vd_cnt() const
{
    return (!is64 ? verdef32->vd_cnt
                  : verdef64->vd_cnt);
}

INLINE_DEF unsigned long Elf_X_Verdef::vd_hash() const
{
    return (!is64 ? verdef32->vd_hash
                  : verdef64->vd_hash);
}

INLINE_DEF unsigned long Elf_X_Verdef::vd_aux() const
{
    return (!is64 ? verdef32->vd_aux
                  : verdef64->vd_aux);
}

INLINE_DEF unsigned long Elf_X_Verdef::vd_next() const
{
    return (!is64 ? verdef32->vd_next
                  : verdef64->vd_next);
}

INLINE_DEF Elf_X_Verdaux *Elf_X_Verdef::get_aux() const
{
    if (vd_cnt() == 0)
        return NULL;
    return new Elf_X_Verdaux(is64, (char *)data+vd_aux());
}

INLINE_DEF Elf_X_Verdef *Elf_X_Verdef::get_next() const
{
    if (vd_next() == 0)
        return NULL;
    return new Elf_X_Verdef(is64, (char *)data+vd_next());
}

// Meta-Info Interface
INLINE_DEF bool Elf_X_Verdef::isValid() const
{
    return (verdef32 || verdef64);
}

// ------------------------------------------------------------------------
// Class Elf_X_Vernaux simulates the Elf(32|64)_Vernaux structure.
INLINE_DEF Elf_X_Vernaux::Elf_X_Vernaux()
    : data(NULL), vernaux32(NULL), vernaux64(NULL), is64(false)
{ }

INLINE_DEF Elf_X_Vernaux::Elf_X_Vernaux(bool is64_, void *input)
    : data(input), vernaux32(NULL), vernaux64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) vernaux32 = (Elf32_Vernaux *)data;
        else       vernaux64 = (Elf64_Vernaux *)data;
    }
}

// Read Interface
INLINE_DEF unsigned long Elf_X_Vernaux::vna_hash() const
{
    return (!is64 ? vernaux32->vna_hash
                  : vernaux64->vna_hash);
}

INLINE_DEF unsigned long Elf_X_Vernaux::vna_flags() const
{
    return (!is64 ? vernaux32->vna_flags
                  : vernaux64->vna_flags);
}

INLINE_DEF unsigned long Elf_X_Vernaux::vna_other() const
{
    return (!is64 ? vernaux32->vna_other
                  : vernaux64->vna_other);
}

INLINE_DEF unsigned long Elf_X_Vernaux::vna_name() const
{
    return (!is64 ? vernaux32->vna_name
                  : vernaux64->vna_name);
}

INLINE_DEF unsigned long Elf_X_Vernaux::vna_next() const
{
    return (!is64 ? vernaux32->vna_next
                  : vernaux64->vna_next);
}

INLINE_DEF Elf_X_Vernaux *Elf_X_Vernaux::get_next() const
{
    if (vna_next() == 0)
        return NULL;
    return new Elf_X_Vernaux(is64, (char *)data+vna_next());
}

// Meta-Info Interface
INLINE_DEF bool Elf_X_Vernaux::isValid() const
{
    return (vernaux32 || vernaux64);
}

// ------------------------------------------------------------------------
// Class Elf_X_Verneed simulates the Elf(32|64)_Verneed structure.
INLINE_DEF Elf_X_Verneed::Elf_X_Verneed()
    : data(NULL), verneed32(NULL), verneed64(NULL), is64(false)
{ }

INLINE_DEF Elf_X_Verneed::Elf_X_Verneed(bool is64_, void *input)
    : data(input), verneed32(NULL), verneed64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) verneed32 = (Elf32_Verneed *)data;
        else       verneed64 = (Elf64_Verneed *)data;
    }
}

// Read Interface
INLINE_DEF unsigned long Elf_X_Verneed::vn_version() const
{
    return (!is64 ? verneed32->vn_version
                  : verneed64->vn_version);
}

INLINE_DEF unsigned long Elf_X_Verneed::vn_cnt() const
{
    return (!is64 ? verneed32->vn_cnt
            : verneed64->vn_cnt);
}

INLINE_DEF unsigned long Elf_X_Verneed::vn_file() const
{
    return (!is64 ? verneed32->vn_file
                  : verneed64->vn_file);
}

INLINE_DEF unsigned long Elf_X_Verneed::vn_aux() const
{
    return (!is64 ? verneed32->vn_aux
                  : verneed64->vn_aux);
}

INLINE_DEF unsigned long Elf_X_Verneed::vn_next() const
{
    return (!is64 ? verneed32->vn_next
            : verneed64->vn_next);
}

INLINE_DEF Elf_X_Vernaux *Elf_X_Verneed::get_aux() const
{
    if (vn_cnt() == 0)
        return NULL;
    return new Elf_X_Vernaux(is64, (char *)data+vn_aux());
}

INLINE_DEF Elf_X_Verneed *Elf_X_Verneed::get_next() const
{
    if (vn_next() == 0)
        return NULL;
    return new Elf_X_Verneed(is64, (char *)data+vn_next());
}

// Meta-Info Interface
INLINE_DEF bool Elf_X_Verneed::isValid() const
{
    return (verneed32 || verneed64);
}


// ------------------------------------------------------------------------
// Class Elf_X_Sym simulates the Elf(32|64)_Sym structure.
INLINE_DEF Elf_X_Sym::Elf_X_Sym()
    : data(NULL), sym32(NULL), sym64(NULL), is64(false)
{ }

INLINE_DEF Elf_X_Sym::Elf_X_Sym(bool is64_, Elf_Data *input)
    : data(input), sym32(NULL), sym64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) sym32 = (Elf32_Sym *)data->d_buf;
        else       sym64 = (Elf64_Sym *)data->d_buf;
    }
}

// Read Interface
INLINE_DEF unsigned long Elf_X_Sym::st_name(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(sym32[i].st_name) :
            static_cast<unsigned long>(sym64[i].st_name));
}

INLINE_DEF unsigned long Elf_X_Sym::st_value(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(sym32[i].st_value) :
            static_cast<unsigned long>(sym64[i].st_value));
}

INLINE_DEF unsigned long Elf_X_Sym::st_size(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(sym32[i].st_size) :
            static_cast<unsigned long>(sym64[i].st_size));
}

INLINE_DEF unsigned char Elf_X_Sym::st_info(int i) const
{
    return (!is64 ?
            sym32[i].st_info :
            sym64[i].st_info);
}

INLINE_DEF unsigned char Elf_X_Sym::st_other(int i) const
{
    return (!is64 ?
            sym32[i].st_other :
            sym64[i].st_other);
}

INLINE_DEF unsigned short Elf_X_Sym::st_shndx(int i) const
{
    return (!is64 ?
            sym32[i].st_shndx :
            sym64[i].st_shndx);
}

INLINE_DEF unsigned char Elf_X_Sym::ST_BIND(int i) const
{
    return (!is64 ?
            static_cast<unsigned char>(ELF32_ST_BIND(sym32[i].st_info)) :
            static_cast<unsigned char>(ELF64_ST_BIND(sym64[i].st_info)));
}

INLINE_DEF unsigned char Elf_X_Sym::ST_TYPE(int i) const
{
    return (!is64 ?
            static_cast<unsigned char>(ELF32_ST_TYPE(sym32[i].st_info)) :
            static_cast<unsigned char>(ELF64_ST_TYPE(sym64[i].st_info)));
}

INLINE_DEF unsigned char Elf_X_Sym::ST_VISIBILITY(int i) const
{
    return (!is64 ?
            static_cast<unsigned char>(ELF32_ST_VISIBILITY(sym32[i].st_other)) :
            static_cast<unsigned char>(ELF64_ST_VISIBILITY(sym64[i].st_other)));
}

INLINE_DEF void *Elf_X_Sym::st_symptr(int i) const
{
    return (!is64 ?
            (void *)(sym32 + i) :
            (void *)(sym64 + i));
}

INLINE_DEF unsigned Elf_X_Sym::st_entsize() const
{
    return (is64 ?
            sizeof(Elf64_Sym) :
            sizeof(Elf32_Sym));
}

// Write Interface
INLINE_DEF void Elf_X_Sym::st_name(int i, unsigned long input)
{
    if (!is64) sym32[i].st_name = input;
    else       sym64[i].st_name = input;
}

INLINE_DEF void Elf_X_Sym::st_value(int i, unsigned long input)
{
    if (!is64) sym32[i].st_value = input;
    else       sym64[i].st_value = input;
}

INLINE_DEF void Elf_X_Sym::st_size(int i, unsigned long input)
{
    if (!is64) sym32[i].st_size = input;
    else       sym64[i].st_size = input;
}

INLINE_DEF void Elf_X_Sym::st_info(int i, unsigned char input)
{
    if (!is64) sym32[i].st_info = input;
    else       sym64[i].st_info = input;
}

INLINE_DEF void Elf_X_Sym::st_other(int i, unsigned char input)
{
    if (!is64) sym32[i].st_other = input;
    else       sym64[i].st_other = input;
}

INLINE_DEF void Elf_X_Sym::st_shndx(int i, unsigned short input)
{
    if (!is64) sym32[i].st_shndx = input;
    else       sym64[i].st_shndx = input;
}

// Meta-Info Interface
INLINE_DEF unsigned long Elf_X_Sym::count() const
{
    return (data->d_size / (!is64 ? sizeof(Elf32_Sym)
                                  : sizeof(Elf64_Sym)));
}

INLINE_DEF bool Elf_X_Sym::isValid() const
{
    return sym32 || sym64;
}

// ------------------------------------------------------------------------
// Class Elf_X_Rel simulates the Elf(32|64)_Rel structure.
INLINE_DEF Elf_X_Rel::Elf_X_Rel()
    : data(NULL), rel32(NULL), rel64(NULL), is64(false)
{ }

INLINE_DEF Elf_X_Rel::Elf_X_Rel(bool is64_, Elf_Data *input)
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
INLINE_DEF unsigned long Elf_X_Rel::r_offset(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(rel32[i].r_offset) :
            static_cast<unsigned long>(rel64[i].r_offset));
}

INLINE_DEF unsigned long Elf_X_Rel::r_info(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(rel32[i].r_info) :
            static_cast<unsigned long>(rel64[i].r_info));
}

INLINE_DEF unsigned long Elf_X_Rel::R_SYM(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(ELF32_R_SYM(rel32[i].r_info)) :
            static_cast<unsigned long>(ELF64_R_SYM(rel64[i].r_info)));
}

INLINE_DEF unsigned long Elf_X_Rel::R_TYPE(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(ELF32_R_TYPE(rel32[i].r_info)) :
            static_cast<unsigned long>(ELF64_R_TYPE(rel64[i].r_info)));
};

// Write Interface
INLINE_DEF void Elf_X_Rel::r_offset(int i, unsigned long input)
{
    if (!is64)
        rel32[i].r_offset = input;
    else
        rel64[i].r_offset = input;
}

INLINE_DEF void Elf_X_Rel::r_info(int i, unsigned long input)
{
    if (!is64)
        rel32[i].r_info = input;
    else
        rel64[i].r_info = input;
}

// Meta-Info Interface
INLINE_DEF unsigned long Elf_X_Rel::count() const
{
    return (data->d_size / (!is64 ? sizeof(Elf32_Rel)
                                  : sizeof(Elf64_Rel)) );
}

INLINE_DEF bool Elf_X_Rel::isValid() const
{
    return (rel32 || rel64);
}

// ------------------------------------------------------------------------
// Class Elf_X_Rela simulates the Elf(32|64)_Rela structure.
INLINE_DEF Elf_X_Rela::Elf_X_Rela()
    : data(NULL), rela32(NULL), rela64(NULL), is64(false)
{ }

INLINE_DEF Elf_X_Rela::Elf_X_Rela(bool is64_, Elf_Data *input)
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
INLINE_DEF unsigned long Elf_X_Rela::r_offset(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(rela32[i].r_offset) :
            static_cast<unsigned long>(rela64[i].r_offset));
}

INLINE_DEF unsigned long Elf_X_Rela::r_info(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(rela32[i].r_info) :
            static_cast<unsigned long>(rela64[i].r_info));
}

INLINE_DEF signed long Elf_X_Rela::r_addend(int i) const
{
    return (!is64 ?
            static_cast<signed long>(rela32[i].r_addend) :
            static_cast<signed long>(rela64[i].r_addend));
}

INLINE_DEF unsigned long Elf_X_Rela::R_SYM(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(ELF32_R_SYM(rela32[i].r_info)) :
            static_cast<unsigned long>(ELF64_R_SYM(rela64[i].r_info)));
}

INLINE_DEF unsigned long Elf_X_Rela::R_TYPE(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(ELF32_R_TYPE(rela32[i].r_info)) :
            static_cast<unsigned long>(ELF64_R_TYPE(rela64[i].r_info)));
}

// Write Interface
INLINE_DEF void Elf_X_Rela::r_offset(int i, unsigned long input)
{
    if (!is64)
        rela32[i].r_offset = input;
    else
        rela64[i].r_offset = input;
}

INLINE_DEF void Elf_X_Rela::r_info(int i, unsigned long input)
{
    if (!is64)
        rela32[i].r_info = input;
    else
        rela64[i].r_info = input;
}

INLINE_DEF void Elf_X_Rela::r_addend(int i, signed long input)
{
    if (!is64)
        rela32[i].r_addend = input;
    else
        rela64[i].r_addend = input;
}

// Meta-Info Interface
INLINE_DEF unsigned long Elf_X_Rela::count() const
{
    return (data->d_size / (!is64 ? sizeof(Elf32_Rela)
                                  : sizeof(Elf64_Rela)));
}

INLINE_DEF bool Elf_X_Rela::isValid() const
{
    return (rela32 || rela64);
}


// ------------------------------------------------------------------------
// Class Elf_X_Dyn simulates the Elf(32|64)_Dyn structure.
INLINE_DEF Elf_X_Dyn::Elf_X_Dyn()
    : data(NULL), dyn32(NULL), dyn64(NULL), is64(false)
{ }

INLINE_DEF Elf_X_Dyn::Elf_X_Dyn(bool is64_, Elf_Data *input)
    : data(input), dyn32(NULL), dyn64(NULL), is64(is64_)
{
    if (input) {
        if (!is64) dyn32 = (Elf32_Dyn *)data->d_buf;
        else       dyn64 = (Elf64_Dyn *)data->d_buf;
    }
}

// Read Interface
INLINE_DEF signed long Elf_X_Dyn::d_tag(int i) const
{ 
    return (!is64 ?
            static_cast<signed long>(dyn32[i].d_tag) :
            static_cast<signed long>(dyn64[i].d_tag));
}

INLINE_DEF unsigned long Elf_X_Dyn::d_val(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(dyn32[i].d_un.d_val) :
            static_cast<unsigned long>(dyn64[i].d_un.d_val));
}

INLINE_DEF unsigned long Elf_X_Dyn::d_ptr(int i) const
{
    return (!is64 ?
            static_cast<unsigned long>(dyn32[i].d_un.d_ptr) :
            static_cast<unsigned long>(dyn64[i].d_un.d_ptr));
}

// Write Interface
INLINE_DEF void Elf_X_Dyn::d_tag(int i, signed long input)
{
    if (!is64) dyn32[i].d_tag = input;
    else       dyn64[i].d_tag = input;
}

INLINE_DEF void Elf_X_Dyn::d_val(int i, unsigned long input)
{
    if (!is64) dyn32[i].d_un.d_val = input;
    else       dyn64[i].d_un.d_val = input;
}

INLINE_DEF void Elf_X_Dyn::d_ptr(int i, unsigned long input)
{
    if (!is64) dyn32[i].d_un.d_ptr = input;
    else       dyn64[i].d_un.d_ptr = input;
}

// Meta-Info Interface
INLINE_DEF unsigned long Elf_X_Dyn::count() const
{
    return (data->d_size / (!is64 ? sizeof(Elf32_Dyn)
                                  : sizeof(Elf64_Dyn) ));
}

INLINE_DEF bool Elf_X_Dyn::isValid() const
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

   char *buffer = (char *) mmap(NULL, fileStat.st_size, PROT_READ, MAP_SHARED, fd, 0);
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
INLINE_DEF bool Elf_X::findDebugFile(std::string origfilename, string &output_name, char* &output_buffer, unsigned long &output_buffer_size)
{
   uint16_t shnames_idx = e_shstrndx();
   Elf_X_Shdr shnames_hdr = get_shdr(shnames_idx);
   if (!shnames_hdr.isValid())
      return false;
   const char *shnames = (const char *) shnames_hdr.get_data().d_buf();
   
  string debugFileFromDebugLink, debugFileFromBuildID;
  unsigned debugFileCrc = 0;

  for(int i = 0; i < e_shnum(); i++) {
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
     else if(strcmp(name, BUILD_ID_NAME) == 0) {
        char *buildId = (char *) scn.get_data().d_buf();
        string filename = string(buildId + 2) + ".debug";
        string subdir = string(buildId, 2);
        debugFileFromBuildID = "/usr/lib/debug/.build-id/" + subdir + "/" + filename;
     }
  }

  if (!debugFileFromBuildID.empty()) {
     bool result = loadDebugFileFromDisk(debugFileFromBuildID, output_buffer, output_buffer_size);
     if (result) {
        output_name = debugFileFromBuildID;
        return true;
     }
  }

  if (debugFileFromDebugLink.empty())
     return false;

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
    return true;
  }

  return false;
}

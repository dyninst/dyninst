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

#ifndef __ELF_X_H__
#define __ELF_X_H__

#include "libelf.h"
#include <stddef.h>
#include <utility>
#include <string>
#include <map>
#include <vector>
#include "util.h"
#include "Architecture.h"

#ifndef EM_CUDA
#define EM_CUDA		190	/* NVIDIA CUDA */
#endif

#ifndef EM_INTEL_GEN9
#define EM_INTEL_GEN9		182	/* INTEL GEN9 */
#endif

#ifndef EM_INTELGT
#define EM_INTELGT		205	/* INTEL Graphics Technology */
#endif

namespace Dyninst {

// Forward declarations
class Elf_X;
class Elf_X_Phdr;
class Elf_X_Shdr;
class Elf_X_Data;
class Elf_X_Versym;
class Elf_X_Verdaux;
class Elf_X_Verdef;
class Elf_X_Vernaux;
class Elf_X_Verneed;
class Elf_X_Options;
class Elf_X_Sym;
class Elf_X_Rel;
class Elf_X_Rela;
class Elf_X_RegInfo;
class Elf_32_RegInfo;
class Elf_64_RegInfo;
class Elf_X_Dyn;
class Elf_X_Nhdr;


// Wrappers to allow word-independant use of libelf routines.

// ------------------------------------------------------------------------
// Class Elf_X simulates the Elf(32|64)_Ehdr structure.
// Also works for ELF archives. 
class DYNELF_EXPORT Elf_X {
  public:
    static Elf_X *newElf_X(int input, Elf_Cmd cmd, Elf_X *ref = NULL, std::string name = std::string());
    static Elf_X *newElf_X(char *mem_image, size_t mem_size, std::string name = std::string());
    void end();

    // Read Interface
    Elf *e_elfp() const;
    unsigned char *e_ident() const;
    unsigned short e_type() const;
    unsigned short e_machine() const;
    unsigned long e_version() const;
    unsigned long e_entry() const;
    unsigned long e_phoff() const;
    unsigned long e_shoff() const;
    unsigned long e_flags() const;
    unsigned short e_ehsize() const;
    unsigned short e_phentsize() const;
    unsigned long e_phnum();
    unsigned short e_shentsize() const;
    unsigned long e_shnum();
    unsigned long e_shstrndx();
    const char *e_rawfile(size_t &nbytes) const;
    unsigned short e_endian() const;

    Elf_X *e_next(Elf_X *ref);
    Elf_X *e_rand(unsigned offset);

    // Write Interface
    void e_ident(unsigned char *input);
    void e_type(unsigned short input);
    void e_machine(unsigned short input);
    void e_version(unsigned long input);
    void e_entry(unsigned long input);
    void e_phoff(unsigned long input);
    void e_shoff(unsigned long input);
    void e_flags(unsigned long input);
    void e_ehsize(unsigned short input);
    void e_phentsize(unsigned short input);
    void e_phnum(unsigned short input);
    void e_shentsize(unsigned short input);
    void e_shnum(unsigned short input);
    void e_shstrndx(unsigned short input);
    void e_endian(unsigned short input);
    // Data Interface
    bool isValid() const;
    int wordSize() const;
    Elf_X_Phdr &get_phdr(unsigned int i = 0);
    Elf_X_Shdr &get_shdr(unsigned int i);

    bool findDebugFile(std::string origfilename, std::string &output_name, char* &output_buffer, unsigned long &output_buffer_size);

    Dyninst::Architecture getArch() const;

  protected:
    Elf *elf{};
    Elf32_Ehdr *ehdr32{};
    Elf64_Ehdr *ehdr64{};
    Elf32_Phdr *phdr32{};
    Elf64_Phdr *phdr64{};
    int filedes{};
    bool is64{};
    bool isArchive{};
    bool isBigEndian{};
    std::vector<Elf_X_Shdr> shdrs;
    std::vector<Elf_X_Phdr> phdrs;
    unsigned int ref_count{};
    std::string filename;

    char *cached_debug_buffer{};
    unsigned long cached_debug_size{};
    std::string cached_debug_name;
    bool cached_debug{};

    Elf_X();
    Elf_X(int input, Elf_Cmd cmd, Elf_X *ref = NULL);
    Elf_X(char *mem_image, size_t mem_size);
    ~Elf_X();

    // Two maps:
    // One name/FD for Elf_Xs created that way
    // One name/baseaddr

    static std::map<std::pair<std::string, int >, Elf_X *> elf_x_by_fd;
    static std::map<std::pair<std::string, char *>, Elf_X *> elf_x_by_ptr;

};

// ------------------------------------------------------------------------
// Class Elf_X_Phdr simulates the Elf(32|64)_Phdr structure.
class DYNELF_EXPORT Elf_X_Phdr {
   friend class Elf_X;
  public:
    Elf_X_Phdr();
    Elf_X_Phdr(bool is64_, void *input);

    // Read Interface
    unsigned long p_type() const;
    unsigned long p_offset() const;
    unsigned long p_vaddr() const;
    unsigned long p_paddr() const;
    unsigned long p_filesz() const;
    unsigned long p_memsz() const;
    unsigned long p_flags() const;
    unsigned long p_align() const;

    // Write Interface
    void p_type(unsigned long input);
    void p_offset(unsigned long input);
    void p_vaddr(unsigned long input);
    void p_paddr(unsigned long input);
    void p_filesz(unsigned long input);
    void p_memsz(unsigned long input);
    void p_flags(unsigned long input);
    void p_align(unsigned long input);

    bool isValid() const;

  private:
    Elf32_Phdr *phdr32;
    Elf64_Phdr *phdr64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Shdr simulates the Elf(32|64)_Shdr structure.
class DYNELF_EXPORT Elf_X_Shdr {
    friend class Elf_X;

  public:
    Elf_X_Shdr();
    Elf_X_Shdr(bool is64_, Elf_Scn *input);

    // Read Interface
    unsigned long sh_name() const;
    unsigned long sh_type() const;
    unsigned long sh_flags() const;
    unsigned long sh_addr() const;
    unsigned long sh_offset() const;
    unsigned long sh_size() const;
    unsigned long sh_link() const;
    unsigned long sh_info() const;
    unsigned long sh_addralign() const;
    unsigned long sh_entsize() const;
    bool isFromDebugFile() const;

    // Write Interface
    void sh_name(unsigned long input);
    void sh_type(unsigned long input);
    void sh_flags(unsigned long input);
    void sh_addr(unsigned long input);
    void sh_offset(unsigned long input);
    void sh_size(unsigned long input);
    void sh_link(unsigned long input);
    void sh_info(unsigned long input);
    void sh_addralign(unsigned long input);
    void sh_entsize(unsigned long input);
    void setDebugFile(bool b);

    // Section Data Interface
    Elf_X_Data get_data() const;

    // For Sections with Multiple Data Sections
    void first_data();
    bool next_data();

    bool isValid() const;
    unsigned wordSize() const;
    Elf_Scn *getScn() const;

    Elf_X_Nhdr get_note() const;

  protected:
    Elf_Scn *scn;
    Elf_Data *data;
    Elf32_Shdr *shdr32;
    Elf64_Shdr *shdr64;
    bool is64;
    bool fromDebugFile;
    const Elf_X *_elf;
};

// ------------------------------------------------------------------------
// Class Elf_X_Data simulates the Elf_Data structure.
class DYNELF_EXPORT Elf_X_Data {
  public:
    Elf_X_Data();
    Elf_X_Data(bool is64_, Elf_Data *input);

    // Read Interface
    void *d_buf() const;
    Elf_Data * elf_data() const;
    Elf_Type d_type() const;
    unsigned int d_version() const;
    size_t d_size() const;
    off_t d_off() const;
    size_t d_align() const;
    void xlatetom(unsigned int encode);
    void xlatetof(unsigned int encode);

    // Write Interface
    void d_buf(void *input);
    void d_type(Elf_Type input);
    void d_version(unsigned int input);
    void d_size(unsigned int input);
    void d_off(signed int input);
    void d_align(unsigned int input);

    // Data Interface
    const char *get_string() const;
    Elf_X_Dyn get_dyn();
    Elf_X_Versym get_versyms();
    Elf_X_Verneed *get_verNeedSym();
    Elf_X_Verdef *get_verDefSym();

    Elf_X_Rel get_rel();
    Elf_X_Rela get_rela();
    Elf_X_Sym get_sym();

    bool isValid() const;

  protected:
    Elf_Data *data;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Versym simulates the SHT_GNU_versym structure.
class DYNELF_EXPORT Elf_X_Versym {
  public:
    Elf_X_Versym();
    Elf_X_Versym(bool is64_, Elf_Data *input);

    // Read Interface
    unsigned long get(int i) const;

    // Meta-Info Interface
    unsigned long count() const;
    bool isValid() const;

  protected:
    Elf_Data *data;
    Elf32_Half *versym32;
    Elf64_Half *versym64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Verdaux simulates the Elf(32|64)_Verdaux structure.
class DYNELF_EXPORT Elf_X_Verdaux {
  public:
    Elf_X_Verdaux();
    Elf_X_Verdaux(bool is64_, void *input);

    // Read Interface
    unsigned long vda_name() const;
    unsigned long vda_next() const;
    Elf_X_Verdaux *get_next() const;

    // Meta-Info Interface
    bool isValid() const;

  protected:
    void *data;
    Elf32_Verdaux *verdaux32;
    Elf64_Verdaux *verdaux64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Verdef simulates the Elf(32|64)_Verdef structure.
class DYNELF_EXPORT Elf_X_Verdef {
  public:
    Elf_X_Verdef();
    Elf_X_Verdef(bool is64_, void *input);

    // Read Interface
    unsigned long vd_version() const;
    unsigned long vd_flags() const;
    unsigned long vd_ndx() const;
    unsigned long vd_cnt() const;
    unsigned long vd_hash() const;
    unsigned long vd_aux() const;
    unsigned long vd_next() const;
    Elf_X_Verdaux *get_aux() const;
    Elf_X_Verdef *get_next() const;

    // Meta-Info Interface
    bool isValid() const;
   
  protected:
    void *data;
    Elf32_Verdef *verdef32;
    Elf64_Verdef *verdef64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Vernaux simulates the Elf(32|64)_Vernaux structure.
class DYNELF_EXPORT Elf_X_Vernaux {
  public:
    Elf_X_Vernaux();
    Elf_X_Vernaux(bool is64_, void *input);

    // Read Interface
    unsigned long vna_hash() const;
    unsigned long vna_flags() const;
    unsigned long vna_other() const;
    unsigned long vna_name() const;
    unsigned long vna_next() const;
    Elf_X_Vernaux *get_next() const;

    // Meta-Info Interface
    bool isValid() const;

  protected:
    void *data;
    Elf32_Vernaux *vernaux32;
    Elf64_Vernaux *vernaux64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Verneed simulates the Elf(32|64)_Verneed structure.
class DYNELF_EXPORT Elf_X_Verneed {
  public:
    Elf_X_Verneed();
    Elf_X_Verneed(bool is64_, void *input);

    // Read Interface
    unsigned long vn_version() const;
    unsigned long vn_cnt() const;
    unsigned long vn_file() const;
    unsigned long vn_aux() const;
    unsigned long vn_next() const;
    Elf_X_Vernaux *get_aux() const;
    Elf_X_Verneed *get_next() const;

    // Meta-Info Interface
    bool isValid() const;

  protected:
    void *data;
    Elf32_Verneed *verneed32;
    Elf64_Verneed *verneed64;
    bool is64;
};


// ------------------------------------------------------------------------
// Class Elf_X_Sym simulates the Elf(32|64)_Sym structure.
class DYNELF_EXPORT Elf_X_Sym {
  public:
    Elf_X_Sym();
    Elf_X_Sym(bool is64_, Elf_Data *input);

    // Read Interface
    unsigned long st_name(int i) const;
    unsigned long st_value(int i) const;
    unsigned long st_size(int i) const;
    unsigned char st_info(int i) const;
    unsigned char st_other(int i) const;
    unsigned short st_shndx(int i) const;
    unsigned char ST_BIND(int i) const;
    unsigned char ST_TYPE(int i) const;
    unsigned char ST_VISIBILITY(int i) const;
    void *st_symptr(int i) const;
    unsigned st_entsize() const;

    // Write Interface
    void st_name(int i, unsigned long input);
    void st_value(int i, unsigned long input);
    void st_size(int i, unsigned long input);
    void st_info(int i, unsigned char input);
    void st_other(int i, unsigned char input);
    void st_shndx(int i, unsigned short input);

    // Meta-Info Interface
    unsigned long count() const;
    bool isValid() const;

  protected:
    Elf_Data *data;
    Elf32_Sym *sym32;
    Elf64_Sym *sym64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Rel simulates the Elf(32|64)_Rel structure.
class DYNELF_EXPORT Elf_X_Rel {
  public:
   Elf_X_Rel();
   Elf_X_Rel(bool is64_, Elf_Data *input);

    // Read Interface
    unsigned long r_offset(int i) const;
    unsigned long r_info(int i) const;
    unsigned long R_SYM(int i) const;
    unsigned long R_TYPE(int i) const;

    // Write Interface
    void r_offset(int i, unsigned long input);
    void r_info(int i, unsigned long input);

    // Meta-Info Interface
    unsigned long count() const;
    bool isValid() const;

  protected:
    Elf_Data *data;
    Elf32_Rel *rel32;
    Elf64_Rel *rel64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Rela simulates the Elf(32|64)_Rela structure.
class DYNELF_EXPORT Elf_X_Rela {
  public:
    Elf_X_Rela();
    Elf_X_Rela(bool is64_, Elf_Data *input);

    // Read Interface
    unsigned long r_offset(int i) const;
    unsigned long r_info(int i) const;
    signed   long r_addend(int i) const;
    unsigned long R_SYM(int i) const;
    unsigned long R_TYPE(int i) const;

    // Write Interface
    void r_offset(int i, unsigned long input);
    void r_info(int i, unsigned long input);
    void r_addend(int i, signed long input);

    // Meta-Info Interface
    unsigned long count() const;
    bool isValid() const;

  protected:
    Elf_Data *data;
    Elf32_Rela *rela32;
    Elf64_Rela *rela64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Dyn simulates the Elf(32|64)_Dyn structure.
class DYNELF_EXPORT Elf_X_Dyn {
  public:
    Elf_X_Dyn();
    Elf_X_Dyn(bool is64_, Elf_Data *input);

    // Read Interface
    signed long d_tag(int i) const;
    unsigned long d_val(int i) const;
    unsigned long d_ptr(int i) const;

    // Write Interface
    void d_tag(int i, signed long input);
    void d_val(int i, unsigned long input);
    void d_ptr(int i, unsigned long input);

    // Meta-Info Interface
    unsigned long count() const;
    bool isValid() const;

  protected:
    Elf_Data *data;
    Elf32_Dyn *dyn32;
    Elf64_Dyn *dyn64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Nhdr simulates the Elf(32|64)_Shdr structure.
class DYNELF_EXPORT Elf_X_Nhdr {
    friend class Elf_X;

  public:
    Elf_X_Nhdr();
    Elf_X_Nhdr(Elf_Data *data_, size_t offset);

    // Read Interface
    unsigned long n_namesz() const;
    unsigned long n_descsz() const;
    unsigned long n_type() const;

    // Meta-Info Interface
    bool isValid() const;

    const char* get_name() const;
    const void* get_desc() const;

    Elf_X_Nhdr next() const;

  protected:
    Elf_Data *data;
    Elf32_Nhdr *nhdr;
};

}

#endif

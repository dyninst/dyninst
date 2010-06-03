/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#ifndef __ELF_X_H__
#define __ELF_X_H__

#include <elf.h>
#include <libelf.h>
#include "common/h/headers.h"

// Wrappers to allow word-independant use of libelf routines.

#if !defined(os_solaris)
// ------------------------------------------------------------------------
// Class Elf_X_Dyn simulates the Elf(32|64)_Dyn structure.
class Elf_X_Dyn {
 public:
   Elf_X_Dyn()
      : data(NULL), dyn32(NULL), dyn64(NULL), is64(false) { }
   
   Elf_X_Dyn(bool is64_, Elf_Data *input)
      : data(input), dyn32(NULL), dyn64(NULL), is64(is64_) {
      
      if (input) {
         if (!is64) dyn32 = (Elf32_Dyn *)data->d_buf;
         else       dyn64 = (Elf64_Dyn *)data->d_buf;
      }
   }

   // Read Interface
      signed long d_tag(int i) const { 
         return (!is64 ? 
                 static_cast<signed long>(dyn32[i].d_tag) :
                 static_cast<signed long>(dyn64[i].d_tag)); 
      }
      unsigned long d_val(int i) const { 
         return (!is64 ? 
                 static_cast<unsigned long>(dyn32[i].d_un.d_val) :
                 static_cast<unsigned long>(dyn64[i].d_un.d_val)); 
      }
      unsigned long d_ptr(int i) const { 
         return (!is64 ? 
                 static_cast<unsigned long>(dyn32[i].d_un.d_ptr) :
                 static_cast<unsigned long>(dyn64[i].d_un.d_ptr)); 
      }
   
   // Write Interface
   void d_tag(int i, signed long input) { if (!is64) dyn32[i].d_tag = input;
   else       dyn64[i].d_tag = input; }
   void d_val(int i, unsigned long input) { if (!is64) dyn32[i].d_un.d_val = input;
   else       dyn64[i].d_un.d_val = input; }
   void d_ptr(int i, unsigned long input) { if (!is64) dyn32[i].d_un.d_ptr = input;
   else       dyn64[i].d_un.d_ptr = input; }
   
   // Meta-Info Interface
   unsigned long count() const { return (data->d_size / (!is64 ? sizeof(Elf32_Dyn)
                                                         : sizeof(Elf64_Dyn) )); }
   bool isValid() const { return (dyn32 || dyn64); }
   
 protected:
   Elf_Data *data;
   Elf32_Dyn *dyn32;
   Elf64_Dyn *dyn64;
   bool is64;
};
#endif

#if defined(arch_mips)
// ------------------------------------------------------------------------
// Class Elf_X_RegInfo simulates the Elf(32|64)_RegInfo structure.
class Elf_X_RegInfo {
  public:
    unsigned long ri_gprmask() const = 0;
    unsigned long ri_gprmask(unsigned long input) = 0;
    unsigned long ri_cprmask(int i) const = 0;
    unsigned long ri_cprmask(int i, unsigned long input) = 0;
    unsigned long ri_gp_value() const = 0;
    unsigned long ri_gp_value(unsigned long input) = 0;
};

class Elf_32_RegInfo : public Elf_X_RegInfo {
  public:
    Elf_32_RegInfo(Elf_Data *input) : reg((Elf32_RegInfo *)input.d_buf) { }

    // Read Interface
    unsigned long ri_gprmask() const { return reg->ri_gprmask; }
    unsigned long ri_cprmask(int i) const { return reg->ri_cprmask[i]; }
    unsigned long ri_gp_value() const { return reg->ri_gp_value; }

    // Write Interface
    unsigned long ri_gprmask(unsigned long input) { return (reg->ri_gprmask = input); }
    unsigned long ri_cprmask(int i, unsigned long input) { return (reg->ri_cprmask[i] = input); }
    unsigned long ri_gp_value(unsigned long input) { return (reg->ri_gp_value = input); }

  private:
    Elf32_RegInfo *reg;
};

class Elf_64_RegInfo : public Elf_X_RegInfo {
  public:
    Elf_64_RegInfo(Elf_Data *input) : reg((Elf64_RegInfo *)input.d_buf) { }

    // Read Interface
    unsigned long ri_gprmask() const { return reg->ri_gprmask; }
    unsigned long ri_cprmask(int i) const { return reg->ri_cprmask[i]; }
    unsigned long ri_gp_value() const { return reg->ri_gp_value; }

    // Write Interface
    unsigned long ri_gprmask(unsigned long input) { return (reg->ri_gprmask = input); }
    unsigned long ri_cprmask(int i, unsigned long input) { return (reg->ri_cprmask[i] = input); }
    unsigned long ri_gp_value(unsigned long input) { return (reg->ri_gp_value = input); }

  private:
    Elf64_RegInfo *reg;
};
#endif

// ------------------------------------------------------------------------
// Class Elf_X_Rel simulates the Elf(32|64)_Rel structure.
class Elf_X_Rel {
  public:
   Elf_X_Rel() : 
      data(NULL), 
      rel32(NULL), 
      rel64(NULL), 
      is64(false) 
   { }

   Elf_X_Rel(bool is64_, Elf_Data *input)	: 
      data(input), 
      rel32(NULL), 
      rel64(NULL), 
      is64(is64_) 
   {
      if (input) {
         if (!is64) 
            rel32 = (Elf32_Rel *)data->d_buf;
         else       
            rel64 = (Elf64_Rel *)data->d_buf;
      }
   }

    // Read Interface
    unsigned long r_offset(int i) const { 
       return (!is64 ? 
               static_cast<unsigned long>(rel32[i].r_offset) :
               static_cast<unsigned long>(rel64[i].r_offset)); 
    }
    unsigned long r_info(int i) const { 
       return (!is64 ? 
               static_cast<unsigned long>(rel32[i].r_info) :
               static_cast<unsigned long>(rel64[i].r_info)); 
    }
    unsigned long R_SYM(int i) const { 
       return (!is64 ?
               static_cast<unsigned long>(ELF32_R_SYM(rel32[i].r_info)) :
               static_cast<unsigned long>(ELF64_R_SYM(rel64[i].r_info))); 
    }
    unsigned long R_TYPE(int i) const { 
       return (!is64 ? 
               static_cast<unsigned long>(ELF32_R_TYPE(rel32[i].r_info)) :
               static_cast<unsigned long>(ELF64_R_TYPE(rel64[i].r_info))); 
    };

    // Write Interface
    void r_offset(int i, unsigned long input) { 
       if (!is64) 
          rel32[i].r_offset = input;
       else       
          rel64[i].r_offset = input; 
    }

    void r_info(int i, unsigned long input) { 
       if (!is64) 
          rel32[i].r_info = input;
       else       
          rel64[i].r_info = input; 
    }

    // Meta-Info Interface
    unsigned long count() const { return (data->d_size / (!is64 ? sizeof(Elf32_Rel)
								: sizeof(Elf64_Rel)) ); }
    bool isValid() const { return (rel32 || rel64); }

  protected:
    Elf_Data *data;
    Elf32_Rel *rel32;
    Elf64_Rel *rel64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Rela simulates the Elf(32|64)_Rela structure.
 class Elf_X_Rela {
 public:
   Elf_X_Rela() :
      data(NULL), 
      rela32(NULL), 
      rela64(NULL), 
      is64(false) 
      { }

   Elf_X_Rela(bool is64_, Elf_Data *input) : 
      data(input), 
      rela32(NULL), 
      rela64(NULL), 
      is64(is64_) 
   {
      if (input) {
         if (!is64) 
            rela32 = (Elf32_Rela *)data->d_buf;
         else       
            rela64 = (Elf64_Rela *)data->d_buf;
      }
   }

    // Read Interface
   unsigned long r_offset(int i) const { 
      return (!is64 ? 
              static_cast<unsigned long>(rela32[i].r_offset) :
              static_cast<unsigned long>(rela64[i].r_offset)); 
   }
   unsigned long r_info(int i) const { 
      return (!is64 ? 
              static_cast<unsigned long>(rela32[i].r_info) :
              static_cast<unsigned long>(rela64[i].r_info));
   }
   
   signed long r_addend(int i) const { 
      return (!is64 ? 
              static_cast<signed long>(rela32[i].r_addend) :
              static_cast<signed long>(rela64[i].r_addend));
   }
   unsigned long R_SYM(int i) const { 
      return (!is64 ? 
              static_cast<unsigned long>(ELF32_R_SYM(rela32[i].r_info)) :
              static_cast<unsigned long>(ELF64_R_SYM(rela64[i].r_info))); 
   }
   unsigned long R_TYPE(int i) const { 
      return (!is64 ? 
              static_cast<unsigned long>(ELF32_R_TYPE(rela32[i].r_info)) :
              static_cast<unsigned long>(ELF64_R_TYPE(rela64[i].r_info))); 
              };

    // Write Interface
    void r_offset(int i, unsigned long input) { 
       if (!is64) 
          rela32[i].r_offset = input;
       else       
          rela64[i].r_offset = input; 
    }
    void r_info(int i, unsigned long input) { 
       if (!is64) 
          rela32[i].r_info = input;
       else       
          rela64[i].r_info = input; 
    }
    void r_addend(int i, signed long input) { 
       if (!is64) 
          rela32[i].r_addend = input;
       else       
          rela64[i].r_addend = input; 
    }

    // Meta-Info Interface
    unsigned long count() const { return (data->d_size / 
                                          (!is64 ? sizeof(Elf32_Rela) :
                                           sizeof(Elf64_Rela))); 
    }
    bool isValid() const { return (rela32 || rela64); }

  protected:
    Elf_Data *data;
    Elf32_Rela *rela32;
    Elf64_Rela *rela64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Sym simulates the Elf(32|64)_Sym structure.
class Elf_X_Sym {
  public:
   Elf_X_Sym() : 
      data(NULL), 
      sym32(NULL), 
      sym64(NULL), 
      is64(false) 
   { }

   Elf_X_Sym(bool is64_, Elf_Data *input) :
      data(input), 
      sym32(NULL), 
      sym64(NULL), 
      is64(is64_) 
   {
      if (input) {
         if (!is64) sym32 = (Elf32_Sym *)data->d_buf;
         else       sym64 = (Elf64_Sym *)data->d_buf;
      }
   }

    // Read Interface
    unsigned long st_name(int i) const { 
       return (!is64 ? 
               static_cast<unsigned long>(sym32[i].st_name) :
               static_cast<unsigned long>(sym64[i].st_name));
    }
    unsigned long st_value(int i) const { 
       return (!is64 ? 
               static_cast<unsigned long>(sym32[i].st_value) :
               static_cast<unsigned long>(sym64[i].st_value)); 
    }
    unsigned long st_size(int i) const { 
       return (!is64 ? 
               static_cast<unsigned long>(sym32[i].st_size) :
               static_cast<unsigned long>(sym64[i].st_size));
    }
    unsigned char st_info(int i) const { 
       return (!is64 ? 
               sym32[i].st_info :
               sym64[i].st_info); 
    }
    unsigned char st_other(int i) const { 
       return (!is64 ? 
               sym32[i].st_other :
               sym64[i].st_other); 
    }
    unsigned short st_shndx(int i) const { 
       return (!is64 ? 
               sym32[i].st_shndx :
               sym64[i].st_shndx); 
    }
    unsigned char ST_BIND(int i) const { 
       return (!is64 ? 
               static_cast<unsigned char>(ELF32_ST_BIND(sym32[i].st_info)) :
               static_cast<unsigned char>(ELF64_ST_BIND(sym64[i].st_info))); 
    }
    unsigned char ST_TYPE(int i) const { 
       return (!is64 ? 
               static_cast<unsigned char>(ELF32_ST_TYPE(sym32[i].st_info)) :
               static_cast<unsigned char>(ELF64_ST_TYPE(sym64[i].st_info))); 
    }
    unsigned char ST_VISIBILITY(int i) const { 
       return (!is64 ? 
               static_cast<unsigned char>(ELF32_ST_VISIBILITY(sym32[i].st_other)) :
               static_cast<unsigned char>(ELF64_ST_VISIBILITY(sym64[i].st_other))); 
    }
    void *st_symptr(int i) const {
       return (!is64 ? (void *) (sym32 + i) : (void *) (sym64 + i));
    }
    unsigned st_entsize() const {
       return is64 ? sizeof(Elf64_Sym) : sizeof(Elf32_Sym);
    }

    // Write Interface
    void st_name(int i, unsigned long input) { 
       if (!is64) sym32[i].st_name = input;
       else       sym64[i].st_name = input; 
    }
    void st_value(int i, unsigned long input) { 
       if (!is64) sym32[i].st_value = input;
       else       sym64[i].st_value = input; 
    }
    void st_size(int i, unsigned long input) { 
       if (!is64) sym32[i].st_size = input;
       else       sym64[i].st_size = input; 
    }
    void st_info(int i, unsigned char input) { 
       if (!is64) sym32[i].st_info = input;
       else       sym64[i].st_info = input; 
    }
    void st_other(int i, unsigned char input) { 
       if (!is64) sym32[i].st_other = input;
       else       sym64[i].st_other = input; 
    }
    void st_shndx(int i, unsigned short input) { 
       if (!is64) sym32[i].st_shndx = input;
       else       sym64[i].st_shndx = input; 
    }

    // Meta-Info Interface
    unsigned long count() const { return (data->d_size / (!is64 ? sizeof(Elf32_Sym)
								: sizeof(Elf64_Sym))); }
    bool isValid() const { return sym32 || sym64; }

  protected:
    Elf_Data *data;
    Elf32_Sym *sym32;
    Elf64_Sym *sym64;
    bool is64;
};

#if defined(arch_mips)
// ------------------------------------------------------------------------
// Class Elf_X_Options simulates the Elf_Options structure.
class Elf_X_Options {
  public:
    Elf_X_Options(Elf_Data *input) : data(input), opt((Elf_Options *)data->d_buf) { }

    // Read Interface
    unsigned char kind(int i) const { return opt[i].kind; }
    unsigned char size(int i) const { return opt[i].size; }
    unsigned short section(int i) const { return opt[i].section; }
    unsigned long info(int i) const { return opt[i].info; }

    // Write Interface
    unsigned char kind(int i, unsigned char input) { return (opt[i].kind = input); }
    unsigned char size(int i, unsigned char input) { return (opt[i].size = input); }
    unsigned short section(int i, unsigned short input) { return (opt[i].section = input); }
    unsigned long info(int i, unsigned long input) { return (opt[i].info = input); }

    // Meta-Info Interface
    unsigned long count() const { return (data->d_size / sizeof(Elf_Options)); }

  private:
    Elf_Data *data;
    Elf_Options *opt;
};
#endif

#if !defined(os_solaris)
// ------------------------------------------------------------------------
// Class Elf_X_Versym simulates the SHT_GNU_versym structure.
class Elf_X_Versym {
 public:
   Elf_X_Versym()
      : data(NULL), versym32(NULL), versym64(NULL), is64(false) { }
   
   Elf_X_Versym(bool is64_, Elf_Data *input)
      : data(input), versym32(NULL), versym64(NULL), is64(is64_) {
      
      if (input) {
         if (!is64) versym32 = (Elf32_Half *)data->d_buf;
         else       versym64 = (Elf64_Half *)data->d_buf;
      }
   }
   // Read Interface
   unsigned long get(int i) const { return (!is64 ? versym32[i]
                                                   : versym64[i]); }
   // Meta-Info Interface
   unsigned long count() const { return (data->d_size / (!is64 ? sizeof(Elf32_Half)
                                                         : sizeof(Elf64_Half) )); }
   bool isValid() const { return (versym32 || versym64); }
   
 protected:
   Elf_Data *data;
   Elf32_Half *versym32;
   Elf64_Half *versym64;
   bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Verdaux simulates the Elf(32|64)_Verdaux structure.
class Elf_X_Verdaux {
 public:
   Elf_X_Verdaux()
      : data(NULL), verdaux32(NULL), verdaux64(NULL), is64(false) { }
   
   Elf_X_Verdaux(bool is64_, void *input)
      : data(input), verdaux32(NULL), verdaux64(NULL), is64(is64_) {
      
      if (input) {
         if (!is64) verdaux32 = (Elf32_Verdaux *)data;
         else       verdaux64 = (Elf64_Verdaux *)data;
      }
   }

   // Read Interface
   unsigned long vda_name() const { return (!is64 ? verdaux32->vda_name
                                                   : verdaux64->vda_name); }
   unsigned long vda_next() const { return (!is64 ? verdaux32->vda_next
                                                 : verdaux64->vda_next); }
   Elf_X_Verdaux *get_next() { 
       if(vda_next() == 0)
           return NULL;
       return new Elf_X_Verdaux(is64, (char *)data+vda_next());
   }

   // Meta-Info Interface
   bool isValid() const { return (verdaux32 || verdaux64); }
   
 protected:
   void *data;
   Elf32_Verdaux *verdaux32;
   Elf64_Verdaux *verdaux64;
   bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Verdef simulates the Elf(32|64)_Verdef structure.
class Elf_X_Verdef {
 public:
   Elf_X_Verdef()
      : data(NULL), verdef32(NULL), verdef64(NULL), is64(false) { }
   
   Elf_X_Verdef(bool is64_, void *input)
      : data(input), verdef32(NULL), verdef64(NULL), is64(is64_) {
      
      if (input) {
         if (!is64) verdef32 = (Elf32_Verdef *)data;
         else       verdef64 = (Elf64_Verdef *)data;
      }
   }
   
   // Read Interface
   unsigned long vd_version() const { return (!is64 ? verdef32->vd_version
                                                   : verdef64->vd_version); }
   unsigned long vd_flags() const { return (!is64 ? verdef32->vd_flags
                                                 : verdef64->vd_flags); }
   unsigned long vd_ndx() const { return (!is64 ? verdef32->vd_ndx
                                                   : verdef64->vd_ndx); }
   unsigned long vd_cnt() const { return (!is64 ? verdef32->vd_cnt
                                                   : verdef64->vd_cnt); }
   unsigned long vd_hash() const { return (!is64 ? verdef32->vd_hash
                                                   : verdef64->vd_hash); }
   unsigned long vd_aux() const { return (!is64 ? verdef32->vd_aux
                                                   : verdef64->vd_aux); }
   unsigned long vd_next() const { return (!is64 ? verdef32->vd_next
                                                   : verdef64->vd_next); }

   Elf_X_Verdaux *get_aux() {
       if(vd_cnt() == 0)
           return NULL;
       return new Elf_X_Verdaux(is64, (char *)data+vd_aux());
   }

   Elf_X_Verdef *get_next() {
       if(vd_next() == 0)
           return NULL;
       return new Elf_X_Verdef(is64, (char *)data+vd_next());
   }

   // Meta-Info Interface
   bool isValid() const { return (verdef32 || verdef64); }
   
 protected:
   void *data;
   Elf32_Verdef *verdef32;
   Elf64_Verdef *verdef64;
   bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Vernaux simulates the Elf(32|64)_Vernaux structure.
class Elf_X_Vernaux {
 public:
   Elf_X_Vernaux()
      : data(NULL), vernaux32(NULL), vernaux64(NULL), is64(false) { }
   
   Elf_X_Vernaux(bool is64_, void *input)
      : data(input), vernaux32(NULL), vernaux64(NULL), is64(is64_) {
      
      if (input) {
         if (!is64) vernaux32 = (Elf32_Vernaux *)data;
         else       vernaux64 = (Elf64_Vernaux *)data;
      }
   }
   
   // Read Interface
   unsigned long vna_hash() const { return (!is64 ? vernaux32->vna_hash
                                                   : vernaux64->vna_hash); }
   unsigned long vna_flags() const { return (!is64 ? vernaux32->vna_flags
                                                   : vernaux64->vna_flags); }
   unsigned long vna_other() const { return (!is64 ? vernaux32->vna_other
                                                   : vernaux64->vna_other); }
   unsigned long vna_name() const { return (!is64 ? vernaux32->vna_name
                                                   : vernaux64->vna_name); }
   unsigned long vna_next() const { return (!is64 ? vernaux32->vna_next
                                                 : vernaux64->vna_next); }
   Elf_X_Vernaux *get_next() { 
       if(vna_next() == 0)
           return NULL;
       return new Elf_X_Vernaux(is64, (char *)data+vna_next());
   }

   // Meta-Info Interface
   bool isValid() const { return (vernaux32 || vernaux64); }
   
 protected:
   void *data;
   Elf32_Vernaux *vernaux32;
   Elf64_Vernaux *vernaux64;
   bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Verneed simulates the Elf(32|64)_Verneed structure.
class Elf_X_Verneed {
 public:
   Elf_X_Verneed()
      : data(NULL), verneed32(NULL), verneed64(NULL), is64(false) { }
   
   Elf_X_Verneed(bool is64_, void *input)
      : data(input), verneed32(NULL), verneed64(NULL), is64(is64_) {
      
      if (input) {
         if (!is64) verneed32 = (Elf32_Verneed *)data;
         else       verneed64 = (Elf64_Verneed *)data;
      }
   }
   
   // Read Interface
   unsigned long vn_version() const { return (!is64 ? verneed32->vn_version
                                                   : verneed64->vn_version); }
   unsigned long vn_cnt() const { return (!is64 ? verneed32->vn_cnt
                                                   : verneed64->vn_cnt); }
   unsigned long vn_file() const { return (!is64 ? verneed32->vn_file
                                                   : verneed64->vn_file); }
   unsigned long vn_aux() const { return (!is64 ? verneed32->vn_aux
                                                   : verneed64->vn_aux); }
   unsigned long vn_next() const { return (!is64 ? verneed32->vn_next
                                                   : verneed64->vn_next); }
   Elf_X_Vernaux *get_aux() {
       if(vn_cnt() == 0)
           return NULL;
       return new Elf_X_Vernaux(is64, (char *)data+vn_aux());
   }

   Elf_X_Verneed *get_next() {
       if(vn_next() == 0)
           return NULL;
       return new Elf_X_Verneed(is64, (char *)data+vn_next());
   }


   // Meta-Info Interface
   bool isValid() const { return (verneed32 || verneed64); }
   
 protected:
   void *data;
   Elf32_Verneed *verneed32;
   Elf64_Verneed *verneed64;
   bool is64;
};

#endif

// ------------------------------------------------------------------------
// Class Elf_X_Data simulates the Elf_Data structure.
class Elf_X_Data {
  public:
    Elf_X_Data()
       : data(NULL), is64(false) { }
    
    Elf_X_Data(bool is64_, Elf_Data *input)
       : data(input), is64(is64_) { }

    // Read Interface
    void *d_buf() const { return data->d_buf; }
    Elf_Type d_type() const { return data->d_type; }
    unsigned int d_version() const { return data->d_version; }
    size_t d_size() const { return data->d_size; }
    off_t d_off() const { return (off_t) data->d_off; }
    size_t d_align() const { return data->d_align; }
    
    // Write Interface
    void d_buf(void *input) { data->d_buf = input; }
    void d_type(Elf_Type input) { data->d_type = input; }
    void d_version(unsigned int input) { data->d_version = input; }
    void d_size(unsigned int input) { data->d_size = input; }
    void d_off(signed int input) { data->d_off = input; }
    void d_align(unsigned int input) { data->d_align = input; }

    // Data Interface
    const char *get_string() { return (const char *)data->d_buf; }
#if !defined(os_solaris)    
    Elf_X_Dyn get_dyn() { return Elf_X_Dyn(is64, data); }
    Elf_X_Versym get_versyms() { return Elf_X_Versym(is64, data); }
    Elf_X_Verneed *get_verNeedSym() { return new Elf_X_Verneed(is64, data->d_buf); }
    Elf_X_Verdef *get_verDefSym() { return new Elf_X_Verdef(is64, data->d_buf); }
#endif

#if defined(arch_mips)
    Elf_X_RegInfo get_regInfo() { return Elf_X_RegInfo(is64, data); }
#endif
    Elf_X_Rel get_rel() { return Elf_X_Rel(is64, data); }
    Elf_X_Rela get_rela() { return Elf_X_Rela(is64, data); }
    Elf_X_Sym get_sym() { return Elf_X_Sym(is64, data); }
#if defined(arch_mips)
    Elf_X_Options get_options() { return Elf_X_Options(is64, data); }
#endif

    bool isValid() { return data; }

  protected:
    Elf_Data *data;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Shdr simulates the Elf(32|64)_Shdr structure.
class Elf_X_Shdr {
  public:
    Elf_X_Shdr()
       : scn(NULL), shdr32(NULL), shdr64(NULL), is64(false), fromDebugFile(false) { }

    Elf_X_Shdr(bool is64_, Elf_Scn *input)
       : scn(input), shdr32(NULL), shdr64(NULL), is64(is64_), fromDebugFile(false) {

	if (input) {
	    first_data();
	    if (!is64) shdr32 = elf32_getshdr(scn);
	    else       shdr64 = elf64_getshdr(scn);
	}
    }

    // Read Interface
    unsigned long sh_name() const { 
       return (!is64 ? 
               static_cast<unsigned long>(shdr32->sh_name) :
               static_cast<unsigned long>(shdr64->sh_name)); 
    }
    unsigned long sh_type() const { 
       return (!is64 ? 
               static_cast<unsigned long>(shdr32->sh_type) :
               static_cast<unsigned long>(shdr64->sh_type)); 
    }
    unsigned long sh_flags() const { 
       return (!is64 ? 
               static_cast<unsigned long>(shdr32->sh_flags) :
               static_cast<unsigned long>(shdr64->sh_flags)); 
    }
    unsigned long sh_addr() const { 
#if !defined(os_vxworks)
       return (!is64 ? 
               static_cast<unsigned long>(shdr32->sh_addr) :
               static_cast<unsigned long>(shdr64->sh_addr)); 
#else
       return (!is64 ? 
               static_cast<unsigned long>(shdr32->sh_offset) :
               static_cast<unsigned long>(shdr64->sh_offset)); 
#endif
    }
    unsigned long sh_offset() const { 
       return (!is64 ? 
               static_cast<unsigned long>(shdr32->sh_offset) :
               static_cast<unsigned long>(shdr64->sh_offset)); 
    }
    unsigned long sh_size() const { 
       return (!is64 ? 
               static_cast<unsigned long>(shdr32->sh_size) :
               static_cast<unsigned long>(shdr64->sh_size)); 
    }
    unsigned long sh_link() const { 
       return (!is64 ? 
               shdr32->sh_link :
               shdr64->sh_link); 
    }
    unsigned long sh_info() const { 
       return (!is64 ? 
               static_cast<unsigned long>(shdr32->sh_info) :
               static_cast<unsigned long>(shdr64->sh_info)); 
    }
    unsigned long sh_addralign() const { 
       return (!is64 ? 
               static_cast<unsigned long>(shdr32->sh_addralign) :
               static_cast<unsigned long>(shdr64->sh_addralign)); 
    }
    unsigned long sh_entsize() const { 
       return (!is64 ? 
               static_cast<unsigned long>(shdr32->sh_entsize) :
               static_cast<unsigned long>(shdr64->sh_entsize)); 
    }

    // Write Interface
    void sh_name(unsigned long input) { if (!is64) shdr32->sh_name = input;
					else       shdr64->sh_name = input; }
    void sh_type(unsigned long input) { if (!is64) shdr32->sh_type = input;
					else       shdr64->sh_type = input; }
    void sh_flags(unsigned long input) { if (!is64) shdr32->sh_flags = input;
					 else       shdr64->sh_flags = input; }
    void sh_addr(unsigned long input) { if (!is64) shdr32->sh_flags = input;
					else       shdr64->sh_flags = input; }
    void sh_offset(unsigned long input) { if (!is64) shdr32->sh_offset = input;
					  else       shdr64->sh_offset = input; }
    void sh_size(unsigned long input) { if (!is64) shdr32->sh_size = input;
					else       shdr64->sh_size = input; }
    void sh_link(unsigned long input) { if (!is64) shdr32->sh_link = input;
					else       shdr64->sh_link = input; }
    void sh_info(unsigned long input) { if (!is64) shdr32->sh_info = input;
					else       shdr64->sh_info = input; }
    void sh_addralign(unsigned long input) { if (!is64) shdr32->sh_addralign = input;
					     else       shdr64->sh_addralign = input; }
    void sh_entsize(unsigned long input) { if (!is64) shdr32->sh_entsize = input;
					   else       shdr64->sh_entsize = input; }
    void setDebugFile(bool b) { fromDebugFile = b; }
    bool isFromDebugFile() { return fromDebugFile; }

    // Section Data Interface
    Elf_X_Data get_data() const { return Elf_X_Data(is64, data); }

    // For Sections with Multiple Data Sections
    void first_data() { data = elf_getdata(scn, NULL); }
    bool next_data() {
        Elf_Data *nextData = elf_getdata(scn, data);
        if (nextData) data = nextData;
        return nextData;
    }

    bool isValid() const { return (shdr32 || shdr64); }
    unsigned wordSize() { return is64 ? 8 : 4; }
    Elf_Scn *getScn() const { return scn; }
  protected:
    Elf_Scn *scn;
    Elf_Data *data;
    Elf32_Shdr *shdr32;
    Elf64_Shdr *shdr64;
    bool is64;
    bool fromDebugFile;
};

// ------------------------------------------------------------------------
// Class Elf_X_Phdr simulates the Elf(32|64)_Phdr structure.
class Elf_X_Phdr {
  public:
    Elf_X_Phdr()
	: phdr32(NULL), phdr64(NULL), is64(false) { }

    Elf_X_Phdr(bool is64_, void *input)
	: phdr32(NULL), phdr64(NULL), is64(is64_) {

	if (input) {
	    if (!is64) phdr32 = (Elf32_Phdr *)input;
	    else       phdr64 = (Elf64_Phdr *)input;
	}
    }

    // Read Interface
    unsigned long p_type() const { 
       return (!is64 ? 
               static_cast<unsigned long>(phdr32->p_type) :
               static_cast<unsigned long>(phdr64->p_type)); 
    }
    unsigned long p_offset() const { 
       return (!is64 ? 
               static_cast<unsigned long>(phdr32->p_offset) :
               static_cast<unsigned long>(phdr64->p_offset)); 
    }
    unsigned long p_vaddr() const { 
       return (!is64 ? 
               static_cast<unsigned long>(phdr32->p_vaddr) :
               static_cast<unsigned long>(phdr64->p_vaddr)); 
    }
    unsigned long p_paddr() const { 
       return (!is64 ? 
               static_cast<unsigned long>(phdr32->p_paddr) :
               static_cast<unsigned long>(phdr64->p_paddr)); 
    }
    unsigned long p_filesz() const { 
       return (!is64 ? 
               static_cast<unsigned long>(phdr32->p_filesz) :
               static_cast<unsigned long>(phdr64->p_filesz)); 
    }
    unsigned long p_memsz() const { 
       return (!is64 ? 
               static_cast<unsigned long>(phdr32->p_memsz) :
               static_cast<unsigned long>(phdr64->p_memsz)); 
    }
    unsigned long p_flags() const { 
       return (!is64 ? 
               static_cast<unsigned long>(phdr32->p_flags) :
               static_cast<unsigned long>(phdr64->p_flags)); 
    }
    unsigned long p_align() const { 
       return (!is64 ? 
               static_cast<unsigned long>(phdr32->p_align) :
               static_cast<unsigned long>(phdr64->p_align)); 
    }

    // Write Interface
    void p_type(unsigned long input) { if (!is64) phdr32->p_type = input;
				       else       phdr64->p_type = input; }
    void p_offset(unsigned long input) { if (!is64) phdr32->p_offset = input;
					 else       phdr64->p_offset = input; }
    void p_vaddr(unsigned long input) { if (!is64) phdr32->p_vaddr = input;
					else       phdr64->p_vaddr = input; }
    void p_paddr(unsigned long input) { if (!is64) phdr32->p_paddr = input;
					else       phdr64->p_paddr = input; }
    void p_filesz(unsigned long input) { if (!is64) phdr32->p_filesz = input;
					 else       phdr64->p_filesz = input; }
    void p_memsz(unsigned long input) { if (!is64) phdr32->p_memsz = input;
					else       phdr64->p_memsz = input; }
    void p_flags(unsigned long input) { if (!is64) phdr32->p_flags = input;
					else       phdr64->p_flags = input; }
    void p_align(unsigned long input) { if (!is64) phdr32->p_align = input;
					else       phdr64->p_align = input; }

    bool isValid() const { return (phdr32 || phdr64); }

  private:
    Elf32_Phdr *phdr32;
    Elf64_Phdr *phdr64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X simulates the Elf(32|64)_Ehdr structure.
// Also works for ELF archives. 
class Elf_X {
  public:
    Elf_X()
    : elf(NULL), ehdr32(NULL), ehdr64(NULL), phdr32(NULL), phdr64(NULL), filedes(-1), is64(false), isArchive(false) { }

    Elf_X(int input, Elf_Cmd cmd, Elf_X *ref = NULL)
    : ehdr32(NULL), ehdr64(NULL), phdr32(NULL), phdr64(NULL), filedes(input), is64(false), isArchive(false) {

       if (elf_version(EV_CURRENT) != EV_NONE) {
          elf_errno(); // Reset elf_errno to zero.
          if(ref)
             elf = elf_begin(input, cmd, ref->e_elfp());
          else
             elf = elf_begin(input, cmd, NULL);
          int errnum;
          if ((errnum = elf_errno()) != 0)
          {
             //const char *msg = elf_errmsg(errnum);
             //fprintf(stderr, "Elf error: %s\n", msg);
          }
          if (elf) {
             if (elf_kind(elf) == ELF_K_ELF) {
                char *identp = elf_getident(elf, NULL);
                is64 = (identp && identp[EI_CLASS] == ELFCLASS64);
             }
             else if(elf_kind(elf) == ELF_K_AR){
                char *identp = elf_getident(elf, NULL);
                is64 = (identp && identp[EI_CLASS] == ELFCLASS64);
                isArchive = true;
             }
             
             if (!is64) ehdr32 = elf32_getehdr(elf);
             else	   ehdr64 = elf64_getehdr(elf);
             
             if (!is64) phdr32 = elf32_getphdr(elf);
             else	   phdr64 = elf64_getphdr(elf);
          }
       }
    }

  Elf_X(char *mem_image, size_t mem_size) : 
   ehdr32(NULL), ehdr64(NULL), phdr32(NULL), phdr64(NULL), is64(false), isArchive(false)
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
            else	   ehdr64 = elf64_getehdr(elf);
            
            if (!is64) phdr32 = elf32_getphdr(elf);
            else	   phdr64 = elf64_getphdr(elf);
         }
      }
   }

    void end() {
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
    Elf *e_elfp() const { return elf; }
    unsigned char *e_ident() const { 
       return (!is64 ? 
               static_cast<unsigned char*>(ehdr32->e_ident) :
               static_cast<unsigned char*>(ehdr64->e_ident)); 
    }
    unsigned short e_type() const { 
       return (!is64 ? 
               static_cast<unsigned short>(ehdr32->e_type) :
               static_cast<unsigned short>(ehdr64->e_type)); 
    }
    unsigned short e_machine() const { 
       return (!is64 ? 
               static_cast<unsigned short>(ehdr32->e_machine) :
               static_cast<unsigned short>(ehdr64->e_machine)); 
    }
    unsigned long e_version() const { 
       return (!is64 ? 
               static_cast<unsigned long>(ehdr32->e_version) :
               static_cast<unsigned long>(ehdr64->e_version)); 
    }
    unsigned long e_entry() const { 
       return (!is64 ? 
               static_cast<unsigned long>(ehdr32->e_entry) :
               static_cast<unsigned long>(ehdr64->e_entry)); 
    }
    unsigned long e_phoff() const { 
       return (!is64 ? 
               static_cast<unsigned long>(ehdr32->e_phoff) :
               static_cast<unsigned long>(ehdr64->e_phoff)); 
    }
    unsigned long e_shoff() const { 
       return (!is64 ? 
               static_cast<unsigned long>(ehdr32->e_shoff) :
               static_cast<unsigned long>(ehdr64->e_shoff)); 
    }
    unsigned long e_flags() const { 
       return (!is64 ? 
               static_cast<unsigned long>(ehdr32->e_flags) :
               static_cast<unsigned long>(ehdr64->e_flags)); 
    }
    unsigned short e_ehsize() const { 
       return (!is64 ? 
               static_cast<unsigned short>(ehdr32->e_ehsize) :
               static_cast<unsigned short>(ehdr64->e_ehsize)); 
    }
    unsigned short e_phentsize() const { 
       return (!is64 ? 
               static_cast<unsigned short>(ehdr32->e_phentsize) :
               static_cast<unsigned short>(ehdr64->e_phentsize)); 
    }
    unsigned short e_phnum() const { 
       return (!is64 ? 
               static_cast<unsigned short>(ehdr32->e_phnum) :
               static_cast<unsigned short>(ehdr64->e_phnum)); 
    }
    unsigned short e_shentsize() const { 
       return (!is64 ? 
               static_cast<unsigned short>(ehdr32->e_shentsize) :
               static_cast<unsigned short>(ehdr64->e_shentsize)); 
    }
    unsigned short e_shnum() const { 
       return (!is64 ? 
               static_cast<unsigned short>(ehdr32->e_shnum) :
               static_cast<unsigned short>(ehdr64->e_shnum)); 
    }
    unsigned short e_shstrndx() const { 
       return (!is64 ? 
               static_cast<unsigned short>(ehdr32->e_shstrndx) :
               static_cast<unsigned short>(ehdr64->e_shstrndx)); 
    }
    const char *e_rawfile(size_t &nbytes) const {
       return elf_rawfile(elf, &nbytes);
    }

    Elf_X *e_next(Elf_X *ref) {
        if(!isArchive)
            return NULL;
        Elf_Cmd cmd = elf_next(ref->e_elfp());
        return new Elf_X(filedes, cmd, this);
    }

    Elf_X *e_rand(unsigned offset){
        if(!isArchive)
            return NULL;
        elf_rand(elf, offset);
        return new Elf_X(filedes, ELF_C_READ, this);
    }

    // Write Interface
    void e_ident(unsigned char *input) { if (!is64) P_memcpy(ehdr32->e_ident, input, EI_NIDENT);
					 else       P_memcpy(ehdr64->e_ident, input, EI_NIDENT); }
    void e_type(unsigned short input) { if (!is64) ehdr32->e_type = input;
					else       ehdr64->e_type = input; }
    void e_machine(unsigned short input) { if (!is64) ehdr32->e_machine = input;
    					   else       ehdr64->e_machine = input; }
    void e_version(unsigned long input) { if (!is64) ehdr32->e_version = input;
					  else       ehdr64->e_version = input; }
    void e_entry(unsigned long input) { if (!is64) ehdr32->e_entry = input;
					else       ehdr64->e_entry = input; }
    void e_phoff(unsigned long input) { if (!is64) ehdr32->e_phoff = input;
					else       ehdr64->e_phoff = input; }
    void e_shoff(unsigned long input) { if (!is64) ehdr32->e_shoff = input;
					else       ehdr64->e_shoff = input; }
    void e_flags(unsigned long input) { if (!is64) ehdr32->e_flags = input;
					else       ehdr64->e_flags = input; }
    void e_ehsize(unsigned short input) { if (!is64) ehdr32->e_ehsize = input;
					  else       ehdr64->e_ehsize = input; }
    void e_phentsize(unsigned short input) { if (!is64) ehdr32->e_phentsize = input;
					     else       ehdr64->e_phentsize = input; }
    void e_phnum(unsigned short input) { if (!is64) ehdr32->e_phnum = input;
					 else       ehdr64->e_phnum = input; }
    void e_shentsize(unsigned short input) { if (!is64) ehdr32->e_shentsize = input;
					     else       ehdr64->e_shentsize = input; }
    void e_shnum(unsigned short input) { if (!is64) ehdr32->e_shnum = input;
					 else       ehdr64->e_shnum = input; }
    void e_shstrndx(unsigned short input) { if (!is64) ehdr32->e_shstrndx = input;
					    else       ehdr64->e_shstrndx = input; }

    // Data Interface
    bool isValid() const { return (ehdr32 || ehdr64); }
    int wordSize() const { return (!is64 ? 4 : 8); }
    Elf_X_Phdr get_phdr(unsigned int i = 0) const {
        if (!is64) return Elf_X_Phdr(is64, phdr32 + i);
        else       return Elf_X_Phdr(is64, phdr64 + i);
    }
    Elf_X_Shdr get_shdr(unsigned int i) const {
        Elf_Scn *scn = elf_getscn(elf, i);
        return Elf_X_Shdr(is64, scn);
    }

  protected:
    Elf *elf;
    Elf32_Ehdr *ehdr32;
    Elf64_Ehdr *ehdr64;
    Elf32_Phdr *phdr32;
    Elf64_Phdr *phdr64;
    int filedes;
    bool is64;
    bool isArchive;
};

#endif

/*
 * Copyright (c) 1996-2001 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/************************************************************************
 * $Id: Object-elf.C,v 1.47 2003/04/14 15:59:17 jodom Exp $
 * Object-elf.C: Object class for ELF file format
************************************************************************/


#include "dyninstAPI/src/Object.h"
#if !defined(_Object_elf_h_)
#error "Object-elf.h not #included"
#endif

#include "common/h/String.h"
#include "common/h/Symbol.h"
#include "common/h/Dictionary.h"
#include "common/h/pathName.h"     // extract_pathname_tail()
#include <elf.h>
#include <stdio.h>

#if defined(USES_DWARF_DEBUG)
#include <dwarf.h>
#include <libdwarf.h>
#endif

#if defined(TIMED_PARSE)
#include <sys/time.h>
#endif

// add some space to avoid looking for functions in data regions
#define EXTRA_SPACE 8

static bool pdelf_check_ehdr(Elf *elfp, bool is64)
{
  if (is64) {
    // Elf64_Ehdr integrity check
#ifndef USES_ELF32_ONLY
    Elf64_Ehdr *ehdrp_64 = elf64_getehdr(elfp);
    if ((ehdrp_64 == NULL) ||
	(ehdrp_64->e_ident[EI_CLASS] != ELFCLASS64) ||
	(ehdrp_64->e_type != ET_EXEC && ehdrp_64->e_type != ET_DYN) ||
	(ehdrp_64->e_phoff == 0) ||
	(ehdrp_64->e_shoff == 0) ||
	(ehdrp_64->e_phnum == 0) ||
	(ehdrp_64->e_shnum == 0)) 
      {
	return false;
      }
#endif
  } else {
    // Elf32_Ehdr integrity check
    Elf32_Ehdr *ehdrp_32 = elf32_getehdr(elfp);
    if ((ehdrp_32 == NULL) ||
	(ehdrp_32->e_ident[EI_CLASS] != ELFCLASS32) ||
	(ehdrp_32->e_type != ET_EXEC && ehdrp_32->e_type != ET_DYN) ||
	(ehdrp_32->e_phoff == 0) ||
	(ehdrp_32->e_shoff == 0) ||
	(ehdrp_32->e_phnum == 0) ||
	(ehdrp_32->e_shnum == 0)) 
      {
	return false;
      }
  }
  return true;
}

const char *pdelf_get_shnames(Elf *elfp, bool is64)
{
  size_t shstrndx = 0;
  if (is64) {
#ifndef USES_ELF32_ONLY
    Elf64_Ehdr *ehdrp_64 = elf64_getehdr(elfp);
    if (ehdrp_64 == NULL) return NULL;
    shstrndx = ehdrp_64->e_shstrndx;
#endif
  } else {
    Elf32_Ehdr *ehdrp_32 = elf32_getehdr(elfp);
    if (ehdrp_32 == NULL) return NULL;
    shstrndx = ehdrp_32->e_shstrndx;
  }

  Elf_Scn *shstrscnp = elf_getscn(elfp, shstrndx);
  if (shstrscnp == NULL) return NULL;
  
  Elf_Data *shstrdatap = elf_getdata(shstrscnp, 0);
  if (shstrdatap == NULL) return NULL;
  
  return (const char *)shstrdatap->d_buf;
}



//
// SectionHeaderSortFunction
// 
// Compare function for use with the Vector<T> sort method.
//
extern "C" {
static int
SectionHeaderSortFunction( const void* v1, const void* v2 )
{
	const pdElfShdr* hdr1 = *(const pdElfShdr* const*)v1;
	const pdElfShdr* hdr2 = *(const pdElfShdr* const*)v2;

	return hdr1->pd_addr - hdr2->pd_addr;
}
}

// loaded_elf(): populate elf section pointers
// for EEL rewritten code, also populate "code_*_" members
bool Object::loaded_elf(bool& did_elf, Elf*& elfp, 
  Address& txtaddr, Address& bssaddr,
  Elf_Scn*& symscnp, Elf_Scn*& strscnp, 
  Elf_Scn*& stabscnp, Elf_Scn*& stabstrscnp, 
  Elf_Scn*& stabs_indxcnp, Elf_Scn*& stabstrs_indxcnp, 
  Elf_Scn*& rel_plt_scnp, Elf_Scn*& plt_scnp, 
  Elf_Scn*& got_scnp,  
  Elf_Scn*& dynsym_scnp, Elf_Scn*& dynstr_scnp, 
  bool 
#if defined(mips_sgi_irix6_4)
  a_out  // variable not used on other platforms
#endif
  ) 
{
  // ELF initialization
  if (elf_version(EV_CURRENT) == EV_NONE) {
    return false;
  }
  elf_errno(); // TODO: ???
  elfp = elf_begin(file_fd_, ELF_C_READ, 0);
  if (elfp == NULL) return false;
  did_elf = true;
  if (elf_kind(elfp) != ELF_K_ELF) return false;

  // ELF class: 32/64-bit
  char *identp = elf_getident(elfp, NULL);
  if (identp == NULL) return false;
  is_elf64_ = (identp[EI_CLASS] == ELFCLASS64);

  // ELF header: sanity check
  if (!pdelf_check_ehdr(elfp, is_elf64_)) {
    log_elferror(err_func_, "ELF header");
    return false;
  }

  // ".shstrtab" section: string table for section header names
  const char *shnames = pdelf_get_shnames(elfp, is_elf64_);
  if (shnames == NULL) {
    log_elferror(err_func_, ".shstrtab section");
    return false;
  }

  const char* EDITED_TEXT_NAME = ".edited.text";
  const char* TEXT_NAME        = ".text";
  const char* BSS_NAME         = ".bss";
  const char* SYMTAB_NAME      = ".symtab";
  const char* STRTAB_NAME      = ".strtab";
  const char* STAB_NAME        = ".stab";
  const char* STABSTR_NAME     = ".stabstr";
  const char* STAB_INDX_NAME   = ".stab.index";
  const char* STABSTR_INDX_NAME= ".stab.indexstr";
  // sections from dynamic executables and shared objects
  const char* PLT_NAME         = ".plt";
  const char* REL_PLT_NAME     = ".rela.plt"; // sparc-solaris
  const char* REL_PLT_NAME2    = ".rel.plt";  // x86-solaris
  const char* GOT_NAME         = ".got";
  const char* DYNSYM_NAME      = ".dynsym";
  const char* DYNSTR_NAME      = ".dynstr";
  const char* DATA_NAME        = ".data";
  const char* RO_DATA_NAME     = ".ro_data";  // mips

  // initialize Object members

	text_addr_ = 0; //ccw 23 jan 2002
	text_size_ = 0; //for determining if a mutation
			//falls within the text section 
			//of a shared library

  dynsym_addr_ = 0;
  dynstr_addr_ = 0;
  got_addr_ = 0;
  got_size_ = 0;
  plt_addr_ = 0;
  plt_size_ = 0;
  plt_entry_size_ = 0;
  rel_plt_addr_ = 0;
  rel_plt_size_ = 0;
  rel_plt_entry_size_ = 0;
  stab_off_ = 0;
  stab_size_ = 0;
  stabstr_off_ = 0;
  stab_indx_off_ = 0;
  stab_indx_size_ = 0;
  stabstr_indx_off_ = 0;
#if defined(mips_sgi_irix6_4)
  MIPS_stubs_addr_ = 0;
  MIPS_stubs_off_ = 0;
  MIPS_stubs_size_ = 0;
  got_zero_index_ = -1;
  dynsym_zero_index_ = -1;
#endif

  txtaddr = 0;

#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

  Elf_Scn *scnp = NULL;
  while ((scnp = elf_nextscn(elfp, scnp)) != NULL) {
    // ELF section header: wrapper object
    pdElfShdr* pd_shdrp = new pdElfShdr(scnp, is_elf64_);
    if (pd_shdrp->err) {
      log_elferror(err_func_, "elf_getshdr");
	  delete pd_shdrp;
      return false;
    }
	allSectionHdrs.push_back( pd_shdrp );

    // resolve section name
    const char *name = (const char *)&shnames[pd_shdrp->pd_name];    

    // section-specific processing
    if (strcmp(name, EDITED_TEXT_NAME) == 0) {
      // EEL rewritten executable
      EEL = true;
      if (txtaddr == 0)
	txtaddr = pd_shdrp->pd_addr;
      code_ptr_ = (Word *)(void*)&file_ptr_[pd_shdrp->pd_offset - EXTRA_SPACE];
      code_off_ = pd_shdrp->pd_addr - EXTRA_SPACE;
      code_len_ = (pd_shdrp->pd_size + EXTRA_SPACE) / sizeof(Word);
    }
    if (strcmp(name, TEXT_NAME) == 0) {
	text_addr_ = pd_shdrp->pd_addr;
	text_size_ = pd_shdrp->pd_size; 

      if (txtaddr == 0)
	txtaddr = pd_shdrp->pd_addr;
    }
    else if (strcmp(name, BSS_NAME) == 0) {
      bssaddr = pd_shdrp->pd_addr;
    }
    else if (strcmp(name, SYMTAB_NAME) == 0) {
      symscnp = scnp;
    }
    else if (strcmp(name, STRTAB_NAME) == 0) {
      strscnp = scnp;
    } else if (strcmp(name, STAB_INDX_NAME) == 0) {
      stabs_indxcnp = scnp;
      stab_indx_off_ = pd_shdrp->pd_offset;
      stab_indx_size_ = pd_shdrp->pd_size;
    } else if (strcmp(name, STABSTR_INDX_NAME) == 0) {
      stabstrs_indxcnp = scnp;
      stabstr_indx_off_ = pd_shdrp->pd_offset;
    } else if (strcmp(name, STAB_NAME) == 0) {
      stabscnp = scnp;
      stab_off_ = pd_shdrp->pd_offset;
      stab_size_ = pd_shdrp->pd_size;
    } else if (strcmp(name, STABSTR_NAME) == 0) {
      stabstrscnp = scnp;
      stabstr_off_ = pd_shdrp->pd_offset;
    } else if ((strcmp(name, REL_PLT_NAME) == 0) || 
	     (strcmp(name, REL_PLT_NAME2) == 0)) {
      rel_plt_scnp = scnp;
      rel_plt_addr_ = pd_shdrp->pd_addr;
      rel_plt_size_ = pd_shdrp->pd_size;
      rel_plt_entry_size_ = pd_shdrp->pd_entsize;
    }
    else if (strcmp(name, PLT_NAME) == 0) {
      plt_scnp = scnp;
      plt_addr_ = pd_shdrp->pd_addr;
      plt_size_ = pd_shdrp->pd_size;
#if defined(i386_unknown_linux2_0)
      //
      // On x86, the GNU linker purposefully sets the PLT
	  // table entry size to an incorrect value to be
	  // compatible with the UnixWare linker.  (See the comment
	  // in the elf_i386_finish_dynamic_sections function of
	  // the BFD library.)  The GNU linker sets this value to 4,
	  // when it should be 16.
	  //
	  // I see no good way to determine this value from the
	  // ELF section header information.  We can either (a) hard-code
	  // the value that is used in the BFD library, or (b) compute
	  // it by dividing the size of the PLT by the number of entries
	  // we think should be in the PLT.  I'm not certain, but I
	  // believe the PLT and the .rel.plt section should have the
	  // same number of "real" entries (the x86 PLT has one extra entry
	  // at the beginning).
	  // 
	  // This code is applicable to any x86 system that uses the
	  // GNU linker.  We currently only support Linux on x86 - if
	  // we start supporting some other x86 OS that uses the GNU
	  // linker in the future, it should be enabled for that platform as well.
	  // Note that this problem does not affect the non-x86 platforms
	  // that might use the GNU linker.  For example, programs linked
	  // with gld on SPARC Solaris have the correct PLT entry size.
	  //
	  // Another potential headache in the future is if we support
	  // some other x86 platform that has both the GNU linker and
	  // some other linker.  (Does BSD fall into this category?)
	  // If the two linkers set the entry size differently, we may
	  // need to re-evaluate this code.
	  //
	  plt_entry_size_ = plt_size_ / ((rel_plt_size_ / rel_plt_entry_size_) + 1);
	  assert( plt_entry_size_ == 16 );
#else
      plt_entry_size_ = pd_shdrp->pd_entsize;
#endif // defined(i386_unknown_linux2_0)
    }
    else if (strcmp(name, GOT_NAME) == 0) {
      got_scnp = scnp;
      got_addr_ = pd_shdrp->pd_addr;
      got_size_ = pd_shdrp->pd_size;
      if (!bssaddr) bssaddr = pd_shdrp->pd_addr;
    }
    else if (strcmp(name, DYNSYM_NAME) == 0) {
      dynsym_scnp = scnp;
      dynsym_addr_ = pd_shdrp->pd_addr;
    }
    else if (strcmp(name, DYNSTR_NAME) == 0) {
      dynstr_scnp = scnp;
      dynstr_addr_ = pd_shdrp->pd_addr;
    }
    else if (strcmp(name, DATA_NAME) == 0) {
      if (!bssaddr) bssaddr = pd_shdrp->pd_addr;	  
    }
    else if (strcmp(name, RO_DATA_NAME) == 0) {
      if (!bssaddr) bssaddr = pd_shdrp->pd_addr;	  
    }
#if defined( ia64_unknown_linux2_4 ) 
    else if (strcmp(name, ".dynamic") == 0) {

	Elf_Data *datap = elf_getdata(scnp, 0);
	Elf64_Dyn *dyns = (Elf64_Dyn *)datap->d_buf;
	unsigned ndyns = datap->d_size / sizeof(Elf64_Dyn);
	for (unsigned i = 0; i < ndyns; i++) {
	  Elf64_Dyn *dyn = &dyns[i];
	  switch(dyn->d_tag) {

		case DT_PLTGOT:
			this->gp = dyn->d_un.d_ptr;
			break;

		default:
			break;
		} // switch
	} // for
    }// .dynamic

#endif /* ia64_unknown_linux2_4 */

#if defined(mips_sgi_irix6_4)
    else if (strcmp(name, ".MIPS.stubs") == 0) {
      MIPS_stubs_addr_ = pd_shdrp->pd_addr;
      MIPS_stubs_size_ = pd_shdrp->pd_size;
      MIPS_stubs_off_ = pd_shdrp->pd_offset;
    }
    else if (strcmp(name, MIPS_OPTIONS) == 0) {
      // see <sys/elf.h>, ".MIPS.options" section
      Elf_Data *datap = elf_getdata(scnp, 0);
      Elf_Options *optionsp = NULL;
      for (unsigned i = 0; i < datap->d_size; i += optionsp->size) {
	optionsp = (Elf_Options *)(void *)(((char *)datap->d_buf) + i);
	if (optionsp->kind != ODK_REGINFO) continue;
	if (is_elf64_) {

	  // RegInfo starts after Options header
	  Elf64_RegInfo *reginfop = (Elf64_RegInfo *)(void *)(optionsp + 1);
	  gp_value = reginfop->ri_gp_value;

	} else { // 32-bit ELF

	  // RegInfo starts after Options header
	  Elf32_RegInfo *reginfop = (Elf32_RegInfo *)(optionsp + 1);
	  gp_value = reginfop->ri_gp_value;

	}
	break;
      }
    }
    else if (strcmp(name, MIPS_REGINFO) == 0) {
      // see <sys/elf.h>, ".reginfo" section
      Elf_Data *datap = elf_getdata(scnp, 0);
      if (is_elf64_) {

	Elf64_RegInfo *reginfop = (Elf64_RegInfo *)datap->d_buf;
	gp_value = reginfop->ri_gp_value;

      } else { // 32-bit ELF

	Elf32_RegInfo *reginfop = (Elf32_RegInfo *)datap->d_buf;
	gp_value = reginfop->ri_gp_value;

      }
    }
    else if (strcmp(name, ELF_DYNAMIC) == 0) {
      // see <sys/elf.h>, ".dynamic" section
      if (is_elf64_) {

	Elf_Data *datap = elf_getdata(scnp, 0);
	Elf64_Dyn *dyns = (Elf64_Dyn *)datap->d_buf;
	unsigned ndyns = datap->d_size / sizeof(Elf64_Dyn);
	for (unsigned i = 0; i < ndyns; i++) {
	  Elf64_Dyn *dyn = &dyns[i];
	  switch(dyn->d_tag) {

	  case DT_MIPS_RLD_TEXT_RESOLVE_ADDR:
	    // "__rld_text_resolve" address
	    rbrk_addr = dyn->d_un.d_ptr;
	    break;
	  case DT_MIPS_BASE_ADDRESS:
	    // object base address
	    base_addr = dyn->d_un.d_ptr;
	    if (a_out) base_addr = 0;
	    break;
	  case DT_MIPS_GOTSYM:
	    // .dynsym index of first external symbol
	    dynsym_zero_index_ = dyn->d_un.d_val;
	    break;
	  case DT_MIPS_LOCAL_GOTNO:
	    // .got index of first external GOT entry
	    got_zero_index_ = dyn->d_un.d_val;
	    break;
	  }
	}

      } else { // 32-bit ELF

	Elf_Data *datap = elf_getdata(scnp, 0);
	Elf32_Dyn *dyns = (Elf32_Dyn *)datap->d_buf;
	unsigned ndyns = datap->d_size / sizeof(Elf32_Dyn);
	for (unsigned i = 0; i < ndyns; i++) {
	  Elf32_Dyn *dyn = &dyns[i];
	  switch(dyn->d_tag) {
	  case DT_MIPS_RLD_TEXT_RESOLVE_ADDR:
	    // "__rld_text_resolve" address
	    rbrk_addr = dyn->d_un.d_ptr;
	    break;
	  case DT_MIPS_BASE_ADDRESS:
	    // object base address
	    base_addr = dyn->d_un.d_ptr;
	    if (a_out) base_addr = 0;
	    break;
	  case DT_MIPS_GOTSYM:
	    // .dynsym index of first external symbol
	    dynsym_zero_index_ = dyn->d_un.d_val;
	    break;
	  case DT_MIPS_LOCAL_GOTNO:
	    // .got index of first external GOT entry
	    got_zero_index_ = dyn->d_un.d_val;
	    break;
	  }
	}

      }
    }
#endif /* mips_sgi_irix6_4 */
    
  }
  if(!symscnp || !strscnp) {
    if(dynsym_scnp && dynstr_scnp){
      symscnp = dynsym_scnp;
      strscnp = dynstr_scnp;
    }
  }

  // sort the section headers by base address
  allSectionHdrs.sort( SectionHeaderSortFunction );
  //sort(allSectionHdrs.begin(), allSectionHdrs.end(), sort_func());
#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << "main loop of loaded elf took "<<dursecs <<" msecs" << endl;
#endif

#ifndef BPATCH_LIBRARY /* Some objects really don't have all sections. */
  if (!bssaddr || !symscnp || !strscnp) {
    log_elferror(err_func_, "no text/bss/symbol/string section");
    return false;
  }
#endif

  //if (is_elf64_) fprintf(stderr, ">>> 64-bit loaded_elf() successful\n");
  return true;
}

#if defined(mips_sgi_irix6_4)
const char *Object::got_entry_name(Address entry_raddr) const
{
  const char *ret = NULL;

  if (got_zero_index_ == -1) return NULL;
  if (dynsym_zero_index_ == -1) return NULL;

  // mapped ELF sections: .dynsym .dynstr
  const char *dynsym_ptr = elf_vaddr_to_ptr(dynsym_addr_);
  const char *dynstr_ptr = elf_vaddr_to_ptr(dynstr_addr_);

  // find corresponding .dynsym index
  Address entry_goff = entry_raddr + base_addr - got_addr_;
  if (entry_goff > got_size_) return NULL;
  int index_got = (is_elf64_) 
    ? (entry_goff / sizeof(Elf64_Got))
    : (entry_goff / sizeof(Elf32_Got));
  //fprintf(stderr, "    .got index #%i\n", index_got);
  if (index_got < got_zero_index_) return NULL; // local GOT entry
  int index_dynsym = (index_got - got_zero_index_) + dynsym_zero_index_;
  //fprintf(stderr, "    .dynsym index #%i\n", index_dynsym);

  // find symbol name
  const char *dynstrs = (const char *)(dynstr_ptr);
  if (is_elf64_) {

    const Elf64_Sym *syms = (const Elf64_Sym *)(const void *)dynsym_ptr;
    int index_dynstr = syms[index_dynsym].st_name;
    ret = &dynstrs[index_dynstr];
    
  } else { // 32-bit ELF

    const Elf32_Sym *syms = (const Elf32_Sym *)(const void *)dynsym_ptr;
    int index_dynstr = syms[index_dynsym].st_name;
    ret = &dynstrs[index_dynstr];

  }

  //fprintf(stderr, ">>> got_entry_name(0x%016lx): \"%s\"\n", entry_raddr, ret);
  return ret;
}

int Object::got_gp_disp(const char *fn_name) const
{
  // check against every external GOT entry
  int got_entry_size = (is_elf64_) ? (sizeof(Elf64_Got)) : (sizeof(Elf32_Got));
  int n = got_size_ / got_entry_size;
  for (int i = got_zero_index_; i < n; i++) {
    Address got_entry = got_addr_ + (i * got_entry_size);

    // lookup name by GOT entry
    const char *got_str = got_entry_name(got_entry - base_addr);
    if (!got_str) continue;
    string got_name = got_str;

    // check against variations of GOT name
    if (strcmp(fn_name, (got_name).c_str()) == 0 ||       // default
	strcmp(fn_name, ("_" + got_name).c_str()) == 0 || // C
	strcmp(fn_name, (got_name + "_").c_str()) == 0 || // Fortran
	strcmp(fn_name, ("__" + got_name).c_str()) == 0)  // libm
      {
      int gp_off = (long int)got_entry - (long int)gp_value;
      return gp_off;
    }
  }
  return -1;
}
#endif /* mips_sgi_irix6_4 */

bool Object::get_relocation_entries(Elf_Scn*& rel_plt_scnp,
				    Elf_Scn*& dynsym_scnp, 
				    Elf_Scn*& dynstr_scnp) 
{
#if defined (i386_unknown_solaris2_5) || defined (i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)
        Elf32_Rel *next_entry = 0;
        Elf32_Rel *entries = 0;
#else
        Elf32_Rela *next_entry = 0;
        Elf32_Rela *entries = 0;
#endif

    if(rel_plt_size_ && rel_plt_addr_) {
	Elf_Data *reldatap = elf_getdata(rel_plt_scnp, 0);
	Elf_Data* symdatap = elf_getdata(dynsym_scnp, 0);
	Elf_Data* strdatap = elf_getdata(dynstr_scnp, 0);
	if(!reldatap || !symdatap || !strdatap) return false;

	Elf32_Sym*  syms   = (Elf32_Sym *) symdatap->d_buf;
	const char* strs   = (const char *) strdatap->d_buf;
	Address next_plt_entry_addr = plt_addr_;

#if defined (i386_unknown_solaris2_5) || defined (i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)
	entries  = (Elf32_Rel *) reldatap->d_buf;
	next_plt_entry_addr += plt_entry_size_;  // 1st PLT entry is special
#else
	entries  = (Elf32_Rela *) reldatap->d_buf;
	next_plt_entry_addr += 4*(plt_entry_size_); //1st 4 entries are special
#endif
	if(!entries) return false;

	next_entry = entries;
	for(u_int i=0; i < (rel_plt_size_/rel_plt_entry_size_); i++) {
	    Elf32_Word sym_index = ELF32_R_SYM(next_entry->r_info); 
	    relocationEntry re(next_plt_entry_addr, next_entry->r_offset,
			       string(&strs[syms[sym_index].st_name]));
            relocation_table_.push_back(re); 
	    next_entry++;
	    next_plt_entry_addr += plt_entry_size_;
	}
    }
    return true;
}

// map object file into memory
// populates: file_fd_, file_size_, file_ptr_
bool Object::mmap_file(const char *file, 
		       bool &did_open, bool &did_mmap)
{
  file_fd_ = open(file, O_RDONLY);
  if (file_fd_ == -1) return false;
  did_open = true;
  
  struct stat st;
  if (fstat(file_fd_, &st) == -1) return false;
  file_size_ = st.st_size;
  
  file_ptr_ = (char *) mmap(0, file_size_, PROT_READ, MAP_SHARED, file_fd_, 0);
  if (file_ptr_ == (char *)MAP_FAILED) return false;
  did_mmap = true;
  
  return true;
}

void Object::load_object()
{
  Elf  *elfp  = 0;  
  bool  did_open = false;
  bool  did_mmap = false;
  bool  did_elf  = false;
  
  { // binding contour (for "goto cleanup")

    const char *file = file_.c_str();
    if (mmap_file(file, did_open, did_mmap) == false) {
      char buf[500];
      sprintf(buf, "open/fstat/mmap failed on: %s", file);
      log_perror(err_func_, buf);
      goto cleanup;
    }
    
    Elf_Scn*    symscnp = 0;
    Elf_Scn*    strscnp = 0;
    Elf_Scn*    stabscnp = 0;
    Elf_Scn*    stabstrscnp = 0;
    Elf_Scn*    stabs_indxcnp = 0;
    Elf_Scn*    stabstrs_indxcnp = 0;
    Address     txtaddr = 0;
    Address     bssaddr = 0;
    Elf_Scn*    rel_plt_scnp = 0;
    Elf_Scn*    plt_scnp = 0; 
    Elf_Scn*    got_scnp = 0;
    Elf_Scn*    dynsym_scnp = 0;
    Elf_Scn*    dynstr_scnp = 0;
    
    // initialize object (for failure detection)
    code_ptr_ = 0;
    code_off_ = 0;
    code_len_ = 0;
    data_ptr_ = 0;
    data_off_ = 0;
    data_len_ = 0;
      
    // And attempt to parse the ELF data structures in the file....
    // EEL, added one more parameter
    if (!loaded_elf(did_elf, elfp, txtaddr,
		    bssaddr, symscnp, strscnp, stabscnp, stabstrscnp, stabs_indxcnp, stabstrs_indxcnp,
		    rel_plt_scnp,plt_scnp,got_scnp,dynsym_scnp,dynstr_scnp,true)) 
    {
      goto cleanup;
    }

    if(is_elf64()) addressWidth_nbytes = 8;
    
    // find code and data segments....
    find_code_and_data(elfp, txtaddr, bssaddr);
    if (!code_ptr_ || !code_len_) {
      log_printf(err_func_, "no text segment\n");
      goto cleanup;
    }
    if (!data_ptr_ || !data_len_) {
      log_printf(err_func_, "no data segment\n");
      goto cleanup;
    }
    
    // find symbol and string data
    Elf_Data* symdatap = elf_getdata(symscnp, 0);
    Elf_Data* strdatap = elf_getdata(strscnp, 0);
    if (!symdatap || !strdatap) {
      log_elferror(err_func_, "no symbol/string data");
      goto cleanup;
    }
    string      module = "DEFAULT_MODULE";
    string      name   = "DEFAULT_NAME";
    
    // global symbols are put in global_symbols. Later we read the
    // stab section to find the module to where they belong.
    // Experiment : lets try to be a bit more intelligent about
    // how we initially size the global_symbols table.  
    // dictionary_lite takes an initial # of bins (2nd param), 
    // a max bin load (3rd param), and a grow factor (4th param).
    // Leaving aside the grow factor, lets allocate an initial #
    // of bins = nsyms / max bin load.
    
#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif
    pdvector<Symbol> allsymbols;
    parse_symbols(allsymbols, symdatap, strdatap, false, module);
    VECTOR_SORT(allsymbols,symbol_compare);
    fix_zero_function_sizes(allsymbols, 0);
    override_weak_symbols(allsymbols);
    
#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000);
  cout << "parsing/fixing/overriding elf took "<<dursecs <<" msecs" << endl;
#endif

    // dump "allsymbols" into "symbols_" (data member)
    // or "global_symbols" (parameter) according to linkage
    dictionary_hash<string, Symbol> global_symbols(string::hash, allsymbols.size(), 100);
    insert_symbols_static(allsymbols, global_symbols);
    
    // try to resolve the module names of global symbols
    bool found = false;
    // Sun compiler stab.index section 
    fix_global_symbol_modules_static_stab( global_symbols, stabs_indxcnp, stabstrs_indxcnp);

    // STABS format (.stab section)
    if (!found) found = fix_global_symbol_modules_static_stab(
			    global_symbols, stabscnp, stabstrscnp);

    // DWARF format (.debug_info section)
    if (!found) found = fix_global_symbol_modules_static_dwarf(
			    global_symbols, elfp);

    // remaining globals are not associated with a module 
    fix_global_symbol_unknowns_static(global_symbols);
    
    // populate "relocation_table_"
    if(rel_plt_scnp && dynsym_scnp && dynstr_scnp) {
      if (!get_relocation_entries(rel_plt_scnp,dynsym_scnp,dynstr_scnp)) {
	goto cleanup;
      }
    }
    
  } // end binding contour (for "goto cleanup2")
  
 cleanup: 
  {
    /* NOTE: The file should NOT be munmap()ed.  The mapped file is
       used for function parsing (see dyninstAPI/src/symtab.C) */
    if (did_elf) elf_end(elfp);
    if (did_open) close(file_fd_);
  }
}

void Object::load_shared_object() 
{
  Elf  *elfp  = 0;  
  bool  did_open = false;
  bool  did_mmap = false;
  bool  did_elf  = false;
  
  { // binding contour (for "goto cleanup2")

    const char *file = file_.c_str();
    if (mmap_file(file, did_open, did_mmap) == false) {
      char buf[500];
      sprintf(buf, "open/fstat/mmap failed on: %s", file);
      log_perror(err_func_, buf);
      goto cleanup2;
    }
    
    Elf_Scn*    symscnp = 0;
    Elf_Scn*    stabscnp = 0;
    Elf_Scn*    stabstrscnp = 0;
    Elf_Scn*    stabs_indxcnp = 0;
    Elf_Scn*    stabstrs_indxcnp = 0;
    Elf_Scn*    strscnp = 0;
    Address     txtaddr = 0;
    Address     bssaddr = 0;
    Elf_Scn*    rel_plt_scnp = 0;
    Elf_Scn*    plt_scnp = 0; 
    Elf_Scn*    got_scnp = 0;
    Elf_Scn*    dynsym_scnp = 0;
    Elf_Scn*    dynstr_scnp = 0;

    if (!loaded_elf(did_elf, elfp, txtaddr,
		    bssaddr, symscnp, strscnp, stabscnp, stabstrscnp, stabs_indxcnp, stabstrs_indxcnp,
		    rel_plt_scnp, plt_scnp, got_scnp, dynsym_scnp, dynstr_scnp)) 
    {
      goto cleanup2;
    }

    // find code and data segments....
    find_code_and_data(elfp, txtaddr, bssaddr);

    Elf_Data *symdatap = elf_getdata(symscnp, 0);
    Elf_Data *strdatap = elf_getdata(strscnp, 0);
    if (!symdatap || !strdatap) {
      log_elferror(err_func_, "locating symbol/string data");
      goto cleanup2;
    }

    // short module name
    string module = extract_pathname_tail(file_);
    string name   = "DEFAULT_NAME";
#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif
    // build symbol dictionary
    pdvector<Symbol> allsymbols;
    parse_symbols(allsymbols, symdatap, strdatap, true, module);
    VECTOR_SORT(allsymbols,symbol_compare);
    fix_zero_function_sizes(allsymbols, 0);
    override_weak_symbols(allsymbols);
    insert_symbols_shared(allsymbols);

    
#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000);
  cout << "parsing/fixing/overriding/insertion elf took "<<dursecs <<" msecs" << endl;
#endif       
    if(rel_plt_scnp && dynsym_scnp && dynstr_scnp) {
      if(!get_relocation_entries(rel_plt_scnp,dynsym_scnp,dynstr_scnp)) { 
	goto cleanup2;
      }
    }

  } // end binding contour (for "goto cleanup2")
  
 cleanup2: 
  {
    /* NOTE: The file should NOT be munmap()ed.  The mapped file is
       used for function parsing (see dyninstAPI/src/symtab.C) */
    if (did_elf) elf_end(elfp);
    if (did_open) close(file_fd_);
  }
}


static Symbol::SymbolType pdelf_type(int elf_type)
{
  switch (elf_type) {
  case STT_FILE:   return Symbol::PDST_MODULE;
  case STT_OBJECT: return Symbol::PDST_OBJECT;
  case STT_FUNC:   return Symbol::PDST_FUNCTION;
  case STT_NOTYPE: return Symbol::PDST_NOTYPE;
  }
  return Symbol::PDST_UNKNOWN;
}

static Symbol::SymbolLinkage pdelf_linkage(int elf_binding)
{
  switch (elf_binding) {
  case STB_LOCAL:  return Symbol::SL_LOCAL;
  case STB_WEAK:   return Symbol::SL_WEAK;
  case STB_GLOBAL: return Symbol::SL_GLOBAL;
  }
  return Symbol::SL_UNKNOWN;
}

// parse_symbols(): populate "allsymbols"
void Object::parse_symbols(pdvector<Symbol> &allsymbols, 
			   Elf_Data *symdatap, Elf_Data *strdatap,
			   bool shared, string smodule)
{
#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif
  if (is_elf64_) {
#ifndef USES_ELF32_ONLY
    Elf64_Sym *syms = (Elf64_Sym *)symdatap->d_buf;
    unsigned nsyms = symdatap->d_size / sizeof(Elf64_Sym);
    const char *strs = (const char *)strdatap->d_buf;
    for (unsigned i = 0; i < nsyms; i++) {
      // skip undefined symbols
      if (syms[i].st_shndx == SHN_UNDEF) continue;
      int etype = ELF64_ST_TYPE(syms[i].st_info);
      int ebinding = ELF64_ST_BIND(syms[i].st_info);
      
      // resolve symbol elements
      string sname = &strs[syms[i].st_name];
      Symbol::SymbolType stype = pdelf_type(etype);
      Symbol::SymbolLinkage slinkage = pdelf_linkage(ebinding);
      unsigned ssize = syms[i].st_size;
      Address saddr = syms[i].st_value;
      // absolute to relative addressing
#if defined(mips_sgi_irix6_4)
      if (saddr >= base_addr) {
	saddr -= base_addr;
	//assert(shared); // TODO
      }
#endif
      if (stype == Symbol::PDST_UNKNOWN) continue;
      if (slinkage == Symbol::SL_UNKNOWN) continue;    
      
      Symbol newsym(sname, smodule, stype, slinkage, saddr, false, ssize);
      
      // register symbol in dictionary
      if ((etype == STT_FILE) && (ebinding == STB_LOCAL) && 
	  (shared) && (sname == smodule)) 
      {
	symbols_[sname] = newsym; // special case
      } else {
	allsymbols.push_back(newsym); // normal case
      }   
    }
#endif
  } else { // 32-bit ELF

    Elf32_Sym *syms = (Elf32_Sym *)symdatap->d_buf;
    unsigned nsyms = symdatap->d_size / sizeof(Elf32_Sym);
    const char *strs = (const char *)strdatap->d_buf;
    for (unsigned i = 0; i < nsyms; i++) {
      // skip undefined symbols
      if (syms[i].st_shndx == SHN_UNDEF) continue;
      int etype = ELF32_ST_TYPE(syms[i].st_info);
      int ebinding = ELF32_ST_BIND(syms[i].st_info);
      
      // resolve symbol elements
      string sname = &strs[syms[i].st_name];
      Symbol::SymbolType stype = pdelf_type(etype);
      Symbol::SymbolLinkage slinkage = pdelf_linkage(ebinding);
      unsigned ssize = syms[i].st_size;
      Address saddr = syms[i].st_value;
      // absolute to relative addressing
#if defined(mips_sgi_irix6_4)
      if (saddr >= base_addr) {
	saddr -= base_addr;
	//assert(shared); // TODO
      }
#endif
      if (stype == Symbol::PDST_UNKNOWN) continue;
      if (slinkage == Symbol::SL_UNKNOWN) continue;    
      
      Symbol newsym(sname, smodule, stype, slinkage, saddr, false, ssize);
      
      // register symbol in dictionary
      if ((etype == STT_FILE) && (ebinding == STB_LOCAL) && 
	  (shared) && (sname == smodule)) 
      {
	symbols_[sname] = newsym; // special case
      } else {
	allsymbols.push_back(newsym); // normal case
      }   
    }

  }
#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 * 1000);
  cout << "parsing elf took "<<dursecs <<" secs" << endl;
#endif
  //if (is_elf64_) fprintf(stderr, ">>> 64-bit parse_symbols() successful\n");
}

/********************************************************
 *
 * Apparently, some compilers do not fill in the symbol sizes
 *  correctly (in the symbol table) for functions.  Run through
 *  symbol vector allsymbols, and compute ?correct? sizes....
 * This patch to symbol sizes runs through allsymbols && tries
 *  to patch all functions symbols recorded with a size of 0,
 *  or which have been EEL overwritten.
 * Assumes that allsymbols is sorted, with e.g. symbol_compare....
 *
********************************************************/
void Object::fix_zero_function_sizes(pdvector<Symbol> &allsymbols, bool isEEL)
{
    unsigned u, v, nsymbols;

    nsymbols = allsymbols.size();
	unsigned int u_section_idx = 0;
    for (u=0; u < nsymbols; u++) {
        //  If function symbol, and size set to 0, or if the
        //   executable has been EEL rewritten, patch the size
		//
		//  Patch the symbol size to the difference between the address
		//  of this symbol and the next, unless the next symbol is beyond
		//  the boundary of the section to which this symbol belongs.  In
		//  that case, set the size to the difference between the section
		//  end and this symbol.
		// 
        if (allsymbols[u].type() == Symbol::PDST_FUNCTION
               && (isEEL || allsymbols[u].size() == 0)) {

			// find the section to which allsymbols[u] belongs
			// (most likely, it is the section to which allsymbols[u-1] 
			// belonged)
			// Note that this assumes the section headers vector is sorted
			// in increasing order
			//
			while( u_section_idx < allSectionHdrs.size() )
			{
				Address slow = allSectionHdrs[u_section_idx]->pd_addr;
				Address shi = slow + allSectionHdrs[u_section_idx]->pd_size;
#if defined(mips_sgi_irix6_4)
				slow -= get_base_addr();
				shi -= get_base_addr();
#endif
				if( (allsymbols[u].addr() >= slow) &&
					(allsymbols[u].addr() <= shi) )
				{
					// we found u's section
					break;
				}

				// try the next section
				u_section_idx++;
			}
			assert( u_section_idx < allSectionHdrs.size() );


			// search for the next symbol after allsymbols[u]
            v = u+1;
            while (v < nsymbols && allsymbols[v].addr() == allsymbols[u].addr())
			{
                v++;
			}

			unsigned int symSize = 0;
			unsigned int v_section_idx = 0;
            if (v < nsymbols) {

				// find the section to which allsymbols[v] belongs
				// (most likely, it is in the same section as symbol u)
				// Note that this assumes the section headers vector is
				// sorted in increasing order
				while( v_section_idx < allSectionHdrs.size() )
				{
					Address slow = allSectionHdrs[v_section_idx]->pd_addr;
					Address shi = slow + allSectionHdrs[v_section_idx]->pd_size;
#if defined(mips_sgi_irix6_4)
					slow -= get_base_addr();
					shi -= get_base_addr();
#endif
					if( (allsymbols[v].addr() >= slow) &&
						(allsymbols[v].addr() <= shi) )
					{
						// we found v's section
						break;
					}

					// try the next section
					v_section_idx++;
				}
				assert( v_section_idx < allSectionHdrs.size() );

				if( u_section_idx == v_section_idx )
				{
					// symbol u and symbol v are in the same section
					// 
					// take the size of symbol u to be the difference
					// between their two addresses
					//
                    symSize = ((unsigned int)allsymbols[v].addr()) - 
								(unsigned int)allsymbols[u].addr();
				}
            }
			
			if( (v == nsymbols) || (u_section_idx != v_section_idx) )
			{
				// u is the last symbol in its section
				//
				// its size is the distance from its address to the
				// end of its section
				//
				symSize = allSectionHdrs[u_section_idx]->pd_addr + 
							allSectionHdrs[u_section_idx]->pd_size -
#if defined(mips_sgi_irix6_4)
								(allsymbols[u].addr() + get_base_addr());
#else
								allsymbols[u].addr();
#endif
			}

			// update the symbol size
            allsymbols[u].change_size( symSize );
        }
    }
}

/********************************************************
 *  
 *  Run over list of symbols found in a SHARED LIBRARY ONLY -
 *   Override weak AND LOCAL symbol references for which there is also
 *   a global symbol reference, by setting the size on the weak
 *   reference to 0.
 *  Also potentially patches the size on such symbols (note that 
 *   Object::fix_symbol_sizes (above) patches sizes on functions
 *   recorded with a size of 0).  This fixes non-zero sized functions
 *   in the case where 2 functions follow each other in the symbol
 *   table, and the first has a size which would extend into the
 *   second.  WHY IS THIS ONLY DONE FOR SHARED LIBRARIES.... 
 *
 *  Assumes that allsymbols is sorted, with e.g. symbol_compare....
 *
********************************************************/
void Object::override_weak_symbols(pdvector<Symbol> &allsymbols) {
    signed i, nsymbols; // these need to be signed
    u_int next_start;
    int next_size;
    bool i_weak_or_local;
    bool ip1_weak_or_local;

    //cerr << "overriding weak symbols for which there is also a global symbol reference...." << endl;
    nsymbols = allsymbols.size();
    for (i=0; i < nsymbols - 1; i++) {
	if((allsymbols[i].type() == Symbol::PDST_FUNCTION)
		&& (allsymbols[i+1].type() == Symbol::PDST_FUNCTION)) {

	    // where symbol i+1 should start, based on start of symbol i
	    //  and size of symbol i....
	    next_start=allsymbols[i].addr()+allsymbols[i].size();
	
	    // do symbols i and i+1 have weak or local bindings....
	    i_weak_or_local = ((allsymbols[i].linkage() == Symbol::SL_WEAK) ||
			       (allsymbols[i].linkage() == Symbol::SL_LOCAL));
	    ip1_weak_or_local = ((allsymbols[i+1].linkage() == Symbol::SL_WEAK) ||
			       (allsymbols[i+1].linkage() == Symbol::SL_LOCAL));

	    // if the symbols have the same address and one is weak or local
	    // and the other is global keeep the global one
	    if((allsymbols[i].addr() == allsymbols[i+1].addr()) && 
		    (((i_weak_or_local) &&
		      (allsymbols[i+1].linkage() == Symbol::SL_GLOBAL)) || 
                     ((allsymbols[i].linkage() == Symbol::SL_GLOBAL) &&
		      (ip1_weak_or_local)))) {

		if (i_weak_or_local) {
		    allsymbols[i].change_size(0);
		    //cerr << " (type 1) removing symbol " << allsymbols[i];
		} else {
		    allsymbols[i+1].change_size(0);
		    //cerr << " (type 1) removing symbol " << allsymbols[i+1];
		}
	    }
	    // looks like may possibly need to patch size of symbol i
	    //  based on start of symbol i + 1????
	    else if (next_start > allsymbols[i+1].addr() && 
                     allsymbols[i].addr() != allsymbols[i+1].addr()) {
	        next_size = allsymbols[i+1].addr() - allsymbols[i].addr();
		allsymbols[i].change_size(next_size);
		  //cerr << " (type 2) changing symbol size of symbol "
		  //     << allsymbols[i] << "to size " << next_size << endl;
	    }
	}
    }
}

static string find_global_symbol(string name,
              dictionary_hash<string, Symbol> &global_symbols)
{
  string name2;

  // pass #1: unmodified
  name2 = name;
  if (global_symbols.defines(name2)) return name2;

  // pass #2: leading underscore (C)
  name2 = "_" + name;
  if (global_symbols.defines(name2)) return name2;

  // pass #3: trailing underscore (Fortran)
  name2 = name + "_";
  if (global_symbols.defines(name2)) return name2;

  return "";
}

/********************************************************
 *
 * For object files only....
 *   read the .debug_info section to find the module of global symbols
 *   see documents...
 *   - "DWARF Debugging Information Format"
 *   - "A Consumer Libary Interface to DWARF"
 *
 ********************************************************/
#if defined(USES_DWARF_DEBUG)
void pd_dwarf_handler(Dwarf_Error error, Dwarf_Ptr userData)
{
  void (*errFunc)(const char *) = (void (*)(const char *))userData;
  char *dwarf_msg = dwarf_errmsg(error);
  log_printf(errFunc, "DWARF error: %s", dwarf_msg);
}
bool Object::fix_global_symbol_modules_static_dwarf(
       dictionary_hash<string, Symbol> &global_symbols,
       Elf *elfp)
{
  Dwarf_Die die, die2;
  Dwarf_Half tag, tag2;
  char *name, *name2;
  int ret, ret2;
  Dwarf_Unsigned hdr;
  Dwarf_Attribute attr;
  Dwarf_Debug dbg;

  ret = dwarf_elf_init(elfp, DW_DLC_READ, &pd_dwarf_handler, err_func_, &dbg, NULL);
  if (ret != DW_DLV_OK) return false;

  while (dwarf_next_cu_header(dbg, NULL, NULL, NULL, NULL, &hdr, NULL)
	 == DW_DLV_OK) 
  {
    // scan each "compile unit" (CU)
    for (ret = dwarf_siblingof(dbg, NULL, &die, NULL);
	 ret == DW_DLV_OK;
	 ret = dwarf_siblingof(dbg, die, &die, NULL)) 
    {	    
      // sanity check
      dwarf_tag(die, &tag, NULL);
      assert(tag == DW_TAG_compile_unit); 
      
      // module name
      dwarf_diename(die, &name, NULL);
      string module = name;
      //fprintf(stderr, ">>> dwarf_module \"%s\"\n", name);
      //string module2 = extract_pathname_tail(module);
      
      // scan each "debugging information entry" (DIE) in this CU
      for (ret2 = dwarf_child(die, &die2, NULL);
	   ret2 == DW_DLV_OK;
	   ret2 = dwarf_siblingof(dbg, die2, &die2, NULL))
      {
	dwarf_tag(die2, &tag2, NULL);
	switch (tag2) {
	case DW_TAG_subprogram:
	case DW_TAG_inlined_subroutine:
	case DW_TAG_entry_point:
	  // function symbol tags
	case DW_TAG_variable:
	case DW_TAG_constant:
	  // variable symbol tags
	  {
	    // get symbol name
	    dwarf_diename(die2, &name2, NULL);
	    // use "linkage name" if present (matches symbol table)
	    if (dwarf_attr(die2, DW_AT_MIPS_linkage_name, &attr, NULL) 
		== DW_DLV_OK) 
	    {
	      ret2 = dwarf_formstring(attr, &name2, NULL);
	      assert(ret2 == DW_DLV_OK);
	    }
		  
	    // check if symbol defined in global_symbols
	    string gsym = find_global_symbol(name2, global_symbols);
	    if (gsym != "") {
	      assert(global_symbols.defines(gsym));
	      // add symbol with module info
	      Symbol &sym = global_symbols[gsym];
	      symbols_[gsym] = Symbol(sym.name(), module, sym.type(), 
				      sym.linkage(), sym.addr(),
				      sym.kludge(), sym.size());
	    }
	  } break;
	default:
	  /* ignore other entries */
	  break;
	}			
      }
    }
  }

  dwarf_finish(dbg, NULL);  

  return true;
}
#else
// dummy definition for non-DWARF platforms
bool Object::fix_global_symbol_modules_static_dwarf(
       dictionary_hash<string, Symbol> & /*global_symbols*/,
       Elf * /*elfp*/)
{ return false; }
#endif // USES_DWARF_DEBUG

/********************************************************
 *
 * For object files only....
 *  read the .stab section to find the module of global symbols
 *
 ********************************************************/
bool Object::fix_global_symbol_modules_static_stab(
        dictionary_hash<string, Symbol> &global_symbols,
	Elf_Scn* stabscnp, Elf_Scn* stabstrscnp) {
    // Read the stab section to find the module of global symbols.
    // The symbols appear in the stab section by module. A module begins
    // with a symbol of type N_UNDF and ends with a symbol of type N_ENDM.
    // All the symbols in between those two symbols belong to the module.
    Elf_Data* stabdatap = elf_getdata(stabscnp, 0);
    Elf_Data* stabstrdatap = elf_getdata(stabstrscnp, 0);
    if (!stabdatap || !stabstrdatap) {
      return false;
    }

    struct stab_entry *stabsyms = (struct stab_entry *) stabdatap->d_buf;
    unsigned stab_nsyms = stabdatap->d_size / sizeof(struct stab_entry);
    const char *stabstrs = (const char *) stabstrdatap->d_buf;
    string module = "DEFAULT_MODULE";

    // the stabstr contains one string table for each module.
    // stabstr_offset gives the offset from the begining of stabstr of the
    // string table for the current module.
    // stabstr_nextoffset gives the offset for the next module.
    unsigned stabstr_offset = 0;
    unsigned stabstr_nextoffset = 0;

    bool is_fortran = false;  // is the current module fortran code?
    for (unsigned i = 0; i < stab_nsyms; i++) {
	// printf("parsing #%d, %s\n", stabsyms[i].type, &stabstrs[stabstr_offset+stabsyms[i].name]);
        switch(stabsyms[i].type) {
	case N_UNDF: /* start of object file */
/*
#if !defined(i386_unknown_linux2_0) && !defined(mips_sgi_irix6_4) && !defined(ia64_unknown_linux2_4)
	    assert(stabsyms[i].name == 1);
#endif
*/
	    stabstr_offset = stabstr_nextoffset;
	    // stabsyms[i].val has the size of the string table of this module.
	    // We use this value to compute the offset of the next string table.
	    stabstr_nextoffset = stabstr_offset + stabsyms[i].val;
	    module = string(&stabstrs[stabstr_offset+stabsyms[i].name]);
	    break;

	case N_ENDM: /* end of object file */
	    is_fortran = false;
	    module = "DEFAULT_MODULE";
	    break;

	case N_SO: /* compilation source or file name */
  	    if ((stabsyms[i].desc == N_SO_FORTRAN) || (stabsyms[i].desc == N_SO_F90))
	      is_fortran = true;

	    module = string(&stabstrs[stabstr_offset+stabsyms[i].name]);
	    break;

        case N_ENTRY: /* fortran alternate subroutine entry point */
	case N_FUN: /* function */
	case N_GSYM: /* global symbol */
	    // the name string of a function or object appears in the stab 
	    // string table as <symbol name>:<symbol descriptor><other stuff>
	    // where <symbol descriptor> is a one char code.
	    // we must extract the name and descriptor from the string
          {
	    const char *p = &stabstrs[stabstr_offset+stabsyms[i].name];
	    // printf("got %d type, str = %s\n", stabsyms[i].type, p);
            if ((stabsyms[i].type==N_FUN) && (strlen(p)==0)) {
                // GNU CC 2.8 and higher associate a null-named function
                // entry with the end of a function.  Just skip it.
                break;
            }
	    const char *q = strchr(p,':');
	    unsigned len;
	    if (q) {
		len = q - p;
	    } else {
		len = strlen(p);
	    }
	    assert(len > 0);
	    char *sname = new char[len+1];
	    strncpy(sname, p, len);
	    sname[len] = 0;
	    
	    string SymName = string(sname);

	    // q points to the ':' in the name string, so 
	    // q[1] is the symbol descriptor. We must check the symbol descriptor
	    // here to skip things we are not interested in, such as local functions
	    // and prototypes.
	    if (q && (q[1] == SD_GLOBAL_FUN || q[1] == SD_GLOBAL_VAR || stabsyms[i].type==N_ENTRY)) { 
	        bool res = global_symbols.defines(SymName);
	        if (!res && is_fortran) {
                    // Fortran symbols usually appear with an '_' appended in .symtab,
                    // but not on .stab
		    SymName += "_";
		    res = global_symbols.defines(SymName);
	        }

                if (!res) break;

	        Symbol sym = global_symbols[SymName];
	        symbols_[SymName] = Symbol(sym.name(), module,
		    sym.type(), sym.linkage(), sym.addr(),
		    sym.kludge(), sym.size());
	    } else if (symbols_.defines(SymName)) {
		//Set module info for local symbol if not a prototype
		if (!q || (q[1] != SD_PROTOTYPE)) {
		    symbols_[SymName].setModule(module);
		}
	    }
          }
	  break;

	default:
	    /* ignore other entries */
	    break;
	}
    }
    return true;
}


/********************************************************
 * Remaining global symbols have no associated module
 ********************************************************/
void Object::fix_global_symbol_unknowns_static(
	dictionary_hash<string, Symbol> &global_symbols)
{
  pdvector<string> k = global_symbols.keys();
    for (unsigned i2 = 0; i2 < k.size(); i2++) {
      Symbol sym = global_symbols[k[i2]];
      if (!(symbols_.defines(sym.name())))
	symbols_[sym.name()] = sym;
    }
}

/********************************************************
 *
 * Run over allsymbols, and stuff symbols contained according 
 *  to following rules:
 * LOCAL symbols - into (data member) symbols_
 * GLOBAL symbols - into (paramater) global_symbols
 * WEAK symbols - looks like this case isn't handled correctly
 *  for static libraries....
 *
********************************************************/
void Object::insert_symbols_static(pdvector<Symbol> allsymbols,
     dictionary_hash<string, Symbol> &global_symbols)
{
  unsigned nsymbols = allsymbols.size();
#ifdef TIMED_PARSE
   cout << __FILE__ << ":" << __LINE__ << ": stuffing "<<nsymbols 
	 << " symbols into symbols_ dictionary" << endl; 
#endif
  for (unsigned u = 0; u < nsymbols; u++) {
    // We are done with the local symbols. We save the global so that
    // we can get their module from the .stab section.
    if (allsymbols[u].linkage() == Symbol::SL_LOCAL) {
      symbols_[allsymbols[u].name()] = allsymbols[u];
    } else {
      // globals should be unique
      assert(!(global_symbols.defines(allsymbols[u].name()))); 
      global_symbols[allsymbols[u].name()] = allsymbols[u];
    }
  }    
}

/********************************************************
 *
 * Run over allsymbols, and stuff symbols it contains into (data
 *  member) symbols_. 
 *
 * Assumes that all kludges, patches, fixes, hacks, to objects
 *  in allsymbols have already been made, and that it safe to
 *  dump them into symbols_ (data member, instead of stack var....)....
 *
********************************************************/
void Object::insert_symbols_shared(pdvector<Symbol> allsymbols) {
    unsigned i, nsymbols;

    nsymbols = allsymbols.size();
#ifdef TIMED_PARSE
    cout << __FILE__ << ":" << __LINE__ << ": stuffing "<<nsymbols 
	 << " symbols into symbols_ dictionary" << endl; 
#endif
    for (i=0;i<nsymbols;i++) {
	symbols_[allsymbols[i].name()] =
		    Symbol(allsymbols[i].name(), allsymbols[i].module(),
		    allsymbols[i].type(), allsymbols[i].linkage(),
		    allsymbols[i].addr(), allsymbols[i].kludge(),
		    allsymbols[i].size());
    }
}

// find_code_and_data(): populates the following members:
//   code_ptr_, code_off_, code_len_
//   data_ptr_, data_off_, data_len_
void Object::find_code_and_data(Elf *elfp,
				Address txtaddr, 
				Address bssaddr) 
{
  if (is_elf64_) {
#ifndef USES_ELF32_ONLY
    Elf64_Ehdr *ehdrp = elf64_getehdr(elfp);
    Elf64_Phdr *phdrp = elf64_getphdr(elfp);
    for (int i = 0; i < ehdrp->e_phnum; i++) {
      if ((phdrp[i].p_vaddr <= txtaddr) && 
	  (phdrp[i].p_vaddr + phdrp[i].p_memsz >= txtaddr)) {
	if (code_ptr_ == 0 && code_off_ == 0 && code_len_ == 0) {
	  code_ptr_ = (Word *)(void*)&file_ptr_[phdrp[i].p_offset];
	  code_off_ = (Address)phdrp[i].p_vaddr;
	  code_len_ = (unsigned)phdrp[i].p_memsz / sizeof(Word);
	}
      } else if ((phdrp[i].p_vaddr <= bssaddr) && 
		 (phdrp[i].p_vaddr + phdrp[i].p_memsz >= bssaddr)) {
	if (data_ptr_ == 0 && data_off_ == 0 && data_len_ == 0) {
	  data_ptr_ = (Word *)(void *)&file_ptr_[phdrp[i].p_offset];
	  data_off_ = (Address)phdrp[i].p_vaddr;
	  data_len_ = (unsigned)phdrp[i].p_memsz / sizeof(Word);
	}
      }
    }
#endif
  } else { // 32-bit ELF

    Elf32_Ehdr *ehdrp = elf32_getehdr(elfp);
    Elf32_Phdr *phdrp = elf32_getphdr(elfp);
    for (int i = 0; i < ehdrp->e_phnum; i++) {
      if ((phdrp[i].p_vaddr <= txtaddr) && 
	  (phdrp[i].p_vaddr + phdrp[i].p_memsz >= txtaddr)) {
	if (code_ptr_ == 0 && code_off_ == 0 && code_len_ == 0) {
	  code_ptr_ = (Word *)(void*)&file_ptr_[phdrp[i].p_offset];
	  code_off_ = (Address)phdrp[i].p_vaddr;
	  code_len_ = (unsigned)phdrp[i].p_memsz / sizeof(Word);
	}
      } else if ((phdrp[i].p_vaddr <= bssaddr) && 
		 (phdrp[i].p_vaddr + phdrp[i].p_memsz >= bssaddr)) {
	if (data_ptr_ == 0 && data_off_ == 0 && data_len_ == 0) {
	  data_ptr_ = (Word *)(void *)&file_ptr_[phdrp[i].p_offset];
	  data_off_ = (Address)phdrp[i].p_vaddr;
	  data_len_ = (unsigned)phdrp[i].p_memsz / sizeof(Word);
	}
      }
    }

  }
#if defined mips_sgi_irix6_4
    // absolute to relative addressing
    code_off_ -= base_addr;
    data_off_ -= base_addr;
#endif
    //if (is_elf64_) fprintf(stderr, ">>> 64-bit find_code_and_data() successful\n");
}

const char *Object::elf_vaddr_to_ptr(Address vaddr) const
{
  const char *ret = NULL;
  unsigned code_size_ = code_len_ * sizeof(Word);
  unsigned data_size_ = data_len_ * sizeof(Word);

#if defined(mips_sgi_irix6_4)
  vaddr -= base_addr;
#endif

  if (vaddr >= code_off_ && vaddr < code_off_ + code_size_) {
    ret = ((char *)code_ptr_) + (vaddr - code_off_);
  } else if (vaddr >= data_off_ && vaddr < data_off_ + data_size_) {
    ret = ((char *)data_ptr_) + (vaddr - data_off_);
  } 

  return ret;
}

void Object::get_stab_info(void **stabs, int &nstabs, void **stabstr)
{
  // check that file has .stab info
  if (!stab_off_) goto fail;
  if (!stab_size_) goto fail;
  if (!stabstr_off_) goto fail;
  
  *stabs = (void *)(file_ptr_ + stab_off_);
  nstabs = stab_size_ / sizeof(struct stab_entry);
  *stabstr = (void *)(file_ptr_ + stabstr_off_);
  return;

 fail:
    *stabs = NULL;
    nstabs = 0;
    *stabstr = NULL;
    return;
}

Object::Object(const string file, void (*err_func)(const char *))
    : AObject(file, err_func), EEL(false) {
    load_object();
    //dump_state_info(cerr);
}

Object::Object(const string file, const Address /*baseAddr*/, 
	       void (*err_func)(const char *))
    : AObject(file, err_func), EEL(false)  {
    load_shared_object();
    //dump_state_info(cerr);
}

Object::Object(fileDescriptor *desc, Address /*baseAddr*/, void (*err_func)(const char *))
  : AObject(desc->file(), err_func) {
  if (desc->isSharedObject())
    load_shared_object();
  else load_object();
    
}

Object::Object(const Object& obj)
    : AObject(obj), EEL(false) {
    load_object();
}

const Object&
Object::operator=(const Object& obj) {

    (void) AObject::operator=(obj);

    dynsym_addr_ = obj.dynsym_addr_;
    dynstr_addr_ = obj.dynstr_addr_;
    got_addr_ = obj.got_addr_;
    plt_addr_ = obj.plt_addr_;
    plt_size_ = obj.plt_size_;
    plt_entry_size_ = obj.plt_entry_size_;
    rel_plt_addr_ = obj.rel_plt_addr_;
    rel_plt_size_ = obj.rel_plt_size_;
    rel_plt_entry_size_ = obj.rel_plt_entry_size_;
    stab_off_ = obj.stab_off_;
    stab_size_ = obj.stab_size_;
    stabstr_off_ = obj.stabstr_off_;
    relocation_table_  = obj.relocation_table_;
    return *this;
}

Object::~Object() {
}

void Object::log_elferror(void (*pfunc)(const char *), const char* msg) {
    const char* err = elf_errmsg(elf_errno());
    log_printf(pfunc, "%s: %s\n", msg, err ? err : "(bad elf error)");
}

inline bool Object::get_func_binding_table(pdvector<relocationEntry> &fbt) const {
    if(!plt_addr_ || (!relocation_table_.size())) return false;
    fbt = relocation_table_;
    return true;
}

inline bool Object::get_func_binding_table_ptr(const pdvector<relocationEntry> *&fbt) const {
    if(!plt_addr_ || (!relocation_table_.size())) return false;
    fbt = &relocation_table_;
    return true;
}

// stream-based debug output
const ostream &Object::dump_state_info(ostream &s) 
{
  s << "Debugging Information for Object (address) : " << this << endl;
  
  s << " <<begin debugging info for base object>>" << endl;
  AObject::dump_state_info(s);
  s << " <<end debuggingo info for base object>>" << endl;
  
  s << " dynsym_addr_ = " << dynsym_addr_ << endl;
  s << " dynstr_addr_ = " << dynstr_addr_ << endl;
  s << " got_addr_ = " << got_addr_ << endl;
  s << " plt_addr_ = " << plt_addr_ << endl;
  s << " plt_size_ = " << plt_size_ << endl;
  s << " plt_entry_size_ = " << plt_entry_size_ << endl;
  s << " rel_plt_addr_ = " << rel_plt_addr_ << endl; 
  s << " rel_plt_size_ = " << rel_plt_size_ << endl;
  s << " rel_plt_entry_size_ = " << rel_plt_entry_size_ << endl;
  s << " stab_off_ = " << stab_off_ << endl;
  s << " stab_size_ = " << stab_size_ << endl;
  s << " stabstr_off_ = " << stabstr_off_ << endl;

  // and dump the relocation table....
  s << " relocation_table_ = (field seperator :: )" << endl;   
  for (unsigned i=0; i < relocation_table_.size(); i++) {
    s << relocation_table_[i] << " :: "; 
  }
  s << endl;

  return s;
} 


Address Object::getPltSlot(string funcName) const{

	relocationEntry re;
	bool found= false;
	Address offset=0;
	for( unsigned int i = 0; i < relocation_table_.size(); i++ ){
		if(funcName == relocation_table_[i].name() ){
			found = true;
			offset =  relocation_table_[i].rel_addr();
		}

	}
	return offset;	

}


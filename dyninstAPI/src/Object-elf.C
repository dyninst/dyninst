/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
 * $Id: Object-elf.C,v 1.95 2005/10/04 18:10:01 legendre Exp $
 * Object-elf.C: Object class for ELF file format
 ************************************************************************/


#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/symtab.h"

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
#include "dwarf.h"
#include "libdwarf.h"
#endif

#include "util.h"

#if defined(TIMED_PARSE)
#include <sys/time.h>
#endif

// add some space to avoid looking for functions in data regions
#define EXTRA_SPACE 8

#if defined(os_linux) && (defined(arch_x86) || defined(arch_x86_64))
static bool find_catch_blocks(Elf_X &elf, Elf_X_Shdr *eh_frame, Elf_X_Shdr *except_scn,
                              pdvector<ExceptionBlock> &catch_addrs);
#endif

static bool pdelf_check_ehdr(Elf_X &elf)
{
    // Elf header integrity check
    return ((elf.e_type() == ET_EXEC || elf.e_type() == ET_DYN) &&
	    (elf.e_phoff() != 0) && (elf.e_shoff() != 0) &&
	    (elf.e_phnum() != 0) && (elf.e_shnum() != 0)) ;
}

const char *pdelf_get_shnames(Elf_X &elf)
{
    const char *result = NULL;
    size_t shstrndx = elf.e_shstrndx();

    Elf_X_Shdr shstrscn = elf.get_shdr(shstrndx);
    if (shstrscn.isValid()) {
	Elf_X_Data shstrdata = shstrscn.get_data();
	if (shstrdata.isValid())
	    result = shstrdata.get_string();
    }
    return result;
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
	const Elf_X_Shdr *hdr1 = *(const Elf_X_Shdr* const*)v1;
	const Elf_X_Shdr *hdr2 = *(const Elf_X_Shdr* const*)v2;

	return hdr1->sh_addr() - hdr2->sh_addr();
}
}

bool Object::shared()
{
    return shared_;
}

// loaded_elf(): populate elf section pointers
// for EEL rewritten code, also populate "code_*_" members
bool Object::loaded_elf(Elf_X &elf, Address& txtaddr, Address& bssaddr,
			Elf_X_Shdr*& symscnp, Elf_X_Shdr*& strscnp, 
			Elf_X_Shdr*& stabscnp, Elf_X_Shdr*& stabstrscnp, 
			Elf_X_Shdr*& stabs_indxcnp, Elf_X_Shdr*& stabstrs_indxcnp, 
			Elf_X_Shdr*& rel_plt_scnp, Elf_X_Shdr*& plt_scnp, 
			Elf_X_Shdr*& got_scnp, Elf_X_Shdr*& dynsym_scnp, 
			Elf_X_Shdr*& dynstr_scnp,  Elf_X_Shdr*& eh_frame, 
			Elf_X_Shdr*& gcc_except,
			bool
#if defined(mips_sgi_irix6_4)
			a_out  // variable not used on other platforms
#endif
    )
{
    elf = Elf_X(file_fd_, ELF_C_READ);

    // ELF header: sanity check
    if (!elf.isValid() || !pdelf_check_ehdr(elf)) {
	log_elferror(err_func_, "ELF header");
	return false;
    }

    // ".shstrtab" section: string table for section header names
    const char *shnames = pdelf_get_shnames(elf);
    if (shnames == NULL) {
	log_elferror(err_func_, ".shstrtab section");
	return false;
    }

    const char* EDITED_TEXT_NAME = ".edited.text";
    // const char* INIT_NAME        = ".init";
    const char* FINI_NAME        = ".fini";
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
#if ! defined( ia64_unknown_linux2_4 )
    const char* REL_PLT_NAME     = ".rela.plt"; // sparc-solaris
#else  
    const char* REL_PLT_NAME     = ".rela.IA_64.pltoff";
#endif  
    const char* REL_PLT_NAME2    = ".rel.plt";  // x86-solaris
    const char* GOT_NAME         = ".got";
    const char* DYNSYM_NAME      = ".dynsym";
    const char* DYNSTR_NAME      = ".dynstr";
    const char* DATA_NAME        = ".data";
    const char* RO_DATA_NAME     = ".ro_data";  // mips
    const char* DYNAMIC_NAME     = ".dynamic";
    const char* EH_FRAME_NAME    = ".eh_frame";
    const char* EXCEPT_NAME      = ".gcc_except_table";
    // initialize Object members

    text_addr_ = 0; //ccw 23 jan 2002
    text_size_ = 0; //for determining if a mutation
    //falls within the text section 
    //of a shared library

    dynamic_addr_ = 0;
    dynsym_addr_ = 0;
    dynstr_addr_ = 0;
    fini_addr_ = 0;
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
    dwarvenDebugInfo = false;

    txtaddr = 0;

#if defined(TIMED_PARSE)
    struct timeval starttime;
    gettimeofday(&starttime, NULL);
#endif

    Elf_X_Shdr *scnp;

    for (int i = 0; i < elf.e_shnum(); ++i) {
	scnp = new Elf_X_Shdr( elf.get_shdr(i) );
	allSectionHdrs.push_back( scnp );

	// resolve section name
	const char *name = &shnames[scnp->sh_name()];

	// section-specific processing
	if (P_strcmp(name, EDITED_TEXT_NAME) == 0) {
	    // EEL rewritten executable
	    EEL = true;
	    if (txtaddr == 0)
		txtaddr = scnp->sh_addr();
	    code_ptr_ = (Word *)(void*)&file_ptr_[scnp->sh_offset() - EXTRA_SPACE];
	    code_off_ = scnp->sh_addr() - EXTRA_SPACE;
	    code_len_ = scnp->sh_size() + EXTRA_SPACE;
	}
	if (strcmp(name, TEXT_NAME) == 0) {
	    text_addr_ = scnp->sh_addr();
	    text_size_ = scnp->sh_size();

	    if (txtaddr == 0)
		txtaddr = scnp->sh_addr();
	}
	else if (strcmp(name, BSS_NAME) == 0) {
	    bssaddr = scnp->sh_addr();
	}
	else if (strcmp( name, FINI_NAME) == 0) {
	    fini_addr_ = scnp->sh_addr();
	}
	else if (strcmp(name, SYMTAB_NAME) == 0) {
	    symscnp = scnp;
	}
	else if (strcmp(name, STRTAB_NAME) == 0) {
	    strscnp = scnp;
	} else if (strcmp(name, STAB_INDX_NAME) == 0) {
	    stabs_indxcnp = scnp;
	    stab_indx_off_ = scnp->sh_offset();
	    stab_indx_size_ = scnp->sh_size();
	} else if (strcmp(name, STABSTR_INDX_NAME) == 0) {
	    stabstrs_indxcnp = scnp;
	    stabstr_indx_off_ = scnp->sh_offset();
	} else if (strcmp(name, STAB_NAME) == 0) {
	    stabscnp = scnp;
	    stab_off_ = scnp->sh_offset();
	    stab_size_ = scnp->sh_size();
	} else if (strcmp(name, STABSTR_NAME) == 0) {
	    stabstrscnp = scnp;
	    stabstr_off_ = scnp->sh_offset();
	} else if ((strcmp(name, REL_PLT_NAME) == 0) || 
		   (strcmp(name, REL_PLT_NAME2) == 0)) {
	    rel_plt_scnp = scnp;
	    rel_plt_addr_ = scnp->sh_addr();
	    rel_plt_size_ = scnp->sh_size();
	    rel_plt_entry_size_ = scnp->sh_entsize();
	}
	else if (strcmp(name, PLT_NAME) == 0) {
	    plt_scnp = scnp;
	    plt_addr_ = scnp->sh_addr();
	    plt_size_ = scnp->sh_size();
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
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
	    plt_entry_size_ = scnp->sh_entsize();
#endif // defined(i386_unknown_linux2_0) || defined(x86_64_unknown_linux2_4)
	}
	else if (strcmp(name, GOT_NAME) == 0) {
	    got_scnp = scnp;
	    got_addr_ = scnp->sh_addr();
	    got_size_ = scnp->sh_size();
	    if (!bssaddr) bssaddr = scnp->sh_addr();
	}
	else if (strcmp(name, DYNSYM_NAME) == 0) {
	    dynsym_scnp = scnp;
	    dynsym_addr_ = scnp->sh_addr();
	}
	else if (strcmp(name, DYNSTR_NAME) == 0) {
	    dynstr_scnp = scnp;
	    dynstr_addr_ = scnp->sh_addr();
	}
	else if (strcmp(name, DATA_NAME) == 0) {
	    if (!bssaddr) bssaddr = scnp->sh_addr();
	}
	else if (strcmp(name, RO_DATA_NAME) == 0) {
	    if (!bssaddr) bssaddr = scnp->sh_addr();
	}
	else if (strcmp(name, ".debug_info") == 0) {
	    dwarvenDebugInfo = true;
	}
	else if (strcmp(name, EH_FRAME_NAME) == 0) {
	    eh_frame = scnp;
	}
	else if (strcmp(name, EXCEPT_NAME) == 0) {
	    gcc_except = scnp;
	}

//TODO clean up this. it is ugly
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_nt4_0) 
	else if (strcmp(name, DYNAMIC_NAME) == 0) {
	    dynamic_addr_ = scnp->sh_addr();
	}
#endif

#if defined(arch_ia64)
	else if (strcmp(name, DYNAMIC_NAME) == 0) {
	    Elf_X_Data data = scnp->get_data();
	    Elf_X_Dyn dyns = data.get_dyn();
	    for (unsigned i = 0; i < dyns.count(); ++i) {
		switch(dyns.d_tag(i)) {
		case DT_PLTGOT:
		    this->gp = dyns.d_ptr(i);
		    break;

		default:
		    break;
		} // switch
	    } // for
	}
#endif /* ia64_unknown_linux2_4 */
    }

    if(!symscnp || !strscnp) {
	if(dynsym_scnp && dynstr_scnp){
	    symscnp = dynsym_scnp;
	    strscnp = dynstr_scnp;
	}
    }

    loadAddress_ = 0x0;
#if defined(os_linux)
    /**
     * If the virtual address of the first PT_LOAD element in the
     * program table is 0, Linux loads the shared object into any
     * free spot into the address space.  If the virtual address is
     * non-zero, it gets loaded only at that address.
     **/
    for (unsigned i = 0; i < elf.e_phnum() && !loadAddress_; ++i) {
	Elf_X_Phdr phdr = elf.get_phdr(i);

	if (phdr.p_type() == PT_LOAD) {
	    loadAddress_ = phdr.p_vaddr();
	    break;
	}
    }
#endif  
  

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

  //if (addressWidth_nbytes == 8) bperr( ">>> 64-bit loaded_elf() successful\n");
  return true;
}

bool Object::is_offset_in_plt(Address offset) const
{
  return (offset > plt_addr_ && offset < plt_addr_ + plt_size_);
}

bool Object::get_relocation_entries( Elf_X_Shdr *&rel_plt_scnp,
				     Elf_X_Shdr *&dynsym_scnp, 
				     Elf_X_Shdr *&dynstr_scnp )
{
    if (rel_plt_size_ && rel_plt_addr_) {
	Elf_X_Data reldata = rel_plt_scnp->get_data();
	Elf_X_Data symdata = dynsym_scnp->get_data();
	Elf_X_Data strdata = dynstr_scnp->get_data();

	if( reldata.isValid() && symdata.isValid() && strdata.isValid() ) {
	    Address next_plt_entry_addr = plt_addr_;

#if defined( ia64_unknown_linux2_4 ) 
	    unsigned int functionDescriptorCount = ( rel_plt_size_ / rel_plt_entry_size_ );
	    /* The IA-64 PLT contains 1 special entry, and two entries for each functionDescriptor
	       in the function descriptor table (.IA_64.pltoff, whose count we're deriving from
	       its relocation entries).  These entries are in two groups, the second of which is
	       sometimes separated from the first by 128 bits of zeroes.  The entries in the latter group
	       are call targets that makes an indirect jump using an FD table entry.  If the
	       function they want to call hasn't been linked yet, the FD will point to one of 
	       former group's entries, which will then jump to the first, special entry, in order
	       to invoke the linker.  The linker, after doing the link, will rewrite the FD to
	       point directly to the requested function. 

	       As we're only interested in call targets, skip the first group and the buffer entirely,
	       if it's present.
	    */
	    next_plt_entry_addr += (0x10 * 3) + (functionDescriptorCount * 0x10);

	    unsigned long long * bufferPtr = (unsigned long long*)elf_vaddr_to_ptr( next_plt_entry_addr );
	    if( bufferPtr[0] == 0 && bufferPtr[1] == 0 ) { next_plt_entry_addr += 0x10; }

#elif defined(i386_unknown_solaris2_5) \
   || defined(i386_unknown_linux2_0) \
   || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
	    next_plt_entry_addr += plt_entry_size_;  // 1st PLT entry is special
#else
	    next_plt_entry_addr += 4*(plt_entry_size_); //1st 4 entries are special
#endif

	    Elf_X_Sym sym = symdata.get_sym();
	    Elf_X_Rel rel = reldata.get_rel();
	    Elf_X_Rela rela = reldata.get_rela();
	    const char *strs = strdata.get_string();

	    if (sym.isValid() && (rel.isValid() || rela.isValid()) && strs) {
		/* Iterate over the entries. */
		for( u_int i = 0; i < (rel_plt_size_/rel_plt_entry_size_); ++i ) {
		    long offset;
		    long index;

		    switch (reldata.d_type()) {
		    case ELF_T_REL:
			offset = rel.r_offset(i);
			index = rel.R_SYM(i);
			break;

		    case ELF_T_RELA:
			offset = rela.r_offset(i);
			index = rela.R_SYM(i);
			break;

		    default:
			// We should never reach this case.
			return false;
		    };

		    // /* DEBUG */ fprintf( stderr, "%s: relocation information for target 0x%lx\n", __FUNCTION__, next_plt_entry_addr );
		    relocationEntry re( next_plt_entry_addr, offset, pdstring( &strs[ sym.st_name(index) ] ) );
		    relocation_table_.push_back(re);

#if ! defined( ia64_unknown_linux2_4 )
		    next_plt_entry_addr += plt_entry_size_;
#else
		    /* IA-64 headers don't declare a size, because it varies. */
		    next_plt_entry_addr += 0x20;
#endif /* ia64_unknown_linux2_4 */
		}
		return true;
	    }
	}
    }
    return false;
}

// map object file into memory
// populates: file_fd_, file_size_, file_ptr_
bool Object::mmap_file(const char *file, 
		       bool &did_open, bool &did_mmap)
{
  fileName = strdup( file );  assert( fileName != NULL );
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
    shared_ = false;
    Elf_X  elf;
    bool  did_open = false;
    bool  did_mmap = false;

    Elf_X_Shdr *symscnp = 0;
    Elf_X_Shdr *strscnp = 0;
    Elf_X_Shdr *stabscnp = 0;
    Elf_X_Shdr *stabstrscnp = 0;
    Elf_X_Shdr *stabs_indxcnp = 0;
    Elf_X_Shdr *stabstrs_indxcnp = 0;
    Address txtaddr = 0;
    Address bssaddr = 0;
    Elf_X_Shdr *rel_plt_scnp = 0;
    Elf_X_Shdr *plt_scnp = 0; 
    Elf_X_Shdr *got_scnp = 0;
    Elf_X_Shdr *dynsym_scnp = 0;
    Elf_X_Shdr *dynstr_scnp = 0;
    Elf_X_Shdr *eh_frame_scnp = 0;
    Elf_X_Shdr *gcc_except = 0;

    { // binding contour (for "goto cleanup")

	const char *file = file_.c_str();
	if (mmap_file(file, did_open, did_mmap) == false) {
	    char buf[500];
	    sprintf(buf, "open/fstat/mmap failed on: %s", file);
	    log_perror(err_func_, buf);
	    goto cleanup;
	}

	// initialize object (for failure detection)
	code_ptr_ = 0;
	code_off_ = 0;
	code_len_ = 0;
	data_ptr_ = 0;
	data_off_ = 0;
	data_len_ = 0;

	// initialize "valid" regions of code and data segments
	code_vldS_ = (Address) -1;
	code_vldE_ = 0;
	data_vldS_ = (Address) -1;
	data_vldE_ = 0;

	// And attempt to parse the ELF data structures in the file....
	// EEL, added one more parameter
	if (!loaded_elf(elf, txtaddr, bssaddr, symscnp, strscnp,
			stabscnp, stabstrscnp, stabs_indxcnp, stabstrs_indxcnp,
			rel_plt_scnp,plt_scnp,got_scnp,dynsym_scnp,
			dynstr_scnp,eh_frame_scnp,gcc_except, true)) {
	    goto cleanup;
	}
	addressWidth_nbytes = elf.wordSize();

	// find code and data segments....
	find_code_and_data(elf, txtaddr, bssaddr);
	if (!code_ptr_ || !code_len_) {
	    bpfatal( "no text segment\n");
	    goto cleanup;
	}
	if (!data_ptr_ || !data_len_) {
	    bpfatal( "no data segment\n");
	    goto cleanup;
	}

	get_valid_memory_areas(elf);
    
	// find symbol and string data
	Elf_X_Data symdata = symscnp->get_data();
	Elf_X_Data strdata = strscnp->get_data();
	if (!symdata.isValid() || !strdata.isValid()) {
	    log_elferror(err_func_, "no symbol/string data");
      
	    //goto cleanup;
	}
	pdstring module = "DEFAULT_MODULE";
	pdstring name   = "DEFAULT_NAME";

#if defined(os_linux) && (defined(arch_x86) || defined(arch_x86_64))
	if (eh_frame_scnp != 0 && gcc_except != 0) {
	    find_catch_blocks(elf, eh_frame_scnp, gcc_except, catch_addrs_);
	}
#endif

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
	parse_symbols(allsymbols, symdata, strdata, false, module);
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
	insert_symbols_static(allsymbols);

	// try to resolve the module names of global symbols
	// Sun compiler stab.index section 
	fix_global_symbol_modules_static_stab(stabs_indxcnp, stabstrs_indxcnp);

	// STABS format (.stab section)
	fix_global_symbol_modules_static_stab(stabscnp, stabstrscnp);

	// DWARF format (.debug_info section)
	fix_global_symbol_modules_static_dwarf(elf);

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

	if (elf.isValid()) elf.end();
	if (did_open) P_close(file_fd_);
    }
}

void Object::load_shared_object() 
{
    shared_ = true;
    Elf_X elf;
    bool did_open = false;
    bool did_mmap = false;

    Elf_X_Shdr *symscnp = 0;
    Elf_X_Shdr *strscnp = 0;
    Elf_X_Shdr *stabscnp = 0;
    Elf_X_Shdr *stabstrscnp = 0;
    Elf_X_Shdr *stabs_indxcnp = 0;
    Elf_X_Shdr *stabstrs_indxcnp = 0;
    Address txtaddr = 0;
    Address bssaddr = 0;
    Elf_X_Shdr *rel_plt_scnp = 0;
    Elf_X_Shdr *plt_scnp = 0; 
    Elf_X_Shdr *got_scnp = 0;
    Elf_X_Shdr *dynsym_scnp = 0;
    Elf_X_Shdr *dynstr_scnp = 0;
    Elf_X_Shdr *eh_frame_scnp = 0;
    Elf_X_Shdr *gcc_except = 0;

    { // binding contour (for "goto cleanup2")

	const char *file = file_.c_str();
	if (mmap_file(file, did_open, did_mmap) == false) {
	    char buf[500];
	    sprintf(buf, "open/fstat/mmap failed on: %s", file);
	    log_perror(err_func_, buf);
	    goto cleanup2;
	}

	// initialize "valid" regions of code and data segments
	code_vldS_ = (Address) -1;
	code_vldE_ = 0;
	data_vldS_ = (Address) -1;
	data_vldE_ = 0;

	if (!loaded_elf(elf, txtaddr, bssaddr, symscnp, strscnp,
			stabscnp, stabstrscnp, stabs_indxcnp, stabstrs_indxcnp,
			rel_plt_scnp, plt_scnp, got_scnp, dynsym_scnp,
			dynstr_scnp, eh_frame_scnp, gcc_except))
	    goto cleanup2;

	addressWidth_nbytes = elf.wordSize();

	// find code and data segments....
	find_code_and_data(elf, txtaddr, bssaddr);

	get_valid_memory_areas(elf);

	Elf_X_Data symdata = symscnp->get_data();
	Elf_X_Data strdata = strscnp->get_data();
	if (!symdata.isValid() || !strdata.isValid()) {
	    log_elferror(err_func_, "locating symbol/string data");
	    goto cleanup2;
	}

	// short module name
	pdstring module = file;
	pdstring name   = "DEFAULT_NAME";

#if defined(os_linux) && (defined(arch_x86) || defined(arch_x86_64))
	if (eh_frame_scnp != 0 && gcc_except != 0) {
	    find_catch_blocks(elf, eh_frame_scnp, gcc_except, catch_addrs_);
	}
#endif

#if defined(TIMED_PARSE)
	struct timeval starttime;
	gettimeofday(&starttime, NULL);
#endif
	// build symbol dictionary
	pdvector<Symbol> allsymbols;
	parse_symbols(allsymbols, symdata, strdata, true, module);
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
	if (rel_plt_scnp && dynsym_scnp && dynstr_scnp) {
	    if (!get_relocation_entries(rel_plt_scnp,dynsym_scnp,dynstr_scnp)) { 
		goto cleanup2;
	    }
	}

    } // end binding contour (for "goto cleanup2")

  cleanup2: 
    {
	/* NOTE: The file should NOT be munmap()ed.  The mapped file is
	   used for function parsing (see dyninstAPI/src/symtab.C) */
	if (elf.isValid()) elf.end();
	if (did_open) P_close(file_fd_);
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


//============================================================================

#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/inst.h"
//#include "dyninstAPI/src/instPoint.h" // includes instPoint-x86.h
//#include "dyninstAPI/src/instP.h" // class returnInstance
//#include "dyninstAPI/src/rpcMgr.h"

//linear search
bool lookUpSymbol( pdvector< Symbol >& allsymbols, Address& addr )
{
    for( unsigned i = 0; i < allsymbols.size(); i++ )
    {
        if( allsymbols[ i ].addr() == addr )
        {
            return true;
        }
    }
    return false;
}

bool lookUpAddress( pdvector< Address >& jumpTargets, Address& addr )
{
    for( unsigned i = 0; i < jumpTargets.size(); i++ )
    {
        if( jumpTargets[ i ] == addr )
        {
            return true;
        }
    }
    return false;
}

/******************************************************************************
findMain: we parse _start for the address of main.  _start pushes the address 
          of main before a call to libc_start_main. we locate the push 
          instruction and return its operand

assumptions: (address of _start) == (address of .text)
******************************************************************************/
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_nt4_0)
void Object::findMain( pdvector< Symbol > &allsymbols )
{
    //TODO add function to get push operand to machine dependent files

    bool foundMain = false;
    bool foundStart = false;
    bool foundFini = false;
    //check if 'main' is in allsymbols
    for( unsigned i = 0; i < allsymbols.size(); i++ )
    {
        if( allsymbols[ i ].name() == "main" ||
            allsymbols[ i ].name() == "_main"   )
        {
            foundMain = true;
        }
        else if( allsymbols[ i ].name() == "_start" )
        {
            foundStart = true;
        }
        else if ( allsymbols[ i ].name() == "_fini" )
        {
            foundFini = true;
        }
    }
    
    if( !foundMain )
    {
        //find and add main to allsymbols
        const unsigned char* p;
        p = ( const unsigned char* )elf_vaddr_to_ptr( text_addr_ );
        
        const int pushCodeSize = 1;
        
        instruction insn;
        insn.setInstruction( p );
    
        while( !insn.isCall() )
        {
            p += insn.size();
            insn.setInstruction( p );
        }
        p -= insn.size() - pushCodeSize;
        
        Address mainAddress =  *( const Address* )p;

        logLine( "No main symbol found: creating symbol for main\n" );
    
        if( mainAddress >= plt_addr_ && mainAddress < plt_addr_ + plt_size_ )
        {
            logLine( "No static symbol for function main\n" );
    
            Symbol newSym("DYNINST_pltMain","DEFAULT_MODULE",
                          Symbol::PDST_FUNCTION,
                          Symbol::SL_GLOBAL, mainAddress, 0, UINT_MAX );
        
            allsymbols.push_back( newSym );
        }
        else
        {
            Symbol newSym( "main", "DEFAULT_MODULE", Symbol::PDST_FUNCTION,
                           Symbol::SL_GLOBAL, mainAddress, 0, UINT_MAX );
            allsymbols.push_back( newSym );
        }
    }
    
    if( !foundStart )
    {
        Symbol startSym( "_start", "DEFAULT_MODULE", Symbol::PDST_FUNCTION,
                   Symbol::SL_GLOBAL, text_addr_, 0, UINT_MAX );
    
        cout << "sim for start!" << endl;
        
        allsymbols.push_back( startSym );
    }
    if( !foundFini )
    {
        Symbol finiSym( "_fini", "DEFAULT_MODULE", Symbol::PDST_FUNCTION,
                        Symbol::SL_GLOBAL, fini_addr_, 0, UINT_MAX );
        allsymbols.push_back( finiSym );
    }
}

#endif


/*****************************************************************************
findDynamic(): looks for "_DYNAMIC" in allsymbols. if not present create
               a symbol for "_DYNAMIC" using dynamic_addr_
******************************************************************************/
Address Object::findDynamic( pdvector< Symbol > &allsymbols )
{
    for( unsigned i = 0; i < allsymbols.size(); i++ )
    {
	if( allsymbols[ i ].name() == "_DYNAMIC" )
	{
	    return allsymbols[ i ].addr();
	}
    }
    
    logLine( "No _DYNAMIC symbol found: creating symbol for _DYNAMIC\n" );
    Symbol newSym( "_DYNAMIC", "DEFAULT_MODULE", Symbol::PDST_OBJECT,
		   Symbol::SL_GLOBAL, dynamic_addr_, 0, 0 );
    allsymbols.push_back( newSym );

    return dynamic_addr_;
}

//utitility function to print vector of symbols
void printSyms( pdvector< Symbol >& allsymbols )
{
    for( unsigned i = 0; i < allsymbols.size(); i++ )
    {
	if( allsymbols[ i ].type() != Symbol::PDST_FUNCTION )
	{
	    continue;
	}
	cout << allsymbols[ i ] << endl;
    } 
} 



// parse_symbols(): populate "allsymbols"
void Object::parse_symbols(pdvector<Symbol> &allsymbols, 
			   Elf_X_Data &symdata, Elf_X_Data &strdata,
			   bool shared, pdstring smodule)
{
#if defined(TIMED_PARSE)
    struct timeval starttime;
    gettimeofday(&starttime, NULL);
#endif

    Elf_X_Sym syms = symdata.get_sym();
    const char *strs = strdata.get_string();
    for (unsigned i = 0; i < syms.count(); i++) {
	// skip undefined symbols
	if (syms.st_shndx(i) == SHN_UNDEF) continue;
	int etype = syms.ST_TYPE(i);
	int ebinding = syms.ST_BIND(i);

	// resolve symbol elements
	pdstring sname = &strs[ syms.st_name(i) ];
	Symbol::SymbolType stype = pdelf_type(etype);
	Symbol::SymbolLinkage slinkage = pdelf_linkage(ebinding);
	unsigned ssize = syms.st_size(i);
	Address saddr = syms.st_value(i);

	if (stype == Symbol::PDST_UNKNOWN) continue;
	if (slinkage == Symbol::SL_UNKNOWN) continue;

	Symbol newsym(sname, smodule, stype, slinkage, saddr, false, ssize);

	// register symbol in dictionary
	if ((etype == STT_FILE) && (ebinding == STB_LOCAL) && 
	    (shared) && (sname == extract_pathname_tail(smodule))) {

	    // symbols_[sname] = newsym; // special case
	    symbols_[sname].push_back( newsym );
	} else {
	    allsymbols.push_back(newsym); // normal case
	}
    }

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_nt4_0) 
  
    if( dynamic_addr_ ) {
	findDynamic( allsymbols );
    }

    if( !shared ) {
	findMain( allsymbols );
    }
#endif

#if defined(TIMED_PARSE)
    struct timeval endtime;
    gettimeofday(&endtime, NULL);
    unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
    unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
    unsigned long difftime = lendtime - lstarttime;
    double dursecs = difftime/(1000 * 1000);
    cout << "parsing elf took "<<dursecs <<" secs" << endl;
#endif
    //if (addressWidth_nbytes == 8) fprintf(stderr, ">>> 64-bit parse_symbols() successful\n");
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
          && (isEEL || allsymbols[u].size() == 0) )
      {
			// find the section to which allsymbols[u] belongs
			// (most likely, it is the section to which allsymbols[u-1] 
			// belonged)
			// Note that this assumes the section headers vector is sorted
			// in increasing order
			//
			while( u_section_idx < allSectionHdrs.size() )
			{
				Address slow = allSectionHdrs[u_section_idx]->sh_addr();
				Address shi = slow + allSectionHdrs[u_section_idx]->sh_size();
#if defined(mips_sgi_irix6_4)
				slow -= get_base_addr();
				shi -= get_base_addr();
#endif
				if( (allsymbols[u].addr() >= slow) &&
                (allsymbols[u].addr() < shi) )
				{
					// we found u's section
					break;
				}

				// try the next section
				u_section_idx++;
			}

         //Some platforms (Fedora Core 2) thought it would be really 
         // funny if they stuck size 0 symbols inbetween sections.  
         // we'll just delete the symbol in this case.
         if (u_section_idx == allSectionHdrs.size())
         {
            //Delete item u
            for (unsigned i = u; i < allsymbols.size()-1; i++)
               allsymbols[u] = allsymbols[u+1];
            allsymbols.pop_back();
            //Search next item
            u--; nsymbols--;
            continue;
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
					Address slow = allSectionHdrs[v_section_idx]->sh_addr();
					Address shi = slow + allSectionHdrs[v_section_idx]->sh_size();
#if defined(mips_sgi_irix6_4)
					slow -= get_base_addr();
					shi -= get_base_addr();
#endif
					if( (allsymbols[v].addr() >= slow) &&
                   (allsymbols[v].addr() < shi) )
					{
						// we found v's section
						break;
					}

					// try the next section
					v_section_idx++;
				}

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
				symSize = allSectionHdrs[u_section_idx]->sh_addr() + 
               allSectionHdrs[u_section_idx]->sh_size() -
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

static pdstring find_symbol(pdstring name,
              dictionary_hash<pdstring, pdvector< Symbol > > &symbols)
{
  pdstring name2;

  // pass #1: unmodified
  name2 = name;
  if (symbols.defines(name2)) return name2;

  // pass #2: leading underscore (C)
  name2 = "_" + name;
  if (symbols.defines(name2)) return name2;

  // pass #3: trailing underscore (Fortran)
  name2 = name + "_";
  if (symbols.defines(name2)) return name2;

  return "";
}

// Insert sym into syms. If syms already contains another symbol with the
// same name, inserts sym under a versioned name (,v%d appended to the original
// name)
static void insertUniqdSymbol(const Symbol &sym,
                              dictionary_hash<pdstring, pdvector< Symbol > > *syms,
                              dictionary_hash<Address, pdstring> *namesByAddr)
{
    Address symAddr = sym.addr();
    const pdstring & symName = sym.name();
    
	/* The old version of this function generated unique names
	   for conflicting symbols, favoring globally-defined symbols
	   over local ones.  This is no longer necessary. */	   
	(*syms)[symName].push_back( sym );

	/* This is stupid (because it's legal and normal for more than
	   one symbol to point at the same address), but duplicates 
	   what happens below.  The look up by name prefers the global 
	   symbols to the local ones, but the look up by address 
	   favors the last-defined. */
	(*namesByAddr)[symAddr] = symName;
		
#if 0
    const pdstring &symName = sym.name();
    Address symAddr = sym.addr();

    if (syms->defines(symName)) {
        Symbol other = syms->get(symName);
        if (sym.linkage() == Symbol::SL_GLOBAL &&
            other.linkage() != Symbol::SL_GLOBAL) {
            // syms contains a local symbol with the same name. Let's
            // replace it with ours (global).
            (*syms)[symName] = sym;
	    (*namesByAddr)[symAddr] = symName;
            // We then re-add the original one under a different name
            insertUniqdSymbol(other, syms, namesByAddr);
        }
        else {
            // Let's make up a name in the form oldname,version. We'll
            // keep incrementing version until we get a unique name
            unsigned versionId = 1;
            pdstring uniqName;
            do {
	      uniqName = symName + ",v" + pdstring(versionId++);
            } while (syms->defines(uniqName));
        
            Symbol modified(uniqName, sym.module(), sym.type(), sym.linkage(),
                            sym.addr(), sym.kludge(), sym.size());
            (*syms)[uniqName] = modified;
	    (*namesByAddr)[symAddr] = uniqName;
        }
    }
    else {
	(*syms)[symName] = sym;
	(*namesByAddr)[symAddr] = symName;
    }
#endif /* 0 */
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

void pd_dwarf_handler(Dwarf_Error error, Dwarf_Ptr /*userData*/)
{
  if (error == NULL)
    return;

  char *dwarf_msg = dwarf_errmsg(error);
  bperr( "DWARF error: %s", dwarf_msg);
}

Dwarf_Signed declFileNo = 0;
char ** declFileNoToName = NULL;
void fixSymbolsInModule( Dwarf_Debug dbg, pdstring & moduleName, Dwarf_Die dieEntry, dictionary_hash< pdstring, pdvector< Symbol > > & symbols, dictionary_hash< Address, pdstring > & symbolNamesByAddr ) {
	start: Dwarf_Half dieTag;
	int status = dwarf_tag( dieEntry, & dieTag, NULL );
	assert( status == DW_DLV_OK );

	pdstring useModuleName = moduleName;

	/* For debugging. */
	Dwarf_Off dieOffset;
	status = dwarf_die_CU_offset( dieEntry, & dieOffset, NULL );
	assert( status == DW_DLV_OK );

	switch( dieTag ) {
		case DW_TAG_subprogram: 
		case DW_TAG_entry_point: {
			/* Let's try ignoring artificial entries, hmm-kay? */
			Dwarf_Bool isArtificial;
			status = dwarf_hasattr( dieEntry, DW_AT_artificial, & isArtificial, NULL );
			assert( status == DW_DLV_OK );

			if( isArtificial ) { break; }

			/* Only entries with a PC must be defining declarations.  However,
			   to avoid trying to decide in which module a DW_TAG_inlined_subroutine's
			   DW_AT_abstract_origin belongs, we just insert the abstract origin itself. */
			Dwarf_Bool hasLowPC;
			status = dwarf_hasattr( dieEntry, DW_AT_low_pc, & hasLowPC, NULL );
			assert( status == DW_DLV_OK );

			Dwarf_Bool isAbstractOrigin;
			status = dwarf_hasattr( dieEntry, DW_AT_inline, & isAbstractOrigin, NULL );
			assert( status == DW_DLV_OK );

			if( ! hasLowPC && ! isAbstractOrigin ) { break; }

			bool isDeclaredNotInlined = false;
			/* Inline functions are "uninstrumentable", so leave them off the where axis. */
			if( isAbstractOrigin ) {
				Dwarf_Attribute inlineAttribute;
				status = dwarf_attr( dieEntry, DW_AT_inline, & inlineAttribute, NULL );
				assert( status == DW_DLV_OK );

				Dwarf_Unsigned inlineTag;
				status = dwarf_formudata( inlineAttribute, & inlineTag, NULL );
				assert( status == DW_DLV_OK );

				if( inlineTag == DW_INL_inlined || inlineTag == DW_INL_declared_inlined ) { break; }
				if( inlineTag == DW_INL_declared_not_inlined ) { isDeclaredNotInlined = true; }
				}

			/* If a DIE has a specification, the specification has its name
			   and (optional) linkage name.  */
			Dwarf_Attribute specificationAttribute;
			status = dwarf_attr( dieEntry, DW_AT_specification, & specificationAttribute, NULL );
			assert( status != DW_DLV_ERROR );

			Dwarf_Die nameEntry = dieEntry;
			if( status == DW_DLV_OK ) {
				Dwarf_Off specificationOffset;
				status = dwarf_global_formref( specificationAttribute, & specificationOffset, NULL );
				assert( status == DW_DLV_OK );

				status = dwarf_offdie( dbg, specificationOffset, & nameEntry, NULL );
				assert( status == DW_DLV_OK );
				} /* end if the DIE has a specification. */

			/* Ignore artificial entries. */
			status = dwarf_hasattr( nameEntry, DW_AT_artificial, & isArtificial, NULL );
			assert( status == DW_DLV_OK );
			if( isArtificial ) { break; }

			/* What's its name? */
			char * dieName = NULL;
			status = dwarf_diename( nameEntry, & dieName, NULL );
			assert( status != DW_DLV_ERROR );

			/* Prefer the linkage (symbol table) name. */
			Dwarf_Attribute linkageNameAttribute;
			status = dwarf_attr( nameEntry, DW_AT_MIPS_linkage_name, & linkageNameAttribute, NULL );
			assert( status != DW_DLV_ERROR );

			bool hasLinkageName = false;
			if( status == DW_DLV_OK ) {
				dwarf_dealloc( dbg, dieName, DW_DLA_STRING );
				status = dwarf_formstring( linkageNameAttribute, & dieName, NULL );
				assert( status == DW_DLV_OK );
				hasLinkageName = true;
				}

			/* Anonymous functions are useless to us. */
			if( dieName == NULL ) { break; }

			/* Try to find the corresponding global symbol name. */
			Dwarf_Die declEntry = nameEntry;
			pdstring globalSymbol = find_symbol( dieName, symbols );
			if( globalSymbol == "" && ! hasLinkageName && isDeclaredNotInlined ) {
				/* Then scan forward for its concrete instance. */
				Dwarf_Die siblingDie = dieEntry;
				while( true ) {
					status = dwarf_siblingof( dbg, siblingDie, & siblingDie, NULL );
					assert( status != DW_DLV_ERROR );
					if( status == DW_DLV_NO_ENTRY ) { break; }

					Dwarf_Attribute abstractOriginAttr;
					status = dwarf_attr( siblingDie, DW_AT_abstract_origin, & abstractOriginAttr, NULL );
					assert( status != DW_DLV_ERROR );

					/* Is its abstract origin the current dieEntry? */
					if( status == DW_DLV_OK ) {
						Dwarf_Off abstractOriginOffset;
						status = dwarf_formref( abstractOriginAttr, & abstractOriginOffset, NULL );
						assert( status == DW_DLV_OK );

						if( abstractOriginOffset == dieOffset ) {
							Dwarf_Addr lowPC;
							status = dwarf_lowpc( siblingDie, & lowPC, NULL );
							assert( status == DW_DLV_OK );

							// bperr( "Found function with pretty name '%s' inlined-not-declared at 0x%lx in module '%s'\n", dieName, (unsigned long)lowPC, useModuleName.c_str() );
							if( symbolNamesByAddr.defines( lowPC ) ) {
								pdstring symName = symbolNamesByAddr.get( lowPC );
								if( symbols.defines( symName ) ) {
									globalSymbol = symName;
									}
								}
							break;
							} /* end if we've found _the_ concrete instance */
						} /* end if we've found a concrete instance */
					} /* end iteration. */
				} /* end if we're trying to do a by-address look up. */

			/* Update the module information. */
			if( globalSymbol != "" ) {
				assert( symbols.defines( globalSymbol ) );

				/* If it's not specified, is an inlined function in the same
				   CU/namespace as its use. */
				if( isDeclaredNotInlined && nameEntry != dieEntry ) {
					/* Then the function's definition is where it was
					   declared, not wherever it happens to be referenced.

					   Use the decl_file as the useModuleName, if available. */
					Dwarf_Attribute fileNoAttribute;
					status = dwarf_attr( declEntry, DW_AT_decl_file, & fileNoAttribute, NULL );
					assert( status != DW_DLV_ERROR );

					if( status == DW_DLV_OK ) {
						Dwarf_Unsigned fileNo;
						status = dwarf_formudata( fileNoAttribute, & fileNo, NULL );
						assert( status == DW_DLV_OK );

						useModuleName = declFileNoToName[ fileNo - 1 ];
						// bperr( "Assuming declared-not-inlined function '%s' to be in module '%s'.\n", dieName, useModuleName.c_str() );
						} /* end if we have declaration file listed */
					else {
						/* This should never happen, but there's not much
						   we can do if it does. */
						}
					} /* end if isDeclaredNotInlined */

				unsigned int count = 0;
				pdvector< Symbol > & syms = symbols[ globalSymbol ];
				
				// /* DEBUG */ fprintf( stderr, "%s[%d]: symbol '%s' is in module '%s'.\n", __FILE__, __LINE__, globalSymbol.c_str(), useModuleName.c_str() );
				/* If there's only one of symbol of that name, set it regardless. */
				if( syms.size() == 1 ) { syms[0].setModule( useModuleName ); }
				else {
					for( unsigned int i = 0; i < syms.size(); i++ ) {
						if( syms[ i ].linkage() == Symbol::SL_GLOBAL ) {
							symbols[ globalSymbol ][i].setModule( useModuleName );
							count++;
							}
						}
					if( count < syms.size() ) {
						// /* DEBUG */ fprintf( stderr, "%s[%d]: DWARF-derived module information not applied to all symbols of name '%s'\n", __FILE__, __LINE__, globalSymbol.c_str() );
						}
					}
				} /* end if we found the name in the global symbols */
			else if( ! isAbstractOrigin && symbols.defines( dieName ) ) {
				unsigned int count = 0;
				pdvector< Symbol > & syms = symbols[ dieName ];

				/* If there's only one, apply regardless. */
				if( syms.size() == 1 ) { symbols[ globalSymbol ][0].setModule( useModuleName ); }
				else { 
					for( unsigned int i = 0; i < syms.size(); i++ ) {
						if( syms[ i ].linkage() == Symbol::SL_LOCAL ) {
							symbols[ globalSymbol ][i].setModule( useModuleName );
							count++;
							}
						}
					if( count < syms.size() ) {
						// /* DEBUG */ fprintf( stderr, "%s[%d]: DWARF-derived module information not applied to all symbols of name '%s'\n", __FILE__, __LINE__, dieName );
						}
					}
				} /* end if we think it's a local symbol */

			dwarf_dealloc( dbg, dieName, DW_DLA_STRING );
			} break;

		case DW_TAG_variable:
		case DW_TAG_constant: {
			/* Is this a declaration? */
			Dwarf_Attribute declAttr;
			status = dwarf_attr( dieEntry, DW_AT_declaration, & declAttr, NULL );
			assert( status != DW_DLV_ERROR );

			if( status != DW_DLV_OK ) { break; }

			/* If a DIE has a specification, the specification has its name
			   and (optional) linkage name.  */
			Dwarf_Attribute specificationAttribute;
			status = dwarf_attr( dieEntry, DW_AT_specification, & specificationAttribute, NULL );
			assert( status != DW_DLV_ERROR );

			Dwarf_Die nameEntry = dieEntry;
			if( status == DW_DLV_OK ) {
				Dwarf_Off specificationOffset;
				status = dwarf_global_formref( specificationAttribute, & specificationOffset, NULL );
				assert( status == DW_DLV_OK );

				status = dwarf_offdie( dbg, specificationOffset, & nameEntry, NULL );
				assert( status == DW_DLV_OK );
				} /* end if the DIE has a specification. */

			/* What's its name? */
			char * dieName = NULL;
			status = dwarf_diename( nameEntry, & dieName, NULL );
			assert( status != DW_DLV_ERROR );

			/* Prefer the linkage (symbol table) name. */
			Dwarf_Attribute linkageNameAttribute;
			status = dwarf_attr( nameEntry, DW_AT_MIPS_linkage_name, & linkageNameAttribute, NULL );
			assert( status != DW_DLV_ERROR );

			if( status == DW_DLV_OK ) {
				dwarf_dealloc( dbg, dieName, DW_DLA_STRING );
				status = dwarf_formstring( linkageNameAttribute, & dieName, NULL );
				assert( status == DW_DLV_OK );
				}

			/* Anonymous variables are useless to us. */
			if( dieName == NULL ) { break; }
			
			/* Update the module information. */
			pdstring symName = find_symbol( dieName, symbols );
			if (symName != "" ) {
				assert(symbols.defines(symName));

				/* We're assuming global variables. */
				unsigned int count = 0;
				pdvector< Symbol > & syms = symbols[ symName ];

				/* If there's only one of symbol of that name, set it regardless. */
				if( syms.size() == 1 ) { symbols[ symName ][0].setModule( useModuleName ); }
				else {
					for( unsigned int i = 0; i < syms.size(); i++ ) {
						if( syms[ i ].linkage() == Symbol::SL_GLOBAL ) {
							symbols[ symName ][i].setModule( useModuleName );
							count++;
							}
						}
					if( count < syms.size() ) {
						// /* DEBUG */ fprintf( stderr, "%s[%d]: DWARF-derived module information not applied to all symbols of name '%s'\n", __FILE__, __LINE__, symName.c_str() );
						}									
					}
				}				
			} break;

		default:
			/* If it's not a function or a variable, ignore it. */
			break;
		} /* end tag switch */

	/* Recurse to its child, if any. */
	Dwarf_Die childDwarf;
	status = dwarf_child( dieEntry, & childDwarf, NULL );
	assert( status != DW_DLV_ERROR );
	if( status == DW_DLV_OK ) {
		fixSymbolsInModule( dbg, moduleName, childDwarf, symbols, symbolNamesByAddr );
		}

	/* Recurse to its sibling, if any. */
	Dwarf_Die siblingDwarf;
	status = dwarf_siblingof( dbg, dieEntry, & siblingDwarf, NULL );
	assert( status != DW_DLV_ERROR );
	if( status == DW_DLV_OK ) {
		/* Force tail-recursion to avoid stack overflows. */
		dieEntry = siblingDwarf;
		goto start;
		// fixSymbolsInModule( dbg, moduleName, siblingDwarf, symbols, symbolNamesByAddr );		
		}
	} /* end fixSymbolsInModule */

void fixSymbolsInModuleByRange(pdstring &moduleName,
			       Dwarf_Addr modLowPC, Dwarf_Addr modHighPC,
			       dictionary_hash<pdstring, pdvector< Symbol > > *symbols_)
{
    pdstring symName;
    pdvector< Symbol > syms;
    Symbol sym;
    
    dictionary_hash_iter< pdstring, pdvector< Symbol > > iter( * symbols_ );
    while (iter.next(symName, syms)) {
    	for( unsigned int i = 0; i < syms.size(); i++ ) {
    		sym = syms[i];
			if (sym.addr() >= modLowPC && sym.addr() < modHighPC) {
			    (*symbols_)[symName][i].setModule(moduleName);
		    }
		}
    }
}

bool Object::fix_global_symbol_modules_static_dwarf(Elf_X &elf)
{
  /* Initialize libdwarf. */
  Dwarf_Debug dbg;
  int status = dwarf_elf_init( elf.e_elfp(), DW_DLC_READ, & pd_dwarf_handler, getErrFunc(), & dbg, NULL);
  if( status != DW_DLV_OK ) {
     return false;
     }

  /* Iterate over the CU headers. */
  Dwarf_Unsigned hdr;
  while( dwarf_next_cu_header( dbg, NULL, NULL, NULL, NULL, & hdr, NULL ) == DW_DLV_OK ) {
	/* Obtain the module DIE. */
	Dwarf_Die moduleDIE;
	status = dwarf_siblingof( dbg, NULL, & moduleDIE, NULL );
	assert( status == DW_DLV_OK );

	/* Make sure we've got the right one. */
	Dwarf_Half moduleTag;
	status = dwarf_tag( moduleDIE, & moduleTag, NULL);
	assert( status == DW_DLV_OK );
	assert( moduleTag == DW_TAG_compile_unit );
                
	/* Extract the name of this module. */
	char * dwarfModuleName = NULL;
	status = dwarf_diename( moduleDIE, & dwarfModuleName, NULL );
	assert( status != DW_DLV_ERROR );

	pdstring moduleName;
	if( status == DW_DLV_NO_ENTRY || dwarfModuleName == NULL ) {
		moduleName = pdstring( "{ANONYMOUS}" );
		assert( moduleName != NULL );
		}
	else {
		moduleName = extract_pathname_tail( dwarfModuleName );
		}

	Dwarf_Addr modLowPC = 0;
	Dwarf_Addr modHighPC = (Dwarf_Addr)(-1);
	Dwarf_Bool hasLowPC;
	Dwarf_Bool hasHighPC;
	
	if( (status = dwarf_hasattr(moduleDIE, DW_AT_low_pc, &hasLowPC, NULL)) == DW_DLV_OK &&
		hasLowPC &&
	    (status = dwarf_hasattr(moduleDIE, DW_AT_high_pc, &hasHighPC, NULL)) == DW_DLV_OK && 
		hasHighPC ) {
	    // Get PC boundaries for the module, if present
	    status = dwarf_lowpc(moduleDIE, &modLowPC, NULL);
	    assert(status == DW_DLV_OK);
	    status = dwarf_highpc(moduleDIE, &modHighPC, NULL);
	    assert(status == DW_DLV_OK);
	    
	    // Set module names for all symbols that belong to the range
	    fixSymbolsInModuleByRange(moduleName, modLowPC, modHighPC,
				      &symbols_);
		}
	else {
		/* Acquire declFileNoToName. */
		status = dwarf_srcfiles( moduleDIE, & declFileNoToName, & declFileNo, NULL );
		assert( status != DW_DLV_ERROR );
	
		if( status == DW_DLV_OK ) {
		    /* Walk the tree. */
		    fixSymbolsInModule( dbg, moduleName, moduleDIE, symbols_, symbolNamesByAddr );
	    
			/* Deallocate declFileNoToName. */
			for( Dwarf_Signed i = 0; i < declFileNo; i++ ) {
				dwarf_dealloc( dbg, declFileNoToName[i], DW_DLA_STRING );
				}
			dwarf_dealloc( dbg, declFileNoToName, DW_DLA_LIST );	
			} /* end if the srcfile information was available */
		else {
			bperr( "Unable to determine modules (%s): no code range or source file information available.\n", moduleName.c_str() );
			} /* end if no source file information available */
		} /* end if code range information unavailable */
		
	} /* end scan over CU headers. */

  /* Clean up. */
  status = dwarf_finish( dbg, NULL );  
  assert( status == DW_DLV_OK );
  return true;
}

#else

// dummy definition for non-DWARF platforms
bool Object::fix_global_symbol_modules_static_dwarf(Elf_X &/*elf*/)
{ return false; }

#endif // USES_DWARF_DEBUG

/********************************************************
 *
 * For object files only....
 *  read the .stab section to find the module of global symbols
 *
 ********************************************************/
bool Object::fix_global_symbol_modules_static_stab(Elf_X_Shdr* stabscnp, Elf_X_Shdr* stabstrscnp) {
    // Read the stab section to find the module of global symbols.
    // The symbols appear in the stab section by module. A module begins
    // with a symbol of type N_UNDF and ends with a symbol of type N_ENDM.
    // All the symbols in between those two symbols belong to the module.

    if (!stabscnp || !stabstrscnp) return false;

    Elf_X_Data stabdata = stabscnp->get_data();
    Elf_X_Data stabstrdata = stabstrscnp->get_data();
    stab_entry *stabptr = NULL;

    if (!stabdata.isValid() || !stabstrdata.isValid()) return false;

    switch (addressWidth_nbytes) {
    case 4:
	stabptr = new stab_entry_32(stabdata.d_buf(),
				    stabstrdata.get_string(),
				    stabscnp->sh_size() / sizeof(stab32));
	break;

    case 8:
	stabptr = new stab_entry_64(stabdata.d_buf(),
				    stabstrdata.get_string(),
				    stabscnp->sh_size() / sizeof(stab64));
	break;
    };
    const char *next_stabstr = stabptr->getStringBase();
    pdstring module = "DEFAULT_MODULE";

    // the stabstr contains one string table for each module.
    // stabstr_offset gives the offset from the begining of stabstr of the
    // string table for the current module.

    bool is_fortran = false;  // is the current module fortran code?
    for (unsigned i = 0; i < stabptr->count(); i++) {
	switch(stabptr->type(i)) {
 	case N_UNDF: /* start of object file */
	    stabptr->setStringBase(next_stabstr);
	    next_stabstr = stabptr->getStringBase() + stabptr->val(i);
	    break;

	case N_ENDM: /* end of object file */
	    is_fortran = false;
	    module = "DEFAULT_MODULE";
	    break;

	case N_SO: /* compilation source or file name */
  	    if ((stabptr->desc(i) == N_SO_FORTRAN) || (stabptr->desc(i) == N_SO_F90))
		is_fortran = true;

	    module = pdstring(stabptr->name(i));
	    break;

        case N_ENTRY: /* fortran alternate subroutine entry point */
	case N_GSYM: /* global symbol */
	    // the name string of a function or object appears in the stab 
	    // string table as <symbol name>:<symbol descriptor><other stuff>
	    // where <symbol descriptor> is a one char code.
	    // we must extract the name and descriptor from the string
        {
	    const char *p = stabptr->name(i);
	    // bperr("got %d type, str = %s\n", stabptr->type(i), p);
	    if (stabptr->type(i) == N_FUN && strlen(p) == 0) {
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

	    pdstring SymName = pdstring(sname);

	    // q points to the ':' in the name pdstring, so 
	    // q[1] is the symbol descriptor. We must check the symbol descriptor
	    // here to skip things we are not interested in, such as prototypes.
	    bool res = symbols_.defines(SymName);
	    if (!res && is_fortran) {
		// Fortran symbols usually appear with an '_' appended in .symtab,
		// but not on .stab
		SymName += "_";
		res = symbols_.defines(SymName);
	    }

	    if (res && (q == 0 || q[1] != SD_PROTOTYPE)) {
	    	unsigned int count = 0;
	    	pdvector< Symbol > & syms = symbols_[SymName];

	    	/* If there's only one, apply regardless. */
	    	if( syms.size() == 1 ) { symbols_[SymName][0].setModule(module); }
	    	else {
		    for( unsigned int i = 0; i < syms.size(); i++ ) {
			if( syms[i].linkage() == Symbol::SL_GLOBAL ) {
			    symbols_[SymName][i].setModule(module);
			    count++;
			}
		    }
		    if( count < syms.size() ) {
			// /* DEBUG */ fprintf( stderr, "%s[%d]: STABS-derived module information not applied to all symbols of name '%s'\n", __FILE__, __LINE__, SymName.c_str() );
		    }
		}
	    }
	}
	break;

	case N_FUN: /* function */ {
	    const char *p = stabptr->name(i);
            if (strlen(p) == 0) {
                // Rumours are that GNU CC 2.8 and higher associate a
                // null-named function entry with the end of a
                // function. Just skip it.
                break;
            }
	    const char *q = strchr(p,':');
	    if (q == 0) {
		// bperr( "Unrecognized stab format: %s\n", p);
		// Happens with the Solaris native compiler (.xstabs entries?)
		break;
	    }

	    if (q[1] == SD_PROTOTYPE) {
		// We see a prototype, skip it
		break;
	    }

	    unsigned long entryAddr = stabptr->val(i);
	    if (entryAddr == 0) {
		// The function stab doesn't contain a function address
		// (happens with the Solaris native compiler). We have to
		// look up the symbol by its name. That's unfortunate, since
		// names may not be unique and we may end up assigning a wrong
		// module name to the symbol.
		unsigned len = q - p;
		assert(len > 0);
		char *sname = new char[len+1];
		strncpy(sname, p, len);
		sname[len] = 0;
		pdstring nameFromStab = pdstring(sname);
		delete[] sname;

		if (symbols_.defines(nameFromStab)) {
		    pdvector< Symbol > & syms = symbols_[nameFromStab];
		    if( syms.size() == 1 ) {
			symbols_[nameFromStab][0].setModule(module);
		    }
		    /* DEBUG */ else { fprintf( stderr, "%s[%d]: Nonunique STABS name '%s' in module.\n", __FILE__, __LINE__, nameFromStab.c_str() ); }
		    /* Otherwise, don't assign a module if we don't know
		       to which symbol this refers. */
		}
	    }
	    else {
		if (!symbolNamesByAddr.defines(entryAddr)) {
		    bperr( "fix_global_symbol_modules_static_stab "
			   "can't find address 0x%lx of STABS entry %s\n", entryAddr, p);
		    break;
		}
		pdstring symName = symbolNamesByAddr[entryAddr];
		assert(symbols_.defines(symName));
		pdvector< Symbol > & syms = symbols_[symName];
		if( syms.size() == 1 ) {
		    symbols_[symName][0].setModule(module);
		}
		/* DEBUG */ else { fprintf( stderr, "%s[%d]: Nonunique id %s in module.\n", __FILE__, __LINE__, symName.c_str() ); }
		/* Otherwise, don't assign a module if we don't know
		   to which symbol this refers. */
	    }
	    break;
	}

	default:
	    /* ignore other entries */
	    break;
	}
    }
    delete stabptr;

    return true;
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
void Object::insert_symbols_static(pdvector<Symbol> allsymbols)
{
  unsigned nsymbols = allsymbols.size();
#ifdef TIMED_PARSE
   cout << __FILE__ << ":" << __LINE__ << ": stuffing "<<nsymbols 
	 << " symbols into symbols_ dictionary" << endl; 
#endif
  for (unsigned u = 0; u < nsymbols; u++) {
      insertUniqdSymbol(allsymbols[u], &symbols_, &symbolNamesByAddr);
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
        insertUniqdSymbol(allsymbols[i], &symbols_, &symbolNamesByAddr);
    }
}

// find_code_and_data(): populates the following members:
//   code_ptr_, code_off_, code_len_
//   data_ptr_, data_off_, data_len_
void Object::find_code_and_data(Elf_X &elf,
				Address txtaddr, 
				Address bssaddr) 
{
    for (int i = 0; i < elf.e_phnum(); ++i) {
	Elf_X_Phdr phdr = elf.get_phdr(i);

	if ((phdr.p_vaddr() <= txtaddr) && 
	    (phdr.p_vaddr() + phdr.p_memsz() >= txtaddr)) {

	    if (code_ptr_ == 0 && code_off_ == 0 && code_len_ == 0) {
		code_ptr_ = (Word *)(void*)&file_ptr_[phdr.p_offset()];
		code_off_ = (Address)phdr.p_vaddr();
		code_len_ = (unsigned)phdr.p_memsz();
	    }

	} else if ((phdr.p_vaddr() <= bssaddr) && 
		   (phdr.p_vaddr() + phdr.p_memsz() >= bssaddr)) {

	    if (data_ptr_ == 0 && data_off_ == 0 && data_len_ == 0) {
		data_ptr_ = (Word *)(void *)&file_ptr_[phdr.p_offset()];
		data_off_ = (Address)phdr.p_vaddr();
		data_len_ = (unsigned)phdr.p_memsz();
	    }
	}
    }
    //if (addressWidth_nbytes == 8) bperr( ">>> 64-bit find_code_and_data() successful\n");
}

const char *Object::elf_vaddr_to_ptr(Address vaddr) const
{
  const char *ret = NULL;
  unsigned code_size_ = code_len_;
  unsigned data_size_ = data_len_;

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

stab_entry *Object::get_stab_info() const
{
    // check that file has .stab info
    if (stab_off_ && stab_size_ && stabstr_off_) 
	switch (addressWidth_nbytes) {
	case 4: // 32-bit object
	    return new stab_entry_32(file_ptr_ + stab_off_,
				     file_ptr_ + stabstr_off_,
				     stab_size_ / sizeof(stab32));
	case 8: // 64-bit object
	    return new stab_entry_64(file_ptr_ + stab_off_,
				     file_ptr_ + stabstr_off_,
				     stab_size_ / sizeof(stab64));
	}
    return new stab_entry_64();
}

Object::Object(const fileDescriptor &desc, void (*err_func)(const char *))
    : AObject(desc.file(), err_func), fileName(NULL), EEL(false), 
   symbolNamesByAddr( addrHash ) { //ccw 8 mar 2004
    if (desc.isSharedObject())
        load_shared_object();
    else load_object();
}

Object::Object(const Object& obj)
    : AObject(obj), fileName(NULL), EEL(false), symbolNamesByAddr( addrHash ) 
{
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
    dwarvenDebugInfo = obj.dwarvenDebugInfo;
    symbolNamesByAddr = obj.symbolNamesByAddr;
    return *this;
}

Object::~Object()
{
   if (fileName) free(fileName);
}

void Object::log_elferror(void (*)(const char *), const char* msg) {
    const char* err = elf_errmsg(elf_errno());
    bperr( "%s: %s\n", msg, err ? err : "(bad elf error)");
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

/**
 * Returns true if the Address range addr -> addr+size contains
 * a catch block, with b pointing to a the appropriate block
 **/
bool Object::getCatchBlock(ExceptionBlock &b, Address addr, 
                           unsigned size) const 
{
   int min = 0;
   int max = catch_addrs_.size();
   int cur = -1, last_cur;

   if (max == 0)
      return false;
   
   //Binary search through vector for address
   while (true) 
   {
      last_cur = cur;
      cur = (min + max) / 2;

      if (last_cur == cur)
         return false;

      Address curAddr = catch_addrs_[cur].catchStart();

      if ((curAddr <= addr && curAddr+size > addr) ||
          (size == 0 && curAddr == addr))
      {
         //Found it
         b = catch_addrs_[cur];
         return true;
      }
      if (addr < curAddr)
         max = cur;
      else if (addr > curAddr)
         min = cur;      
   }
}

#ifdef DEBUG

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
  s << " dwarvenDebugInfo = " << dwarvenDebugInfo << endl;

  // and dump the relocation table....
  s << " relocation_table_ = (field seperator :: )" << endl;   
  for (unsigned i=0; i < relocation_table_.size(); i++) {
    s << relocation_table_[i] << " :: "; 
  }
  s << endl;

  return s;
} 

#endif


Address Object::getPltSlot(pdstring funcName) const{

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

//
// get_valid_memory_areas - get ranges of code/data segments that have
//                       sections mapped to them
//

void Object::get_valid_memory_areas(Elf_X &elf)
{
    for (unsigned i = 0; i < elf.e_shnum(); ++i) {
	Elf_X_Shdr shdr = elf.get_shdr(i);

	if (shdr.sh_flags() & SHF_ALLOC) { // This section is in memory
	    if (code_off_ <= shdr.sh_addr() &&
		shdr.sh_addr() <= code_off_ + code_len_) {
		if (shdr.sh_addr() < code_vldS_)
		    code_vldS_ = shdr.sh_addr();
		if (shdr.sh_addr() + shdr.sh_size() > code_vldE_)
		    code_vldE_ = shdr.sh_addr() + shdr.sh_size();

	    } else if (data_off_ <= shdr.sh_addr() &&
		       shdr.sh_addr() <= data_off_ + data_len_) {
		if (shdr.sh_addr() < data_vldS_)
		    data_vldS_ = shdr.sh_addr();
		if (shdr.sh_addr() + shdr.sh_size() > data_vldE_)
		    data_vldE_ = shdr.sh_addr() + shdr.sh_size();
	    }
	}
    }
}

//
// parseCompilerType - parse for compiler that was used to generate object
//
//
//
bool parseCompilerType(Object *objPtr) 
{
   stab_entry *stabptr = objPtr->get_stab_info();
   const char *next_stabstr = stabptr->getStringBase();
   
   for (unsigned int i=0; i < stabptr->count(); ++i) {
      // if (stabstrs) bperr("parsing #%d, %s\n", stabptr->type(i), stabptr->name(i));
      switch (stabptr->type(i)) {

         case N_UNDF: /* start of object file */
            /* value contains offset of the next string table for next module */
            // assert(stabptr.nameIdx(i) == 1);
            stabptr->setStringBase(next_stabstr);
            next_stabstr = stabptr->getStringBase() + stabptr->val(i);
            break;

         case N_OPT: /* Compiler options */
#if defined(os_solaris) 
            if (strstr(stabptr->name(i), "Sun") != NULL ||
                strstr(stabptr->name(i), "Forte") != NULL)
            {
               delete stabptr;
               return true;
            }
#endif
            delete stabptr;
            return false;
      }
   }
   delete stabptr;
   return false; // Shouldn't happen - maybe N_OPT stripped
}


#if defined(os_linux) && (defined(arch_x86) || defined(arch_x86_64))

static unsigned long read_uleb128(const char *data, unsigned *bytes_read)
{
   unsigned long result = 0;
   unsigned shift = 0;
   *bytes_read = 0;
   while (1)
   {
      result |= (data[*bytes_read] & 0x7f) << shift;
      if ((data[(*bytes_read)++] & 0x80) == 0)
         break;
      shift += 7;
   }
   return result;
}

static signed long read_sleb128(const char *data, unsigned *bytes_read)
{
   unsigned long result = 0;
   unsigned shift = 0;
   *bytes_read = 0;
   while (1)
   {
      result |= (data[*bytes_read] & 0x7f) << shift;
      shift += 7;
      if ((data[*bytes_read] & 0x80) == 0)
         break;
      (*bytes_read)++;
   }
   if (shift < sizeof(int) && (data[*bytes_read] & 0x40))
      result |= -(1 << shift);
   (*bytes_read)++;
   return result;
}

#define DW_EH_PE_uleb128 0x01
#define DW_EH_PE_udata2  0x02
#define DW_EH_PE_udata4  0x03
#define DW_EH_PE_udata8  0x04
#define DW_EH_PE_sleb128 0x09
#define DW_EH_PE_sdata2  0x0A
#define DW_EH_PE_sdata4  0x0B
#define DW_EH_PE_sdata8  0x0C
#define DW_EH_PE_absptr  0x00
#define DW_EH_PE_pcrel   0x10
#define DW_EH_PE_datarel 0x30
#define DW_EH_PE_omit    0xff

static int get_ptr_of_type(int type, unsigned long *value, const char *addr)
{
   unsigned size;
   if (type == DW_EH_PE_omit)
      return 0;

   switch (type & 0xf)
   {
      case DW_EH_PE_uleb128:
      case DW_EH_PE_absptr:
         *value = read_uleb128(addr, &size);
         return size;
      case DW_EH_PE_sleb128:
         *value = read_sleb128(addr, &size);
         return size;
      case DW_EH_PE_udata2:
      case DW_EH_PE_sdata2:         
         *value = (unsigned long) *((const int16_t *) addr);
         return 2;
      case DW_EH_PE_udata4:
      case DW_EH_PE_sdata4:
         *value = (unsigned long) *((const int32_t *) addr);
         return 4;
      case DW_EH_PE_udata8:
      case DW_EH_PE_sdata8:
         *value = (unsigned long) *((const int64_t *) addr);
         return 8;
      default:
         fprintf(stderr, "Error Unexpected type %x\n", (unsigned) type);
         return 1;
   }
}

/**
 * On GCC 3.x/x86 we find catch blocks as follows:
 *   1. We start with a list of FDE entries in the .eh_frame
 *      table.  
 *   2. Each FDE entry has a pointer to a  CIE entry.  The CIE
 *      tells us whether the FDE has any 'Augmentations', and
 *      the types of those augmentations.  The FDE also
 *      contains a pointer to a function.
 *   3. If the FDE has a 'Language Specific Data Area' 
 *      augmentation then we have a pointer to one or more
 *      entires in the gcc_except_table.
 *   4. The gcc_except_table contains entries that point 
 *      to try and catch blocks, all encoded as offsets
 *      from the function start (it doesn't tell you which
 *      function, however). 
 *   5. We can add the function offsets from the except_table
 *      to the function pointer from the FDE to get all of
 *      the try/catch blocks.  
 **/ 
static int read_except_table_gcc3(Elf_X_Shdr *except_table, 
				  Address eh_frame_base, Address except_base,
				  Dwarf_Fde *fde_data, Dwarf_Signed fde_count,
				  pdvector<ExceptionBlock> &addresses)
{
    Dwarf_Error err = (Dwarf_Error) NULL;
    Dwarf_Addr low_pc, except_ptr;
    Dwarf_Unsigned fde_byte_length, bytes_in_cie, outlen;
    Dwarf_Ptr fde_bytes, lsda, outinstrs;
    Dwarf_Off fde_offset;
    Dwarf_Fde fde;
    Dwarf_Cie cie;
    int has_lsda = 0, is_pic = 0, has_augmentation_length = 0;
    int augmentor_len, lsda_size, status;
    unsigned bytes_read;
    char *augmentor;
    unsigned char lpstart_format, ttype_format, table_format;
    unsigned long value, table_end, region_start, region_size, 
	catch_block, action;
    int i, j;

    //For each FDE
    for (i = 0; i < fde_count; i++) {
	//Get the FDE
	status = dwarf_get_fde_n(fde_data, (Dwarf_Unsigned) i, &fde, &err);
	if (status != DW_DLV_OK) {
	    pd_dwarf_handler(err, NULL);
	    return false;
	}

	//Get address of the function associated with this CIE
	status = dwarf_get_fde_range(fde, &low_pc, NULL, &fde_bytes, 
				     &fde_byte_length, NULL, NULL, &fde_offset, &err);
	if (status != DW_DLV_OK) {
	    pd_dwarf_handler(err, NULL);
	    return false;
	}

	//Get the CIE for the FDE
	status = dwarf_get_cie_of_fde(fde, &cie, &err);
	if (status != DW_DLV_OK) {
	    pd_dwarf_handler(err, NULL);
	    return false;
	}

	//Get the Augmentation string for the CIE
	status = dwarf_get_cie_info(cie, &bytes_in_cie, NULL, &augmentor, 
				    NULL, NULL, NULL, NULL, NULL, &err); 
	if (status != DW_DLV_OK) {
	    pd_dwarf_handler(err, NULL);
	    return false;
	}
	augmentor_len = (augmentor == NULL) ? 0 : strlen(augmentor);
	for (j = 0; j < augmentor_len; j++) {
	    if (augmentor[j] == 'z')
		has_augmentation_length = 1;
	    if (augmentor[j] == 'L')
		has_lsda = 1;
	    if (augmentor[j] == 'R')
		is_pic = 1;
	}

	//If we don't have a language specific data area, then
	// we don't care about this FDE.
	if (!has_lsda)
	    continue;

	//Get the Language Specific Data Area pointer
	status = dwarf_get_fde_instr_bytes(fde, &outinstrs, &outlen, &err);
	if (status != DW_DLV_OK) {
	    pd_dwarf_handler(err, NULL);
	    return false;
	}
	lsda = ((char *) fde_bytes) + sizeof(int) * 4;
	if (lsda == outinstrs) {
	    continue;
	}
	lsda_size = (unsigned) read_uleb128((char *) lsda, &bytes_read);
	lsda = ((char *) lsda) + bytes_read;

	//Read the exception table pointer from the LSDA, adjust for PIC
	except_ptr = (Dwarf_Addr) *((long *) lsda);
	if (!except_ptr) {
	    // Sometimes the FDE doesn't have an associated 
	    // exception table.
	    continue;
	}
	if (is_pic) {
	    low_pc = eh_frame_base + fde_offset + sizeof(int)*2 + (signed) low_pc;
	    except_ptr += eh_frame_base + fde_offset + 
		sizeof(int)*4 + bytes_read;
	}
	except_ptr -= except_base;

	// Get the exception data from the section.
	Elf_X_Data data = except_table->get_data();
	if (!data.isValid()) {
	    return true;
	}
	const char *datap = data.get_string();
	int except_size = data.d_size();

	j = except_ptr;
	if (j < 0 || j >= except_size)
	    continue;

	// Read some variable length header info that we don't really
	// care about.
	lpstart_format = datap[j++];
	if (lpstart_format != DW_EH_PE_omit)
	    j += get_ptr_of_type(DW_EH_PE_uleb128, &value, datap + j);
	ttype_format = datap[j++];
	if (ttype_format != DW_EH_PE_omit)
	    j += get_ptr_of_type(DW_EH_PE_uleb128, &value, datap + j);

	// This 'type' byte describes the data format of the entries in the 
	// table and the format of the table_size field.
	table_format = datap[j++];
	j += get_ptr_of_type(table_format, &table_end, datap + j);
	table_end += j;

	while (j < (signed) table_end && j < (signed) except_size) {
	    //The entries in the gcc_except_table are the following format:
	    //   <type>   region start
	    //   <type>   region length
	    //   <type>   landing pad
	    //  uleb128   action
	    //The 'region' is the try block, the 'landing pad' is the catch.
	    j += get_ptr_of_type(table_format, &region_start, datap + j);
	    j += get_ptr_of_type(table_format, &region_size, datap + j);
	    j += get_ptr_of_type(table_format, &catch_block, datap + j);
	    j += get_ptr_of_type(DW_EH_PE_uleb128, &action, datap + j);

	    if (catch_block == 0)
		continue;
	    ExceptionBlock eb(region_start + low_pc, region_size, 
			      catch_block + low_pc);
	    addresses.push_back(eb);
	}
    }

    return true;
}

/**
 * Things were much simpler in the old days.  On gcc 2.x
 * the gcc_except_table looks like:
 *   <long> try_start
 *   <long> try_end
 *   <long> catch_start
 * Where everything is an absolute address, even when compiled
 * with PIC.  All we got to do is read the catch_start entries
 * out of it.
 **/
static bool read_except_table_gcc2(Elf_X_Shdr *except_table, 
                                   pdvector<ExceptionBlock> &addresses)
{
    Address try_start;
    Address try_end;
    Address catch_start;

    Elf_X_Data data = except_table->get_data();
    const char *datap = data.get_string();
    unsigned except_size = data.d_size();

    unsigned i;
    while (i < except_size) {
	i += get_ptr_of_type(DW_EH_PE_udata4, &try_start, datap + i);
	i += get_ptr_of_type(DW_EH_PE_udata4, &try_end, datap + i);
	i += get_ptr_of_type(DW_EH_PE_udata4, &catch_start, datap + i);

	if (try_start != (Address) -1 && try_end != (Address) -1) {
	    ExceptionBlock eb(try_start, try_end - try_start, catch_start);
	    addresses.push_back(eb);
	}
    }
    return true;
}

static int exception_compare(const ExceptionBlock &e1, const ExceptionBlock &e2)
{
   if (e1.tryStart() < e2.tryStart())
      return -1;
   else if (e1.tryStart() > e2.tryStart())
      return 1;
   else
      return 0;
}

/**
 * Finds the addresses of catch blocks in a g++ generated elf file.
 *  'except_scn' should point to the .gcc_except_table section
 *  'eh_frame' should point to the .eh_frame section
 *  the addresses will be pushed into 'addresses'
 **/
static bool find_catch_blocks(Elf_X &elf, Elf_X_Shdr *eh_frame, Elf_X_Shdr *except_scn,
                              pdvector<ExceptionBlock> &catch_addrs)
{
    Dwarf_Cie *cie_data;
    Dwarf_Fde *fde_data;
    Dwarf_Signed cie_count, fde_count;
    Dwarf_Error err = (Dwarf_Error) NULL;
    Dwarf_Unsigned bytes_in_cie;
    Address eh_frame_base, except_base;
    Dwarf_Debug dbg;
    char *augmentor;
    int status, gcc_ver = 3;
    unsigned i;
    bool result = false;

    if (except_scn == NULL) {
	//likely to happen if we're not using gcc
	return true;
    }

    eh_frame_base = eh_frame->sh_addr();
    except_base = except_scn->sh_addr();

    //Open dwarf object
    status = dwarf_elf_init(elf.e_elfp(), DW_DLC_READ, &pd_dwarf_handler, NULL,
			    &dbg, &err);
    if( status != DW_DLV_OK ) {
#if !defined(x86_64_unknown_linux2_4)
	// GCC 3.3.3 seems to generate incorrect DWARF entries
	// on the x86_64 platform right now.  Hopefully this will
	// be fixed, and this #if macro can be removed.

	pd_dwarf_handler(err, NULL);
#endif
	goto err_noclose;
    }

    //Read the FDE and CIE information
    status = dwarf_get_fde_list_eh(dbg, &cie_data, &cie_count,
				   &fde_data, &fde_count, &err);
    if (status != DW_DLV_OK) {
	//No actual stackwalk info in this object
	goto err_noalloc;
    }

    //GCC 2.x has "eh" as its augmentor string in the CIEs
    for (i = 0; i < cie_count; i++) {
	status = dwarf_get_cie_info(cie_data[i], &bytes_in_cie, NULL,
				    &augmentor, NULL, NULL, NULL, NULL, NULL, &err);
	if (status != DW_DLV_OK) {
	    pd_dwarf_handler(err, NULL);
	    goto cleanup;
	}
	if (augmentor[0] == 'e' && augmentor[1] == 'h') {
	    gcc_ver = 2;
	}
    }

    //Parse the gcc_except_table
    if (gcc_ver == 2) {
	result = read_except_table_gcc2(except_scn, catch_addrs);

    } else if (gcc_ver == 3) {
	result = read_except_table_gcc3(except_scn, eh_frame_base, except_base,
					fde_data, fde_count, catch_addrs);
    }
    VECTOR_SORT(catch_addrs, exception_compare);

  cleanup:
    //Unallocate fde and cie information 
    for (i = 0; i < cie_count; i++)
	dwarf_dealloc(dbg, cie_data[i], DW_DLA_CIE);
    for (i = 0; i < fde_count; i++)
	dwarf_dealloc(dbg, fde_data[i], DW_DLA_FDE);
    dwarf_dealloc(dbg, cie_data, DW_DLA_LIST);
    dwarf_dealloc(dbg, fde_data, DW_DLA_LIST);

  err_noalloc:
    dwarf_finish(dbg, &err);

  err_noclose:
    return result;
}

#endif

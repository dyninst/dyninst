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
 * $Id: Object-elf.h,v 1.59 2004/03/29 23:42:20 mirg Exp $
 * Object-elf.h: Object class for ELF file format
************************************************************************/


#if !defined(_Object_elf_h_)
#define _Object_elf_h_


#include "common/h/String.h"
#include "common/h/Symbol.h"
#include "common/h/Types.h"
#include "common/h/Vector.h"
#include <elf.h>
#include <libelf.h>

extern "C" {
#include <libelf.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
}

/*
 * The standard symbol table in an elf file is the .symtab section. This section does
 * not have information to find the module to which a global symbol belongs, so we must
 * also read the .stab section to get this info.
 */

// Declarations for the .stab section.
// These are not declared in any system header files, so we must provide our own
// declarations. The declarations below were taken from:
//       SPARCWorks 3.0.x Debugger Interface, July 1994

struct stab_entry { // an entry in the stab section
  unsigned long name;  // stabstr table index for this symbol
  unsigned char type; // type of this symbol
  unsigned char other; 
  unsigned short desc; 
  unsigned long val; // value of this symbol -- usually zero. The real value must
                     // be obtained from the symtab section
};

// Types 
#define N_UNDF  0x00 /* start of object file */
#define N_GSYM  0x20 /* global symbol */
#define N_FUN   0x24 /* function or procedure */
#define N_STSYM 0x26 /* initialized static symbol */
#define N_LCSYM 0x28 /* unitialized static symbol */
#define N_ROSYM 0x2c /* read-only static symbol */
#define N_OPT   0x3c /* compiler options */
#define N_ENDM  0x62 /* end module */
#define N_SO    0x64 /* source directory and file */
#define N_ENTRY 0xa4 /* fortran alternate subroutine entry point */
#define N_BCOMM 0xe2 /* start fortran named common block */
#define N_ECOMM 0xe4 /* start fortran named common block */

// Language code -- the desc field in a N_SO entry is a language code
#define N_SO_AS      1 /* assembler source */
#define N_SO_C       2 /* K & R C source */
#define N_SO_ANSI_C  3 /* ANSI C source */
#define N_SO_CC      4 /* C++ source */
#define N_SO_FORTRAN 5 /* fortran source */
#define N_SO_PASCAL  6 /* Pascal source */
#define N_SO_F90     7 /* Fortran90 source */

//line information data
#define N_SLINE  0x44 /* line number in text segment */
#define N_SOL    0x84 /* name of the include file*/

// Symbol descriptors
// The format of a name is "<name>:<symbol descriptor><rest of name>
// The following are the descriptors of interest
#define SD_GLOBAL_FUN 'F' /* global function or procedure */
#define SD_PROTOTYPE  'P'  /* function prototypes */
#define SD_GLOBAL_VAR 'G' /* global variable */

// end of stab declarations

class pdElfShdr;

class Object : public AObject {
 public:

    Symbol findMain( pdvector< Symbol > &allsymbols );
    Address findDynamic( pdvector< Symbol > &allsymbols );
    bool shared();
  // executable ctor
  Object(const pdstring, 
	 void (*)(const char *) = log_msg);
  // shared object ctor
  Object(const pdstring, const Address base, 
	 void (*)(const char *) = log_msg);  
  // "Filedescriptor" ctor
  Object(fileDescriptor *desc, Address baseAddr =0 , void (*)(const char *) = log_msg);
  Object(const Object &);
  virtual ~Object();
  const Object& operator=(const Object &);
  
  bool is_elf64() const { return is_elf64_; }
  const char *elf_vaddr_to_ptr(Address vaddr) const;
  bool hasStabInfo() const { return ! ( !stab_off_ || !stab_size_ || !stabstr_off_ ); }
  bool hasDwarfInfo() const { return dwarvenDebugInfo; }
  void get_stab_info(void **stabs, int &nstabs, void **stabstr) const;
  const char * getFileName() const { return fileName; }

  bool needs_function_binding() const { return (plt_addr_ > 0); } 
  bool get_func_binding_table(pdvector<relocationEntry> &fbt) const;
  bool get_func_binding_table_ptr(const pdvector<relocationEntry> *&fbt) const;

#if defined(ia64_unknown_linux2_4)
  Address getTOCoffset() const { return gp; }
#endif

  const ostream &dump_state_info(ostream &s);
  bool isEEL() { return EEL; }

	//to determine if a mutation falls in the text section of
	// a shared library
	bool isinText(Address addr, Address baseaddr) const { 
		//printf(" baseaddr %x TESTING %x %x \n", baseaddr, text_addr_ + baseaddr  , text_addr_ + baseaddr + text_size_ );
		if(addr > text_addr_ + baseaddr     &&
		   addr < text_addr_ + baseaddr + text_size_ ) {
			return true;
		}
		return false;
	} 
	// to determine where in the .plt this function is listed 
	// returns an offset from the base address of the object
	// so the entry can easily be located in memory
	Address getPltSlot(pdstring funcName) const ;
	bool hasSymAtAddr( Address adr )
	{
	    return symbolNamesByAddr.defines( adr );
	}
	Address textAddress(){ return text_addr_;}
	bool isText( Address addr ) const
	{
	    if( addr >= text_addr_ && addr <= text_addr_ + text_size_ )
		return true;
	    return false;
	}

 private:
  static void log_elferror (void (*)(const char *), const char *);
    
  int       file_fd_;            // mapped ELF file
  unsigned  file_size_;          // mapped ELF file
  char     *file_ptr_;           // mapped ELF file
  
  const char * fileName;
  //Symbol    mainSym_;
  Address   text_addr_; //.text section 
  Address   text_size_; //.text section size
  Address   dynamic_addr_;//.dynamic section
  Address   dynsym_addr_;        // .dynsym section
  Address   dynstr_addr_;        // .dynstr section
  Address   got_addr_;           // global offset table
  unsigned  got_size_;           // global offset table
  Address   plt_addr_;           // procedure linkage table
  unsigned  plt_size_;           // procedure linkage table
  unsigned  plt_entry_size_;     // procedure linkage table
  Address   rel_plt_addr_;       // .rel[a].plt section
  unsigned  rel_plt_size_;       // .rel[a].plt section
  unsigned  rel_plt_entry_size_; // .rel[a].plt section

  Address   stab_off_;           // .stab section
  unsigned  stab_size_;          // .stab section
  Address   stabstr_off_;        // .stabstr section

  Address   stab_indx_off_;	 // .stab.index section
  unsigned  stab_indx_size_;	 // .stab.index section
  Address   stabstr_indx_off_;	 // .stabstr.index section

  bool      dwarvenDebugInfo;    // is DWARF debug info present?

#if defined(ia64_unknown_linux2_4)
  Address   gp;			 // The gp for this object.
#endif
  bool shared_;
  bool      EEL;                 // true if EEL rewritten
  bool      is_elf64_;           // true if Elf64 file type 

  // for sparc-solaris this is a table of PLT entry addr, function_name
  // for x86-solaris this is a table of GOT entry addr, function_name
  // on sparc-solaris the runtime linker modifies the PLT entry when it
  // binds a function, on X86 the PLT entry is not modified, but it uses
  // an indirect jump to a GOT entry that is modified when the function 
  // is bound....is this correct???? or should it be <PLTentry_addr, name> 
  // for both?
  pdvector<relocationEntry> relocation_table_;

  // all section headers, sorted by address
  // we use these to do a better job of finding the end of symbols
  pdvector<pdElfShdr*> allSectionHdrs;

  // It doesn't look like image's equivalent hashtable is built by
  // the time we need it, and it's hard to get to anyway.
  dictionary_hash< Address, pdstring > symbolNamesByAddr;

  // populates: file_fd_, file_size_, file_ptr_
  bool mmap_file(const char *file, 
		 bool &did_open, bool &did_mmap);

  bool loaded_elf(bool &, Elf* &, 
		  Address &, Address &, Elf_Scn* &, Elf_Scn* &, 
		  Elf_Scn* &, Elf_Scn* &, 
		  Elf_Scn* &, Elf_Scn* &,
		  Elf_Scn*& rel_plt_scnp, Elf_Scn*& plt_scnp, 
		  Elf_Scn*& got_scnp,  Elf_Scn*& dynsym_scnp,
		  Elf_Scn*& dynstr_scnp, bool a_out=false);

  void load_object();
  void load_shared_object();

  // initialize relocation_table_ from .rel[a].plt section entries 
  bool get_relocation_entries(Elf_Scn*& rel_plt_scnp,
			      Elf_Scn*& dynsym_scnp, 
			      Elf_Scn*& dynstr_scnp);

  void parse_symbols(pdvector<Symbol> &allsymbols, 
		     Elf_Data *symdatap, Elf_Data *strdatap,
		     bool shared_library,
		     pdstring module);
  
  void fix_zero_function_sizes(pdvector<Symbol> &allsymbols, bool EEL);
  void override_weak_symbols(pdvector<Symbol> &allsymbols);
  void insert_symbols_shared(pdvector<Symbol> allsymbols);
  void find_code_and_data(Elf *elfp,
       Address txtaddr, Address bssaddr);
  void insert_symbols_static(pdvector<Symbol> allsymbols);
  bool fix_global_symbol_modules_static_stab(Elf_Scn* stabscnp,
					     Elf_Scn* stabstrscnp);
  bool fix_global_symbol_modules_static_dwarf(Elf *elfp);

  void get_valid_memory_areas(Elf *elfp);

#if defined(mips_sgi_irix6_4)

 public:
  Address     get_gp_value()  const { return gp_value; }
  Address     get_rbrk_addr() const { return rbrk_addr; }
  Address     get_base_addr() const { return base_addr; }
  const char *got_entry_name(Address entry_off) const;
  int         got_gp_disp(const char *entry_name) const;

  Address     MIPS_stubs_addr_;   // .MIPS.stubs section
  Address     MIPS_stubs_off_;    // .MIPS.stubs section
  unsigned    MIPS_stubs_size_;   // .MIPS.stubs section

 private:
  Address     gp_value;
  Address     rbrk_addr;
  Address     base_addr;

  int         got_zero_index_;
  int         dynsym_zero_index_;

#endif /* mips_sgi_irix6_4 */
};
 

// ABI-generic wrapper for ELF section header
class pdElfShdr {
public:
  unsigned pd_name;
  Address  pd_addr;
  unsigned pd_offset;
  unsigned pd_size;
  unsigned pd_entsize;
  bool err;

  inline pdElfShdr(Elf_Scn *scnp, bool is_elf64) 
    {
      err = false;
      if (is_elf64) {
	// parse ELF section header (64-bit)
#ifndef USES_ELF32_ONLY
	Elf64_Shdr *shdrp_64 = elf64_getshdr(scnp);
	if (shdrp_64 == NULL) {
	  err = true;
	  return;
	}
	pd_name =    shdrp_64->sh_name;
	pd_addr =    shdrp_64->sh_addr;
	pd_offset =  shdrp_64->sh_offset;
	pd_size =    shdrp_64->sh_size;
	pd_entsize = shdrp_64->sh_entsize;
#endif
      } else {
	// parse ELF section header (32-bit)
	Elf32_Shdr *shdrp_32 = elf32_getshdr(scnp);
	if (shdrp_32 == NULL) {
	  err = true;
	  return;
	}
	pd_name =    shdrp_32->sh_name;
	pd_addr =    shdrp_32->sh_addr;
	pd_offset =  shdrp_32->sh_offset;
	pd_size =    shdrp_32->sh_size;
	pd_entsize = shdrp_32->sh_entsize;
      }
    }  
};

const char *pdelf_get_shnames(Elf *elfp, bool is64);

#endif /* !defined(_Object_elf_h_) */

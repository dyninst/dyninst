/*
 * Copyright (c) 1996 Barton P. Miller
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
 * $Id: Object-elf.h,v 1.33 1999/03/19 00:05:17 csserra Exp $
 * Object-elf32.h: ELF-32 object files.
************************************************************************/


#if !defined(_Object_elf32_h_)
#define _Object_elf32_h_



/************************************************************************
 * header files.
************************************************************************/
#include "util/h/String.h"
#include "util/h/Symbol.h"
#include "util/h/Types.h"
#include "util/h/Vector.h"
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

/***
   The standard symbol table in an elf file is the .symtab section. This section does
   not have information to find the module to which a global symbol belongs, so we must
   also read the .stab section to get this info.
***/

// Declarations for the .stab section.
// These are not declared in any system header files, so we must provide our own
// declarations. The declarations below were taken from:
//       SPARCWorks 3.0.x Debugger Interface, July 1994/

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
#define N_ENDM  0x62 /* end module */
#define N_SO    0x64 /* source directory and file */
#define N_ENTRY 0xa4 /* fortran alternate subroutine entry point */

// Language code -- the desc field in a N_SO entry is a language code
#define N_SO_AS      1 /* assembler source */
#define N_SO_C       2 /* K & R C source */
#define N_SO_ANSI_C  3 /* ANSI C source */
#define N_SO_CC      4 /* C++ source */
#define N_SO_FORTRAN 5 /* fortran source */
#define N_SO_PASCAL  6 /* Pascal source */

// Symbol descriptors
// The format of a name is "<name>:<symbol descriptor><rest of name>
// The following are the descriptors of interest
#define SD_GLOBAL_FUN 'F' /* global function or procedure */
#define SD_PROTOTYPE 'P'  /* function prototypes */
#define SD_GLOBAL_VAR 'G' /* global variable */

// end of stab declarations




/************************************************************************
 * class Object
************************************************************************/

class Object : public AObject {
public:
             // executable file version of constructor....
             Object (const string, void (*)(const char *) = log_msg);
             // shared library version of constructor....
             Object (const string, const Address baseAddr, 
		     void (*)(const char *) = log_msg);
             Object (const Object &);
    virtual ~Object ();

    const Object& operator= (const Object &);

    // for debuggering ....
    const ostream &dump_state_info(ostream &s);

    bool     needs_function_binding() const {return (plt_addr_  > 0);} 
    bool     get_func_binding_table(vector<relocationEntry> &fbt) const;
    bool     get_func_binding_table_ptr(const vector<relocationEntry> *&fbt) const;

private:
    static
    void    log_elferror (void (*)(const char *), const char *);

    bool    EEL ; //set to true if EEL rewritten
    //added char *ptr, to deal with EEL rewritten software
    //
    bool      loaded_elf (int, char *, bool &, Elf* &, Elf32_Ehdr* &, 
			  Elf32_Phdr* &, Address &, Address &, Elf_Scn* &, 
			  Elf_Scn* &, Elf_Scn* &, Elf_Scn* &,
    			  Elf_Scn*& rel_plt_scnp, Elf_Scn*& plt_scnp, 
			  Elf_Scn*& got_scnp,  Elf_Scn*& dynsym_scnp,
			  Elf_Scn*& dynstr_scnp, bool a_out=false);

    // Code for loading in executable files && shared libraries, 
    //  respectively....
    void     load_object ();
    void     load_shared_object ();

    // initialize relocation_table_ from .rel.plt or .rela.plt section entryies 
    bool     get_relocation_entries(Elf_Scn*& rel_plt_scnp,
			Elf_Scn*& dynsymscnp, Elf_Scn*& dynstrcnp);

    // elf-specific stuff from dynamic executables and shared objects
    Address 	plt_addr_;	// address of _PROCEDURE_LINKAGE_TABLE_ 
    u_int 	plt_size_;
    u_int 	plt_entry_size_;
    Address 	got_addr_;	// address of _GLOBAL_OFFSET_TABLE_
    Address 	rel_plt_addr_;	// address of .rela.plt or .rel.plt section 
    u_int 	rel_plt_size_;
    u_int 	rel_plt_entry_size_;
    Address 	dyn_sym_addr_;	// address of .dynsym section
    Address 	dyn_str_addr_;	// address of .dynstr section

    // for sparc-solaris this is a table of PLT entry addr, function_name
    // for x86-solaris this is a table of GOT entry addr, function_name
    // on sparc-solaris the runtime linker modifies the PLT entry when it
    // binds a function, on X86 the PLT entry is not modified, but it uses
    // an indirect jump to a GOT entry that is modified when the function 
    // is bound....is this correct???? or should it be <PLTentry_addr, name> 
    // for both?
    vector<relocationEntry> relocation_table_;


    // NEW FUNCTIONS - mcheyney, 970910
    void parse_symbols(vector<Symbol> &allsymbols, Elf32_Sym *syms,
        unsigned nsyms, const char *strs, bool shared_library,
        string module);

    void fix_zero_function_sizes(vector<Symbol> &allsymbols, bool EEL);
    void override_weak_symbols(vector<Symbol> &allsymbols);
    void insert_symbols_shared(vector<Symbol> allsymbols);
    void find_code_and_data(Elf32_Ehdr* ehdrp, Elf32_Phdr* phdrp,
        char *ptr, Address txtaddr, Address bssaddr);
    void insert_symbols_static(vector<Symbol> allsymbols,
        dictionary_hash<string, Symbol> &global_symbols);
    void fix_global_symbol_modules_static_stab(
        dictionary_hash<string, Symbol> &global_symbols,
        Elf_Scn* stabscnp, Elf_Scn* stabstrscnp);
    void fix_global_symbol_unknowns_static(
        dictionary_hash<string, Symbol> &global_symbols);

#if defined(mips_sgi_irix6_4)
    void fix_global_symbol_modules_static_dwarf(
	dictionary_hash<string, Symbol> &global_symbols, Elf *elfp);

 public:
    Address get_gp_value()  const { return gp_value; }
    Address get_rbrk_addr() const { return rbrk_addr; }
    Address get_base_addr() const { return base_addr; }
 private:
    Address gp_value;
    Address rbrk_addr;
    Address base_addr;
#endif /* mips_sgi_irix6_4 */

};
 

#endif /* !defined(_Object_elf32_h_) */







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
 * Object-elf32.h: ELF-32 object files.
************************************************************************/


#if !defined(_Object_elf32_h_)
#define _Object_elf32_h_



/************************************************************************
 * header files.
************************************************************************/

#include <util/h/DictionaryLite.h>
#include <util/h/String.h>
#include <util/h/Symbol.h>
#include <util/h/Types.h>
#include <util/h/Vector.h>
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
             Object (const string, void (*)(const char *) = log_msg);
             Object (const string, u_int baseAddr, 
		     void (*)(const char *) = log_msg);
             Object (const Object &);
    virtual ~Object ();

    const Object& operator= (const Object &);

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
			  Elf32_Phdr* &, unsigned &, unsigned &, Elf_Scn* &, 
			  Elf_Scn* &, Elf_Scn* &, Elf_Scn* &,
    			  Elf_Scn*& rel_plt_scnp, Elf_Scn*& plt_scnp, 
			  Elf_Scn*& got_scnp,  Elf_Scn*& dynsym_scnp,
			  Elf_Scn*& dynstr_scnp);
    bool      loaded_elf_obj (int, bool &, Elf* &, Elf32_Ehdr* &,Elf32_Phdr* &,
			      unsigned &, unsigned &, Elf_Scn* &, Elf_Scn*&,
    			      Elf_Scn*& rel_plt_scnp, Elf_Scn*& plt_scnp, 
			      Elf_Scn*& got_scnp,  Elf_Scn*& dynsym_scnp,
			      Elf_Scn*& dynstr_scnp);
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
};

inline
Object::~Object() {
}

inline
const Object&
Object::operator=(const Object& obj) {

    (void) AObject::operator=(obj);

    plt_addr_ = obj.plt_addr_;
    plt_size_ = obj.plt_size_;
    plt_entry_size_ = obj.plt_entry_size_;
    got_addr_ = obj.got_addr_;
    rel_plt_addr_ = obj.rel_plt_addr_;
    rel_plt_size_ = obj.rel_plt_size_;
    rel_plt_entry_size_ = obj.rel_plt_entry_size_;
    dyn_sym_addr_ = obj.dyn_sym_addr_;
    dyn_str_addr_ = obj.dyn_str_addr_;
    relocation_table_  = obj.relocation_table_;
    return *this;
}

inline void Object::log_elferror(void (*pfunc)(const char *), const char* msg) {
    const char* err = elf_errmsg(elf_errno());
    log_printf(pfunc, "%s: %s\n", msg, err ? err : "(bad elf error)");
}



//EEL
//
#define EXTRA_SPACE 8     //added some EXTRA_SPACE space, such that,
                       // we won't look for functions in the data space


// Added one extra parameter 'char *ptr', for EEL rewritten software
// code_ptr_, code_offset_, code_len_ is calculated in this function
// For more detail , see comments with the word 'EEL'
//
inline
bool
Object::loaded_elf(int fd, char *ptr, bool& did_elf, Elf*& elfp, 
    Elf32_Ehdr*& ehdrp,
    Elf32_Phdr*& phdrp, unsigned& txtaddr, unsigned& bssaddr,
    Elf_Scn*& symscnp, Elf_Scn*& strscnp, Elf_Scn*& stabscnp, 
    Elf_Scn*& stabstrscnp, Elf_Scn*& rel_plt_scnp, Elf_Scn*& plt_scnp, 
    Elf_Scn*& got_scnp,  Elf_Scn*& dynsym_scnp, Elf_Scn*& dynstr_scnp) {

    elf_version(EV_CURRENT);
    elf_errno();

    if (((elfp = elf_begin(fd, ELF_C_READ, 0)) == 0)
        || (elf_kind(elfp) != ELF_K_ELF)) {
        log_elferror(err_func_, "opening file");
        return false;
    }
    did_elf = true;

    if (((ehdrp = elf32_getehdr(elfp)) == 0)
        || (ehdrp->e_ident[EI_CLASS] != ELFCLASS32)
        || (ehdrp->e_type != ET_EXEC)
        || (ehdrp->e_phoff == 0)
        || (ehdrp->e_shoff == 0)
        || (ehdrp->e_phnum == 0)
        || (ehdrp->e_shnum == 0)) {
        log_elferror(err_func_, "2: loading eheader");
        return false;
    }

    if ((phdrp = elf32_getphdr(elfp)) == 0) {
        log_elferror(err_func_, "loading pheader");
        return false;
    }

    Elf_Scn*    shstrscnp  = 0;
    Elf32_Shdr* shstrshdrp = 0;
    Elf_Data*   shstrdatap = 0;
    if (((shstrscnp = elf_getscn(elfp, ehdrp->e_shstrndx)) == 0)
        || ((shstrshdrp = elf32_getshdr(shstrscnp)) == 0)
        || ((shstrdatap = elf_getdata(shstrscnp, 0)) == 0)) {
        log_elferror(err_func_, "loading header section");
        return false;
    }

    const char* shnames = (const char *) shstrdatap->d_buf;
    Elf_Scn*    scnp    = 0;
    while ((scnp = elf_nextscn(elfp, scnp)) != 0) {
        Elf32_Shdr* shdrp = elf32_getshdr(scnp);
        if (!shdrp) {
            log_elferror(err_func_, "scanning sections");
            return false;
        }

        const char* EDITED_TEXT_NAME   = ".edited_text";
        const char* TEXT_NAME   = ".text";
        const char* BSS_NAME    = ".bss";
        const char* SYMTAB_NAME = ".symtab";
        const char* STRTAB_NAME = ".strtab";
        const char* STAB_NAME   = ".stab";
        const char* STABSTR_NAME= ".stabstr";

	// sections from dynamic executables and shared objects
        const char* PLT_NAME = ".plt";
        const char* REL_PLT_NAME = ".rela.plt";	   // sparc-solaris
        const char* REL_PLT_NAME2 = ".rel.plt";	   // x86-solaris
        const char* GOT_NAME = ".got";	   
        const char* DYNSYM_NAME = ".dynsym";	   
        const char* DYNSTR_NAME = ".dynstr";	   

        const char* name        = (const char *) &shnames[shdrp->sh_name];

	if (strcmp(name, EDITED_TEXT_NAME) == 0) {
        	// EEL rewriten executable
                printf("This is an EEL rewritten executable \n") ;
		EEL = true ;
                txtaddr = shdrp->sh_addr;
                code_ptr_ = (Word *) ((void*)&ptr[shdrp->sh_offset-EXTRA_SPACE]);
                code_off_ = (Address) shdrp->sh_addr -EXTRA_SPACE ;
                code_len_ = ((unsigned) shdrp->sh_size + EXTRA_SPACE) / sizeof(Word);
        }
        if (strcmp(name, TEXT_NAME) == 0) {
            txtaddr = shdrp->sh_addr;
        }
        else if (strcmp(name, BSS_NAME) == 0) {
            bssaddr = shdrp->sh_addr;
        }
        else if (strcmp(name, SYMTAB_NAME) == 0) {
            symscnp = scnp;
        }
        else if (strcmp(name, STRTAB_NAME) == 0) {
            strscnp = scnp;
        }
        else if (strcmp(name, STAB_NAME) == 0) {
            stabscnp = scnp;
        }
        else if (strcmp(name, STABSTR_NAME) == 0) {
            stabstrscnp = scnp;
        }
        else if ((strcmp(name, REL_PLT_NAME) == 0) 
		|| (strcmp(name, REL_PLT_NAME2) == 0)) {
             rel_plt_scnp = scnp;
	     rel_plt_addr_ = shdrp->sh_addr;
	     rel_plt_size_ = shdrp->sh_size;
	     rel_plt_entry_size_ = shdrp->sh_entsize;
        }
        else if (strcmp(name, PLT_NAME) == 0) {
            plt_scnp = scnp;
	    plt_addr_ = shdrp->sh_addr;
	    plt_size_ = shdrp->sh_size;
	    plt_entry_size_ = shdrp->sh_entsize;
        }
        else if (strcmp(name, GOT_NAME) == 0) {
	    got_scnp = scnp;
	    got_addr_ = shdrp->sh_addr;
	}
	else if (strcmp(name, DYNSYM_NAME) == 0) {
            dynsym_scnp = scnp;
	    dyn_sym_addr_ = shdrp->sh_addr;
	}
        else if (strcmp(name, DYNSTR_NAME) == 0) {
	    dynstr_scnp = scnp;
	    dyn_str_addr_ = shdrp->sh_addr;
	}

    }
    if (!txtaddr || !bssaddr || !symscnp || !strscnp) {
        log_elferror(err_func_, "no text/bss/symbol/string section");
        return false;
    }

    return true;
}


inline
bool
Object::loaded_elf_obj(int fd, bool& did_elf, Elf*& elfp, Elf32_Ehdr*& ehdrp,
    Elf32_Phdr*& phdrp, unsigned& txtaddr, unsigned& bssaddr,
    Elf_Scn*& symscnp, Elf_Scn*& strscnp, Elf_Scn*& rel_plt_scnp, 
    Elf_Scn*& plt_scnp, Elf_Scn*& got_scnp, Elf_Scn*& dynsym_scnp,
    Elf_Scn*& dynstr_scnp) {

    elf_version(EV_CURRENT);
    elf_errno();

    if (((elfp = elf_begin(fd, ELF_C_READ, 0)) == 0)
        || (elf_kind(elfp) != ELF_K_ELF)) {
        log_elferror(err_func_, "opening file");
        return false;
    }
    did_elf = true;

    if (((ehdrp = elf32_getehdr(elfp)) == 0)
        || (ehdrp->e_ident[EI_CLASS] != ELFCLASS32)
        || (ehdrp->e_type != ET_DYN)
        || (ehdrp->e_phoff == 0)
        || (ehdrp->e_shoff == 0)
        || (ehdrp->e_phnum == 0)
        || (ehdrp->e_shnum == 0)) {
        log_elferror(err_func_, "loading eheader");
        return false;
    }

    if ((phdrp = elf32_getphdr(elfp)) == 0) {
        log_elferror(err_func_, "loading pheader");
        return false;
    }

    Elf_Scn*    shstrscnp  = 0;
    Elf32_Shdr* shstrshdrp = 0;
    Elf_Data*   shstrdatap = 0;
    if (((shstrscnp = elf_getscn(elfp, ehdrp->e_shstrndx)) == 0)
        || ((shstrshdrp = elf32_getshdr(shstrscnp)) == 0)
        || ((shstrdatap = elf_getdata(shstrscnp, 0)) == 0)) {
        log_elferror(err_func_, "loading header section");
        return false;
    }

    const char* shnames = (const char *) shstrdatap->d_buf;
    Elf_Scn*    scnp    = 0;
    while ((scnp = elf_nextscn(elfp, scnp)) != 0) {
        Elf32_Shdr* shdrp = elf32_getshdr(scnp);
        if (!shdrp) {
            log_elferror(err_func_, "scanning sections");
            return false;
        }

        const char* TEXT_NAME   = ".text";
        const char* BSS_NAME    = ".bss";
        const char* SYMTAB_NAME = ".symtab";
        const char* STRTAB_NAME = ".strtab";

	// sections from dynamic executables and shared objects
        const char* PLT_NAME = ".plt";
        const char* REL_PLT_NAME = ".rela.plt";	   // sparc-solaris
        const char* REL_PLT_NAME2 = ".rel.plt";	   // x86-solaris
        const char* GOT_NAME = ".got";	   
        const char* DYNSYM_NAME = ".dynsym";	   
        const char* DYNSTR_NAME = ".dynstr";	   

        const char* name        = (const char *) &shnames[shdrp->sh_name];

        if (strcmp(name, TEXT_NAME) == 0) {
            txtaddr = shdrp->sh_addr;
        }
        else if (strcmp(name, BSS_NAME) == 0) {
            bssaddr = shdrp->sh_addr;
        }
        else if (strcmp(name, SYMTAB_NAME) == 0) {
            symscnp = scnp;
        }
        else if (strcmp(name, STRTAB_NAME) == 0) {
            strscnp = scnp;
        }
        else if ((strcmp(name, REL_PLT_NAME) == 0) 
		|| (strcmp(name, REL_PLT_NAME2) == 0)) {
             rel_plt_scnp = scnp;
	     rel_plt_addr_ = shdrp->sh_addr;
	     rel_plt_size_ = shdrp->sh_size;
	     rel_plt_entry_size_ = shdrp->sh_entsize;
        }
        else if (strcmp(name, PLT_NAME) == 0) {
            plt_scnp = scnp;
	    plt_addr_ = shdrp->sh_addr;
	    plt_size_ = shdrp->sh_size;
	    plt_entry_size_ = shdrp->sh_entsize;
        }
        else if (strcmp(name, GOT_NAME) == 0) {
	    got_scnp = scnp;
	    got_addr_ = shdrp->sh_addr;
	}
        else if (strcmp(name, DYNSYM_NAME) == 0) {
	    dynsym_scnp = scnp;
	    dyn_sym_addr_ = shdrp->sh_addr;
	}
        else if (strcmp(name, DYNSTR_NAME) == 0) {
	    dynstr_scnp = scnp;
	    dyn_str_addr_ = shdrp->sh_addr;
	}
    }
    string temp = string(" text: ");
    temp += string((u_int)txtaddr);
    temp += string(" bss: ");
    temp += string((u_int)bssaddr);
    temp += string(" symtab: ");
    temp += string((u_int)symscnp);
    temp += string(" strtab: ");
    temp += string((u_int)strscnp);
    temp += string(" ehdrp: ");
    temp += string((u_int)ehdrp);
    temp += string(" phdrp: ");
    temp += string((u_int)phdrp);
    temp += string("\n");
    // log_elferror(err_func_, P_strdup(temp.string_of()));

    if (!txtaddr || !bssaddr || !symscnp || !strscnp) {
        log_elferror(err_func_, "no text/bss/symbol/string section");
        return false;
    }

    return true;
}


static int symbol_compare(const void *x, const void *y) {
    const Symbol *s1 = (const Symbol *)x;
    const Symbol *s2 = (const Symbol *)y;
    return (s1->addr() - s2->addr());
}

inline bool Object::get_relocation_entries(Elf_Scn*& rel_plt_scnp,
			Elf_Scn*& dynsymscnp, Elf_Scn*& dynstrscnp) {

#if defined (i386_unknown_solaris2_5)
        Elf32_Rel *next_entry = 0;
        Elf32_Rel *entries = 0;
#else
        Elf32_Rela *next_entry = 0;
        Elf32_Rela *entries = 0;
#endif

    if(rel_plt_size_ && rel_plt_addr_) {
	Elf_Data *reldatap = elf_getdata(rel_plt_scnp, 0);
	Elf_Data* symdatap = elf_getdata(dynsymscnp, 0);
	Elf_Data* strdatap = elf_getdata(dynstrscnp, 0);
	if(!reldatap || !symdatap || !strdatap) return false;

	Elf32_Sym*  syms   = (Elf32_Sym *) symdatap->d_buf;
	const char* strs   = (const char *) strdatap->d_buf;
	Address next_plt_entry_addr = plt_addr_;

#if defined (i386_unknown_solaris2_5)
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
            relocation_table_ += re; 
	    next_entry++;
	    next_plt_entry_addr += plt_entry_size_;
	}
    }
    return true;
}


inline
void
Object::load_object() {
    const char* file = file_.string_of();
    struct stat st;
    int         fd   = -1;
    char*       ptr  = 0;
    Elf*        elfp = 0;

    bool        did_open = false;
    bool        did_elf  = false;

    /* try */ {
        if (((fd = open(file, O_RDONLY)) == -1) || (fstat(fd, &st) == -1)) {
            log_perror(err_func_, file);
            /* throw exception */ goto cleanup;
        }
        did_open = true;

        if ((ptr = (char *) mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0))
            == (char *) -1) {
            log_perror(err_func_, "mmap");
            /* throw exception */ goto cleanup;
        }

        Elf32_Ehdr* ehdrp   = 0;
        Elf32_Phdr* phdrp   = 0;
        Elf_Scn*    symscnp = 0;
        Elf_Scn*    strscnp = 0;
        Elf_Scn*    stabscnp = 0;
        Elf_Scn*    stabstrscnp = 0;
        unsigned    txtaddr = 0;
        unsigned    bssaddr = 0;
    	Elf_Scn* rel_plt_scnp = 0;
	Elf_Scn* plt_scnp = 0; 
	Elf_Scn* got_scnp = 0;
	Elf_Scn* dynsym_scnp = 0;
	Elf_Scn* dynstr_scnp = 0;

	// EEL, initialize the stuff to zero, so that we know, it is not 
	// EEL rewritten, if they have not been changed by loaded_elf
	//
        code_ptr_ = 0; code_off_ = 0 ;  code_len_ = 0 ;

	// EEL, added one more parameter
        if (!loaded_elf(fd, ptr, did_elf, elfp, ehdrp, phdrp, txtaddr,
                bssaddr, symscnp, strscnp, stabscnp, stabstrscnp,
		rel_plt_scnp,plt_scnp,got_scnp,dynsym_scnp,dynstr_scnp)) {
                /* throw exception */ goto cleanup;
	}

        for (unsigned i0 = 0; i0 < ehdrp->e_phnum; i0++) {
            if(!code_ptr_ && !code_off_ && !code_len_)
            {//NON EEL
	     //This can be tested differently, by testing if EEL is false
             //  printf("This is a regular executable\n") ;

            	if ((phdrp[i0].p_vaddr <= txtaddr)
                	&& ((phdrp[i0].p_vaddr+phdrp[i0].p_memsz) >= txtaddr)) {
                	code_ptr_ = (Word *) ((void*)&ptr[phdrp[i0].p_offset]);
                	code_off_ = (Address) phdrp[i0].p_vaddr;
                	code_len_ = (unsigned) phdrp[i0].p_memsz / sizeof(Word);
            	}
            }
            if ((phdrp[i0].p_vaddr <= bssaddr)
                && ((phdrp[i0].p_vaddr+phdrp[i0].p_memsz) >= bssaddr)) {
                data_ptr_ = (Word *) ((void *) &ptr[phdrp[i0].p_offset]);
                data_off_ = (Address) phdrp[i0].p_vaddr;
                data_len_ = (unsigned) phdrp[i0].p_memsz / sizeof(Word);
            }
        }
        if (!code_ptr_ || !code_off_ || !code_len_) {
            log_printf(err_func_, "cannot locate instructions\n");
            /* throw exception */ goto cleanup;
        }
        if (!data_ptr_ || !data_off_ || !data_len_) {
            log_printf(err_func_, "cannot locate data segment\n");
            /* throw exception */ goto cleanup;
        }

        Elf_Data* symdatap = elf_getdata(symscnp, 0);
        Elf_Data* strdatap = elf_getdata(strscnp, 0);
        if (!symdatap || !strdatap) {
            log_elferror(err_func_, "locating symbol/string data");
            /* throw exception */ goto cleanup;
        }

        Elf32_Sym*  syms   = (Elf32_Sym *) symdatap->d_buf;
        unsigned    nsyms  = symdatap->d_size / sizeof(Elf32_Sym);
        const char* strs   = (const char *) strdatap->d_buf;
        string      module = "DEFAULT_MODULE";
        string      name   = "DEFAULT_NAME";

        // global symbols are put in global_symbols. Later we read the
        // stab section to find the module to where they belong.
        dictionary_hash<string, Symbol> global_symbols(string::hash);

        vector<Symbol> allsymbols;

        for (unsigned i1 = 0; i1 < nsyms; i1++) {
	  // First, we must check st_shndx. 
	  // if st_shndx == SHN_UNDEF, this is an undefined symbol,
	  // probably defined in a shared object that will be dynamically
	  // linked with the executable. We just ignore it for now.
	  if (syms[i1].st_shndx != SHN_UNDEF) {
            bool st_kludge = false;
            Symbol::SymbolType type = Symbol::PDST_UNKNOWN;
            switch (ELF32_ST_TYPE(syms[i1].st_info)) {
            case STT_FILE:
                module = string(&strs[syms[i1].st_name]);
                type   = Symbol::PDST_MODULE;
                break;

            case STT_OBJECT:
                type = Symbol::PDST_OBJECT;
                break;

            case STT_FUNC:
                type = Symbol::PDST_FUNCTION;
                break;

            default:
                continue;
            }

            name = string(&strs[syms[i1].st_name]);

            if (ELF32_ST_BIND(syms[i1].st_info) == STB_LOCAL) {
               allsymbols += Symbol(name, module, type, Symbol::SL_LOCAL,
                                    syms[i1].st_value, st_kludge, 
                                    syms[i1].st_size);
	    }
            else {
	       allsymbols += Symbol(name, string(""), type, Symbol::SL_GLOBAL,
                                    syms[i1].st_value, st_kludge,
                                    syms[i1].st_size);
	    }
	  }
        }

        // some functions may have size zero in the symbol table,
        // we need to find the correct size
	// EEL: for EEL rewritten software, the size maybe incorrect
	//      So that we have to recalculate the size for all functions

        allsymbols.sort(symbol_compare);
        unsigned nsymbols = allsymbols.size();
        for (unsigned u = 0; u < nsymbols; u++) {
          if (allsymbols[u].type() == Symbol::PDST_FUNCTION
               && (EEL || allsymbols[u].size() == 0)) {
	    unsigned v = u+1;
	    while (v < nsymbols && allsymbols[v].addr() == allsymbols[u].addr())
              v++;
            if (v < nsymbols) {
              allsymbols[u].change_size((unsigned)allsymbols[v].addr()
                                        - (unsigned)allsymbols[u].addr());
            }
          }

          // We are done with the local symbols. We save the global so that
          // we can get their module from the .stab section.
          if (allsymbols[u].linkage() == Symbol::SL_LOCAL)
	    symbols_[allsymbols[u].name()] = allsymbols[u];
          else {
            // globals should be unique
            assert(!(global_symbols.defines(allsymbols[u].name()))); 
	    global_symbols[allsymbols[u].name()] = allsymbols[u];
	  }
	}      
	

        // Read the stab section to find the module of global symbols.
        // The symbols appear in the stab section by module. A module begins
        // with a symbol of type N_UNDF and ends with a symbol of type N_ENDM.
        // All the symbols in between those two symbols belong to the module.

        Elf_Data* stabdatap = elf_getdata(stabscnp, 0);
        Elf_Data* stabstrdatap = elf_getdata(stabstrscnp, 0);
        struct stab_entry *stabsyms = 0;
        unsigned stab_nsyms;
        const char *stabstrs = 0;

        if (stabdatap && stabstrdatap) {
	  stabsyms = (struct stab_entry *) stabdatap->d_buf;
	  stab_nsyms = stabdatap->d_size / sizeof(struct stab_entry);
	  stabstrs = (const char *) stabstrdatap->d_buf;
	}
	else 
	  stab_nsyms = 0;

        // the stabstr contains one string table for each module.
        // stabstr_offset gives the offset from the begining of stabstr of the
        // string table for the current module.
        // stabstr_nextoffset gives the offset for the next module.
        unsigned stabstr_offset = 0;
        unsigned stabstr_nextoffset = 0;

        bool is_fortran = false;  // is the current module fortran code?
           /* we must know if we are reading a fortran module because fortran
              symbol names are different in the .stab and .symtab sections.
              A symbol that appears as 'foo' in the .stab section, appears
              as 'foo_' in .symtab.
           */
        module = "";
  
        for (unsigned i = 0; i < stab_nsyms; i++) {
	  switch(stabsyms[i].type) {
	  case N_UNDF: /* start of object file */
	    assert(stabsyms[i].name == 1);
	    stabstr_offset = stabstr_nextoffset;
	    // stabsyms[i].val has the size of the string table of this module.
	    // We use this value to compute the offset of the next string table.
	    stabstr_nextoffset = stabstr_offset + stabsyms[i].val;
	    module = string(&stabstrs[stabstr_offset+stabsyms[i].name]);
	    break;

	  case N_ENDM: /* end of object file */
	    is_fortran = false;
	    module = "";
	    break;

	  case N_SO: /* compilation source or file name */
	    if (stabsyms[i].desc == N_SO_FORTRAN)
	      is_fortran = true;
	    break;

          case N_ENTRY: /* fortran alternate subroutine entry point */
	  case N_FUN: /* function */
	  case N_GSYM: /* global symbol */
	    // the name string of a function or object appears in the stab string table
	    // as <symbol name>:<symbol descriptor><other stuff>
	    // where <symbol descriptor> is a one char code.
	    // we must extract the name and descriptor from the string
          {
	    const char *p = &stabstrs[stabstr_offset+stabsyms[i].name];
	    const char *q = strchr(p,':');
	    assert(q);
	    unsigned len = q - p;
	    assert(len > 0);
	    char *sname = new char[len+1];
	    strncpy(sname, p, len);
	    sname[len] = 0;
	    
	    // q points to the ':' in the name string, so 
	    // q[1] is the symbol descriptor. We must check the symbol descriptor
	    // here to skip things we are not interested in, such as local functions
	    // and prototypes.
	    if (q[1] == SD_GLOBAL_FUN || q[1] == SD_GLOBAL_VAR || stabsyms[i].type==N_ENTRY) { 
	      string SymName = string(sname);
	      bool res = global_symbols.defines(SymName);
	      if (!res && is_fortran) {
                // Fortran symbols usually appear with an '_' appended in .symtab,
                // but not on .stab
		SymName += "_";
		res = global_symbols.defines(SymName);
	      }

              if (!res) break;
//              assert(res); // All globals in .stab should be defined in .symtab

	      Symbol sym = global_symbols[SymName];
	      symbols_[SymName] = Symbol(sym.name(), module, sym.type(), sym.linkage(), 
				  sym.addr(), sym.kludge(), sym.size());
	    }
          }
	    break;

	  default:
	    /* ignore other entries */
	    break;
	  }
	}

        /* The remaing symbols go without module */
        vector<string> k = global_symbols.keys();
        for (unsigned i2 = 0; i2 < k.size(); i2++) {
	  Symbol sym = global_symbols[k[i2]];
	  if (!(symbols_.defines(sym.name())))
             symbols_[sym.name()] = sym;
	}

	if(rel_plt_scnp && dynsym_scnp && dynstr_scnp) {
	    if(!get_relocation_entries(rel_plt_scnp,dynsym_scnp,dynstr_scnp)) {
		goto cleanup;
            }
	}

    }  /* try */

    /* catch */
cleanup: {
        if (did_elf && (elf_end(elfp) != 0)) {
            log_elferror(err_func_, "closing file");
        }
        if (did_open && (close(fd) == -1)) {
            log_perror(err_func_, "close");
        }
    }
}


inline
void
Object::load_shared_object() {
    const char* file = file_.string_of();
    struct stat st;
    int         fd   = -1;
    char*       ptr  = 0;
    Elf*        elfp = 0;

    bool        did_open = false;
    bool        did_elf  = false;

    /* try */ {
        if (((fd = open(file, O_RDONLY)) == -1) || (fstat(fd, &st) == -1)) {
            log_perror(err_func_, file);
            /* throw exception */ goto cleanup2;
        }
        did_open = true;

        if ((ptr = (char *) mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0))
            == (char *) -1) {
            log_perror(err_func_, "mmap");
            /* throw exception */ goto cleanup2;
        }

        Elf32_Ehdr* ehdrp   = 0;  /* ELF header */
        Elf32_Phdr* phdrp   = 0;  /* program header */
        Elf_Scn*    symscnp = 0;
        Elf_Scn*    strscnp = 0;
        unsigned    txtaddr = 0;
        unsigned    bssaddr = 0;
    	Elf_Scn* rel_plt_scnp = 0;
	Elf_Scn* plt_scnp = 0; 
	Elf_Scn* got_scnp = 0;
	Elf_Scn* dynsym_scnp = 0;
	Elf_Scn* dynstr_scnp = 0;

        if (!loaded_elf_obj(fd, did_elf, elfp, ehdrp, phdrp, txtaddr,
			    bssaddr, symscnp, strscnp, rel_plt_scnp,
			    plt_scnp,got_scnp,dynsym_scnp, dynstr_scnp)) {
                /* throw exception */ goto cleanup2;
	}

        for (unsigned i0 = 0; i0 < ehdrp->e_phnum; i0++) {
            if ((phdrp[i0].p_vaddr <= txtaddr)
                && ((phdrp[i0].p_vaddr+phdrp[i0].p_memsz) >= txtaddr)) {
                code_ptr_ = (Word *) ((void*)&ptr[phdrp[i0].p_offset]);
                code_off_ = (Address) phdrp[i0].p_vaddr;
                code_len_ = (unsigned) phdrp[i0].p_memsz / sizeof(Word);
            }
            else if ((phdrp[i0].p_vaddr <= bssaddr)
                && ((phdrp[i0].p_vaddr+phdrp[i0].p_memsz) >= bssaddr)) {
                data_ptr_ = (Word *) ((void *) &ptr[phdrp[i0].p_offset]);
                data_off_ = (Address) phdrp[i0].p_vaddr;
                data_len_ = (unsigned) phdrp[i0].p_memsz / sizeof(Word);
            }
        }

        Elf_Data* symdatap = elf_getdata(symscnp, 0);
        Elf_Data* strdatap = elf_getdata(strscnp, 0);
        if (!symdatap || !strdatap) {
            log_elferror(err_func_, "locating symbol/string data");
            /* throw exception */ goto cleanup2;
        }

        Elf32_Sym*  syms   = (Elf32_Sym *) symdatap->d_buf;
        unsigned    nsyms  = symdatap->d_size / sizeof(Elf32_Sym);
        const char* strs   = (const char *) strdatap->d_buf;
        string      module = "DEFAULT_MODULE";
        string      name   = "DEFAULT_NAME";

	// for shared objects add a module that is the file name
  	// and add all the global symbols as functions 
        const char *libname = file_.string_of();
	// find short name
	const char *last = 0;
	for(u_int i=0; i < file_.length();i++) {
	    if(libname[i] == '/'){
	        last = (const char *)(&(libname[i]));
                // log_elferror(err_func_, P_strdup(last));
            }
	}
	if(last){
	    module = (const char *)(last +1);  
	}
	else{
	    module = string("DEFAULT_MODULE");  
	}
	// string blah = string("module name: ");
        // blah += module.string_of();
	// blah += ("\n");
        // log_elferror(err_func_, P_strdup(blah.string_of()));

        vector<Symbol> allsymbols;
	bool found = false;
        for (unsigned i1 = 0; i1 < nsyms; i1++) {
	  // First, we must check st_shndx. 
	  // if st_shndx == SHN_UNDEF, this is an undefined symbol,
	  // probably defined in a shared object that will be dynamically
	  // linked with the executable. We just ignore it for now.
	  if (syms[i1].st_shndx != SHN_UNDEF) {
            bool st_kludge = false;
            Symbol::SymbolType type = Symbol::PDST_UNKNOWN;
            switch (ELF32_ST_TYPE(syms[i1].st_info)) {
            case STT_FILE: {
		string temp2 = string(&strs[syms[i1].st_name]);
		if(temp2 == module){
                    module = string(&strs[syms[i1].st_name]);
                    type   = Symbol::PDST_MODULE;
		    found = true;
		}
                break;
		}

            case STT_OBJECT:
                type = Symbol::PDST_OBJECT;
                break;

            case STT_FUNC:
                type = Symbol::PDST_FUNCTION;
                break;

            case STT_NOTYPE:
                type = Symbol::PDST_NOTYPE;
                break;

            default:
                continue;
            }

            name = string(&strs[syms[i1].st_name]);

  	    // only add symbols of type STB_LOCAL and  FILE if they are 
	    // the shared object name
            if ((ELF32_ST_BIND(syms[i1].st_info) == STB_LOCAL) && (found)){
	           symbols_[name] = Symbol(name, module, type, Symbol::SL_LOCAL,
                                    syms[i1].st_value, st_kludge, 
                                    syms[i1].st_size);
                   found = false;
	    }
	    else if((ELF32_ST_BIND(syms[i1].st_info) == STB_LOCAL)
			&& (ELF32_ST_TYPE(syms[i1].st_info) != STT_FUNC)) {
	        allsymbols += Symbol(name, module, type, Symbol::SL_LOCAL,
						syms[i1].st_value, st_kludge,
		  			        syms[i1].st_size);
            }
	    else {
	       if(ELF32_ST_BIND(syms[i1].st_info) == STB_WEAK){
	           allsymbols += Symbol(name, module, 
				        type, Symbol::SL_WEAK,
                                        syms[i1].st_value, st_kludge,
                                        syms[i1].st_size);
	       }
	       else{
	           allsymbols += Symbol(name, module, 
				        type, Symbol::SL_GLOBAL,
                                        syms[i1].st_value, st_kludge,
                                        syms[i1].st_size);
               }
	    }
	  }
        }


	// Sort all the symbols, and fix the sizes
	allsymbols.sort(symbol_compare);

	// if the symbol is type PDST_FUNCTION and the next symbol is 
	// type PDST_FUNCTION size needs to be changed...this occurs when
	// one function's binding is WEAK and the other is GLOBAL, or when
	// two functions overlap 
	for(u_int i=0; i < (allsymbols.size() -1); i++){
	    u_int new_size = 0;
	    bool  change_size = false;

            // some functions may have size zero in the symbol table
            // we have to fix the size of the global functions
            if (allsymbols[i].type() == Symbol::PDST_FUNCTION
                && allsymbols[i].linkage() == Symbol::SL_GLOBAL
                && allsymbols[i].size() == 0) {
               unsigned j = i+1;
               while (j < allsymbols.size() 
                      && allsymbols[j].addr() == allsymbols[i].addr())
                 j++;
               allsymbols[i].change_size(allsymbols[j].addr()-allsymbols[i].addr());
            }

	    if((allsymbols[i].type() == Symbol::PDST_FUNCTION)
		&& (allsymbols[i+1].type() == Symbol::PDST_FUNCTION)){
		u_int next_start=allsymbols[i].addr()+allsymbols[i].size();

		// if the symbols have the same address and one is weak
		// and the other is global keeep the global one
		if((allsymbols[i].addr() == allsymbols[i+1].addr()) && 
		    (((allsymbols[i].linkage() == Symbol::SL_WEAK) &&
		      (allsymbols[i+1].linkage() == Symbol::SL_GLOBAL)) || 
                     ((allsymbols[i].linkage() == Symbol::SL_GLOBAL) &&
		      (allsymbols[i+1].linkage() == Symbol::SL_WEAK)))) {

		      if(allsymbols[i].linkage() == Symbol::SL_WEAK){
			  allsymbols[i].change_size(0);
		      } else {
			  allsymbols[i+1].change_size(0);
		      }
		}
		else if(next_start > allsymbols[i+1].addr()){
		        new_size =  allsymbols[i+1].addr()-allsymbols[i].addr();
		        change_size = true;
	        }
            }

	    if((allsymbols[i].type() == Symbol::PDST_FUNCTION) && change_size){
                symbols_[allsymbols[i].name()] =
		    Symbol(allsymbols[i].name(), allsymbols[i].module(),
		    allsymbols[i].type(), allsymbols[i].linkage(),
		    allsymbols[i].addr(), allsymbols[i].kludge(),
		    new_size);
	    }
	    else {
                symbols_[allsymbols[i].name()] =
		    Symbol(allsymbols[i].name(), allsymbols[i].module(),
		    allsymbols[i].type(), allsymbols[i].linkage(),
		    allsymbols[i].addr(), allsymbols[i].kludge(),
		    allsymbols[i].size());
	    }
        }
	// add last symbol
	u_int last_sym = allsymbols.size()-1;
        symbols_[allsymbols[last_sym].name()] =
	    Symbol(allsymbols[last_sym].name(), allsymbols[last_sym].module(),
	    allsymbols[last_sym].type(), allsymbols[last_sym].linkage(),
	    allsymbols[last_sym].addr(), allsymbols[last_sym].kludge(),
	    allsymbols[last_sym].size());

	if(rel_plt_scnp && dynsym_scnp && dynstr_scnp) {
	    if(!get_relocation_entries(rel_plt_scnp,dynsym_scnp,dynstr_scnp)) { 
		goto cleanup2;
            }
	}

    }  /* try */

    /* catch */

cleanup2: {
        if (did_elf && (elf_end(elfp) != 0)) {
            log_elferror(err_func_, "closing file");
        }
        if (did_open && (close(fd) == -1)) {
            log_perror(err_func_, "close");
        }
    }
}


inline
Object::Object(const string file, void (*err_func)(const char *))
    : AObject(file, err_func), EEL(false) {
    load_object();
}

inline
Object::Object(const string file, u_int ,void (*err_func)(const char *))
    : AObject(file, err_func), EEL(false)  {
    load_shared_object();
}

inline
Object::Object(const Object& obj)
    : AObject(obj), EEL(false) {
    load_object();
}

inline bool Object::get_func_binding_table(vector<relocationEntry> &fbt) const {
    if(!plt_addr_ || (!relocation_table_.size())) return false;
    fbt = relocation_table_;
    return true;
}

inline bool Object::get_func_binding_table_ptr(const vector<relocationEntry> *&fbt) const {
    if(!plt_addr_ || (!relocation_table_.size())) return false;
    fbt = &relocation_table_;
    return true;
}

#endif /* !defined(_Object_elf32_h_) */

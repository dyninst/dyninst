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

// $Id: Object-elf32.C,v 1.5 1998/08/16 23:18:26 wylie Exp $

/**********************************************
 *
 * Implementation of "Object" (descriptive name, huh??) class for 
 *  32 bit ELF format....
 *
 **********************************************/

#include "util/h/Object.h"
#include "util/h/Object-elf32.h"

#ifdef _Object_elf32_h_

Object::~Object() {
}

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

void Object::log_elferror(void (*pfunc)(const char *), const char* msg) {
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
        || (ehdrp->e_type != ET_EXEC && ehdrp->e_type != ET_DYN)
           // support shared library code here also....
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

    // moved const char declrs out of loop....
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

    while ((scnp = elf_nextscn(elfp, scnp)) != 0) {
        Elf32_Shdr* shdrp = elf32_getshdr(scnp);
        if (!shdrp) {
            log_elferror(err_func_, "scanning sections");
            return false;
        }

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
    if(!symscnp || !strscnp) {
      if(dynsym_scnp && dynstr_scnp){
          symscnp = dynsym_scnp;
	  strscnp = dynstr_scnp;
      }
    }
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

bool Object::get_relocation_entries(Elf_Scn*& rel_plt_scnp,
			Elf_Scn*& dynsymscnp, Elf_Scn*& dynstrscnp) {

#if defined (i386_unknown_solaris2_5) || defined (i386_unknown_linux2_0)
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

#if defined (i386_unknown_solaris2_5) || defined (i386_unknown_linux2_0)
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

/*************************************************************
 *
 *  New (Experimenta) version of code for loading statically linked
 *   object files....
 *
*************************************************************/
void Object::load_object() {
        const char* file = file_.string_of();
    struct stat st;
    int         fd   = -1;
    char*       ptr  = 0;
    Elf*        elfp = 0;

    bool        did_open = false;
    bool        did_elf  = false;

    /* try */ {
        //  try to open file for reading, and also use fstat to read info
        //   about the file in st structure....
        if (((fd = open(file, O_RDONLY)) == -1) || (fstat(fd, &st) == -1)) {
            log_perror(err_func_, file);
            /* throw exception */ goto cleanup;
        }
        did_open = true;

        //  attempt to memory map entire file.  st.st_size should contain file
        //   size (in bytes)....
        if ((ptr = (char *) mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0))
            == (char *) -1) {
            log_perror(err_func_, "mmap");
            /* throw exception */ goto cleanup;
        }

        Elf32_Ehdr* ehdrp   = 0;  /* ELF header */
        Elf32_Phdr* phdrp   = 0;  /* program header */
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

        // And attempt to parse the ELF data structures in the file....
	// EEL, added one more parameter
        if (!loaded_elf(fd, ptr, did_elf, elfp, ehdrp, phdrp, txtaddr,
                bssaddr, symscnp, strscnp, stabscnp, stabstrscnp,
		rel_plt_scnp,plt_scnp,got_scnp,dynsym_scnp,dynstr_scnp)) {
                /* throw exception */ goto cleanup;
	}

 	// find code and data segments....
        find_code_and_data(ehdrp, phdrp, ptr, txtaddr, bssaddr);

	//  if could not find code or data segments, log error....
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
	// Experiment : lets try to be a bit more intelligent about
	// how we initially size the global_symbols table.  
	// dictionary_lite takes an initial # of bins (2nd param), 
	// a max bin load (3rd param), and a grow factor (4th param).
	// Leaving aside the grow factor, lets allocate an initial #
	// of bins = nsyms / max bin load.
        dictionary_hash<string, Symbol> global_symbols(string::hash, nsyms, 1.0);

        vector<Symbol> allsymbols;

	// try to resolve all symbols found in symbol table + 
	//  enter them into <allsymbols>.
	parse_symbols(allsymbols, syms, nsyms, strs, 1, module);

	// Sort all the symbols - for patching symbol data sizes....
	allsymbols.sort(symbol_compare);

	// patch 0 symbol sizes....
	fix_zero_function_sizes(allsymbols, 0);
	
	// override instances of weak symbols w/ instances of strong ones,
	//  also currently has effect of making sure that symbols dont
	//  extend beyond the marked beginning of the next symbo....
	override_weak_symbols(allsymbols);

	// and dump <allsymbols> into (data member) symbols_
	//  or (paramater) <global_symbols> accoriding to linkage.... 
	insert_symbols_static(allsymbols, global_symbols);

	// try to use the .stab section to figure modules for symbols
	//  in global <global_symbols>....
	fix_global_symbol_modules_static(global_symbols, stabscnp, stabstrscnp);               

	if(rel_plt_scnp && dynsym_scnp && dynstr_scnp) {
	    if(!get_relocation_entries(rel_plt_scnp,dynsym_scnp,dynstr_scnp)) {
		goto cleanup;
            }
	}
	
    }  /* try */

        /* catch */
cleanup: 
    {
        if (did_elf && (elf_end(elfp) != 0)) {
            log_elferror(err_func_, "closing file");
        }
        if (did_open && (close(fd) == -1)) {
            log_perror(err_func_, "close");
        }
    }
}


/*************************************************************
 *
 *  New (Experimental) version of code for loading shared
 *   libraries....
 *
*************************************************************/

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
        Elf_Scn*    stabscnp = 0;
        Elf_Scn*    stabstrscnp = 0;
        Elf_Scn*    strscnp = 0;
        unsigned    txtaddr = 0;
        unsigned    bssaddr = 0;
    	Elf_Scn* rel_plt_scnp = 0;
	Elf_Scn* plt_scnp = 0; 
	Elf_Scn* got_scnp = 0;
	Elf_Scn* dynsym_scnp = 0;
	Elf_Scn* dynstr_scnp = 0;

        /***  ptr, stabscnp, stabstrscnp should NOT be filled in if the parsed
              object was a shared library  ***/
	/* The above is not necessarily true...if the shared object was 
	 * compiled with -g then these will be filled in
	 */
        if (!loaded_elf(fd, ptr, did_elf, elfp, ehdrp, phdrp, txtaddr,
			    bssaddr, symscnp, strscnp, stabscnp, stabstrscnp,
                            rel_plt_scnp, plt_scnp,got_scnp,dynsym_scnp, dynstr_scnp)) {
                /* throw exception */ goto cleanup2;
	}

	// find code and data segments....
        find_code_and_data(ehdrp, phdrp, ptr, txtaddr, bssaddr);

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
	
	// temporary vector of symbols
        vector<Symbol> allsymbols;
	
	// try to resolve all symbols found in symbol table + 
	//  enter them into <allsymbols>.
	parse_symbols(allsymbols, syms, nsyms, strs, 1, module);

	// Sort all the symbols - for patching symbol data sizes....
	allsymbols.sort(symbol_compare);

	// patch 0 symbol sizes....
	fix_zero_function_sizes(allsymbols, 0);
	
	// override instances of weak symbols w/ instances of strong ones,
	//  also currently has effect of making sure that symbols dont
	//  extend beyond the marked beginning of the next symbo....
	override_weak_symbols(allsymbols);

	// and inset all found (and theoretically patched) symbols
	//  into (data member) symbols_.
	insert_symbols_shared(allsymbols);
       
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


/*************************************************************
*
*  Run through symbol table <syms> (with <nsyms> entries, where
*   <strs> points to string table), finding all symbols
*   of type in {STT_FILE, STT_OBJECT, STT_FUNC, STT_NOTYPE}, and
*   registering the symbol in allsymbols with appropriate type,
*   scope, etc..
*   shared - indicates whether the object file being parsed is a
*    shared library.  
*   module - only filled in for shared libraries.  Contains name
*    of shared library module.  As per original (spaghetti) code
*    scattered in old load_object && load_shared_object, the symbol
*    reference with the same name as the shared library itself
*    s stuffed directly in1to (data member) symbols_ .... 
*
**************************************************************/
void Object::parse_symbols(vector<Symbol> &allsymbols, Elf32_Sym* syms,
	unsigned nsyms, const char *strs, bool /*shared_library*/, 
        string module) {

    // local vars....
    //  name of symbol, and name of module under which to register function,
    //  respectively.... 
    string name, module_name_used;
    unsigned i1;

    Symbol::SymbolType type;
    Symbol::SymbolLinkage linkage;
    int binding;

    //cerr << "PARSING SYMBOLS FOR MODULE " << module << endl;
    for (i1 = 0; i1 < nsyms; i1++) {
	// First, we must check st_shndx. 
	// if st_shndx == SHN_UNDEF, this is an undefined symbol,
	// probably defined in a shared object that will be dynamically
	// linked with the executable. We just ignore it for now.
	if (syms[i1].st_shndx != SHN_UNDEF) {
	    name = string(&strs[syms[i1].st_name]);
	    //cerr << " Found non undefined symbol : " << name << endl;
            bool st_kludge = false;
            type = Symbol::PDST_UNKNOWN;
	    // resolve symbol type....
            switch (type = (Symbol::SymbolType) ELF32_ST_TYPE(syms[i1].st_info)) {
	    case STT_FILE: {
	        //cerr << "    name matches module name" << endl;
		type   = Symbol::PDST_MODULE;
		//cerr << "  ELF32_ST_TYPE = STT_FILE";
                break;
	    }
            case STT_OBJECT:
	        //cerr << "  ELF32_ST_TYPE = STT_OBJECT";
                type = Symbol::PDST_OBJECT;
                break;

            case STT_FUNC:
	        //cerr << "  ELF32_ST_TYPE = STT_FUNC";
                type = Symbol::PDST_FUNCTION;
                break;

            case STT_NOTYPE:
	        //cerr << "  ELF32_ST_TYPE = STT_NOTYPE";
                type = Symbol::PDST_NOTYPE;
                break;

            default:
	        //cerr << "  ELF32_ST_TYPE not supported";
                continue;
            }

	    // and resolve symbol binding....
	    switch (binding = ELF32_ST_BIND(syms[i1].st_info)) {
	    case STB_LOCAL: 
	        //cerr << "  ELF_32_ST_BIND = STB_LOCAL" << endl;
		linkage = Symbol::SL_LOCAL;
	        break;
            case STB_WEAK:
	        //cerr << "  ELF_32_ST_BIND = STB_WEAK" << endl;
	  	linkage = Symbol::SL_WEAK;
		break;
	    case STB_GLOBAL: 
	        //cerr << "  ELF_32_ST_BIND = STB_GLOBAL" << endl;
		linkage = Symbol::SL_GLOBAL;
		break;
	    default :
	        //cerr << "  ELF_32_ST_BIND UNKNOWN!!!!" << endl;
	        continue;
	    } 

	    // special case for shared libraries.  If the symbol
	    //  is of type file, and the binding is local binding, and
	    //  name matches the module name, then stick the symbol
	    //  directly into (data member) symbols_ (as SL_LOCAL).
	    if (type == STT_FILE && binding == STB_LOCAL && name == module) {
		symbols_[name] = Symbol(name, module, type, linkage,
                                    syms[i1].st_value, st_kludge, 
                                    syms[i1].st_size);
	    } else {
		// otherwise, register found symbol under its name && type....
		allsymbols += Symbol(name, module, 
				        type, linkage,
                                        syms[i1].st_value, st_kludge,
                                        syms[i1].st_size);
	    }
	
	}

    }
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
void Object::fix_zero_function_sizes(vector<Symbol> &allsymbols, bool EEL) {
    unsigned u, v, nsymbols;

    nsymbols = allsymbols.size();
    for (u=0; u < nsymbols; u++) {
	//  If function symbol, and size set to 0, or if the
	//   executable has been EEL rewritten, patch the size
        //   to the offset of next symbol - offset of this symbol....
	if (allsymbols[u].type() == Symbol::PDST_FUNCTION
               && (EEL || allsymbols[u].size() == 0)) {
	    v = u+1;
	    while (v < nsymbols && allsymbols[v].addr() == allsymbols[u].addr())
                v++;
            if (v < nsymbols) {
                allsymbols[u].change_size((unsigned)allsymbols[v].addr()
                                        - (unsigned)allsymbols[u].addr());
            }
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
void Object::override_weak_symbols(vector<Symbol> &allsymbols) {
    unsigned i, nsymbols;
    u_int next_start;
    int next_size;
    bool i_weak_or_local;
    bool ip1_weak_or_local;

    //cerr << "overriding weak symbols for which there is also a global symbol reference...." << endl;
    nsymbols = allsymbols.size();
    for (i=0; i < nsymbols; i++) {
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
	    else if (next_start > allsymbols[i+1].addr()) {
	        next_size = allsymbols[i+1].addr() - allsymbols[i].addr();
		allsymbols[i].change_size(next_size);
		  //cerr << " (type 2) changing symbol size of symbol "
		  //     << allsymbols[i] << "to size " << next_size << endl;
	    }
	}
    }
}

/********************************************************
 *
 * For object files only....
 *  read the .stab section to find the module of global symbols
 *
********************************************************/
void Object::fix_global_symbol_modules_static(
        dictionary_hash<string, Symbol> global_symbols,
	Elf_Scn* stabscnp, Elf_Scn* stabstrscnp) {
    // Read the stab section to find the module of global symbols.
    // The symbols appear in the stab section by module. A module begins
    // with a symbol of type N_UNDF and ends with a symbol of type N_ENDM.
    // All the symbols in between those two symbols belong to the module.

    Elf_Data* stabdatap = elf_getdata(stabscnp, 0);
    Elf_Data* stabstrdatap = elf_getdata(stabstrscnp, 0);
    struct stab_entry *stabsyms = 0;
    unsigned stab_nsyms;
    const char *stabstrs = 0;
    string module;

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
            if ((stabsyms[i].type==N_FUN) && (strlen(p)==0)) {
                // GNU CC 2.8 and higher associate a null-named function
                // entry with the end of a function.  Just skip it.
                break;
            }
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
	        symbols_[SymName] = Symbol(sym.name(), module,
		    sym.type(), sym.linkage(), sym.addr(),
		    sym.kludge(), sym.size());
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
void Object::insert_symbols_static(vector<Symbol> allsymbols,
        dictionary_hash<string, Symbol> &global_symbols) {
    unsigned u, nsymbols = allsymbols.size();

    for (u=0;u<nsymbols;u++) {
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
void Object::insert_symbols_shared(vector<Symbol> allsymbols) {
    unsigned i, nsymbols;

    nsymbols = allsymbols.size();
    for (i=0;i<nsymbols;i++) {
	symbols_[allsymbols[i].name()] =
		    Symbol(allsymbols[i].name(), allsymbols[i].module(),
		    allsymbols[i].type(), allsymbols[i].linkage(),
		    allsymbols[i].addr(), allsymbols[i].kludge(),
		    allsymbols[i].size());
    }
}

/*********************************************************
 *
 * Object::find_code_and_data - find addressm offset, and
 *  length of code and data sections of file described
 *  by ehdrp && phdrp.
 *
 * fills in (data members) code_ptr_, code_off_, code_len_,
 *  data_ptr_, data_off_, data_len_.  As per origional code
 *  of which this is a severely cleaned up version, does not
 *  check here for success or failure.
 *
 * mcheyney - 970904
 *
*********************************************************/
void Object::find_code_and_data(Elf32_Ehdr* ehdrp, Elf32_Phdr* phdrp,
        char *ptr, unsigned txtaddr, unsigned bssaddr) {
    unsigned i0;
  
    for (i0 = 0; i0 < ehdrp-> e_phnum;i0++) {
	if ((phdrp[i0].p_vaddr <= txtaddr)
                && ((phdrp[i0].p_vaddr+phdrp[i0].p_memsz) >= txtaddr)) {
	    if (code_ptr_ == 0 && code_off_ == 0 && code_len_ == 0) {
                code_ptr_ = (Word *) ((void*)&ptr[phdrp[i0].p_offset]);
                code_off_ = (Address) phdrp[i0].p_vaddr;
                code_len_ = (unsigned) phdrp[i0].p_memsz / sizeof(Word);
	    }

        }
        else if ((phdrp[i0].p_vaddr <= bssaddr)
                && ((phdrp[i0].p_vaddr+phdrp[i0].p_memsz) >= bssaddr)) {
	  if (data_ptr_ == 0 && data_off_ == 0 && data_len_ == 0) {
                data_ptr_ = (Word *) ((void *) &ptr[phdrp[i0].p_offset]);
                data_off_ = (Address) phdrp[i0].p_vaddr;
                data_len_ = (unsigned) phdrp[i0].p_memsz / sizeof(Word);
	  }
        }
    }
}

Object::Object(const string file, void (*err_func)(const char *))
    : AObject(file, err_func), EEL(false) {
    load_object();
    //dump_state_info(cerr);
}

Object::Object(const string file, u_int ,void (*err_func)(const char *))
    : AObject(file, err_func), EEL(false)  {
    load_shared_object();
    //dump_state_info(cerr);
}

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

/**************************************************
 *
 *  Stream based debuggering output - for resgreesion testing.
 *  Dump info on state of object *this....
 *
**************************************************/

const ostream &Object::dump_state_info(ostream &s) {
    unsigned i;

    s << "Debugging Information for Object (address) : " << this << endl;

    s << " <<begin debugging info for base object>>" << endl;
    AObject::dump_state_info(s);
    s << " <<end debuggingo info for base object>>" << endl;

    s << " plt_addr_ = " << plt_addr_ << endl;
    s << " plt_size_ = " << plt_size_ << endl;
    s << " plt_entry_size_ = " << plt_entry_size_ << endl;
    s << " get_addr_ = " << got_addr_ << endl;
    s << " rel_plt_addr_ = " << rel_plt_addr_ << endl; 
    s << " rel_plt_size_ = " << rel_plt_size_ << endl;
    s << " rel_plt_entry_size_ = " << rel_plt_entry_size_ << endl;
    s << " dyn_sym_addr_ = " << dyn_sym_addr_ << endl;
    s << " dyn_str_addr_ = " << dyn_str_addr_ << endl;

    // and dump the relocation table....
    s << " relocation_table_ = (field seperator :: )" << endl;   
    for (i=0;i<relocation_table_.size();i++) {
        s << relocation_table_[i] << " :: "; 
    }
    s << endl;

    return s;
} 

#endif  /* #ifdef Object_elf32_h_ */


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
             Object (const Object &);
    virtual ~Object ();

    const Object& operator= (const Object &);

private:
    static
    void    log_elferror (void (*)(const char *), const char *);

    bool      loaded_elf (int, bool &, Elf* &, Elf32_Ehdr* &, Elf32_Phdr* &,
                          unsigned &, unsigned &, Elf_Scn* &, Elf_Scn* &,
                          Elf_Scn* &, Elf_Scn* &);
    void     load_object ();
};

inline
Object::~Object() {
}

inline
const Object&
Object::operator=(const Object& obj) {
    (void) AObject::operator=(obj);
    return *this;
}

inline
void
Object::log_elferror(void (*pfunc)(const char *), const char* msg) {
    const char* err = elf_errmsg(elf_errno());
    log_printf(pfunc, "%s: %s\n", msg, err ? err : "(bad elf error)");
}

inline
bool
Object::loaded_elf(int fd, bool& did_elf, Elf*& elfp, Elf32_Ehdr*& ehdrp,
    Elf32_Phdr*& phdrp, unsigned& txtaddr, unsigned& bssaddr,
    Elf_Scn*& symscnp, Elf_Scn*& strscnp, Elf_Scn*& stabscnp, Elf_Scn*& stabstrscnp) {

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
        const char* STAB_NAME   = ".stab";
        const char* STABSTR_NAME= ".stabstr";
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
        else if (strcmp(name, STAB_NAME) == 0) {
            stabscnp = scnp;
        }
        else if (strcmp(name, STABSTR_NAME) == 0) {
            stabstrscnp = scnp;
        }
    }
    if (!txtaddr || !bssaddr || !symscnp || !strscnp) {
        log_elferror(err_func_, "no text/bss/symbol/string section");
        return false;
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
        if (!loaded_elf(fd, did_elf, elfp, ehdrp, phdrp, txtaddr,
            bssaddr, symscnp, strscnp, stabscnp, stabstrscnp)) {
            /* throw exception */ goto cleanup;
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

            // We are done with the local symbols. We save the global so that
            // we can get their module from the .stab section.
            if (ELF32_ST_BIND(syms[i1].st_info) == STB_LOCAL) {
	       symbols_[name] = Symbol(name, module, type, Symbol::SL_LOCAL,
                                    syms[i1].st_value, st_kludge, 
                                    syms[i1].st_size);
	    }
            else {
               assert(!(global_symbols.defines(name))); // globals should be unique
	       global_symbols[name] = Symbol(name, string(""), type, Symbol::SL_GLOBAL,
                                    syms[i1].st_value, st_kludge,
                                    syms[i1].st_size);
	    }
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
              assert(res); // All globals in .stab should be defined in .symtab
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

    }

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
Object::Object(const string file, void (*err_func)(const char *))
    : AObject(file, err_func) {
    load_object();
}

inline
Object::Object(const Object& obj)
    : AObject(obj) {
    load_object();
}





#endif /* !defined(_Object_elf32_h_) */

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
 * BSD object files.
 * $Id: Object-bsd.h,v 1.15 1998/12/25 21:48:35 wylie Exp $
************************************************************************/





#if !defined(_Object_bsd_h_)
#define _Object_bsd_h_





/************************************************************************
 * header files.
************************************************************************/

#include "util/h/Dictionary.h"
#include "util/h/String.h"
#include "util/h/Symbol.h"
#include "util/h/Types.h"
#include "util/h/Vector.h"

extern "C" {
#include <a.out.h>
};

#include <fcntl.h>
#include <stab.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>





/************************************************************************
 * class Object
************************************************************************/

class Object : public AObject {
public:
             Object (const string, void (*)(const char *) = log_msg);
             Object (const Object &);
	     Object (const string, const Address baseAddr,
                void (*)(const char *) = log_msg);
    virtual ~Object ();

    Object&   operator= (const Object &);

private:
    void    load_object ();
};

static int symbol_compare(const void *x, const void *y) {
    const Symbol *s1 = (const Symbol *)x;
    const Symbol *s2 = (const Symbol *)y;
    return (s1->addr() - s2->addr());
}

inline
void
Object::load_object() {
    const char* file = file_.string_of();
    struct stat st;
    int         fd  = -1;
    char*       ptr = 0;

    bool        did_open = false;

    /* try */ {
        if (((fd = open(file, O_RDONLY)) == -1) || (fstat(fd, &st) == -1)) {
            log_perror(err_func_, file);
            /* throw exception */ goto cleanup;
        }
        did_open = true;

        if ((ptr = (char *) P_mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0))
            == (char *) -1) {
            log_perror(err_func_, "mmap");
            /* throw exception */ goto cleanup;
        }

        struct exec* execp = (struct exec *) ((void*) ptr);
        if (N_BADMAG(*execp)) {
            /* throw exception */ goto cleanup;
        }

        code_ptr_ = (Word *) ((void*)&ptr[(unsigned)(N_TXTOFF(*execp))]);
        code_off_ = (unsigned) N_TXTADDR(*execp);
        code_len_ = unsigned(execp->a_text / sizeof(Word));

        data_ptr_ = (Word *) ((void*)&ptr[(unsigned)(N_DATOFF(*execp))]);
        data_off_ = (unsigned) N_DATADDR(*execp);
        data_len_ = unsigned(execp->a_data / sizeof(Word));

        struct nlist* syms   = (struct nlist *)
	  ((void*)&ptr[(unsigned)(N_SYMOFF(*execp))]);
        unsigned      nsyms  = execp->a_syms / sizeof(struct nlist);
        char*         strs   = &ptr[(unsigned)(N_STROFF(*execp))];
        string        module = "DEFAULT_MODULE";
        string        name   = "DEFAULT_SYMBOL";

	vector<Symbol> allsymbols;

        for (unsigned i = 0; i < nsyms; i++) {
            unsigned char sstab = syms[i].n_type & (N_TYPE | N_STAB);

            Symbol::SymbolLinkage linkage =
                ((syms[i].n_type & N_EXT)
                    ? Symbol::SL_GLOBAL
                    : Symbol::SL_LOCAL);
            Symbol::SymbolType type = Symbol::PDST_UNKNOWN;
	    bool st_kludge = false;

            switch (sstab) {
            // we do not want header files to become modules
            // case N_BINCL:
            // case N_SOL:

            case N_FN:
            case N_SO:
                module = string(&strs[syms[i].n_un.n_strx]);
                type   = Symbol::PDST_MODULE;
                break;

            case N_BSS:
            case N_DATA:
		// This information is not needed for now
                type = Symbol::PDST_OBJECT;
                break;

            case N_TEXT: {
		// KLUDGE: <file>.o entries have a nasty habit of showing up in
		// symbol tables as N_TEXT when debugging info is not
		// in the symbol table
		int len = strlen(&strs[syms[i].n_un.n_strx]);
		type = Symbol::PDST_OBJECT;
		if ((len > 2) &&
		    ((&strs[syms[i].n_un.n_strx])[len-2] == '.') &&
		    (linkage == Symbol::SL_LOCAL) &&
		    (!(0x3 & syms[i].n_value))) {
		  type = Symbol::PDST_MODULE;
		  st_kludge = true;
		  break;
		} else if (linkage != Symbol::SL_GLOBAL) {
		  break;
		} else if (0x3 & syms[i].n_value) {
		  break;
		} else {
		  // this may be a function
		  st_kludge = true;
		}
		break;
            }
            case N_FNAME:
            case N_FUN:
                type = Symbol::PDST_FUNCTION;
		break;

            case N_SLINE:
                // process line numbers here, when we know how to

            default:
                continue;
            }

	    name = string(&strs[syms[i].n_un.n_strx]);
            allsymbols += Symbol(name, module, type, linkage,
                                 syms[i].n_value, st_kludge);

	  }

          // Sort the symbols on address to find the function boundaries
          allsymbols.sort(symbol_compare);

          unsigned nsymbols = allsymbols.size();
          for (unsigned u = 0; u < nsymbols; u++) {
	    unsigned v = u+1;
	    while (v < nsymbols && allsymbols[v].addr() == allsymbols[u].addr())
              v++;
            unsigned size = 0;
            if (v < nsymbols)
              size = (unsigned)allsymbols[v].addr() - (unsigned)allsymbols[u].addr();

            symbols_[allsymbols[u].name()] =
               Symbol(allsymbols[u].name(), allsymbols[u].module(),
                      allsymbols[u].type(), allsymbols[u].linkage(),
                      allsymbols[u].addr(), allsymbols[u].kludge(),
                      size);
	   }      
	 }

    /* catch */
cleanup: {
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

// for shared object files: not currently implemented
// this should call a load_sharedobject routine to parse the shared obj. file
inline
Object::Object(const string file,u_int,void (*err_func)(const char *))
    : AObject(file, err_func) {
   printf("ERROR: this should not execute\n"); 
}

inline
Object::Object(const Object& obj)
    : AObject(obj) {
    load_object();
}


inline
Object::~Object() {
}

inline
Object&
Object::operator=(const Object& obj) {
    (void) AObject::operator=(obj);
    return *this;
}




#endif /* !defined(_Object_bsd_h_) */

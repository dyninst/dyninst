/************************************************************************
 * Object-bsd.h: BSD object files.
************************************************************************/





#if !defined(_Object_bsd_h_)
#define _Object_bsd_h_





/************************************************************************
 * header files.
************************************************************************/

#include <util/h/Dictionary.h>
#include <util/h/Line.h>
#include <util/h/String.h>
#include <util/h/Symbol.h>
#include <util/h/Types.h>
#include <util/h/Vector.h>

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
    virtual ~Object ();

    Object&   operator= (const Object &);

private:
    void    load_object ();
};

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

inline
Object::~Object() {
}

inline
Object&
Object::operator=(const Object& obj) {
    (void) AObject::operator=(obj);
    return *this;
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

        code_ptr_ = (Word *) ((void*)&ptr[unsigned(N_TXTOFF(*execp))]);
        code_off_ = (unsigned) N_TXTADDR(*execp);
        code_len_ = unsigned(execp->a_text / sizeof(Word));

        data_ptr_ = (Word *) ((void*)&ptr[unsigned(N_DATOFF(*execp))]);
        data_off_ = (unsigned) N_DATADDR(*execp);
        data_len_ = unsigned(execp->a_data / sizeof(Word));

        struct nlist* syms   = (struct nlist *)
	  ((void*)&ptr[unsigned(N_SYMOFF(*execp))]);
        unsigned      nsyms  = execp->a_syms / sizeof(struct nlist);
        char*         strs   = &ptr[unsigned(N_STROFF(*execp))];
        string        module = "DEFAULT_MODULE";
        string        name   = "DEFAULT_SYMBOL";
        for (unsigned i = 0; i < nsyms; i++) {
            unsigned char sstab = syms[i].n_type & (N_TYPE | N_STAB);

            Symbol::SymbolLinkage linkage =
                ((syms[i].n_type & N_EXT)
                    ? Symbol::SL_GLOBAL
                    : Symbol::SL_LOCAL);
            Symbol::SymbolType type = Symbol::ST_UNKNOWN;
	    bool st_kludge = false;

            switch (sstab) {
            // we do not want header files to become modules
            // case N_BINCL:
            // case N_SOL:

            case N_FN:
            case N_SO:
                module = string(&strs[syms[i].n_un.n_strx]);
                type   = Symbol::ST_MODULE;
                break;

            case N_BSS:
            case N_DATA:
		// This information is not needed for now
                type = Symbol::ST_OBJECT;
                break;

            case N_TEXT:
		// KLUDGE: <file>.o entries have a nasty habit of showing up in
		// symbol tables as N_TEXT when debugging info is not
		// in the symbol table
		int len = strlen(&strs[syms[i].n_un.n_strx]);
		type = Symbol::ST_OBJECT;
		if ((len > 2) &&
		    ((&strs[syms[i].n_un.n_strx])[len-2] == '.') &&
		    (linkage == Symbol::SL_LOCAL) &&
		    (!(0x3 & syms[i].n_value))) {
		  type = Symbol::ST_MODULE;
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

            case N_FNAME:
            case N_FUN:
                type = Symbol::ST_FUNCTION;
		break;

            case N_SLINE:
                // process line numbers here, when we know how to

            default:
                continue;
            }

	    name = string(&strs[syms[i].n_un.n_strx]);
	    symbols_[name] = Symbol(name, module, type, linkage,
				    syms[i].n_value, st_kludge);
	  }
      }

    /* catch */
cleanup: {
        if (did_open && (close(fd) == -1)) {
            log_perror(err_func_, "close");
        }
    }
}





#endif /* !defined(_Object_bsd_h_) */

/************************************************************************
 * Object-bsd.h: BSD object files.
************************************************************************/





#if !defined(_Object_bsd_h_)
#define _Object_bsd_h_


/* 
 *
 * Note - this class reads the node portion of the "fat" a.out for the cm5.
 *        The node portion will be the second half of the executable file
 *        and its exact location can be determined by reading the end of
 *        the file. 
 * 
 * code_ptr_ and data_ptr_ must be adjusted by this offset
 * code_len_, data_len_, code_off_, and data_off_ must not since the addresses
 *        in the node portion of the executable are not indexed from the
 *        start of the file,
 *
 *            file[nodeFileOffset_] = node_portion_memory[0]
 *
 */




/************************************************************************
 * header files.
************************************************************************/

#include <Dictionary.h>
#include <Line.h>
#include <String.h>
#include <Symbol.h>
#include <Types.h>
#include <Vector.h>
#include "kludges.h"

extern "C" {
#include <a.out.h>
#include <fcntl.h>
#include <stab.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <cmsys/cm_a.out.h>
}





/************************************************************************
 * class Object
************************************************************************/

class Object : public AObject {
public:
             Object (const char *, void (*)(const char *) = log_msg);
             Object (const Object &);
    virtual ~Object ();

    Object&   operator= (const Object &);

private:
    void    load_object ();
    unsigned nodeFileOffset_;
};

inline
Object::Object(const char* file, void (*err_func)(const char *))
    : AObject(file, err_func), nodeFileOffset_(0) {
    load_object();
}

inline
Object::Object(const Object& obj)
    : AObject(obj), nodeFileOffset_(0) {
    load_object();
}

inline
Object::~Object() {
}

inline
Object&
Object::operator=(const Object& obj) {
    (void) AObject::operator=(obj);
    nodeFileOffset_ = obj.nodeFileOffset_;
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

        if ((ptr = (char *) mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0))
            == (char *) -1) {
            log_perror(err_func_, "mmap");
            /* throw exception */ goto cleanup;
        }

	nodeFileOffset_ = st.st_size - sizeof(cm_par_id_t);
	cm_par_id_t header;
	memcpy((void*) &header, (void*)(ptr+nodeFileOffset_), sizeof(cm_par_id_t));
	if (header.magic != CM_PAR_MAGIC) {
	  log_perror(err_func_, "Not a CM5 executable\n");
	  /* throw exception */ goto cleanup;
	}

	nodeFileOffset_ = header.par_offset;
        struct exec* execp = (struct exec *) &ptr[nodeFileOffset_];
        if (N_BADMAG(*execp)) {
            /* throw exception */ goto cleanup;
        }

        code_ptr_ = (Word *) &ptr[unsigned(N_TXTOFF(*execp)+nodeFileOffset_)];
        code_off_ = (unsigned) (N_TXTADDR(*execp));
        code_len_ = unsigned(execp->a_text / sizeof(Word));

        data_ptr_ = (Word *) &ptr[unsigned(N_DATOFF(*execp)+nodeFileOffset_)];
        data_off_ = (unsigned) (N_DATADDR(*execp));
        data_len_ = unsigned(execp->a_data / sizeof(Word));

        struct nlist* syms   = (struct nlist *)
	  &ptr[unsigned(N_SYMOFF(*execp)+nodeFileOffset_)];
        unsigned      nsyms  = execp->a_syms / sizeof(struct nlist);
        char*         strs   = &ptr[unsigned(N_STROFF(*execp)+nodeFileOffset_)];
        string        module = "*DUMMY_MODULE*";
        string        name   = "*DUMMY_SYMBOL*";
        for (unsigned i = 0; i < nsyms; i++) {
            unsigned char sstab = syms[i].n_type & (N_TYPE | N_STAB);

            Symbol::SymbolLinkage linkage =
                ((syms[i].n_type & N_EXT)
                    ? Symbol::SL_GLOBAL
                    : Symbol::SL_LOCAL);
            Symbol::SymbolType type = Symbol::ST_UNKNOWN;

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
                type = Symbol::ST_OBJECT;
                break;

            case N_FNAME:
            case N_FUN:
            case N_TEXT:
                type = Symbol::ST_FUNCTION;
                break;

            case N_SLINE:
                // process line numbers here, when we know how to

            default:
                continue;
                break;
            }

            name = string(&strs[syms[i].n_un.n_strx]);
            symbols_[name] = Symbol(name, module, type, linkage,
                                    syms[i].n_value);
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

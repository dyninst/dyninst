/************************************************************************
 * Object-hpux.h: HPUX object files.
************************************************************************/





#if !defined(_Object_hpux_h_)
#define _Object_hpux_h_





/************************************************************************
 * header files.
************************************************************************/

#include <util/h/Dictionary.h>
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
extern "C" {
#include <sys/mman.h>
};
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

    static const struct som_exec_auxhdr*
        get_som_exec_auxhdr (const struct header *);
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
const struct som_exec_auxhdr*
Object::get_som_exec_auxhdr(const struct header* hdrp) {
    const struct aux_id* idp = (const struct aux_id *)
        ((const void *) ((const char *) hdrp + (hdrp->aux_header_location)));
    unsigned aux_seen = 0;
    while (aux_seen < hdrp->aux_header_size) {
        if (idp->type == HPUX_AUX_ID) {
            return (const struct som_exec_auxhdr *) ((const void *) idp);
        }
        unsigned skip = sizeof(struct aux_id) + idp->length;

        idp = (const struct aux_id *)
            ((const void *) ((const char *) idp + skip));
        aux_seen += skip;
    }
    return 0;
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

        if ((ptr = (char *) P_mmap(0, st.st_size,
            PROT_READ, MAP_PRIVATE, fd, 0)) == (char *) -1) {
            log_perror(err_func_, "mmap");
            /* throw exception */ goto cleanup;
        }

        const struct header* hdrp = (struct header *) ((void*) ptr);
        const struct som_exec_auxhdr* som_hdrp = 0;
        if ((som_hdrp = get_som_exec_auxhdr(hdrp)) == 0) {
            log_perror(err_func_, "cannot locate HPUX_AUX_ID");
            /* throw exception */ goto cleanup;
        }

        unsigned file_type = hdrp->a_magic;
        if ((file_type != EXEC_MAGIC) && (file_type != SHARE_MAGIC)) {
            log_perror(err_func_, "application is not EXEC_MAGIC/SHARE_MAGIC");
            /* throw exception */ goto cleanup;
        }

        code_ptr_ = (Word *) ((void*) &ptr[som_hdrp->exec_tfile]);
        code_off_ = (unsigned) som_hdrp->exec_tmem;
        code_len_ = unsigned(som_hdrp->exec_tsize / sizeof(Word));

        data_ptr_ = (Word *) ((void*) &ptr[som_hdrp->exec_dfile]);
        data_off_ = (unsigned) som_hdrp->exec_dmem;
        data_len_ = unsigned(som_hdrp->exec_dsize / sizeof(Word));

        const struct symbol_dictionary_record* syms =
            (const struct symbol_dictionary_record *)
            ((const void*) &ptr[hdrp->symbol_location]);
        unsigned      nsyms  = hdrp->symbol_total;
        const char*   strs   = &ptr[hdrp->symbol_strings_location];
        string        module = "DEFAULT_MODULE";
        string        name   = "DEFAULT_SYMBOL";

        for (unsigned i = 0; i < nsyms; i++) {
            Symbol::SymbolType type = Symbol::PDST_UNKNOWN;
            Symbol::SymbolLinkage linkage =
                ((syms[i].symbol_scope == SS_UNIVERSAL)
                ? Symbol::SL_GLOBAL
                : Symbol::SL_LOCAL);
            unsigned value = syms[i].symbol_value;

            switch (syms[i].symbol_type) {
            case ST_DATA:
                type = Symbol::PDST_OBJECT;
                break;

            case ST_CODE:
            case ST_ENTRY:
                type   = Symbol::PDST_FUNCTION;
                value  = (value >> 2) << 2; /* get rid of the lower-order 2 bits */
                break;

            case ST_MODULE: /* this may be a _pascal_ module !?! */
                module = string(&strs[syms[i].name.n_strx]);
                type   = Symbol::PDST_MODULE;
                break;

            default:
                break;
            }

            string name = string(&strs[syms[i].name.n_strx]);
            if (!symbols_.defines(name)
                || (symbols_[name].linkage() == Symbol::SL_LOCAL)) {
                symbols_[name] = Symbol(name, module, type, linkage,
                    value, false);
            }
          }
      }

    /* catch */
cleanup: {
        if (did_open && (close(fd) == -1)) {
            log_perror(err_func_, "close");
        }
    }
}





#endif /* !defined(_Object_hpux_h_) */

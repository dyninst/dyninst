/************************************************************************
 * Object.h: interface to objects, symbols, lines and instructions.
************************************************************************/





#if !defined(_Object_h_)
#define _Object_h_





/************************************************************************
 * header files.
************************************************************************/

#include <util/h/Dictionary.h>
#include <util/h/String.h>
#include <util/h/Symbol.h>
#include <util/h/Types.h>
#include <util/h/Vector.h>

#include "util/h/lprintf.h"





/************************************************************************
 * class AObject
************************************************************************/

class AObject {
public:

    unsigned          nsymbols ()                         const;

    bool            get_symbol (const string &, Symbol &);

    const Word*       code_ptr ()                         const;
    unsigned          code_off ()                         const;
    unsigned          code_len ()                         const;

    const Word*       data_ptr ()                         const;
    unsigned          data_off ()                         const;
    unsigned          data_len ()                         const;

protected:
    AObject (const string, void (*)(const char *)); // explicitly protected
    AObject                    (const AObject &);   // explicitly protected
    AObject&         operator= (const AObject &);   // explicitly protected

    string                          file_;
    dictionary_hash<string, Symbol> symbols_;

    Word*    code_ptr_;
    unsigned code_off_;
    unsigned code_len_;

    Word*    data_ptr_;
    unsigned data_off_;
    unsigned data_len_;

    void (*err_func_)(const char*);

private:
    friend class SymbolIter;
};

inline
AObject::AObject(const string file, void (*err_func)(const char*))
    : file_(file), symbols_(string::hash), 
    code_ptr_(0), code_off_(0), code_len_(0),
    data_ptr_(0), data_off_(0), data_len_(0),
    err_func_(err_func) {
}

inline
AObject::AObject(const AObject& obj)
    : file_(obj.file_), symbols_(obj.symbols_), 
    code_ptr_(obj.code_ptr_), code_off_(obj.code_off_),
    code_len_(obj.code_len_),
    data_ptr_(obj.data_ptr_), data_off_(obj.data_off_),
    data_len_(obj.data_len_),
    err_func_(obj.err_func_) {
}

inline
AObject&
AObject::operator=(const AObject& obj) {
    if (this == &obj) {
        return *this;
    }

    file_      = obj.file_;
    symbols_   = obj.symbols_;
    code_ptr_  = obj.code_ptr_;
    code_off_  = obj.code_off_;
    code_len_  = obj.code_len_;
    data_ptr_  = obj.data_ptr_;
    data_off_  = obj.data_off_;
    data_len_  = obj.data_len_;
    err_func_  = obj.err_func_;

    return *this;
}

inline
unsigned
AObject::nsymbols() const {
    return symbols_.size();
}

inline
bool
AObject::get_symbol(const string& name, Symbol& symbol) {
    if (!symbols_.defines(name)) {
        return false;
    }
    symbol = symbols_[name];
    return true;
}

inline
const Word*
AObject::code_ptr() const {
    return code_ptr_;
}

inline
unsigned
AObject::code_off() const {
    return code_off_;
}

inline
unsigned
AObject::code_len() const {
    return code_len_;
}

inline
const Word*
AObject::data_ptr() const {
    return data_ptr_;
}

inline
unsigned
AObject::data_off() const {
    return data_off_;
}

inline
unsigned
AObject::data_len() const {
    return data_len_;
}





/************************************************************************
 * include the architecture-operating system specific object files.
************************************************************************/

#undef HAVE_SPECIFIC_OBJECT

#if defined(hppa1_1_hp_hpux)
#include <util/h/Object-hpux.h>
#define HAVE_SPECIFIC_OBJECT
#endif /* defined(hppa1_1_hp_hpux) */

#if defined(sparc_sun_solaris2_4)
#include <util/h/Object-elf32.h>
#define HAVE_SPECIFIC_OBJECT
#endif /* defined(sparc_sun_solaris2_4) */

#if defined(sparc_sun_sunos4_1_3)
#include <util/h/Object-bsd.h>
#define HAVE_SPECIFIC_OBJECT
#endif /* defined(sparc_sun_sunos4_1_3) */

#if defined(sparc_tmc_cmost7_3)
#include <util/h/Object-cm5.h>
#define HAVE_SPECIFIC_OBJECT
#endif /* defined(sparc_tmc_cmost7_3) */

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
#include <util/h/Object-aix.h>
#define HAVE_SPECIFIC_OBJECT
#endif /* defined(sparc_tmc_cmost7_3) */

#if !defined(HAVE_SPECIFIC_OBJECT)
#error "unable to locate system-specific object files"
#endif /* !defined(HAVE_SPECIFIC_OBJECT) */





/************************************************************************
 * class SymbolIter
************************************************************************/

class SymbolIter {
public:
     SymbolIter (const Object &);
    ~SymbolIter ();

    bool  next (string &, Symbol &);
    void reset ();

private:
    dictionary_hash_iter<string, Symbol> si_;

    SymbolIter            (const SymbolIter &); // explicitly disallowed
    SymbolIter& operator= (const SymbolIter &); // explicitly disallowed
};

inline
SymbolIter::SymbolIter(const Object& obj)
    : si_(obj.symbols_) {
}

inline
SymbolIter::~SymbolIter() {
}

inline
bool
SymbolIter::next(string& name, Symbol& sym) {
    return si_.next(name, sym);
}

inline
void
SymbolIter::reset() {
    si_.reset();
}








#endif /* !defined(_Object_h_) */

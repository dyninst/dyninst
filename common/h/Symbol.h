/************************************************************************
 * Symbol.h: symbol table objects.
************************************************************************/





#if !defined(_Symbol_h_)
#define _Symbol_h_





/************************************************************************
 * header files.
************************************************************************/

#include <util/h/String.h>
#include <util/h/Types.h>





/************************************************************************
 * class Symbol
************************************************************************/

class Symbol {
public:
    enum SymbolType {
        ST_UNKNOWN,
        ST_FUNCTION,
        ST_OBJECT,
        ST_MODULE
    };

    enum SymbolLinkage {
        SL_UNKNOWN,
        SL_GLOBAL,
        SL_LOCAL
    };

    enum SymbolTag {
        TAG_UNKNOWN,
        TAG_USER,
        TAG_LIBRARY,
        TAG_INTERNAL
    };

     Symbol ();
     Symbol (unsigned);
     Symbol (const string &, const string &, SymbolType, SymbolLinkage,
             Address, const bool);
     Symbol (const Symbol &);
    ~Symbol ();

    Symbol&        operator= (const Symbol &);
    bool          operator== (const Symbol &) const;

    const string&       name ()               const;
    const string&     module ()               const;
    SymbolType          type ()               const;
    SymbolLinkage    linkage ()               const;
    Address             addr ()               const;
    bool              kludge ()               const;
    SymbolTag&           tag ()               const;

    friend
    ostream&      operator<< (ostream &os, const Symbol &s) {
      return os << "{"
        << " name="    << s.name_
	<< " module="  << s.module_
	<< " type="    << (unsigned) s.type_
	<< " linkage=" << (unsigned) s.linkage_
	<< " addr="    << s.addr_
	<< " tag="     << (unsigned) s.tag_
	<< " kludge="  << s.kludge_
        << " }" << endl;
    }



private:
    string        name_;
    string        module_;
    SymbolType    type_;
    SymbolLinkage linkage_;
    Address       addr_;
    SymbolTag     tag_;
    bool          kludge_;
};

inline
Symbol::Symbol()
    : name_("*bad-symbol*"), module_("*bad-module*"),
    type_(ST_UNKNOWN), linkage_(SL_UNKNOWN), addr_(0),
    tag_(TAG_UNKNOWN), kludge_(false) {
}

inline
Symbol::Symbol(unsigned dummy)
    : name_("*bad-symbol*"), module_("*bad-module*"),
    type_(ST_UNKNOWN), linkage_(SL_UNKNOWN), addr_(0),
    tag_(TAG_UNKNOWN), kludge_(false) {
}

inline
Symbol::Symbol(const string& name, const string& module,
    SymbolType type, SymbolLinkage linkage, Address addr, const bool kl)
    : name_(name), module_(module),
    type_(type), linkage_(linkage), addr_(addr),
    tag_(TAG_UNKNOWN), kludge_(kl) {
}

inline
Symbol::Symbol(const Symbol& s)
    : name_(s.name_), module_(s.module_),
    type_(s.type_), linkage_(s.linkage_), addr_(s.addr_),
    tag_(s.tag_), kludge_(s.kludge_) {
}

inline
Symbol::~Symbol() {
}

inline
Symbol&
Symbol::operator=(const Symbol& s) {
    name_    = s.name_;
    module_  = s.module_;
    type_    = s.type_;
    linkage_ = s.linkage_;
    addr_    = s.addr_;
    tag_     = s.tag_;
    kludge_  = s.kludge_;

    return *this;
}

inline
bool
Symbol::operator==(const Symbol& s) const {
    // explicitly ignore tags when comparing symbols
    return ((name_   == s.name_)
        && (module_  == s.module_)
        && (type_    == s.type_)
        && (linkage_ == s.linkage_)
        && (addr_    == s.addr_)
	&& (kludge_  == s.kludge_));
}

inline
const string&
Symbol::name() const {
    return name_;
}

inline
const string&
Symbol::module() const {
    return module_;
}

inline
Symbol::SymbolType
Symbol::type() const {
    return type_;
}

inline
Symbol::SymbolLinkage
Symbol::linkage() const {
    return linkage_;
}

inline
Address
Symbol::addr() const {
    return addr_;
}

inline
Symbol::SymbolTag&
Symbol::tag() const {
    return (SymbolTag &) tag_;
}

inline
bool
Symbol::kludge() const {
    return kludge_;
}





#endif /* !defined(_Symbol_h_) */

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
 * $Id: Symbol.h,v 1.16 2000/07/26 23:02:37 hollings Exp $
 * Symbol.h: symbol table objects.
************************************************************************/





#if !defined(_Symbol_h_)
#define _Symbol_h_





/************************************************************************
 * header files.
************************************************************************/

#include "common/h/String.h"
// trace data streams
// #include "util/h/ByteArray.h"
#include "common/h/Types.h"





/************************************************************************
 * class Symbol
************************************************************************/

class Symbol {
public:
    enum SymbolType {
        PDST_UNKNOWN,
        PDST_FUNCTION,
        PDST_OBJECT,
        PDST_MODULE,
	PDST_NOTYPE
    };

    enum SymbolLinkage {
        SL_UNKNOWN,
        SL_GLOBAL,
        SL_LOCAL,
	SL_WEAK
    };

    enum SymbolTag {
        TAG_UNKNOWN,
        TAG_USER,
        TAG_LIBRARY,
        TAG_INTERNAL
    };

   Symbol (); // note: this ctor is called surprisingly often!
     Symbol (unsigned);
     Symbol (const string &name, const string &modulename, SymbolType, SymbolLinkage,
             Address, const bool, unsigned size = 0);
     Symbol (const Symbol &);
    ~Symbol ();

    Symbol&        operator= (const Symbol &);
    bool          operator== (const Symbol &) const;

    const string&       name ()               const;
    const string&     module ()               const;
    SymbolType          type ()               const;
    SymbolLinkage    linkage ()               const;
    Address             addr ()               const;

   void setAddr (Address newAddr) {
      addr_ = newAddr;
   }
   
    unsigned            size ()               const;
    bool              kludge ()               const;
    SymbolTag&           tag ()               const;
    void	change_size(unsigned ns){ size_ = ns;}
    void	setModule(string& module) { module_ = module; }

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
    unsigned      size_;  // size of this symbol. This is NOT available on
                          // all platforms.
    SymbolTag     tag_;
    bool          kludge_;
};

inline
Symbol::Symbol()
   : //name_("*bad-symbol*"), module_("*bad-module*"),
    type_(PDST_UNKNOWN), linkage_(SL_UNKNOWN), addr_(0), size_(0),
    tag_(TAG_UNKNOWN), kludge_(false) {
   // note: this ctor is called surprisingly often (when we have
   // vectors of Symbols and/or dictionaries of Symbols).  So, make it fast.
}

inline
Symbol::Symbol(unsigned)
   : //name_("*bad-symbol*"), module_("*bad-module*"),
    type_(PDST_UNKNOWN), linkage_(SL_UNKNOWN), addr_(0), size_(0),
    tag_(TAG_UNKNOWN), kludge_(false) {
}

inline
Symbol::Symbol(const string& iname, const string& imodule,
    SymbolType itype, SymbolLinkage ilinkage, Address iaddr, const bool ikl,
    unsigned size)
    : name_(iname), module_(imodule),
    type_(itype), linkage_(ilinkage), addr_(iaddr), size_(size),
    tag_(TAG_UNKNOWN), kludge_(ikl) {
}

inline
Symbol::Symbol(const Symbol& s)
    : name_(s.name_), module_(s.module_),
    type_(s.type_), linkage_(s.linkage_), addr_(s.addr_), size_(s.size_),
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
    size_    = s.size_;
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
	&& (size_    == s.size_)
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
unsigned
Symbol::size() const {
    return size_;
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

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
 * $Id: Object.h,v 1.39 2003/03/28 23:34:10 pcroth Exp $
 * Object.h: interface to objects, symbols, lines and instructions.
************************************************************************/


#if !defined(_Object_h_)
#define _Object_h_

/************************************************************************
 * header files.
************************************************************************/

#include "common/h/Dictionary.h"
#include "common/h/String.h"
// trace data streams
#include "common/h/Symbol.h"
#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/lprintf.h"

extern int symbol_compare(const Symbol &s1, const Symbol &s2);

// File descriptor information
class fileDescriptor {
 public:
  fileDescriptor():file_(0), addr_(0), shared_(false){}
  fileDescriptor(string file):file_(file), addr_(0), shared_(false){}
  fileDescriptor(string file, Address addr):file_(file), addr_(addr), shared_(true){}
  fileDescriptor(const fileDescriptor &fd) : file_(fd.file_), addr_(fd.addr_),
    shared_(fd.shared_) {}
  ~fileDescriptor() {}
  
  bool operator==(const fileDescriptor &fd) const
  {
    return IsEqual( &fd );
  }

  
  const string &file() const { return file_; }
  Address addr() const { return addr_; };
  bool isSharedObject() const { return shared_; }

 protected:
  string file_;
  Address addr_;
  bool shared_;

  virtual bool IsEqual( const fileDescriptor* fd ) const
  {
    if ((file_ == fd->file_) &&
	(addr_ == fd->addr_) &&
	(shared_ == fd->shared_))
      return true;
    else
      return false;
  }
};

// relocation information for calls to functions not in this image
// on sparc-solaris: target_addr_ = rel_addr_ = PLT entry addr
// on x86-solaris: target_addr_ = PLT entry addr
//		   rel_addr_ =  GOT entry addr  corr. to PLT_entry
class relocationEntry {
public:
    relocationEntry():target_addr_(0),rel_addr_(0),name_(0){}   
    relocationEntry(Address ta,Address ra, string n):target_addr_(ta),
	rel_addr_(ra),name_(n){}   
    relocationEntry(const relocationEntry& ra): target_addr_(ra.target_addr_), 
    	rel_addr_(ra.rel_addr_), name_(ra.name_) { }
    ~relocationEntry(){}

    const relocationEntry& operator= (const relocationEntry &ra) {
	target_addr_ = ra.target_addr_; rel_addr_ = ra.rel_addr_; 
	name_ = ra.name_; 
	return *this;
    }
    Address target_addr() const { return target_addr_; }
    Address rel_addr() const { return rel_addr_; }
    const string &name() const { return name_; }

    // dump output.  Currently setup as a debugging aid, not really
    //  for object persistance....
    ostream & operator<<(ostream &s) const;
    friend ostream &operator<<(ostream &os, relocationEntry &q);

private:
   Address target_addr_;	// target address of call instruction 
   Address rel_addr_;		// address of corresponding relocation entry 
   string  name_;
};

/************************************************************************
 * class AObject
 *
 *  WHAT IS THIS CLASS????  COMMENTS????
 *  Looks like it has a dictionary hash of symbols, as well as
 *   a ptr to to the code section, an offset into the code section,
 *   and a length of the code section, and ditto for the data
 *   section....
************************************************************************/

class AObject {
public:

    unsigned  nsymbols () const { return symbols_.size(); }

    bool      get_symbol (const string &name, Symbol &symbol) {
    	if (!symbols_.defines(name)) {
       	    return false;
    	}
    	symbol = symbols_[name];
    	return true;
    }

    const Word*       code_ptr () const { return code_ptr_; } 
    Address           code_off () const { return code_off_; }
    Address           code_len () const { return code_len_; }

    const Word*       data_ptr () const { return data_ptr_; }
    Address           data_off () const { return data_off_; }
    Address           data_len () const { return data_len_; }

    int getAddressWidth() const { return addressWidth_nbytes; }

    virtual  bool   needs_function_binding()  const;
    virtual  bool   get_func_binding_table(pdvector<relocationEntry> &) const;
    virtual  bool   get_func_binding_table_ptr(const pdvector<relocationEntry> *&) const;
    
    bool have_deferred_parsing( void ) const        { return deferredParse; }
    // for debuggering....
    const ostream &dump_state_info(ostream &s);

protected:
    virtual ~AObject() {}
    // explicitly protected
    AObject(const string file , void (*err_func)(const char *)): file_(file), 
	symbols_(string::hash), code_ptr_(0), code_off_(0), code_len_(0), 
	data_ptr_(0), data_off_(0), data_len_(0),
    deferredParse(false),
    err_func_(err_func),
        addressWidth_nbytes(4) { }

    AObject(const AObject &obj): file_(obj.file_), symbols_(obj.symbols_), 
        code_ptr_(obj.code_ptr_), code_off_(obj.code_off_), 
	code_len_(obj.code_len_), data_ptr_(obj.data_ptr_), 
	data_off_(obj.data_off_), data_len_(obj.data_len_), 
    deferredParse(false),
	err_func_(obj.err_func_), addressWidth_nbytes(4) { }

    AObject&  operator= (const AObject &obj) {   

	 if (this == &obj) { return *this; }
	 file_      = obj.file_; 	symbols_   = obj.symbols_;
	 code_ptr_  = obj.code_ptr_; 	code_off_  = obj.code_off_;
	 code_len_  = obj.code_len_; 	data_ptr_  = obj.data_ptr_;
	 data_off_  = obj.data_off_; 	data_len_  = obj.data_len_;
	 err_func_  = obj.err_func_;
	 return *this;
    }

    string                          file_;
    dictionary_hash<string, Symbol> symbols_;
    
    Word*   code_ptr_;
    Address code_off_;
    Address code_len_;

    Word*   data_ptr_;
    Address data_off_;
    Address data_len_;

    bool deferredParse;

    void (*err_func_)(const char*);

    int addressWidth_nbytes;

private:
    friend class SymbolIter;
};

/************************************************************************
 * include the architecture-operating system specific object files.
************************************************************************/

#if defined(sparc_sun_solaris2_4)
#include "dyninstAPI/src/Object-elf.h"

#elif defined(i386_unknown_solaris2_5)
#include "dyninstAPI/src/Object-elf.h"

#elif defined(i386_unknown_linux2_0)
#include "dyninstAPI/src/Object-elf.h"

#elif defined(ia64_unknown_linux2_4)
#include "dyninstAPI/src/Object-elf.h"

#elif defined(mips_sgi_irix6_4)
#include "dyninstAPI/src/Object-elf.h"

#elif defined(rs6000_ibm_aix4_1)
#include "dyninstAPI/src/Object-xcoff.h"

#elif defined(i386_unknown_nt4_0)  || (defined mips_unknown_ce2_11) //ccw 20 july 2000 : 28 mar 2001
#include "dyninstAPI/src/Object-nt.h"

#elif defined(alpha_dec_osf4_0)
#include "dyninstAPI/src/Object-coff.h"

#else
#error "unknown platform"
#endif

/************************************************************************
 * class SymbolIter
************************************************************************/

class SymbolIter {
public:
     SymbolIter (const Object     &obj): si_(obj.symbols_) {}
     SymbolIter (const SymbolIter &src): si_(src.si_) {}
    ~SymbolIter () {}

    void  reset () { si_.reset(); }

   operator bool() const {
      return si_;
   }
   void operator++(int) { si_++; }
   
   const string &currkey() const {
      return si_.currkey();
   }
   const Symbol &currval() const {
      return si_.currval();
   }

private:
    dictionary_hash_iter<string, Symbol> si_;

    SymbolIter& operator= (const SymbolIter &); // explicitly disallowed
};

#endif /* !defined(_Object_h_) */

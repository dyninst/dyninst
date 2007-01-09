/*
 * Copyright (c) 1996-2006 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
 * $Id: Object.h,v 1.1 2007/01/09 02:02:21 giri Exp $
 * Object.h: interface to objects, symbols, lines and instructions.
************************************************************************/


#if !defined(_Object_h_)
#define _Object_h_

/************************************************************************
 * header files.
************************************************************************/

// trace data streams
#include "symtabAPI/h/Dyn_Symbol.h"
#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/lprintf.h"
#include <string>
#include <vector>

using namespace std;

#if defined(os_windows)			//On windows it is just hash_map otherwise its in ext/hash_map
	#include <hash_map>
	using namespace stdext;
#else
    #include <ext/hash_map>
	using namespace __gnu_cxx;
	
	namespace __gnu_cxx {
		template<> struct hash<std::string> {
    		hash<char*> h;  
    		size_t operator()(const std::string &s) const {
				return h(s.c_str());
    		};
		};              
	}; //namespace 
#endif



typedef unsigned long OFFSET;


extern bool symbol_compare(const Dyn_Symbol *s1, const Dyn_Symbol *s2);

DLLEXPORT string extract_path_tail(const string &path);
class Dyn_Symtab;

class Dyn_Section{
  public:
  	DLLEXPORT Dyn_Section() {}
  	DLLEXPORT Dyn_Section(unsigned sidnumber, string sname, OFFSET saddr, unsigned long ssize, void *secPtr, unsigned long sflags= 0):
		sidnumber_(sidnumber), sname_(sname), saddr_(saddr), ssize_(ssize), 
		rawDataPtr_(secPtr), sflags_(sflags){}
	DLLEXPORT Dyn_Section(unsigned sidnumber, string sname, unsigned long ssize, void *secPtr, unsigned long sflags= 0):
		sidnumber_(sidnumber), sname_(sname), saddr_(0), ssize_(ssize), 
		rawDataPtr_(secPtr), sflags_(sflags){}
	DLLEXPORT Dyn_Section(const Dyn_Section &sec):sidnumber_(sec.sidnumber_),sname_(sec.sname_),
		saddr_(sec.saddr_), ssize_(sec.ssize_), rawDataPtr_(sec.rawDataPtr_), 
		sflags_(sec.sflags_){}
	DLLEXPORT Dyn_Section& operator=(const Dyn_Section &sec){
		sidnumber_ = sec.sidnumber_;
		sname_ = sec.sname_;
		saddr_ = sec.saddr_;
		ssize_ = sec.ssize_;
		rawDataPtr_ = sec.rawDataPtr_;
		sflags_ = sec.sflags_;

		return *this;
	}
	
	DLLEXPORT ostream& operator<< (ostream &os) 
	{
		return os << "{"
			  << " id="      << sidnumber_
        		  << " name="    << sname_
        		  << " addr="    << saddr_
        		  << " size="    << ssize_
        		  << " }" << endl;
        }

	DLLEXPORT bool operator== (const Dyn_Section &sec)
	{
		return ((sidnumber_ == sec.sidnumber_)&&
		        (sname_ == sec.sname_)&&
			(saddr_ == sec.saddr_)&&
			(ssize_ == sec.ssize_)&&
			(rawDataPtr_ == sec.rawDataPtr_));
	}

	DLLEXPORT ~Dyn_Section() {}
	
 	DLLEXPORT unsigned getSecNumber() const { return sidnumber_; }
	DLLEXPORT string getSecName() const { return sname_; }
	DLLEXPORT OFFSET getSecAddr() const { return saddr_; }
	DLLEXPORT void *getPtrToRawData() const { return rawDataPtr_; }
	DLLEXPORT unsigned long getSecSize() const { return ssize_; }
	DLLEXPORT bool isBSS() const { return sname_==".bss"; }
	DLLEXPORT bool isText() const { return sname_ == ".text"; }
	DLLEXPORT bool isData() const { return (sname_ == ".data"||sname_ == ".data2"); }
	DLLEXPORT bool isOffsetInSection(const OFFSET &offset) const
	{
		return (offset > saddr_ && offset < saddr_ + ssize_);
	}
  
  private:	
	unsigned sidnumber_;
	string sname_;
	OFFSET saddr_;
	unsigned long ssize_;
	void *rawDataPtr_;
	unsigned long sflags_;  //holds the type of section(text/data/bss/except etc)
};

/**
 * Used to represent something like a C++ try/catch block.  
 * Currently only used on Linux/x86
 **/
class Dyn_ExceptionBlock {
  public:
   	DLLEXPORT Dyn_ExceptionBlock(OFFSET tStart, unsigned tSize, OFFSET cStart) :
     	tryStart_(tStart), trySize_(tSize), catchStart_(cStart), hasTry_(true) {}
   	DLLEXPORT Dyn_ExceptionBlock(OFFSET cStart) :
      		tryStart_(0), trySize_(0), catchStart_(cStart), hasTry_(false) {}
   	DLLEXPORT Dyn_ExceptionBlock(const Dyn_ExceptionBlock &eb) : tryStart_(eb.tryStart_), 
      		trySize_(eb.trySize_), catchStart_(eb.catchStart_), hasTry_(eb.hasTry_) {}
	DLLEXPORT ~Dyn_ExceptionBlock();
	DLLEXPORT Dyn_ExceptionBlock();

   	DLLEXPORT bool hasTry() const { return hasTry_; }
   	DLLEXPORT OFFSET tryStart() const { return tryStart_; }
   	DLLEXPORT OFFSET tryEnd() const { return tryStart_ + trySize_; }
   	DLLEXPORT OFFSET trySize() const { return trySize_; }
	DLLEXPORT OFFSET catchStart() const;
   	DLLEXPORT bool contains(OFFSET a) const 
      	{ return (a >= tryStart_ && a < tryStart_ + trySize_); }

  private:
   	OFFSET tryStart_;
   	unsigned trySize_;
   	OFFSET catchStart_;
   	bool hasTry_;
 };
 
// relocation information for calls to functions not in this image
// on sparc-solaris: target_addr_ = rel_addr_ = PLT entry addr
// on x86-solaris: target_addr_ = PLT entry addr
//		   rel_addr_ =  GOT entry addr  corr. to PLT_entry
class relocationEntry {
public:
    DLLEXPORT relocationEntry():target_addr_(0),rel_addr_(0){}   
    DLLEXPORT relocationEntry(OFFSET ta,OFFSET ra, string n):target_addr_(ta),
														rel_addr_(ra),name_(n){}   
    DLLEXPORT relocationEntry(const relocationEntry& ra);
    DLLEXPORT ~relocationEntry();

    DLLEXPORT const relocationEntry& operator= (const relocationEntry &ra) {
		target_addr_ = ra.target_addr_; rel_addr_ = ra.rel_addr_; 
		name_ = ra.name_; 
		return *this;
    }
    DLLEXPORT OFFSET target_addr() const;
    DLLEXPORT OFFSET rel_addr() const;
    DLLEXPORT const string &name() const;

    // dump output.  Currently setup as a debugging aid, not really
    //  for object persistance....
    DLLEXPORT ostream & operator<<(ostream &s) const;
    friend ostream &operator<<(ostream &os, relocationEntry &q);

private:
   OFFSET target_addr_;	// target address of call instruction 
   OFFSET rel_addr_;		// address of corresponding relocation entry 
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
    DLLEXPORT AObject(){}	
    DLLEXPORT unsigned  nsymbols () const { return symbols_.size(); }
    
    DLLEXPORT bool get_symbols( string & name, vector< Dyn_Symbol *> & symbols ) {
    	if( symbols_.find(name) == symbols_.end()) {
       	    return false;
    	}
    	symbols = symbols_[name];
    	return true;
    }

//    OFFSET getLoadAddress() const { return loadAddress_; }
//    OFFSET getEntryAddress() const { return entryAddress_; }
//    OFFSET getBaseAddress() const { return baseAddress_; }

    DLLEXPORT Word*       code_ptr () const { return code_ptr_; } 
    DLLEXPORT OFFSET           code_off () const { return code_off_; }
    DLLEXPORT OFFSET           code_len () const { return code_len_; }

    DLLEXPORT Word*       data_ptr () const { return data_ptr_; }
    DLLEXPORT OFFSET           data_off () const { return data_off_; }
    DLLEXPORT OFFSET           data_len () const { return data_len_; }

    DLLEXPORT OFFSET           code_vldS () const { return code_vldS_; }
    DLLEXPORT OFFSET           code_vldE () const { return code_vldE_; }
    DLLEXPORT OFFSET           data_vldS () const { return data_vldS_; }
    DLLEXPORT OFFSET           data_vldE () const { return data_vldE_; }

    DLLEXPORT bool 	      is_aout  () const { return is_aout_;  }

    DLLEXPORT unsigned	      no_of_sections () const { return no_of_sections_; }
    DLLEXPORT unsigned	      no_of_symbols  ()	const { return no_of_symbols_;  }

    DLLEXPORT bool getAllExceptions(vector<Dyn_ExceptionBlock *>&excpBlocks) const
    {
    	for(unsigned i=0;i<catch_addrs_.size();i++)
            excpBlocks.push_back(new Dyn_ExceptionBlock(catch_addrs_[i]));
        return true;
    }

    DLLEXPORT vector<Dyn_Section *> getAllSections() const
    {
    	    return sections_;	
    }

    DLLEXPORT OFFSET loader_off() const { return loader_off_; }
    DLLEXPORT unsigned loader_len() const { return loader_len_; }

    DLLEXPORT int getAddressWidth() const { return addressWidth_nbytes; }

    DLLEXPORT virtual char *  mem_image() const;

    DLLEXPORT virtual  bool   needs_function_binding()  const;
    DLLEXPORT virtual  bool   get_func_binding_table(vector<relocationEntry> &) const;
    DLLEXPORT virtual  bool   get_func_binding_table_ptr(const vector<relocationEntry> *&) const;
    
    DLLEXPORT bool have_deferred_parsing( void ) const        { return deferredParse; }
    // for debuggering....
    DLLEXPORT const ostream &dump_state_info(ostream &s);

    DLLEXPORT void * getErrFunc() const { return (void *) err_func_; }
    DLLEXPORT hash_map< string, vector< Dyn_Symbol *> > *getAllSymbols() { return &(symbols_); }

protected:
    DLLEXPORT virtual ~AObject() {}
    // explicitly protected
    DLLEXPORT AObject(const string file , void (*err_func)(const char *)) :
       file_(file), code_ptr_(0), code_off_(0),
       code_len_(0), data_ptr_(0), data_off_(0), data_len_(0),loader_off_(0),
       loader_len_(0), deferredParse(false), err_func_(err_func),
       addressWidth_nbytes(4) { }

    DLLEXPORT AObject(const AObject &obj): file_(obj.file_), symbols_(obj.symbols_), 
       code_ptr_(obj.code_ptr_), code_off_(obj.code_off_), 
       code_len_(obj.code_len_), data_ptr_(obj.data_ptr_), 
       data_off_(obj.data_off_), data_len_(obj.data_len_), 
       loader_off_(obj.loader_off_), loader_len_(obj.loader_len_),
       deferredParse(false), err_func_(obj.err_func_), addressWidth_nbytes(4)
    { } 

    DLLEXPORT AObject&  operator= (const AObject &obj) {   
       if (this == &obj) {
          return *this;
       }
       file_      = obj.file_; 	symbols_   = obj.symbols_;
       code_ptr_  = obj.code_ptr_; 	code_off_  = obj.code_off_;
       code_len_  = obj.code_len_; 	data_ptr_  = obj.data_ptr_;
       data_off_  = obj.data_off_; 	data_len_  = obj.data_len_;
       err_func_  = obj.err_func_;
       loader_off_ = obj.loader_off_; 
       loader_len_ = obj.loader_len_;
       addressWidth_nbytes = obj.addressWidth_nbytes;
       return *this;
    }

    string file_;
    vector< Dyn_Section *> sections_;
    hash_map< string, vector< Dyn_Symbol *> > symbols_;

    Word*   code_ptr_;
    OFFSET code_off_;
    OFFSET code_len_;

    Word*   data_ptr_;
    OFFSET data_off_;
    OFFSET data_len_;

    OFFSET code_vldS_;
    OFFSET code_vldE_;

    OFFSET data_vldS_;
    OFFSET data_vldE_;

    OFFSET loader_off_; //only used on aix right now.  could be
    OFFSET loader_len_; //needed on other platforms in the future

//    OFFSET loadAddress_;
//    OFFSET entryAddress_;
//    OFFSET baseAddress_;
    
    bool is_aout_;

    unsigned no_of_sections_;
    unsigned no_of_symbols_;

    bool deferredParse;
    void (*err_func_)(const char*);
    int addressWidth_nbytes;

    vector<Dyn_ExceptionBlock> catch_addrs_; //Addresses of C++ try/catch blocks;
    
private:
    friend class SymbolIter;
    friend class Dyn_Symtab;
};

/************************************************************************
 * include the architecture-operating system specific object files.
************************************************************************/

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) \
 || defined(ia64_unknown_linux2_4) \
 || defined(mips_sgi_irix6_4)
#include "symtabAPI/h/Object-elf.h"

#elif defined(rs6000_ibm_aix4_1)
#include "symtabAPI/h/Object-xcoff.h"

#elif defined(i386_unknown_nt4_0) \
   || defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 28 mar 2001
#include "symtabAPI/h/Object-nt.h"

#elif defined(alpha_dec_osf4_0)
#include "symtabAPI/h/Object-coff.h"

#else
#error "unknown platform"
#endif

/************************************************************************
 * class SymbolIter
************************************************************************/

class SymbolIter {

	private:
		hash_map< string, vector< Dyn_Symbol *> > *symbols;
		unsigned int currentPositionInVector;
		hash_map< string, vector< Dyn_Symbol *> >::iterator symbolIterator;

	public:
		SymbolIter( Object & obj ) : 
			symbols(obj.getAllSymbols()),
			currentPositionInVector( 0 ) {
				symbolIterator = obj.getAllSymbols()->begin();
					
			}
		SymbolIter( const SymbolIter & src ) : 
			symbols(src.symbols),
			currentPositionInVector( 0 ),
			symbolIterator( src.symbolIterator ) {}
			
		~SymbolIter () {
		}

		void reset () {
			currentPositionInVector = 0;
			symbolIterator = symbols->begin();
			}

	operator bool() const {
			return (symbolIterator!=symbols->end());
		}
	
	void operator++ ( int ) {
		if( currentPositionInVector + 1 < (symbolIterator->second).size())
		{
			currentPositionInVector++;
			return;
		}
		
		/* Otherwise, we need a new vector. */
		currentPositionInVector = 0;			
		symbolIterator++;
		}
	
	const string & currkey() const {
			return symbolIterator->first;
		}
    
    	/* If it's important that this be const, we could try to initialize
       	currentVector to '& symbolIterator.currval()' in the constructor. */
	Dyn_Symbol *currval() {
			return ((symbolIterator->second)[ currentPositionInVector ]);
		}
   
	private:	

		SymbolIter & operator = ( const SymbolIter & ); // explicitly disallowed
	}; /* end class SymbolIter() */

#endif /* !defined(_Object_h_) */

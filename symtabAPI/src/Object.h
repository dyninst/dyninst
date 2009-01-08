/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/************************************************************************
 * $Id: Object.h,v 1.20 2008/06/23 18:45:48 legendre Exp $
 * Object.h: interface to objects, symbols, lines and instructions.
************************************************************************/


#if !defined(_Object_h_)
#define _Object_h_

/************************************************************************
 * header files.
************************************************************************/

// trace data streams
#include <string>
#include <vector>

#include "symtabAPI/h/Symbol.h"
#include "symtabAPI/h/LineInformation.h"
#include "common/h/headers.h"
#include "common/h/MappedFile.h"
#include "common/h/lprintf.h"

namespace Dyninst{
namespace SymtabAPI{

extern bool symbol_compare(const Symbol *s1, const Symbol *s2);

class Symtab;
class Region;
class ExceptionBlock;
class relocationEntry;

const char WILDCARD_CHARACTER = '?';
const char MULTIPLE_WILDCARD_CHARACTER = '*';

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
    SYMTABEXPORT AObject();
    SYMTABEXPORT unsigned nsymbols () const;
    
    SYMTABEXPORT bool get_symbols( std::string & name, std::vector< Symbol *> & symbols);

    SYMTABEXPORT char*       code_ptr () const; 
    SYMTABEXPORT Offset           code_off () const;
    SYMTABEXPORT Offset           code_len () const;

    SYMTABEXPORT char*       data_ptr () const;
    SYMTABEXPORT Offset           data_off () const;
    SYMTABEXPORT Offset           data_len () const;

    SYMTABEXPORT bool 	      is_aout  () const;
    SYMTABEXPORT bool        isDynamic() const;

    SYMTABEXPORT unsigned	      no_of_sections () const;
    SYMTABEXPORT unsigned	      no_of_symbols  ()	const;

    SYMTABEXPORT bool getAllExceptions(std::vector<ExceptionBlock *>&excpBlocks) const;
    SYMTABEXPORT std::vector<Region *> getAllRegions() const;


    SYMTABEXPORT supportedLanguages pickLanguage(std::string &working_module, char *working_options,
                                                                    supportedLanguages working_lang);

    SYMTABEXPORT Offset loader_off() const;
    SYMTABEXPORT unsigned loader_len() const;
    SYMTABEXPORT int getAddressWidth() const;

    SYMTABEXPORT virtual char *  mem_image() const;

    SYMTABEXPORT virtual  bool   needs_function_binding()  const;
    SYMTABEXPORT virtual  bool   get_func_binding_table(std::vector<relocationEntry> &) const;
    SYMTABEXPORT virtual  bool   get_func_binding_table_ptr(const std::vector<relocationEntry> *&) const; 
    SYMTABEXPORT virtual  bool   addRelocationEntry(relocationEntry &re);
    SYMTABEXPORT bool   getSegments(std::vector<Segment> &segs) const;

    SYMTABEXPORT bool have_deferred_parsing( void ) const;
    // for debuggering....
    SYMTABEXPORT const std::ostream &dump_state_info(std::ostream &s);

    SYMTABEXPORT void * getErrFunc() const;
    SYMTABEXPORT dyn_hash_map< std::string, std::vector< Symbol *> > *getAllSymbols();
    SYMTABEXPORT MappedFile *getMappedFileForDebugInfo() { return mfForDebugInfo; }



protected:
    SYMTABEXPORT virtual ~AObject();
    // explicitly protected
    SYMTABEXPORT AObject(MappedFile * , MappedFile *, void (*err_func)(const char *));
    SYMTABEXPORT AObject(MappedFile * , MappedFile *, 
                      dyn_hash_map<std::string, LineInformation> &, 
                      void (*)(const char *)) { assert(0); }
    SYMTABEXPORT AObject(const AObject &obj);
    SYMTABEXPORT AObject&  operator= (const AObject &obj);

    MappedFile *mf;
    MappedFile *mfForDebugInfo;

    std::vector< Region *> regions_;
    dyn_hash_map< std::string, std::vector< Symbol *> > symbols_;

    char*   code_ptr_;
    Offset code_off_;
    Offset code_len_;

    char*   data_ptr_;
    Offset data_off_;
    Offset data_len_;

    Offset code_vldS_;
    Offset code_vldE_;

    Offset data_vldS_;
    Offset data_vldE_;

    Offset loader_off_; //only used on aix right now.  could be
    Offset loader_len_; //needed on other platforms in the future

//    Offset loadAddress_;
//    Offset entryAddress_;
//    Offset baseAddress_;
    
    bool is_aout_;
    bool is_dynamic_;

    unsigned no_of_sections_;
    unsigned no_of_symbols_;

    bool deferredParse;
    void (*err_func_)(const char*);
    int addressWidth_nbytes;

    std::vector<ExceptionBlock> catch_addrs_; //Addresses of C++ try/catch blocks;
    
private:
    friend class SymbolIter;
    friend class Symtab;

};

}//namepsace Symtab
}//namespace Dyninst

/************************************************************************
 * include the architecture-operating system specific object files.
************************************************************************/

#if defined(os_solaris) || defined(os_linux)
#include "Object-elf.h"
#elif defined(os_aix)
#include "Object-xcoff.h"
#elif defined(os_windows)
#include "Object-nt.h"
#elif defined(os_osf)
#include "Object-coff.h"
#else
#error "unknown platform"
#endif

/************************************************************************
 * class SymbolIter
************************************************************************/

namespace Dyninst{
namespace SymtabAPI{

class SymbolIter {
 private:
   dyn_hash_map< std::string, std::vector< Symbol *> > *symbols;
   unsigned int currentPositionInVector;
   dyn_hash_map< std::string, std::vector< Symbol *> >::iterator symbolIterator;
   
 public:
   SymbolIter( Object & obj );
   SymbolIter( const SymbolIter & src );
   ~SymbolIter ();
   
   void reset ();
   
   operator bool() const;
   void operator++ ( int );
   const std::string & currkey() const;
   
   /* If it's important that this be const, we could try to initialize
      currentVector to '& symbolIterator.currval()' in the constructor. */
   Symbol *currval();
   
 private:	
   
   SymbolIter & operator = ( const SymbolIter & ); // explicitly disallowed
}; /* end class SymbolIter() */

}//namepsace SymtabAPI
}//namespace Dyninst

#endif /* !defined(_Object_h_) */

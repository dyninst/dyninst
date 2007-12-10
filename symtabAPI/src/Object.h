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
 * $Id: Object.h,v 1.12 2007/12/10 22:33:38 giri Exp $
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
#include "common/h/lprintf.h"

namespace Dyninst{
namespace SymtabAPI{

extern bool symbol_compare(const Symbol *s1, const Symbol *s2);

class Symtab;
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
    DLLEXPORT AObject();
    DLLEXPORT unsigned nsymbols () const;
    
    DLLEXPORT bool get_symbols( std::string & name, std::vector< Symbol *> & symbols);

    DLLEXPORT char*       code_ptr () const; 
    DLLEXPORT Offset           code_off () const;
    DLLEXPORT Offset           code_len () const;

    DLLEXPORT char*       data_ptr () const;
    DLLEXPORT Offset           data_off () const;
    DLLEXPORT Offset           data_len () const;

    DLLEXPORT Offset           code_vldS () const;
    DLLEXPORT Offset           code_vldE () const;
    DLLEXPORT Offset           data_vldS () const;
    DLLEXPORT Offset           data_vldE () const;

    DLLEXPORT bool 	      is_aout  () const;

    DLLEXPORT unsigned	      no_of_sections () const;
    DLLEXPORT unsigned	      no_of_symbols  ()	const;

    DLLEXPORT bool getAllExceptions(std::vector<ExceptionBlock *>&excpBlocks) const;
    DLLEXPORT std::vector<Section *> getAllSections() const;


    DLLEXPORT supportedLanguages pickLanguage(std::string &working_module, char *working_options,
                                                                    supportedLanguages working_lang);

    DLLEXPORT Offset loader_off() const;
    DLLEXPORT unsigned loader_len() const;
    DLLEXPORT int getAddressWidth() const;

    DLLEXPORT virtual char *  mem_image() const;

    DLLEXPORT virtual  bool   needs_function_binding()  const;
    DLLEXPORT virtual  bool   get_func_binding_table(std::vector<relocationEntry> &) const;
    DLLEXPORT virtual  bool   get_func_binding_table_ptr(const std::vector<relocationEntry> *&) const; 
    DLLEXPORT virtual  bool   addRelocationEntry(relocationEntry &re);
    DLLEXPORT bool   getSegments(std::vector<Segment> &segs) const;
    DLLEXPORT bool   getMappedRegions(std::vector<Region> &regs) const;

    DLLEXPORT bool have_deferred_parsing( void ) const;
    // for debuggering....
    DLLEXPORT const std::ostream &dump_state_info(std::ostream &s);

    DLLEXPORT void * getErrFunc() const;
    DLLEXPORT hash_map< std::string, std::vector< Symbol *> > *getAllSymbols();

protected:
    DLLEXPORT virtual ~AObject();
    // explicitly protected
    DLLEXPORT AObject(const std::string file , void (*err_func)(const char *));
    DLLEXPORT AObject(const AObject &obj);
    DLLEXPORT AObject&  operator= (const AObject &obj);

    std::string file_;
    std::vector< Section *> sections_;
    hash_map< std::string, std::vector< Symbol *> > symbols_;

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
   hash_map< std::string, std::vector< Symbol *> > *symbols;
   unsigned int currentPositionInVector;
   hash_map< std::string, std::vector< Symbol *> >::iterator symbolIterator;
   
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

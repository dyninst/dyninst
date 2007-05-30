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
 * $Id: Object.h,v 1.4 2007/05/30 19:20:54 legendre Exp $
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

#include "symtabAPI/h/Dyn_Symbol.h"
#include "symtabAPI/h/Dyn_Symtab.h"
#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/lprintf.h"

extern bool symbol_compare(const Dyn_Symbol *s1, const Dyn_Symbol *s2);

class Dyn_Symtab;
class Dyn_ExceptionBlock;


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
    
    DLLEXPORT bool get_symbols( string & name, vector< Dyn_Symbol *> & symbols);

    DLLEXPORT Word*       code_ptr () const; 
    DLLEXPORT OFFSET           code_off () const;
    DLLEXPORT OFFSET           code_len () const;

    DLLEXPORT Word*       data_ptr () const;
    DLLEXPORT OFFSET           data_off () const;
    DLLEXPORT OFFSET           data_len () const;

    DLLEXPORT OFFSET           code_vldS () const;
    DLLEXPORT OFFSET           code_vldE () const;
    DLLEXPORT OFFSET           data_vldS () const;
    DLLEXPORT OFFSET           data_vldE () const;

    DLLEXPORT bool 	      is_aout  () const;

    DLLEXPORT unsigned	      no_of_sections () const;
    DLLEXPORT unsigned	      no_of_symbols  ()	const;

    DLLEXPORT bool getAllExceptions(vector<Dyn_ExceptionBlock *>&excpBlocks) const;
    DLLEXPORT vector<Dyn_Section *> getAllSections() const;

    DLLEXPORT supportedLanguages pickLanguage(string &working_module, char *working_options,
                                                                    supportedLanguages working_lang);

    DLLEXPORT OFFSET loader_off() const;
    DLLEXPORT unsigned loader_len() const;
    DLLEXPORT int getAddressWidth() const;

    DLLEXPORT virtual char *  mem_image() const;

    DLLEXPORT virtual  bool   needs_function_binding()  const;
    DLLEXPORT virtual  bool   get_func_binding_table(vector<relocationEntry> &) const;
    DLLEXPORT virtual  bool   get_func_binding_table_ptr(const vector<relocationEntry> *&) const;
    
    DLLEXPORT bool have_deferred_parsing( void ) const;
    // for debuggering....
    DLLEXPORT const ostream &dump_state_info(ostream &s);

    DLLEXPORT void * getErrFunc() const;
    DLLEXPORT hash_map< string, vector< Dyn_Symbol *> > *getAllSymbols();

protected:
    DLLEXPORT virtual ~AObject();
    // explicitly protected
    DLLEXPORT AObject(const string file , void (*err_func)(const char *));
    DLLEXPORT AObject(const AObject &obj);
    DLLEXPORT AObject&  operator= (const AObject &obj);

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

class SymbolIter {
 private:
   hash_map< string, vector< Dyn_Symbol *> > *symbols;
   unsigned int currentPositionInVector;
   hash_map< string, vector< Dyn_Symbol *> >::iterator symbolIterator;
   
 public:
   SymbolIter( Object & obj );
   SymbolIter( const SymbolIter & src );
   ~SymbolIter ();
   
   void reset ();
   
   operator bool() const;
   void operator++ ( int );
   const string & currkey() const;
   
   /* If it's important that this be const, we could try to initialize
      currentVector to '& symbolIterator.currval()' in the constructor. */
   Dyn_Symbol *currval();
   
 private:	
   
   SymbolIter & operator = ( const SymbolIter & ); // explicitly disallowed
}; /* end class SymbolIter() */

#endif /* !defined(_Object_h_) */

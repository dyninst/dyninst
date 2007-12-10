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
 * $Id: Object-elf.h,v 1.11 2007/12/10 22:27:49 giri Exp $
 * Object-elf.h: Object class for ELF file format
************************************************************************/


#if !defined(_Object_elf_h_)
#define _Object_elf_h_


#include "symtabAPI/h/Symbol.h"
#include "common/h/Types.h"
#include <elf.h>
#include <libelf.h>

#include "Elf_X.h"

#include <libelf.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

namespace Dyninst{
namespace SymtabAPI{
/*
 * The standard symbol table in an elf file is the .symtab section. This section does
 * not have information to find the module to which a global symbol belongs, so we must
 * also read the .stab section to get this info.
 */

// Declarations for the .stab section.
// These are not declared in any system header files, so we must provide our own
// declarations. The declarations below were taken from:
//       SPARCWorks 3.0.x Debugger Interface, July 1994
// 
struct stab32 {
    unsigned int name;  // stabstr table index for this symbol
    unsigned char type; // type of this symbol
    unsigned char other;
    unsigned short desc;
    unsigned int val;   // value of this symbol -- usually zero. The real value must
			// be obtained from the symtab section
};
struct stab64 {
    // XXX ELF stabs are currently not implementing actual 64-bit support
    //     on AMD-64, for which this separation was introduced. Until we
    //     start seeing stab records with fields of the appropriate length,
    //     we should assume the smaller records.
    //unsigned long name; // stabstr table index for this symbol
    unsigned int name; // stabstr table index for this symbol
    unsigned char type; // type of this symbol
    unsigned char other;
    unsigned short desc;
    //unsigned long val;
    unsigned int val;  // value of this symbol -- usually zero. The real value must
			// be obtained from the symtab section
};

// 
// Extended to a class for 32/64-bit stab entries at runtime. - Ray
// 
class stab_entry {
  public:
    stab_entry(void *_stabptr = 0, const char *_stabstr = 0, long _nsyms = 0)
	: stabptr(_stabptr), stabstr(_stabstr), nsyms(_nsyms) { }
    virtual ~stab_entry() {};

    virtual const char *name(int i) = 0;
    virtual unsigned long nameIdx(int i) = 0;
    virtual unsigned char type(int i) = 0;
    virtual unsigned char other(int i) = 0;
    virtual unsigned short desc(int i) = 0;
    virtual unsigned long val(int i) = 0;

    unsigned long count() { return nsyms; }
    void setStringBase(const char *ptr) { stabstr = const_cast<char *>(ptr); }
    const char *getStringBase() { return stabstr; }

  protected:
    void *stabptr;
    const char *stabstr;
    long nsyms;
};

class stab_entry_32 : public stab_entry {
  public:
    stab_entry_32(void *_stabptr = 0, const char *_stabstr = 0, long _nsyms = 0)
	: stab_entry(_stabptr, _stabstr, _nsyms) { }
    virtual ~stab_entry_32() {};

    const char *name(int i = 0) { return stabstr + ((stab32 *)stabptr)[i].name; }
    unsigned long nameIdx(int i = 0) { return ((stab32 *)stabptr)[i].name; }
    unsigned char type(int i = 0) { return ((stab32 *)stabptr)[i].type; }
    unsigned char other(int i = 0) { return ((stab32 *)stabptr)[i].other; }
    unsigned short desc(int i = 0) { return ((stab32 *)stabptr)[i].desc; }
    unsigned long val(int i = 0) { return ((stab32 *)stabptr)[i].val; }
};

class stab_entry_64 : public stab_entry {
  public:
    stab_entry_64(void *_stabptr = 0, const char *_stabstr = 0, long _nsyms = 0)
	: stab_entry(_stabptr, _stabstr, _nsyms) { }
    virtual ~stab_entry_64() {};

    const char *name(int i = 0) { return stabstr + ((stab64 *)stabptr)[i].name; }
    unsigned long nameIdx(int i = 0) { return ((stab64 *)stabptr)[i].name; }
    unsigned char type(int i = 0) { return ((stab64 *)stabptr)[i].type; }
    unsigned char other(int i = 0) { return ((stab64 *)stabptr)[i].other; }
    unsigned short desc(int i = 0) { return ((stab64 *)stabptr)[i].desc; }
    unsigned long val(int i = 0) { return ((stab64 *)stabptr)[i].val; }
};

// Types 
#define N_UNDF  0x00 /* start of object file */
#define N_GSYM  0x20 /* global symbol */
#define N_FUN   0x24 /* function or procedure */
#define N_STSYM 0x26 /* initialized static symbol */
#define N_LCSYM 0x28 /* unitialized static symbol */
#define N_ROSYM 0x2c /* read-only static symbol */
#define N_OPT   0x3c /* compiler options */
#define N_ENDM  0x62 /* end module */
#define N_SO    0x64 /* source directory and file */
#define N_ENTRY 0xa4 /* fortran alternate subroutine entry point */
#define N_BCOMM 0xe2 /* start fortran named common block */
#define N_ECOMM 0xe4 /* start fortran named common block */

// Language code -- the desc field in a N_SO entry is a language code
#define N_SO_AS      1 /* assembler source */
#define N_SO_C       2 /* K & R C source */
#define N_SO_ANSI_C  3 /* ANSI C source */
#define N_SO_CC      4 /* C++ source */
#define N_SO_FORTRAN 5 /* fortran source */
#define N_SO_PASCAL  6 /* Pascal source */
#define N_SO_F90     7 /* Fortran90 source */

//line information data
#define N_SLINE  0x44 /* line number in text segment */
#define N_SOL    0x84 /* name of the include file*/

// Symbol descriptors
// The format of a name is "<name>:<symbol descriptor><rest of name>
// The following are the descriptors of interest
#define SD_GLOBAL_FUN 'F' /* global function or procedure */
#define SD_PROTOTYPE  'P'  /* function prototypes */
#define SD_GLOBAL_VAR 'G' /* global variable */

// end of stab declarations

class pdElfShdr;

class Symtab;
class Section;

class Object : public AObject {
 public:
  Object(){}
  //void findMain( std::vector< Symbol *> &allsymbols );
  //Offset findDynamic( std::vector< Symbol *> &allsymbols );
  // "Filedescriptor" ctor
  Object(std::string &filename, void (*)(const char *) = log_msg);
  Object(char *mem_image, size_t image_size, void (*)(const char *) = log_msg);
  Object(const Object &);
  virtual ~Object();
  const Object& operator=(const Object &);

  bool emitDriver(Symtab *obj, std::string fName, std::vector<Symbol *>&functions, std::vector<Symbol *>&variables, std::vector<Symbol *>&mods, std::vector<Symbol *>&notypes, unsigned flag);  
  
  const char *elf_vaddr_to_ptr(Offset vaddr) const;
  bool hasStabInfo() const { return ! ( !stab_off_ || !stab_size_ || !stabstr_off_ ); }
  bool hasDwarfInfo() const { return dwarvenDebugInfo; }
  stab_entry * get_stab_info() const;
  const char * getFileName() const { return fileName; }
  void getModuleLanguageInfo(hash_map<std::string, supportedLanguages> *mod_langs);
  void parseFileLineInfo();
  void parseTypeInfo(Symtab *obj);
  bool getMappedRegions(std::vector<Region> &regs) const;

  bool needs_function_binding() const { return (plt_addr_ > 0); } 
  bool get_func_binding_table(std::vector<relocationEntry> &fbt) const;
  bool get_func_binding_table_ptr(const std::vector<relocationEntry> *&fbt) const;

  bool addRelocationEntry(relocationEntry &re);

  //getLoadAddress may return 0 on shared objects
  Offset getLoadAddress() const { return loadAddress_; }

  Offset getEntryAddress() const { return entryAddress_; }
  // To be changed later - Giri
  Offset getBaseAddress() const { return 0; }
  static bool truncateLineFilenames;
 
  virtual char *mem_image() const { 
  	if(fileName == NULL)
		return mem_image_;
  	return file_ptr_; 
  }

  DLLEXPORT ObjectType objType() const;
  const char *interpreter_name() const;


//TODO Later - change this #ifdef later.. make getTOCoffset available for all platforms  

#if defined(arch_ia64)
  Offset getTOCoffset() const { return gp; }
#elif defined(os_linux) && defined(arch_power) && defined(arch_64bit)
  // 64-bit PowerPC ELF ABI Supplement, Version 1.9, 2004-10-23:
  //   The TOC section contains a conventional ELF GOT, and may optionally
  //   contain a small data area.
  //   The TOC base is typically the first address in the TOC plus 0x8000.
  // I don't understand why I can find a ".got" within binaries, but I can't
  // find a ".toc".  ssuen  August 7, 2007
  Offset getTOCoffset() const { return got_addr_ + 0x8000; }
#else
  Offset getTOCoffset() const { return 0; }
#endif

  const std::ostream &dump_state_info(std::ostream &s);
  bool isEEL() { return EEL; }

	//to determine if a mutation falls in the text section of
	// a shared library
	bool isinText(Offset addr, Offset baseaddr) const { 
		//printf(" baseaddr %x TESTING %x %x \n", baseaddr, text_addr_ + baseaddr  , text_addr_ + baseaddr + text_size_ );
		if(addr > text_addr_ + baseaddr     &&
		   addr < text_addr_ + baseaddr + text_size_ ) {
			return true;
		}
		return false;
	} 
	// to determine where in the .plt this function is listed 
	// returns an offset from the base address of the object
	// so the entry can easily be located in memory
	Offset getPltSlot(std::string funcName) const ;
	bool hasSymAtAddr( Offset adr )
	{
	    return (symbolNamesByAddr.find( adr )!=symbolNamesByAddr.end());
	}
	Offset textAddress(){ return text_addr_;}
	bool isText( Offset addr ) const
	{
	    if( addr >= text_addr_ && addr <= text_addr_ + text_size_ )
		return true;
	    return false;
	}

	bool is_offset_in_plt(Offset offset) const;
        hash_map<std::string, LineInformation > &getLineInfo()
        {
	    //Parse Line Info
	    parseFileLineInfo();
		   
            return lineInfo_;
        }

 private:
  static void log_elferror (void (*)(const char *), const char *);
    
  int       file_fd_;            // mapped ELF file
  unsigned  file_size_;          // mapped ELF file
  char     *file_ptr_;           // mapped ELF file
  Elf_X    elfHdr;
  
  const char * fileName;
  char *mem_image_;
  //Symbol    mainSym_;
  Offset   fini_addr_;
  Offset   text_addr_; 	 //.text section 
  Offset   text_size_; 	 //.text section size
  Offset   dynamic_addr_;	 //.dynamic section
  Offset   dynsym_addr_;        // .dynsym section
  Offset   dynstr_addr_;        // .dynstr section
  Offset   got_addr_;           // global offset table
  unsigned  got_size_;           // global offset table
  Offset   plt_addr_;           // procedure linkage table
  unsigned  plt_size_;           // procedure linkage table
  unsigned  plt_entry_size_;     // procedure linkage table
  Offset   rel_plt_addr_;       // .rel[a].plt section
  unsigned  rel_plt_size_;       // .rel[a].plt section
  unsigned  rel_plt_entry_size_; // .rel[a].plt section
  
  unsigned  rel_size_;       // DT_REL/DT_RELA in dynamic section
  unsigned  rel_entry_size_; // DT_REL/DT_RELA in dynamic section

  Offset   stab_off_;           // .stab section
  unsigned  stab_size_;          // .stab section
  Offset   stabstr_off_;        // .stabstr section

  Offset   stab_indx_off_;	 // .stab.index section
  unsigned  stab_indx_size_;	 // .stab.index section
  Offset   stabstr_indx_off_;	 // .stabstr.index section

  bool      dwarvenDebugInfo;    // is DWARF debug info present?
  Offset   loadAddress_;      // The object may specify a load address
                               //   Set to 0 if it may load anywhere
  Offset entryAddress_;
  char *interpreter_name_;
  bool  isStripped;

#if defined(arch_ia64)
  Offset   gp;			 // The gp for this object.
#endif
  bool      EEL;                 // true if EEL rewritten
  bool 	    did_open;		// true if the file has been mmapped
  ObjectType obj_type_;

  // for sparc-solaris this is a table of PLT entry addr, function_name
  // for x86-solaris this is a table of GOT entry addr, function_name
  // on sparc-solaris the runtime linker modifies the PLT entry when it
  // binds a function, on X86 the PLT entry is not modified, but it uses
  // an indirect jump to a GOT entry that is modified when the function 
  // is bound....is this correct???? or should it be <PLTentry_addr, name> 
  // for both?
  std::vector<relocationEntry> relocation_table_;
  std::vector<relocationEntry> fbt_;

  // all section headers, sorted by address
  // we use these to do a better job of finding the end of symbols
  std::vector<Elf_X_Shdr*> allSectionHdrs;

  // It doesn't look like image's equivalent hashtable is built by
  // the time we need it, and it's hard to get to anyway.
  hash_map< Offset, std::string > symbolNamesByAddr;

  // LineNumber information
  hash_map< std::string, LineInformation>lineInfo_;

  // populates: file_fd_, file_size_, file_ptr_
  bool mmap_file(const char *file, 
		 bool &did_open, bool &did_mmap);

  bool loaded_elf( Offset &, Offset &,
		    Elf_X_Shdr* &, Elf_X_Shdr* &, 
		    Elf_X_Shdr* &, Elf_X_Shdr* &, 
		    Elf_X_Shdr* &, Elf_X_Shdr* &, 
		    Elf_X_Shdr*& rel_plt_scnp, Elf_X_Shdr*& plt_scnp, 
		    Elf_X_Shdr*& got_scnp,  Elf_X_Shdr*& dynsym_scnp,
		    Elf_X_Shdr*& dynstr_scnp, Elf_X_Shdr*& dynamic_scnp, Elf_X_Shdr*& eh_frame,
		    Elf_X_Shdr*& gcc_except, Elf_X_Shdr *& interp_scnp,
          bool a_out=false);
  
  void parseStabFileLineInfo();
  void parseDwarfFileLineInfo();

  void parseDwarfTypes(Symtab *obj);
  void parseStabTypes(Symtab *obj);

  void load_object();
  void load_shared_object();

  // initialize relocation_table_ from .rel[a].plt section entries 
  bool get_relocation_entries(Elf_X_Shdr *&rel_plt_scnp,
			      Elf_X_Shdr *&dynsym_scnp, 
			      Elf_X_Shdr *&dynstr_scnp);

  bool get_relocationDyn_entries( Elf_X_Shdr *&rel_scnp,
                     Elf_X_Shdr *&dynsym_scnp,
                     Elf_X_Shdr *&dynstr_scnp );
  
  void parseDynamic(Elf_X_Shdr *& dyn_scnp, Elf_X_Shdr *&dynsym_scnp, 
                    Elf_X_Shdr *&dynstr_scnp);
  
  void parse_symbols(std::vector<Symbol *> &allsymbols, 
		     Elf_X_Data &symdata, Elf_X_Data &strdata,
		     bool shared_library,
		     std::string module);
  
  void parse_dynamicSymbols( Elf_X_Data &symdata,
             Elf_X_Data &strdata, bool shared_library,
		     std::string module);

  void fix_zero_function_sizes(std::vector<Symbol *> &allsymbols, bool EEL);
  void override_weak_symbols(std::vector<Symbol *> &allsymbols);
  void insert_symbols_shared(std::vector<Symbol *> &allsymbols);
  void find_code_and_data(Elf_X &elf,
       Offset txtaddr, Offset dataddr);
  void insert_symbols_static(std::vector<Symbol *> &allsymbols);
  bool fix_global_symbol_modules_static_stab(Elf_X_Shdr *stabscnp,
					     Elf_X_Shdr *stabstrscnp);
  bool fix_global_symbol_modules_static_dwarf(Elf_X &elf);

  void get_valid_memory_areas(Elf_X &elf);

  Elf_X_Shdr *getSectionHdrByAddr(Offset addr);
#if defined(os_irix)

 public:
  Offset     get_gp_value()  const { return gp_value; }
  Offset     get_rbrk_addr() const { return rbrk_addr; }
  Offset     get_base_addr() const { return base_addr; }
  const char *got_entry_name(Offset entry_off) const;
  int         got_gp_disp(const char *entry_name) const;
  bool       

  Offset     MIPS_stubs_addr_;   // .MIPS.stubs section
  Offset     MIPS_stubs_off_;    // .MIPS.stubs section
  unsigned    MIPS_stubs_size_;   // .MIPS.stubs section
  
 private:
  Offset     gp_value;
  Offset     rbrk_addr;
  Offset     base_addr;
  
  int         got_zero_index_;
  int         dynsym_zero_index_;

#endif /* mips_sgi_irix6_4 */
};

//const char *pdelf_get_shnames(Elf *elfp, bool is64);

}//namespace SymtabAPI
}//namespace Dyninst

#endif /* !defined(_Object_elf_h_) */

/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/************************************************************************
 * $Id: Object-elf.h,v 1.21 2008/06/23 18:45:42 legendre Exp $
 * Object-elf.h: Object class for ELF file format
************************************************************************/


#if !defined(_Object_elf_h_)
#define _Object_elf_h_

#if defined(cap_dwarf)
//#include "dwarf.h"
#include "elfutils/libdw.h"
#include "dwarfHandle.h"
#endif

#include <assert.h>
#include <ostream>
#include <map>
#include <stddef.h>
#include <unordered_map>
#include <utility>
#include <vector>
#include "headers.h"
#include "MappedFile.h"
#include "IntervalTree.h"
#include "Module.h"
#include <elf.h>
#include <libelf.h>
#include <string>

#include "Elf_X.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <set>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>


namespace Dyninst{

namespace DwarfDyninst {
   class DwarfFrameParser;
   typedef boost::shared_ptr<DwarfFrameParser> DwarfFrameParserPtr;
}

namespace SymtabAPI{

class pdElfShdr;
class Symtab;
class Region;
class Object;
class InlinedFunction;

class open_statement {
    public:
        open_statement() { reset(); }
        open_statement(const open_statement&) = default;
        Dwarf_Addr noAddress() { return (Dwarf_Addr) ~0; }
        bool uninitialized() {
            return start_addr == noAddress();
        }
        void reset() {
            string_table_index = -1;
            start_addr = noAddress();
            end_addr = noAddress();
            line_number = 0;
            column_number = 0;
            context = nullptr;
            funcname = nullptr;
        }
        bool sameFileLineColumn(const open_statement &rhs) {
            return ((string_table_index == rhs.string_table_index) &&
                    (line_number == rhs.line_number) &&
                    (column_number == rhs.column_number));
        }
        void operator=(const open_statement &rhs) {
            string_table_index = rhs.string_table_index;
            start_addr = rhs.start_addr;
            end_addr = rhs.end_addr;
            line_number = rhs.line_number;
            column_number = rhs.column_number;
            context = rhs.context;
            funcname = rhs.funcname;
        }
        friend std::ostream& operator<<(std::ostream& os, const open_statement& st)
        {
	    st.dump(os, true);
            return os;
	}

        void dump(std::ostream& os, bool addrRange) const {
	    // to facilitate comparison with nvdisasm output, where each function starts at 0,
	    // set o to an offset that makes a function of interest report addresses that
	    // match its unrelocated offsets reported by nvdisasm
	    unsigned int o = 0;
	    if (addrRange) os << "[" << std::hex << start_addr - o << ", " << end_addr - o << "]";
	    else os << "  inlined at";
	    os << " file:" << string_table_index;
	    os << " line:" << std::dec << line_number;
	    os << " col:" << column_number;
	    if (context != nullptr) {
	        os << " context " << context;
           os << " function name " << funcname;
	    }
	    os << std::endl;
        }
    public:
        Dwarf_Word string_table_index;
        Dwarf_Addr start_addr;
        Dwarf_Addr end_addr;
        int line_number;
        int column_number;
        Dwarf_Line* context;
        const char* funcname;
};


class Object : public AObject 
{
  friend class Module;

  // declared but not implemented; no copying allowed
  Object(const Object &);
  const Object& operator=(const Object &);

public:

  Object(MappedFile *, bool, void (*)(const char *) = log_msg, bool alloc_syms = true, Symtab* st = NULL);
  virtual ~Object();

  bool emitDriver(std::string fName, std::set<Symbol *> &allSymbols, unsigned flag);
    
  bool hasDwarfInfo() const { return dwarvenDebugInfo; }
  void getModuleLanguageInfo(dyn_hash_map<std::string, supportedLanguages> *mod_langs);
  void parseFileLineInfo();
  void parseTypeInfo();
  void addModule(SymtabAPI::Module* m) override;

  bool needs_function_binding() const override { return (plt_addr_ > 0); }
  bool get_func_binding_table(std::vector<relocationEntry> &fbt) const override;
  bool get_func_binding_table_ptr(const std::vector<relocationEntry> *&fbt) const override;
  void getDependencies(std::vector<std::string> &deps);
  std::vector<std::string> &libsRMd();

  bool addRelocationEntry(relocationEntry &re) override;

  //getLoadAddress may return 0 on shared objects
  Offset getLoadAddress() const { return loadAddress_; }

  Offset getEntryAddress() const { return entryAddress_; }
  // To be changed later - Giri
  Offset getBaseAddress() const { return 0; }
  static bool truncateLineFilenames;

  void insertPrereqLibrary(std::string libname);
  bool removePrereqLibrary(std::string libname);
  void insertDynamicEntry(long name, long value);
 
  virtual char *mem_image() const override
  {
     assert(mf);
     return (char *)mf->base_addr();
  }

  SYMTAB_EXPORT ObjectType objType() const;
  const char *interpreter_name() const;


  // On most platforms, the TOC offset doesn't exist and is thus null. 
  // On PPC64, it varies _by function_ and is used to point into the GOT,
  // a big data table. We can look it up by parsing the OPD, a function
  // descriptor table. 
  Offset getTOCoffset(Offset off) const;

  // This is an override for the whole thing; we could do per-function but 
  // we're missing a _lot_ of hardware for that. 
  void setTOCoffset(Offset off);

  const std::ostream &dump_state_info(std::ostream &s);
  bool isEEL() { return EEL; }

	//to determine if a mutation falls in the text section of
	// a shared library
	bool isinText(Offset addr, Offset baseaddr) const { 
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
	bool isText( Offset addr ) const
	{
	    if( addr >= text_addr_ && addr <= text_addr_ + text_size_ )
		return true;
	    return false;
	}

   Dyninst::Architecture getArch() const override;
   bool isBigEndianDataEncoding() const override;
   bool getABIVersion(int &major, int &minor) const override;
	bool is_offset_in_plt(Offset offset) const;
    Elf_X_Shdr *getRegionHdrByAddr(Offset addr);
    int getRegionHdrIndexByAddr(Offset addr);
    Elf_X_Shdr *getRegionHdrByIndex(unsigned index);
    bool isRegionPresent(Offset segmentStart, Offset segmentSize, unsigned newPerms);

    bool getRegValueAtFrame(Address pc, 
                            Dyninst::MachRegister reg, 
                            Dyninst::MachRegisterVal &reg_result,
                            MemRegReader *reader) override;
    bool hasFrameDebugInfo() override;
    
    bool convertDebugOffset(Offset off, Offset &new_off);

    std::vector< std::vector<Offset> > getMoveSecAddrRange() const {return moveSecAddrRange;}
    dyn_hash_map<int, Region*> getTagRegionMapping() const { return secTagRegionMapping;}

    bool hasReldyn() const {return hasReldyn_;}
    bool hasReladyn() const {return hasReladyn_;}
    bool hasRelplt() const {return hasRelplt_;}
    bool hasRelaplt() const {return hasRelaplt_;}
    bool hasNoteSection() const {return hasNoteSection_;}
    Region::RegionType getRelType() const override { return relType_; }

    Offset getTextAddr() const {return text_addr_;}
    Offset getSymtabAddr() const {return symtab_addr_;}
    Offset getStrtabAddr() const {return strtab_addr_;}
    Offset getDynamicAddr() const {return dynamic_addr_;}
    Offset getDynsymSize() const {return dynsym_size_;}
    Offset getElfHashAddr() const {return elf_hash_addr_;}
    Offset getGnuHashAddr() const {return gnu_hash_addr_;}
    Offset getRelPLTAddr() const { return rel_plt_addr_; }
    Offset getRelPLTSize() const { return rel_plt_size_; }
    Offset getRelDynAddr() const { return rel_addr_; }
    Offset getRelDynSize() const { return rel_size_; }
    const char* getSoname() const { return soname_; }
    bool hasPieFlag() const { return hasPieFlag_; }
    bool hasProgramLoad() const { return hasProgramLoad_; }
    bool hasDtDebug() const { return hasDtDebug_; }
    bool hasBitsAlloc() const { return hasBitsAlloc_; }
    bool hasDebugSections() const { return hasDebugSections_; }
    bool hasModinfo() const { return hasModinfo_; }
    bool hasGnuLinkonceThisModule() const { return hasGnuLinkonceThisModule_; }
    bool isLoadable() const;
    SYMTAB_EXPORT bool isOnlyExecutable() const;
    SYMTAB_EXPORT bool isExecutable() const;
    SYMTAB_EXPORT bool isSharedLibrary() const;
    SYMTAB_EXPORT bool isOnlySharedLibrary() const;
    SYMTAB_EXPORT bool isDebugOnly() const;
    SYMTAB_EXPORT bool isLinuxKernelModule() const;

    std::vector<relocationEntry> &getPLTRelocs() { return fbt_; }
    std::vector<relocationEntry> &getDynRelocs() { return relocation_table_; }

    Offset getInitAddr() const {return init_addr_; }
    Offset getFiniAddr() const { return fini_addr_; }

    virtual void setTruncateLinePaths(bool value) override;
    virtual bool getTruncateLinePaths() override;
    
    Elf_X * getElfHandle() { return elfHdr; }

    unsigned gotSize() const { return got_size_; }
    Offset gotAddr() const { return got_addr_; }

    SYMTAB_EXPORT virtual void getSegmentsSymReader(std::vector<SymSegment> &segs) override;

    private:
    std::vector<std::vector<boost::shared_ptr<void> > > freeList;
  static void log_elferror (void (*)(const char *), const char *);
    
  Elf_X *elfHdr;
 
  std::vector< std::vector<Offset> > moveSecAddrRange;
  dyn_hash_map<Offset, int> secAddrTagMapping;
  dyn_hash_map<int, unsigned long> secTagSizeMapping;
  dyn_hash_map<int, Region*> secTagRegionMapping;

  bool hasReldyn_;
  bool hasReladyn_;
  bool hasRelplt_;
  bool hasRelaplt_;
  Region::RegionType relType_;

  bool hasNoteSection_;

  Offset   elf_hash_addr_; 	 //.hash section 
  Offset   gnu_hash_addr_; 	 //.gnu.hash section 

  Offset   dynamic_offset_;
  size_t   dynamic_size_;
  size_t   dynsym_size_;
  Offset   init_addr_;
  Offset   fini_addr_;
  Offset   text_addr_; 	 //.text section 
  Offset   text_size_; 	 //.text section size
  Offset   symtab_addr_;
  Offset   strtab_addr_;
  Offset   dynamic_addr_;	 //.dynamic section
  Offset   dynsym_addr_;        // .dynsym section
  Offset   dynstr_addr_;        // .dynstr section
  Offset   got_addr_;           // global offset table
  unsigned got_size_;           // global offset table
  Offset   plt_addr_;           // procedure linkage table
  unsigned plt_size_;           // procedure linkage table
  unsigned plt_entry_size_;     // procedure linkage table
  Offset   rel_plt_addr_;       // .rel[a].plt section
  unsigned rel_plt_size_;       // .rel[a].plt section
  unsigned rel_plt_entry_size_; // .rel[a].plt section
  Offset    rel_addr_;
  unsigned  rel_size_;       // DT_REL/DT_RELA in dynamic section
  unsigned  rel_entry_size_; // DT_REL/DT_RELA in dynamic section
  Offset   opd_addr_;
  unsigned opd_size_;

  bool      dwarvenDebugInfo;    // is DWARF debug info present?
  Offset   loadAddress_;      // The object may specify a load address
                               //   Set to 0 if it may load anywhere
  Offset entryAddress_;
  char *interpreter_name_;
  bool  hasPieFlag_;
  bool  hasDtDebug_;
  bool  hasProgramLoad_;
  bool  hasBitsAlloc_;
  bool  hasDebugSections_;
  bool  hasModinfo_;
  bool  hasGnuLinkonceThisModule_;
  bool  isStripped;

  std::map<Offset, Offset> TOC_table_;

  public:
  Dyninst::DwarfDyninst::DwarfHandle::ptr dwarf;
  private:

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
  std::vector<Elf_X_Shdr*> allRegionHdrs;
  std::vector<Elf_X_Shdr*> allRegionHdrsByShndx;

  // Symbol version mappings. used to store symbol version names.
  dyn_hash_map<unsigned, std::vector<std::string> >versionMapping;
  dyn_hash_map<unsigned, std::string> versionFileNameMapping;

  std::vector<std::string> deps_;
  std::vector<std::string> rmd_deps;

  bool loaded_elf( Offset &, Offset &,
  		    Elf_X_Shdr* &,
		    Elf_X_Shdr* &, Elf_X_Shdr* &, 
		    Elf_X_Shdr*& rel_plt_scnp, Elf_X_Shdr*& plt_scnp, 
		    Elf_X_Shdr*& got_scnp, Elf_X_Shdr*& dynsym_scnp,
		    Elf_X_Shdr*& dynstr_scnp, Elf_X_Shdr*& dynamic_scnp, Elf_X_Shdr*& eh_frame,
		    Elf_X_Shdr*& gcc_except, Elf_X_Shdr *& interp_scnp,
		   Elf_X_Shdr *&opd_scnp, Elf_X_Shdr*& symtab_shndx_scnp,
          bool a_out=false);
  
  Symbol *handle_opd_symbol(Region *opd, Symbol *sym);
  void handle_opd_relocations();
  void parse_opd(Elf_X_Shdr *);
 public:
  void parseDwarfFileLineInfo();
  void parseLineInfoForAddr(Offset addr_to_find);

  bool hasDebugInfo();

private:
    void parseLineInfoForCU(Offset offset, LineInformation* li) override;
    void recordLine(
       Region *debug_str,
       open_statement &saved_statement,
       std::vector<open_statement> &inline_context
    );
    InlinedFunction* recordAnInlinedFunction(
       open_statement&,
       open_statement&,
       StringTablePtr,
       FunctionBase*,
       Dwarf_Addr,
       Dwarf_Addr
    );

    void lookupInlinedContext( std::vector<open_statement> &, open_statement &);
    
    LineInformation* li_for_object;
    LineInformation* parseLineInfoForObject(StringTablePtr strings);

  void parseDwarfTypes(Symtab *obj);

  void load_object(bool);

  // initialize relocation_table_ from .rel[a].plt section entries 
  bool get_relocation_entries(Elf_X_Shdr *&rel_plt_scnp,
			      Elf_X_Shdr *&dynsym_scnp, 
			      Elf_X_Shdr *&dynstr_scnp);

  bool get_relocationDyn_entries( unsigned rel_index,
                     Elf_X_Shdr *&dynsym_scnp,
                     Elf_X_Shdr *&dynstr_scnp );

  // Parses sections with relocations and links these relocations to
  // existing symbols
  bool parse_all_relocations(Elf_X_Shdr *, Elf_X_Shdr *,
          Elf_X_Shdr *, Elf_X_Shdr *);
  
  void parseDynamic(Elf_X_Shdr *& dyn_scnp, Elf_X_Shdr *&dynsym_scnp, 
                    Elf_X_Shdr *&dynstr_scnp);
  
  bool parse_symbols(Elf_X_Data &symdata, Elf_X_Data &strdata,
                     Elf_X_Shdr* bssscnp,
                     Elf_X_Shdr* symscnp,
                     Elf_X_Shdr* symtab_shndx_scnp,
                     bool shared_library);
  
  void parse_dynamicSymbols( Elf_X_Shdr *& dyn_scnp, Elf_X_Data &symdata,
                             Elf_X_Data &strdata, bool shared_library);

  void find_code_and_data(Elf_X &elf,
       Offset txtaddr, Offset dataddr);

  bool fix_global_symbol_modules_static_dwarf();

  void get_valid_memory_areas(Elf_X &elf);

  bool find_catch_blocks(Elf_X_Shdr *eh_frame, Elf_X_Shdr *except_scn,
                         Address textaddr, Address dataaddr,
                         std::vector<ExceptionBlock> &catch_addrs);
  // Line info: CUs to skip
  std::set<std::string> modules_parsed_for_line_info;

 public:
  struct DbgAddrConversion_t {
     DbgAddrConversion_t() : dbg_offset(0x0), dbg_size(0x0), orig_offset(0x0) {}
     std::string name;
     Offset dbg_offset;
     unsigned dbg_size;
     Offset orig_offset;
  };
 private:
  bool DbgSectionMapSorted;
  dyn_mutex dsm_lock;
  std::vector<DbgAddrConversion_t> DebugSectionMap;

 public:  
  std::set<std::string> prereq_libs;
  std::vector<std::pair<long, long> > new_dynamic_entries;
 private:
  const char* soname_;
  Function* containingFunc;
  std::unordered_map<void*, std::vector<open_statement> > contextMap;

        };

}//namespace SymtabAPI
}//namespace Dyninst

#endif /* !defined(_Object_elf_h_) */

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

#if !defined(_emit_Elf_64_h_)
#define _emit_Elf_64_h_

#include "Object.h"
#include <vector>
using namespace std;

namespace Dyninst{
namespace SymtabAPI{

class emitElf64{
  public:
    emitElf64(Elf_X *oldElfHandle_, bool isStripped_ = false, Object *obj_ = NULL, void (*)(const char *) = log_msg);
    ~emitElf64() {
        if( linkedStaticData ) delete linkedStaticData;
    }
    bool createSymbolTables(Symtab *obj, vector<Symbol *>&allSymbols);
    bool driver(Symtab *obj, std::string fName);
 
  private:
    Elf_X *oldElfHandle;
    Elf *newElf;
    Elf *oldElf;
    
    //New Section & Program Headers
    Elf64_Ehdr *newEhdr;
    Elf64_Ehdr *oldEhdr;
    
    Elf64_Phdr *newPhdr;
    Elf64_Phdr *oldPhdr;
    Offset phdr_offset;

    //important data sections in the
    //new Elf that need updated
    Elf_Data *textData;
    Elf_Data *symStrData;
    Elf_Data *dynStrData;
    char *olddynStrData;
    unsigned olddynStrSize;
    Elf_Data *symTabData;
    Elf_Data *dynsymData;
    Elf_Data *dynData;

    Elf_Scn *phdrs_scn;

    std::vector<Region *>nonLoadableSecs;
    std::vector<Region *> newSecs;
    std::map<unsigned, std::vector<Elf64_Dyn *> > dynamicSecData;
    std::vector<std::string> DT_NEEDEDEntries;
    std::vector<std::pair<long, long> > new_dynamic_entries;    
    std::vector<std::string> unversionedNeededEntries;

    // Symbol version table data
    std::map<std::string, std::map<std::string, unsigned> >verneedEntries;    //verneed entries
    std::map<std::string, unsigned> verdefEntries;                            //verdef entries
    std::map<unsigned, std::vector<std::string> > verdauxEntries;
    std::map<std::string, unsigned> versionNames;
    std::vector<Elf64_Half> versionSymTable;
    int curVersionNum, verneednum, verdefnum;

    // Needed when adding a new segment
    Elf64_Off newSegmentStart;
    Elf64_Shdr *firstNewLoadSec;// initialize to NULL
 
    // data segment end
    Elf64_Off dataSegEnd;
	 Elf64_Off dynSegOff, dynSegAddr, phdrSegOff, phdrSegAddr;
    unsigned dynSegSize;

    //Section Names for all sections
    vector<std::string> secNames;
    unsigned secNameIndex;
    Offset currEndOffset;
    Address currEndAddress;

    // Pointer to all relocatable code and data allocated during a static link,
    // to be deleted after written out
    char *linkedStaticData;

    //flags
    // Expand NOBITS sections within the object file to their size
    bool BSSExpandFlag;
    bool movePHdrsFirst;
    bool createNewPhdr;
    bool replaceNOTE;
    unsigned loadSecTotalSize; 

    bool isStripped;
    int library_adjust;
    Object *object;

    void (*err_func_)(const char*);

    bool createElfSymbol(Symbol *symbol, unsigned strIndex, vector<Elf64_Sym *> &symbols, bool dynSymFlag = false);
    void findSegmentEnds();
    void renameSection(const std::string &oldStr, const std::string &newStr, bool renameAll=true);
    void fixPhdrs(unsigned &);
    void createNewPhdrRegion(dyn_hash_map<std::string, unsigned> &newNameIndexMapping);
    bool addSectionHeaderTable(Elf64_Shdr *shdr);
    bool createNonLoadableSections(Elf64_Shdr *& shdr);
    bool createLoadableSections( Symtab * obj,
			 Elf64_Shdr* &shdr, unsigned &extraAlignSize,
                       dyn_hash_map<std::string, unsigned>& newIndexMapping, 
                       unsigned &sectionNumber);
    void createRelocationSections(Symtab *obj, std::vector<relocationEntry> &relocation_table, bool isDynRelocs, dyn_hash_map<std::string, unsigned long> &dynSymNameMapping);

    void updateSymbols(Elf_Data* symtabData,Elf_Data* strData, unsigned long loadSecsSize);

    bool hasRewrittenTLS;
    bool TLSExists;
    Elf64_Shdr *newTLSData;

    void updateDynamic(unsigned tag, Elf64_Addr val);
    void createSymbolVersions(Symtab *obj, Elf64_Half *&symVers, char*&verneedSecData, unsigned &verneedSecSize, char
*&verdefSecData, unsigned &verdefSecSize, unsigned &dynSymbolNamesLength, std::vector<std::string> &dynStrs);
    void createHashSection(Symtab *obj, Elf64_Word *&hashsecData, unsigned &hashsecSize, std::vector<Symbol *>&dynSymbols);
    void createDynamicSection(void *dynData, unsigned size, Elf64_Dyn *&dynsecData, unsigned &dynsecSize, unsigned &dynSymbolNamesLength, std::vector<std::string> &dynStrs);

    void addDTNeeded(std::string s);

    void log_elferror(void (*err_func)(const char *), const char* msg);
    bool hasPHdrSectionBug();
    bool cannotRelocatePhdrs();
};

} // namespace SymtabAPI
} // namespace Dyninst

#endif

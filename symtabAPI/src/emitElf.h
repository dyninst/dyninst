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
#include "debug.h"
#include "Elf_X.h"
#include <iostream>

#include <map>
#include <set>
#include <stddef.h>
#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::vector;

extern const char *STRTAB_NAME;
extern const char *SYMTAB_NAME;
extern const char *INTERP_NAME;

extern const char *pdelf_get_shnames(Dyninst::Elf_X *elf);

#define PT_PAX_FLAGS  (PT_LOOS + 0x5041580) /* PaX flags */

namespace Dyninst {
    namespace SymtabAPI {
// Error reporting

        struct sortByOffsetNewIndices {
            bool operator()(Symbol *lhs, Symbol *rhs) const {
                auto lIndex = lhs->getIndex();
                auto rIndex = rhs->getIndex();
                if(lIndex==-1 && rIndex==-1)
                    return lhs->getOffset() < rhs->getOffset(); 
                lIndex = ULONG_MAX ? lIndex==-1 : lIndex;  
                rIndex = ULONG_MAX ? rIndex==-1 : rIndex;  
                return lIndex < rIndex;
            }
        };
        struct sortByIndex {
            bool operator()(Symbol *lhs, Symbol *rhs) const {
                return lhs->getIndex() < rhs->getIndex();
            }
        };

        struct ElfTypes32 {
            typedef Elf32_Ehdr Elf_Ehdr;
            typedef Elf32_Phdr Elf_Phdr;
            typedef Elf32_Shdr Elf_Shdr;
            typedef Elf32_Dyn Elf_Dyn;
            typedef Elf32_Half Elf_Half;
            typedef Elf32_Addr Elf_Addr;
            typedef Elf32_Off Elf_Off;
            typedef Elf32_Word Elf_Word;
            typedef Elf32_Sym Elf_Sym;
            typedef Elf32_Section Elf_Section;
            typedef Elf32_Rel Elf_Rel;
            typedef Elf32_Rela Elf_Rela;
            typedef Elf32_Verneed Elf_Verneed;
            typedef Elf32_Vernaux Elf_Vernaux;
            typedef Elf32_Verdef Elf_Verdef;
            typedef Elf32_Verdaux Elf_Verdaux;

            Elf_Ehdr *elf_newehdr(Elf *elf) { return elf32_newehdr(elf); }

            Elf_Phdr *elf_newphdr(Elf *elf, size_t num) { return elf32_newphdr(elf, num); }

            Elf_Ehdr *elf_getehdr(Elf *elf) { return elf32_getehdr(elf); }

            Elf_Phdr *elf_getphdr(Elf *elf) { return elf32_getphdr(elf); }

            Elf_Shdr *elf_getshdr(Elf_Scn *scn) { return elf32_getshdr(scn); }

            Elf32_Word makeRelocInfo(Elf32_Word sym, Elf32_Word type) { return ELF32_R_INFO(sym, type); }
        };

        struct ElfTypes64 {
            typedef Elf64_Ehdr Elf_Ehdr;
            typedef Elf64_Phdr Elf_Phdr;
            typedef Elf64_Shdr Elf_Shdr;
            typedef Elf64_Dyn Elf_Dyn;
            typedef Elf64_Half Elf_Half;
            typedef Elf64_Addr Elf_Addr;
            typedef Elf64_Off Elf_Off;
            typedef Elf64_Word Elf_Word;
            typedef Elf64_Sym Elf_Sym;
            typedef Elf64_Section Elf_Section;
            typedef Elf64_Rel Elf_Rel;
            typedef Elf64_Rela Elf_Rela;
            typedef Elf64_Verneed Elf_Verneed;
            typedef Elf64_Vernaux Elf_Vernaux;
            typedef Elf64_Verdef Elf_Verdef;
            typedef Elf64_Verdaux Elf_Verdaux;

            Elf_Ehdr *elf_newehdr(Elf *elf) { return elf64_newehdr(elf); }

            Elf_Phdr *elf_newphdr(Elf *elf, size_t num) { return elf64_newphdr(elf, num); }

            Elf_Ehdr *elf_getehdr(Elf *elf) { return elf64_getehdr(elf); }

            Elf_Phdr *elf_getphdr(Elf *elf) { return elf64_getphdr(elf); }

            Elf_Shdr *elf_getshdr(Elf_Scn *scn) { return elf64_getshdr(scn); }

            Elf64_Xword makeRelocInfo(Elf64_Word sym, Elf64_Word type) { return ELF64_R_INFO(sym, type); }
        };

        template<class ElfTypes = ElfTypes64> class emitElf : public ElfTypes {
        public:
            emitElf(Elf_X *pX, bool i, Object *pObject, void (*pFunction)(const char *), Symtab *pSymtab);

            typedef typename ElfTypes::Elf_Ehdr Elf_Ehdr;
            typedef typename ElfTypes::Elf_Phdr Elf_Phdr;
            typedef typename ElfTypes::Elf_Shdr Elf_Shdr;
            typedef typename ElfTypes::Elf_Dyn Elf_Dyn;
            typedef typename ElfTypes::Elf_Half Elf_Half;
            typedef typename ElfTypes::Elf_Addr Elf_Addr;
            typedef typename ElfTypes::Elf_Off Elf_Off;
            typedef typename ElfTypes::Elf_Word Elf_Word;
            typedef typename ElfTypes::Elf_Sym Elf_Sym;
            typedef typename ElfTypes::Elf_Section Elf_Section;
            typedef typename ElfTypes::Elf_Rel Elf_Rel;
            typedef typename ElfTypes::Elf_Rela Elf_Rela;
            typedef typename ElfTypes::Elf_Verneed Elf_Verneed;
            typedef typename ElfTypes::Elf_Vernaux Elf_Vernaux;
            typedef typename ElfTypes::Elf_Verdef Elf_Verdef;
            typedef typename ElfTypes::Elf_Verdaux Elf_Verdaux;

            ~emitElf() {
                if( linkedStaticData ) delete[] linkedStaticData;
                for(auto *b : buffers) free(b);
            }

            bool createSymbolTables(std::set<Symbol *> &allSymbols);

            bool driver(std::string fName);

        private:
            Elf_X *oldElfHandle;
            Elf *newElf;
            Elf *oldElf;
            Symtab *obj;
            //New Section & Program Headers
            Elf_Ehdr *newEhdr;
            Elf_Ehdr *oldEhdr;

            Elf_Phdr *newPhdr;
            Elf_Phdr *oldPhdr;
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
            std::map<unsigned, std::vector<Elf_Dyn *> > dynamicSecData;
            std::vector<std::string> DT_NEEDEDEntries;
            std::vector<std::pair<long, long> > new_dynamic_entries;
            std::vector<std::string> unversionedNeededEntries;

            // Symbol version table data
            std::map<std::string, std::map<std::string, unsigned> >verneedEntries;    //verneed entries
            std::map<std::string, unsigned> verdefEntries;                            //verdef entries
            std::map<unsigned, std::vector<std::string> > verdauxEntries;
            std::map<std::string, unsigned> versionNames;
            std::vector<Elf_Half> versionSymTable;
            int curVersionNum, verneednum, verdefnum, dynsym_info{};

            // Needed when adding a new segment
            Elf_Off newSegmentStart;
            Elf_Shdr *firstNewLoadSec;// initialize to NULL

            // data segment end
            Elf_Off dataSegEnd;
            Elf_Off dynSegOff, dynSegAddr, phdrSegOff, phdrSegAddr;
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

            bool createElfSymbol(Symbol *symbol, unsigned strIndex, vector<Elf_Sym *> &symbols,
                                 bool dynSymFlag = false);
            void findSegmentEnds();
            void renameSection(const std::string &oldStr, const std::string &newStr, bool renameAll=true);
            void fixPhdrs(unsigned &);
            void createNewPhdrRegion(std::unordered_map<std::string, unsigned> &newNameIndexMapping);

            bool addSectionHeaderTable(Elf_Shdr *shdr);

            bool createNonLoadableSections(Elf_Shdr *&shdr);

            bool createLoadableSections(Elf_Shdr *&shdr, unsigned &extraAlignSize,
                                        std::unordered_map<std::string, unsigned> &newIndexMapping,
                                        unsigned &sectionNumber);

            void createRelocationSections(std::vector<relocationEntry> &relocation_table, bool isDynRelocs,
                                          std::unordered_map<std::string, unsigned long> &dynSymNameMapping);

            void updateSymbols(Elf_Data* symtabData,Elf_Data* strData, unsigned long loadSecsSize);

            bool hasRewrittenTLS;
            bool TLSExists;
            Elf_Shdr *newTLSData;

            void updateDynamic(unsigned tag, Elf_Addr val);

            void createSymbolVersions(Elf_Half *&symVers, char *&verneedSecData, unsigned &verneedSecSize,
                                      char *&verdefSecData,
                                      unsigned &verdefSecSize, unsigned &dynSymbolNamesLength,
                                      std::vector<std::string> &dynStrs);

            void createHashSection(Elf_Word *&hashsecData, unsigned &hashsecSize, std::vector<Symbol *> &dynSymbols);

            void createDynamicSection(void *dynData, unsigned size, Elf_Dyn *&dynsecData, unsigned &dynsecSize,
                                      unsigned &dynSymbolNamesLength, std::vector<std::string> &dynStrs);

            void addDTNeeded(std::string s);

            void log_elferror(void (*err_func)(const char *), const char* msg);
            bool cannotRelocatePhdrs();

            bool isStaticBinary;
            std::vector<void*> buffers;
            char* allocate_buffer(size_t);

        };
        extern template class emitElf<ElfTypes32>;
        extern template class emitElf<ElfTypes64>;

    } // namespace SymtabAPI
} // namespace Dyninst

#endif

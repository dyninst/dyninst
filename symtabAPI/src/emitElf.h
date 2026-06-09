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

#include "Object-elf.h"
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

#ifndef STT_GNU_IFUNC
#define STT_GNU_IFUNC 10
#endif
#ifndef STB_GNU_UNIQUE
#define STB_GNU_UNIQUE 10
#endif
#ifndef PT_GNU_PROPERTY
#define PT_GNU_PROPERTY 0x6474e553
#endif

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
            using Elf_Ehdr = Elf32_Ehdr;
            using Elf_Phdr = Elf32_Phdr;
            using Elf_Shdr = Elf32_Shdr;
            using Elf_Dyn = Elf32_Dyn;
            using Elf_Half = Elf32_Half;
            using Elf_Addr = Elf32_Addr;
            using Elf_Off = Elf32_Off;
            using Elf_Word = Elf32_Word;
            using Elf_Sym = Elf32_Sym;
            using Elf_Section = Elf32_Section;
            using Elf_Rel = Elf32_Rel;
            using Elf_Rela = Elf32_Rela;
            using Elf_Verneed = Elf32_Verneed;
            using Elf_Vernaux = Elf32_Vernaux;
            using Elf_Verdef = Elf32_Verdef;
            using Elf_Verdaux = Elf32_Verdaux;

            Elf_Ehdr *elf_newehdr(Elf *elf) { return elf32_newehdr(elf); }

            Elf_Phdr *elf_newphdr(Elf *elf, size_t num) { return elf32_newphdr(elf, num); }

            Elf_Ehdr *elf_getehdr(Elf *elf) { return elf32_getehdr(elf); }

            Elf_Phdr *elf_getphdr(Elf *elf) { return elf32_getphdr(elf); }

            Elf_Shdr *elf_getshdr(Elf_Scn *scn) { return elf32_getshdr(scn); }

            Elf32_Word makeRelocInfo(Elf32_Word sym, Elf32_Word type) { return ELF32_R_INFO(sym, type); }
        };

        struct ElfTypes64 {
            using Elf_Ehdr = Elf64_Ehdr;
            using Elf_Phdr = Elf64_Phdr;
            using Elf_Shdr = Elf64_Shdr;
            using Elf_Dyn = Elf64_Dyn;
            using Elf_Half = Elf64_Half;
            using Elf_Addr = Elf64_Addr;
            using Elf_Off = Elf64_Off;
            using Elf_Word = Elf64_Word;
            using Elf_Sym = Elf64_Sym;
            using Elf_Section = Elf64_Section;
            using Elf_Rel = Elf64_Rel;
            using Elf_Rela = Elf64_Rela;
            using Elf_Verneed = Elf64_Verneed;
            using Elf_Vernaux = Elf64_Vernaux;
            using Elf_Verdef = Elf64_Verdef;
            using Elf_Verdaux = Elf64_Verdaux;

            Elf_Ehdr *elf_newehdr(Elf *elf) { return elf64_newehdr(elf); }

            Elf_Phdr *elf_newphdr(Elf *elf, size_t num) { return elf64_newphdr(elf, num); }

            Elf_Ehdr *elf_getehdr(Elf *elf) { return elf64_getehdr(elf); }

            Elf_Phdr *elf_getphdr(Elf *elf) { return elf64_getphdr(elf); }

            Elf_Shdr *elf_getshdr(Elf_Scn *scn) { return elf64_getshdr(scn); }

            Elf64_Xword makeRelocInfo(Elf64_Word sym, Elf64_Word type) { return ELF64_R_INFO(sym, type); }
        };

        template<class ElfTypes = ElfTypes64> class emitElf : public ElfTypes {
        public:
            emitElf(Elf_X *pX, bool i, ObjectELF *pObject, void (*pFunction)(const char *), Symtab *pSymtab);

            using Elf_Ehdr = typename ElfTypes::Elf_Ehdr;
            using Elf_Phdr = typename ElfTypes::Elf_Phdr;
            using Elf_Shdr = typename ElfTypes::Elf_Shdr;
            using Elf_Dyn = typename ElfTypes::Elf_Dyn;
            using Elf_Half = typename ElfTypes::Elf_Half;
            using Elf_Addr = typename ElfTypes::Elf_Addr;
            using Elf_Off = typename ElfTypes::Elf_Off;
            using Elf_Word = typename ElfTypes::Elf_Word;
            using Elf_Sym = typename ElfTypes::Elf_Sym;
            using Elf_Section = typename ElfTypes::Elf_Section;
            using Elf_Rel = typename ElfTypes::Elf_Rel;
            using Elf_Rela = typename ElfTypes::Elf_Rela;
            using Elf_Verneed = typename ElfTypes::Elf_Verneed;
            using Elf_Vernaux = typename ElfTypes::Elf_Vernaux;
            using Elf_Verdef = typename ElfTypes::Elf_Verdef;
            using Elf_Verdaux = typename ElfTypes::Elf_Verdaux;

            ~emitElf() {
                if( linkedStaticData ) delete[] linkedStaticData;
                for(auto *b : buffers) free(b);
            }

            bool createSymbolTables(std::set<Symbol *> &allSymbols);

            bool driver(std::string fName);

        private:
            Elf_X *oldElfHandle{};
            Elf *newElf{};
            Elf *oldElf{};
            Symtab *obj{};
            //New Section & Program Headers
            Elf_Ehdr *newEhdr{};
            Elf_Ehdr *oldEhdr{};

            Elf_Phdr *newPhdr{};
            Elf_Phdr *oldPhdr{};
            Offset phdr_offset{};

            //important data sections in the
            //new Elf that need updated
            Elf_Data *textData{};
            Elf_Data *symStrData{};
            Elf_Data *dynStrData{};
            char *olddynStrData{};
            unsigned olddynStrSize{};
            Elf_Data *symTabData{};
            Elf_Data *dynsymData{};
            Elf_Data *dynData{};

            Elf_Scn *phdrs_scn{};

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
            int curVersionNum{2};
            int verneednum{};
            int verdefnum{};

            // Needed when adding a new segment
            Elf_Off newSegmentStart{};
            Elf_Shdr *firstNewLoadSec{};// initialize to NULL

            // data segment end
            unsigned lastLoadedSectionNum{};
            Elf_Off dynSegOff{};
            Elf_Off dynSegAddr{};
            Elf_Off phdrSegOff{};
            Elf_Off phdrSegAddr{};
            unsigned dynSegSize{};

            //Section Names for all sections
            vector<std::string> secNames{};
            unsigned secNameTableTotalBytes{};

            // add name to secNames, update total bytes including null
            // returns old total bytes (the offset of just added name)
            unsigned addSectionName(std::string name)  {
                auto oldOffset{secNameTableTotalBytes};
                secNameTableTotalBytes += name.size() + 1;
                secNames.push_back(std::move(name));
                return oldOffset;
            }

            Offset currEndOffset{};
            Address currEndAddress{};

            // Pointer to all relocatable code and data allocated during a static link,
            // to be deleted after written out
            char *linkedStaticData{};

            //flags
            // Expand NOBITS sections within the object file to their size
            unsigned loadSecTotalSize{};

            bool isStripped{};
            ObjectELF *object{};

            void (*err_func_)(const char*);

            bool hasRewrittenTLS{};
            bool TLSExists{};
            Elf_Shdr *newTLSData{};
            bool isStaticBinary{};

            bool createElfSymbol(Symbol *symbol, unsigned strIndex, vector<Elf_Sym *> &symbols,
                                 bool dynSymFlag = false);
            void getSectionAndSegmentProperties(const char *shnames);
            void renameSection(const std::string &oldName);
            void fixPhdrs();
            void createNewPhdrRegion(Elf_Shdr* &newshdr, std::unordered_map<std::string, unsigned> &newNameIndexMapping);

            bool addSectionHeaderTable(Elf_Shdr *shdr);

            bool createNonLoadableSections(Elf_Shdr *&shdr);

            bool createLoadableSections(Elf_Shdr *&shdr, unsigned &extraAlignSize,
                                        std::unordered_map<std::string, unsigned> &newIndexMapping,
                                        unsigned &sectionNumber);

            void createRelocationSections(std::vector<relocationEntry> &relocation_table, bool isDynRelocs,
                                          std::unordered_map<std::string, unsigned long> &dynSymNameMapping);

            void updateSymbols(Elf_Data* symtabData,Elf_Data* strData, unsigned long loadSecsSize);

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

            std::vector<void*> buffers;
            char* allocate_buffer(size_t);

            #define jk_rewrite_printf rewrite_printf
            #define jk_log_shdr(msg, name, index, shdr) do {jk_rewrite_printf("%s%s ", msg, (msg && msg[0]) ? " " : "");jk_dump_shdr(shdr, index, name);}while(0)
            bool jk_dump_16_byte_line(const unsigned char* buf, unsigned long offset, unsigned low, unsigned high)
            {
                offset -= offset % 16;
                if (offset >= high)  {
                    return false;
                }
                jk_rewrite_printf("%06lx: ", offset);
                for (auto i = offset; i < offset + 16; ++i)  {
                    if (i % 8 == 0)  {
                        jk_rewrite_printf(" ");
                    }
                    if (low <= i && i < high)  {
                        jk_rewrite_printf(" %02x", buf[i]);
                    }  else  {
                        jk_rewrite_printf("   ");
                    }
                }
                jk_rewrite_printf("   ");
                for (auto i = offset; i < offset + 16; ++i)  {
                    if (i % 8 == 0)  {
                        jk_rewrite_printf("  ");
                    }
                    if (low <= i && i < high)  {
                        unsigned char c = buf[i];
                        if (c < 0x20 || (c >= 0x80 && c < 0xa0))  {
                            c = '.';
                        }
                        jk_rewrite_printf("%c", c);
                    }  else  {
                        jk_rewrite_printf("   ");
                    }
                }
                jk_rewrite_printf("\n");
                return true;
            }

            void jk_dump_bytes(const unsigned char* buf, unsigned long len, std::string msg = "", unsigned long start = 0)
            {
                if (!msg.empty())
                    msg += ":  ";
                jk_rewrite_printf("\n-------------------\n%sDump @%p for %lu bytes\n-------------------\n", msg.c_str(), buf, len);
                unsigned long offset{start};
                while (jk_dump_16_byte_line(buf, offset, start, len))  {
                    offset += 16;
                }
            }
            void jk_dump_bytes(const char* buf, unsigned long len, std::string msg = "", unsigned long start = 0)
            {
                jk_dump_bytes((const unsigned char*) buf, len, msg, start);
            }
            void jk_dump_bytes(void* buf, unsigned long len, std::string msg = "", unsigned long start = 0)
            {
                jk_dump_bytes((const unsigned char*) buf, len, msg, start);
            }
            void jk_dump_ehdr(Elf *elf)
            {
                #define ehdr_field_log(fld, type) jk_rewrite_printf("%-16s " #type "\n", "elf." #fld, (unsigned long)ehdr->fld);
                auto ehdr = this->elf_getehdr(elf);

                ehdr_field_log(e_type,		%lu);
                ehdr_field_log(e_machine,	%lu);
                ehdr_field_log(e_version,	%lu);
                ehdr_field_log(e_entry,		%lx);
                ehdr_field_log(e_phoff,		%lx);
                ehdr_field_log(e_shoff,		%lx);
                ehdr_field_log(e_flags,		%lx);
                ehdr_field_log(e_ehsize,	%lu);
                ehdr_field_log(e_phentsize,	%lu);
                ehdr_field_log(e_phnum,		%lu);
                ehdr_field_log(e_shentsize,	%lu);
                ehdr_field_log(e_shnum,		%lu);
                ehdr_field_log(e_shstrndx,	%lu);
            }
            const char* jk_scnTypeStr(Elf64_Word type) {
                static char unknownBuf[50];
                #define shTypeToStrCase(s) case s: return & #s [4]
                switch (type)  {
                    shTypeToStrCase(SHT_NULL);
                    shTypeToStrCase(SHT_PROGBITS);
                    shTypeToStrCase(SHT_SYMTAB);
                    shTypeToStrCase(SHT_STRTAB);
                    shTypeToStrCase(SHT_RELA);
                    shTypeToStrCase(SHT_HASH);
                    shTypeToStrCase(SHT_DYNAMIC);
                    shTypeToStrCase(SHT_NOTE);
                    shTypeToStrCase(SHT_NOBITS);
                    shTypeToStrCase(SHT_REL);
                    shTypeToStrCase(SHT_SHLIB);
                    shTypeToStrCase(SHT_DYNSYM);
                    shTypeToStrCase(SHT_INIT_ARRAY);
                    shTypeToStrCase(SHT_FINI_ARRAY);
                    shTypeToStrCase(SHT_PREINIT_ARRAY);
                    shTypeToStrCase(SHT_GROUP);
                    shTypeToStrCase(SHT_SYMTAB_SHNDX);
                    shTypeToStrCase(SHT_GNU_ATTRIBUTES);
                    shTypeToStrCase(SHT_GNU_HASH);
                    shTypeToStrCase(SHT_GNU_LIBLIST);
                    shTypeToStrCase(SHT_GNU_verdef);
                    shTypeToStrCase(SHT_GNU_verneed);
                    shTypeToStrCase(SHT_GNU_versym);
                    default:
                        sprintf(unknownBuf, "%08lx", (unsigned long)type);
                        return unknownBuf;
                }
            }
            std::string jk_scnFlagsStr(Elf64_Word flags)
            {
                Elf64_Word mask = ~(Elf64_Word(SHF_WRITE) | SHF_ALLOC | SHF_EXECINSTR | SHF_TLS | SHF_MERGE | SHF_INFO_LINK | SHF_STRINGS);
                if (flags & mask)  {
                    char buf[50];
                    sprintf(buf, "%lx", (unsigned long)flags);
                    return buf;
                }
                std::string r{"       "};
                if (flags & SHF_WRITE)          r[0] = 'W';
                if (flags & SHF_ALLOC)          r[1] = 'A';
                if (flags & SHF_EXECINSTR)      r[2] = 'X';
                if (flags & SHF_TLS)            r[3] = 'T';
                if (flags & SHF_MERGE)          r[4] = 'M';
                if (flags & SHF_STRINGS)        r[5] = 's';
                if (flags & SHF_INFO_LINK)      r[6] = 'L';
                return r;
            }
            void jk_dump_shdr(Elf_Shdr *shdr, unsigned index=99, std::string name = "")
            {
                jk_rewrite_printf("[%2lu] (%3lu) %-21s %-15s %08lx %08lx %08lx %02lx %-6s %02lx %04lx %08lx\n",
                    (unsigned long)index,
                    (unsigned long)shdr->sh_name,
                    name.c_str(),
                    jk_scnTypeStr(shdr->sh_type),
                    (unsigned long)shdr->sh_addr,
                    (unsigned long)shdr->sh_offset,
                    (unsigned long)shdr->sh_size,
                    (unsigned long)shdr->sh_entsize,
                    jk_scnFlagsStr(shdr->sh_flags).c_str(),
                    (unsigned long)shdr->sh_link,
                    (unsigned long)shdr->sh_info,
                    (unsigned long)shdr->sh_addralign
                );
            }
            void jk_dump_scn(Elf_Scn *scn, unsigned index=99, std::string name = "")
            {
                auto shdr{this->elf_getshdr(scn)};
                jk_dump_shdr(shdr, index, name);
            }
            void jk_dump_shdrs(Elf *elf)
            {
                jk_rewrite_printf("indx namei name                  type            address  offset   size     es flag   lk info algn\n---\n");
                auto ehdr = this->elf_getehdr(elf);
                auto shstrndx = ehdr->e_shstrndx;
                const char *names{};
                unsigned namesBytes{};
                if (shstrndx < ehdr->e_shnum)  {
                    auto shstrscn = elf_getscn(elf, shstrndx);
                    auto shstrdata = elf_getdata(shstrscn, NULL);
                    names = (const char*)shstrdata->d_buf;
                    namesBytes = shstrdata->d_size;
                }
                Elf_Scn *scn{};
                for (unsigned i = 1; (scn = elf_nextscn(elf, scn)); ++i)  {
                    std::string name{"shstr_scn_not_found"};
                    auto shdr{ElfTypes::elf_getshdr(scn)};
                    if (names)  {
                        auto nameIndex = shdr->sh_name;
                        if (nameIndex < namesBytes)  {
                            name = &names[nameIndex];
                        }  else  {
                            name = "nameIndex_out_of_bounds";
                        }
                    }
                    jk_dump_shdr(shdr, i, name);
                }
            }
            void jk_dump_new_shdrs(Elf *elf)
            {
                Elf_Scn *scn{};
                for (unsigned i = 1; (scn = elf_nextscn(elf, scn)); ++i)  {
                    std::string name{"index_too_big"};
                    if (i < secNames.size())
                        name = secNames[i];
                    jk_dump_scn(scn, i);
                }
            }
            const char* jk_phdrTypeStr(Elf64_Word type) {
                static char unknownBuf[50];
                #define phTypeToStrCase(s) case s: return & #s [3]
                switch (type) {
                    phTypeToStrCase(PT_NULL);
                    phTypeToStrCase(PT_LOAD);
                    phTypeToStrCase(PT_DYNAMIC);
                    phTypeToStrCase(PT_INTERP);
                    phTypeToStrCase(PT_NOTE);
                    phTypeToStrCase(PT_SHLIB);
                    phTypeToStrCase(PT_PHDR);
                    phTypeToStrCase(PT_TLS);
                    phTypeToStrCase(PT_GNU_EH_FRAME);
                    phTypeToStrCase(PT_GNU_STACK);
                    phTypeToStrCase(PT_GNU_RELRO);
                    phTypeToStrCase(PT_GNU_PROPERTY);
                    phTypeToStrCase(PT_PAX_FLAGS);
                    default:
                        sprintf(unknownBuf, "%08lx", (unsigned long)type);
                        return unknownBuf;
                }

            }
            std::string jk_phFlagsStr(Elf64_Word flags)
            {
                Elf64_Word mask = ~(Elf64_Word(PF_X) | PF_W | PF_R);
                if (flags & mask)  {
                    char buf[50];
                    sprintf(buf, "%lx", (unsigned long)flags);
                    return buf;
                }
                std::string r{"   "};
                if (flags & PF_R)       r[0] = 'R';
                if (flags & PF_W)       r[1] = 'W';
                if (flags & PF_X)       r[2] = 'X';
                return r;
            }
            void jk_dump_phdr(Elf_Phdr *phdr)
            {
                jk_rewrite_printf("%-12s %08lx %08lx %08lx %08lx %08lx %-3s %08lx\n",
                    jk_phdrTypeStr(phdr->p_type),
                    (unsigned long)phdr->p_offset,
                    (unsigned long)phdr->p_vaddr,
                    (unsigned long)phdr->p_paddr,
                    (unsigned long)phdr->p_filesz,
                    (unsigned long)phdr->p_memsz,
                    jk_phFlagsStr(phdr->p_flags).c_str(),
                    (unsigned long)phdr->p_align
                );
            }
            void jk_dump_phdrs(Elf *elf)
            {
                jk_rewrite_printf("type         offset   vaddr    paddr    filesz   memsz    flg align\n---\n");
                auto ehdr = this->elf_getehdr(elf);
                auto phdrs = ElfTypes::elf_getphdr(elf);
                for (unsigned i = 0; i < ehdr->e_phnum; ++i)  {
                    auto phdr{&phdrs[i]};
                    jk_dump_phdr(phdr);
                }
            }
            void jk_dump_phdrs_bytes(Elf *elf)
            {
                auto ehdr = this->elf_getehdr(elf);
                auto phdrs = ElfTypes::elf_getphdr(elf);
                jk_dump_bytes((unsigned char*)phdrs, ehdr->e_phnum * sizeof(phdrs[0]), "elf_getphdr bytes");
            }
            void jk_dump_scn_bytes(Elf_Scn *scn, const std::string msg = "")
            {
                if (!scn)  {
                    jk_rewrite_printf("jk_dump_scn_data: ERROR scn == NULL\n");
                    return;
                }
                Elf_Data * data = NULL;
                while ((data = elf_getdata(scn, data)))  {
                    auto bytes = data->d_buf;
                    auto len = data->d_size;
                    jk_dump_bytes(bytes, len, msg);
                }
            }
            void jk_dump_scn_id_bytes(Elf *elf, unsigned long i, const char *msg = nullptr)
            {
                auto scn = elf_getscn(elf, i);
                jk_dump_scn_bytes(scn, msg);
            }
            void jk_dump_elf(Elf *elf, const char *msg = nullptr)
            {
                if (msg)  {
                    jk_rewrite_printf("\n-------------------\n%s\n-------------------\n", msg);
                }
                jk_rewrite_printf("Ehdr\n-----\n");
                jk_dump_ehdr(elf);
                auto ehdr = this->elf_getehdr(elf);
                jk_dump_scn_id_bytes(elf, ehdr->e_shstrndx, "shstr section");
                jk_rewrite_printf("\nPhdrs\n-----\n");
                jk_dump_phdrs(elf);
                jk_dump_phdrs_bytes(elf);
                jk_rewrite_printf("\nSections\n-----\n");
                jk_dump_shdrs(elf);
            }


        };
        extern template class emitElf<ElfTypes32>;
        extern template class emitElf<ElfTypes64>;

    } // namespace SymtabAPI
} // namespace Dyninst

#endif

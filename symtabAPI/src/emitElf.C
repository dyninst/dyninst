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

#include <cstring>
#include <algorithm>
#include <numeric>
#include "emitElf.h"
#include "emitElfStatic.h"
#include "common/src/dyninst_filesystem.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "unaligned_memory_access.h"


#if defined(os_linux)

#include "common/src/linuxKludges.h"

#endif

extern void symtab_log_perror(const char *msg);
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

unsigned int elfHash(const std::string &name) {
    unsigned int h = 0, g;

    for (auto c : name) {
        h = (h << 4) + c;
        if ((g = h & 0xf0000000))
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

template<class ElfTypes>
emitElf<ElfTypes>::emitElf(Elf_X *oldElfHandle_, bool isStripped_, ObjectELF *obj_, void (*err_func)(const char *),
                               Symtab *st) :
    oldElfHandle{oldElfHandle_},
    oldElf{oldElfHandle->e_elfp()},
    obj(st),
    isStripped(isStripped_),
    object(obj_),
    err_func_(err_func),
    isStaticBinary{obj_->isStaticBinary()}
{
    assert(obj && object && object == dynamic_cast<ObjectELF*>(obj->getObject()));
}

static int elfSymType(Symbol *sym)
{
  switch (sym->getType()) {
     case Symbol::ST_MODULE: return STT_FILE;
     case Symbol::ST_SECTION: return STT_SECTION;
     case Symbol::ST_OBJECT: return STT_OBJECT;
     case Symbol::ST_FUNCTION: return STT_FUNC;
     case Symbol::ST_TLS: return STT_TLS;
     case Symbol::ST_NOTYPE : return STT_NOTYPE;
     case Symbol::ST_UNKNOWN: return sym->getInternalType();
     case Symbol::ST_CODE: return STT_FUNC;     // in ELF, ST_CODE maps to STT_FUNC
     case Symbol::ST_INDIRECT: return STT_GNU_IFUNC;
     default: return STT_SECTION;
  }
}

static int elfSymBind(Symbol::SymbolLinkage sLinkage)
{
  switch (sLinkage) {
  case Symbol::SL_LOCAL: return STB_LOCAL;
  case Symbol::SL_WEAK: return STB_WEAK;
  case Symbol::SL_GLOBAL: return STB_GLOBAL;
  case Symbol::SL_UNIQUE: return STB_GNU_UNIQUE;
  default: return STB_LOPROC;
  }
}

static int elfSymVisibility(Symbol::SymbolVisibility sVisibility)
{
  switch (sVisibility) {
  case Symbol::SV_DEFAULT: return STV_DEFAULT;
  case Symbol::SV_INTERNAL: return STV_INTERNAL;
  case Symbol::SV_HIDDEN: return STV_HIDDEN;
  case Symbol::SV_PROTECTED: return STV_PROTECTED;
  default: return STV_DEFAULT;
  }
}

std::string phdrTypeStr(Elf64_Word phdr_type) {
    switch (phdr_type) {
        case PT_NULL:
            return "NULL";
        case PT_LOAD:
            return "LOAD";
        case PT_DYNAMIC:
            return "DYNAMIC";
        case PT_INTERP:
            return "INTERP";
        case PT_NOTE:
            return "NOTE";
        case PT_SHLIB:
            return "SHLIB";
        case PT_PHDR:
            return "PHDR";
        case PT_TLS:
            return "TLS";
        case PT_GNU_EH_FRAME:
            return "EH_FRAME";
        case PT_GNU_STACK:
            return "STACK";
        case PT_GNU_RELRO:
            return "RELRO";
        case PT_GNU_PROPERTY:
            return "PROPERTY";
        case PT_PAX_FLAGS:
            return "PAX";
        default:
            return "<UNKNOWN>";
            break;
    }
}

template<class ElfTypes>
bool emitElf<ElfTypes>::createElfSymbol(Symbol *symbol, unsigned strIndex, vector<Elf_Sym *> &symbols,
                                          bool dynSymFlag) {
    Elf_Sym *sym = new Elf_Sym();
    sym->st_name = strIndex;

    Elf_Off addr_adjust{};
    if(elfSymType(symbol) == STT_TLS) {
        addr_adjust = 0; // offset relative to TLS section, any new entries go at end
    } else {
        addr_adjust = address_adjust;
    }
    // OPD-based systems
    if (symbol->getPtrOffset()) {
        sym->st_value = symbol->getPtrOffset() + addr_adjust;
    } else if (symbol->getOffset()) {
        sym->st_value = symbol->getOffset() + addr_adjust;
    }

    sym->st_size = symbol->getSize();
    sym->st_other = ELF64_ST_VISIBILITY(elfSymVisibility(symbol->getVisibility()));
    sym->st_info = (unsigned char) ELF64_ST_INFO(elfSymBind(symbol->getLinkage()), elfSymType(symbol));

    if (symbol->getRegion()) {
        sym->st_shndx = (Elf_Section) symbol->getRegion()->getRegionNumber();
    } else if (symbol->isAbsolute()) {
        sym->st_shndx = SHN_ABS;
    } else {
        sym->st_shndx = 0;
    }

    symbols.push_back(sym);

    if (dynSymFlag) {
        string fileName;

        if (!symbol->getVersionFileName(fileName)) {
            //verdef entry
            vector<string> *vers;
            if (!symbol->getVersions(vers)) {
                if (symbol->getLinkage() == Symbol::SL_GLOBAL) {
                    versionSymTable.push_back(1);

                } else {
                    versionSymTable.push_back(0);
                    rewrite_printf( "  local\n");
                }
            } else {
                if (vers->size() > 0) {
                    // new verdef entry
                    rewrite_printf( "verdef: symbol=%s  version=%s ", symbol->getMangledName().c_str(),
                                    (*vers)[0].c_str());
                    if (verdefEntries.find((*vers)[0]) != verdefEntries.end()) {
                        unsigned short index = verdefEntries[(*vers)[0]];
                        if (symbol->getVersionHidden()) index += 0x8000;
                        versionSymTable.push_back(index);
                    } else {
                        unsigned short index = curVersionNum;
                        if (symbol->getVersionHidden()) index += 0x8000;
                        versionSymTable.push_back(index);

                        verdefEntries[(*vers)[0]] = curVersionNum;
                        curVersionNum++;
                    }
                }
                // add all versions to the verdef entry
                for (unsigned i = 0; i < vers->size(); i++) {
                    rewrite_printf( "  {%s}", (*vers)[i].c_str());
                    if (versionNames.find((*vers)[i]) == versionNames.end()) {
                        versionNames[(*vers)[i]] = 0;
                    }

                    if (find(verdauxEntries[verdefEntries[(*vers)[0]]].begin(),
                             verdauxEntries[verdefEntries[(*vers)[0]]].end(),
                             (*vers)[i]) == verdauxEntries[verdefEntries[(*vers)[0]]].end()) {
                        verdauxEntries[verdefEntries[(*vers)[0]]].push_back((*vers)[i]);
                    }
                }
                rewrite_printf( "\n");
            }
        } else {
            //verneed entry
            rewrite_printf( "need: symbol=%s    filename=%s\n",
                            symbol->getMangledName().c_str(), fileName.c_str());

            vector<string> *vers;

            if (!symbol->getVersions(vers) || (vers && vers->size() != 1)) {
                // add an unversioned dependency
                if (fileName != "") {
                    // If the file is not an executable, then add to unversioned entries
                    if (!symbol->getReferringSymbol()->getSymtab()->isExec()) {
                        if (find(unversionedNeededEntries.begin(),
                                 unversionedNeededEntries.end(),
                                 fileName) == unversionedNeededEntries.end()) {
                            rewrite_printf( "  new unversioned: %s\n", fileName.c_str());
                            unversionedNeededEntries.push_back(fileName);
                        }
                    }

                    if (symbol->getLinkage() == Symbol::SL_GLOBAL) {
                        rewrite_printf( "  global (w/ filename)\n");
                        versionSymTable.push_back(1);
                    } else {
                        versionSymTable.push_back(0);
                    }
                }
            } else {
                if (vers) {
                    // There should only be one version string by this time
                    //If the version name already exists then add the same version number to the version symbol table
                    //Else give a new number and add it to the mapping.
                    if (versionNames.find((*vers)[0]) == versionNames.end()) {
                        rewrite_printf( "  new version name: %s\n", (*vers)[0].c_str());
                        versionNames[(*vers)[0]] = 0;
                    }

                    if (verneedEntries.find(fileName) != verneedEntries.end()) {
                        if (verneedEntries[fileName].find((*vers)[0]) != verneedEntries[fileName].end()) {
                            rewrite_printf( "  vernum: %u\n", verneedEntries[fileName][(*vers)[0]]);
                            versionSymTable.push_back((unsigned short) verneedEntries[fileName][(*vers)[0]]);
                        } else {
                            rewrite_printf( "  new entry #%d: %s [%s]\n",
                                            curVersionNum, (*vers)[0].c_str(), fileName.c_str());
                            versionSymTable.push_back((unsigned short) curVersionNum);
                            verneedEntries[fileName][(*vers)[0]] = curVersionNum;
                            curVersionNum++;
                        }
                    } else {
                        rewrite_printf( "  new entry #%d: %s [%s]\n",
                                        curVersionNum, (*vers)[0].c_str(), fileName.c_str());
                        versionSymTable.push_back((unsigned short) curVersionNum);
                        verneedEntries[fileName][(*vers)[0]] = curVersionNum;
                        curVersionNum++;
                    }
                }
            }
        }
    }

    return true;
}


// Get section name table from .shstrtab section, find the last loaded section,
// maximum section/segment alignment and if TLS is used, compute address_adjust
// and offset_adjust
template<class ElfTypes>
bool emitElf<ElfTypes>::getSectionAndSegmentInfo() {
    oldNumSections = oldElfHandle->e_shnum();
    oldShstrndx = oldElfHandle->e_shstrndx();
    oldNumSegments = oldElfHandle->e_phnum();

    if (oldNumSegments + 2 >= PN_XNUM)  {
        log_elferror(err_func_, "Too many segments, libelf fails if > PN_XNUM");
        return false;
    }

    sectionNameTable = pdelf_get_shnames(oldElfHandle);
    if (sectionNameTable == NULL) {
        log_elferror(err_func_, ".shstrtab section not found");
        return false;
    }

    Elf_Phdr *phdrs = ElfTypes::elf_getphdr(oldElf);
    if (oldNumSegments && !phdrs)  {
        log_elferror(err_func_, "program header table missing or corrupted");
        return false;
    }

    // Find the maximum of the loaded segment end addresses, the max segment alignment
    // LOAD segment containing offset 0, and if TLS exists
    Elf_Off maxSegmentEndAddr{};
    bool foundLoadSegment{};
    offset0LoadSegmentIndex = oldNumSegments;  // not found value
    for (unsigned i = 0; i < oldNumSegments; ++i) {
        auto phdr{&phdrs[i]};
        if (phdr->p_type == PT_LOAD) {
            foundLoadSegment = true;
            auto segmentEndAddr{phdr->p_vaddr + phdr->p_memsz};
            if (maxSegmentEndAddr < segmentEndAddr)
                maxSegmentEndAddr = segmentEndAddr;
            if (maxSegmentAlignment < phdr->p_align)
                maxSegmentAlignment = phdr->p_align;
            if (!phdr->p_offset && !foundOffset0LoadSegment())
                offset0LoadSegmentIndex = i;
        } else if (PT_TLS == phdr->p_type) {
            TLSExists = true;
        }
    }

    if (!foundLoadSegment)  {
        log_elferror(err_func_, "no loadable segments found");
        return false;
    }

    // Find the section index containing the max section end address, that is
    // contained in the max loaded segment.  Ignore the shstrndx section since
    // this section is always moved to the end.
    Elf_Off maxSectionEndAddr{};
    for (unsigned i = 0; i < oldNumSections; ++i)  {
        if (i == oldShstrndx)
            continue;                   // .shstrtab shstrndx section always moved, ignore
        auto scn{elf_getscn(oldElf, i)};
        auto shdr{ElfTypes::elf_getshdr(scn)};
        if (!shdr || !(shdr->sh_flags & SHF_ALLOC))
            continue;                   // not allocated, so no address
        if (maxSectionAlignment < shdr->sh_addralign)
            maxSectionAlignment = shdr->sh_addralign;
        auto endAddr{shdr->sh_addr + shdr->sh_size};
        if ((shdr->sh_flags & SHF_TLS) && shdr->sh_type == SHT_NOBITS)
            continue;                   // .tbss does not occupy address space
        if (maxSectionEndAddr <= endAddr && endAddr <= maxSegmentEndAddr)  {
            maxSectionEndAddr = endAddr;
            lastLoadedSectionIndex = i;
        }
    }

    if (!lastLoadedSectionIndex)  {
        log_elferror(err_func_, "no loadable sections found to insert after");
        return false;
    }

    /* Room has to be made at the front of the file for the new program header
     * table, which shifts every file offset of the old image by offset_adjust
     * and, for a PIE/shared object, every address by address_adjust.  There
     * are two requirements that must be maintained:
     *
     *  - a LOAD segment requires p_vaddr == p_offset (mod p_align), so
     *    address_adjust == offset_adjust (mod p_align) for every segment
     *  - data alignment requirements of basic types or from alignas implies
     *    address_adjust must be a multiple of the section alignment holding it
     *
     * For a PIE/shared object the whole image moves up, so using one value for
     * both shifts satisfies the first requirement for any p_align, and it only
     * has to be a multiple of the largest section alignment.
     *
     * An executable loaded at a fixed address keeps its addresses instead: the
     * segment at file offset 0 grows downwards to cover the new space, leaving
     * every address unchanged.  Nothing moves, so section alignment is not a
     * concern, but the file shift alone has to keep every segment congruent,
     * which makes it a multiple of the largest segment alignment.
     */
    auto pageSize = static_cast<Elf_Off>(getpagesize());
    if (oldEhdr->e_type == ET_DYN)  {
        address_adjust = std::max(pageSize, maxSectionAlignment); // address can be adjusted for ET_DYN
        offset_adjust = address_adjust;
    }  else  {
        offset_adjust = std::max(pageSize, maxSegmentAlignment);  // addresses fixed so don't move addrs
    }

    if (offset_adjust % pageSize)  {
        log_elferror(err_func_, "segment or section alignment is not a multiple of the pagesize");
        return false;
    }
    if (!foundOffset0LoadSegment())  {
        log_elferror(err_func_, "no loaded segment at file offset 0 to cover the program table");
        return false;
    }
    if (address_adjust == 0 && phdrs[offset0LoadSegmentIndex].p_vaddr <= offset_adjust)  {
        log_elferror(err_func_, "no room below the first loaded segment for the program headers");
        return false;
    }

    return true;
}

// Renames 1st oldName section by changing 2nd char to 'o'
template<class ElfTypes>
void emitElf<ElfTypes>::renameSection(const std::string &oldName) {
    assert(oldName.length() >= 2 && oldName[1] != 'o');
    for (auto &secName : secNames)  {
        if (secName == oldName)  {
            secName[1] = 'o';
            break;
        }
    }
}

class emitElfResourceMgr
{
    private:
        int  fd{-1};
        std::string fn;
        Elf* elf{};
    public:
        emitElfResourceMgr(int f, std::string s) : fd(f), fn(s) {}
        ~emitElfResourceMgr() { closeElf(); closeFd(); unlink(); }
        emitElfResourceMgr(const emitElfResourceMgr&) = delete;
        emitElfResourceMgr(const emitElfResourceMgr&&) = delete;
        emitElfResourceMgr& operator=(const emitElfResourceMgr&) = delete;
        emitElfResourceMgr& operator=(const emitElfResourceMgr&&) = delete;
        void setElf(Elf *e) { elf = e; }
        void noUnlink() { fn = ""; }
        void closeFd()  { if (fd == -1) return;   close(fd);            fd = -1; }
        void unlink()   { if (fn.empty()) return; ::unlink(fn.c_str()); noUnlink(); }
        void closeElf() { if (!elf) return;       elf_end(elf);         elf = nullptr; }
};

template<class ElfTypes>
bool emitElf<ElfTypes>::driver(std::string fName, std::set<Symbol *> &allSymbols) {
    rewrite_printf("::driver for emitElf\n");

    oldEhdr = ElfTypes::elf_getehdr(oldElf);

    if (!getSectionAndSegmentInfo())
        return false;

    if (!createSymbolTables(allSymbols))  {
        return false;  // createSymbolTables failed
    }

    Region *foundSec = NULL;
    Region *shstrtabRegion = NULL;

    string newFName = fName + "XXXXXX";
    auto buf = std::unique_ptr<char[]>(new char[newFName.length() + 1]);
    strncpy(buf.get(), newFName.c_str(), newFName.length() + 1);

    auto newfd = mkstemp(buf.get());
    newFName = buf.get();

    if (newfd == -1) {
        log_elferror(err_func_, "error opening file to write symbols");
        return false;
    }
    emitElfResourceMgr emitElfResources(newfd, newFName);

    struct stat statBuf;
    decltype(statBuf.st_mode) origFdMode{};
    if (fstat(object->getFD(), &statBuf) == 0) {
        origFdMode = statBuf.st_mode & 0777;  // clear sticky, setXid bits
    } else {
        origFdMode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP;
    }
    fchmod(newfd, S_IRUSR | S_IWUSR);  // just user while updating
    rewrite_printf("Emitting to temporary file %s\n", buf.get());

    if ((newElf = elf_begin(newfd, ELF_C_WRITE, NULL)) == NULL) {
        log_elferror(err_func_, "NEWELF_BEGIN_FAIL");
        fflush(stdout);
        cerr << "Failed to elf_begin" << endl;
        return false;
    }
    emitElfResources.setElf(newElf);

    addSectionName("");  // section 0 is always ST_NULL with an empty name
    loadSecTotalSize = 0;
    int dirtySecsChange = 0;
    unsigned extraAlignSize = 0;

    // Write the Elf header first!
    newEhdr = ElfTypes::elf_newehdr(newElf);
    if (!newEhdr) {
        log_elferror(err_func_, "newEhdr failed\n");
        return false;
    }
    *newEhdr = *oldEhdr;

    unsigned insertPoint = oldNumSections + 1;  // section index of inserted sections
    Elf_Off insertPointOffset{};                // file offset of inserted sections

    newEhdr->e_phoff = sizeof(Elf_Ehdr);
    newEhdr->e_entry += address_adjust;

    /* flag the file for no auto-layout */
    elf_flagelf(newElf, ELF_C_SET, ELF_F_LAYOUT);

    Elf_Scn *scn = NULL, *newscn = NULL;
    Elf_Data *newdata = NULL, *olddata = NULL;
    Elf_Shdr *newshdr = NULL, *shdr = NULL;
    std::unordered_map<unsigned, unsigned> secLinkMapping;
    std::unordered_map<unsigned, unsigned> secInfoMapping;
    std::unordered_map<unsigned, bool> changeMapping;
    std::unordered_map<string, unsigned> newNameIndexMapping;

    std::unordered_set<string> updateLinkInfoSecs = {
        ".dynsym", /*".dynstr",*/ ".rela.dyn", ".rela.plt", ".dynamic", ".symtab"};
    std::unordered_map<string, pair<unsigned, unsigned>> dataLinkInfo;

    bool createdLoadableSections = false;
    unsigned sectionNumber = 0;

    auto moveSecAddrRange = object->getMoveSecAddrRange();

    for (unsigned scncount = 1; (scn = elf_nextscn(oldElf, scn)); scncount++) {
        //copy sections from oldElf to newElf
        shdr = ElfTypes::elf_getshdr(scn);

        // resolve section name
        const char *name = getSectionName(shdr);
        bool result = obj->findRegion(foundSec, shdr->sh_addr, shdr->sh_size);
        if (!result || foundSec->isDirty()) {
            result = obj->findRegion(foundSec, name);
        }

        // write the shstrtabsection at the end
        if (scncount == oldShstrndx) {
            shstrtabRegion = foundSec;
            continue;
        }

        sectionNumber++;
        changeMapping[sectionNumber] = false;
        newNameIndexMapping[name] = sectionNumber;
        if (foundSec)
            regionNewIndex[foundSec] = sectionNumber;

        newscn = elf_newscn(newElf);
        newshdr = ElfTypes::elf_getshdr(newscn);
        newdata = elf_newdata(newscn);
        olddata = elf_getdata(scn, NULL);
        *newshdr = *shdr;
        *newdata = *olddata;

        newshdr->sh_name = addSectionName(name);

        if (newshdr->sh_addr) {
            newshdr->sh_addr += address_adjust;

#if defined(DYNINST_CODEGEN_ARCH_AARCH64)
            if (strcmp(name, ".plt")==0)
                updateDynamic(DT_TLSDESC_PLT, address_adjust);
            if (strcmp(name, ".got")==0)
                updateDynamic(DT_TLSDESC_GOT, address_adjust);
#endif
        }

        if (foundSec->isDirty()) {
            newdata->d_buf = allocate_buffer(foundSec->getDiskSize());
            memcpy(newdata->d_buf, foundSec->getPtrToRawData(), foundSec->getDiskSize());
            newdata->d_size = foundSec->getDiskSize();
            newshdr->sh_size = foundSec->getDiskSize();
        } else if (olddata->d_buf) {    //copy the data buffer from oldElf
            newdata->d_buf = allocate_buffer(olddata->d_size);
            memcpy(newdata->d_buf, olddata->d_buf, olddata->d_size);
        }

        if (newshdr->sh_entsize && (newshdr->sh_size % newshdr->sh_entsize != 0)) {
            newshdr->sh_entsize = 0x0;
        }

        if (address_adjust != 0 && newdata->d_buf && newdata->d_size) {
            for (auto relr_addr : object->getRelrDynRelocs()) {
                if (relr_addr < shdr->sh_addr ||
                    relr_addr + sizeof(Elf_Addr) > shdr->sh_addr + shdr->sh_size)
                    continue;

                Offset relr_off = relr_addr - shdr->sh_addr;
                if (relr_off + sizeof(Elf_Addr) > newdata->d_size)
                    continue;

                char *loc = static_cast<char *>(newdata->d_buf) + relr_off;
                auto val = Dyninst::read_memory_as<Elf_Addr>(loc);
                if (!val) continue;
                val += address_adjust;
                Dyninst::write_memory_as(loc, val);
            }
        }

        for (const auto &m : moveSecAddrRange) {
            if ((m[0] == shdr->sh_addr) ||
                (m[0] <= shdr->sh_addr && shdr->sh_addr < m[1])) {
                newshdr->sh_type = SHT_PROGBITS;
                changeMapping[sectionNumber] = true;
                renameSection(name);
            }
        }

        if ((object->getStrtabAddr() != 0 &&
             object->getStrtabAddr() == shdr->sh_addr) ||
            !strcmp(name, STRTAB_NAME)) {
            symStrData = newdata;
            symtabStrIndex = sectionNumber;
            updateSymbols(symTabData, symStrData, loadSecTotalSize);
        }

        // .symtab's sh_link is set to .strtab's index in the link fixup pass below
        if ((object->getSymtabAddr() != 0 &&
             object->getSymtabAddr() == shdr->sh_addr) ||
            !strcmp(name, SYMTAB_NAME)) {
            changeMapping[sectionNumber] = true;
            symTabData = newdata;
        }

        if (object->getTextAddr() != 0 &&
            object->getTextAddr() == shdr->sh_addr) {
            textData = newdata;
        }

        if (object->getDynamicAddr() != 0 &&
            object->getDynamicAddr() == shdr->sh_addr) {
            dynData = newdata;
            dynSegOff = newshdr->sh_offset;
            dynSegAddr = newshdr->sh_addr;
            // Change the data to update the relocation addr
            newshdr->sh_type = SHT_PROGBITS;
            changeMapping[sectionNumber] = true;
            renameSection(name);
        }

        // Only need to rewrite data section
        if (hasRewrittenTLS && foundSec->isTLS()
            && foundSec->getRegionType() == Region::RT_DATA) {
            // Clear TLS flag
            newshdr->sh_flags &= ~SHF_TLS;
            renameSection(name);
        }

        if (isStaticBinary && ((strcmp(name, ".rel.plt") == 0) || (strcmp(name, ".rela.plt") == 0 ))) {
            renameSection(name);
            // The old sections are no longer REL or RELA, change to PROGBITS
            newshdr->sh_type = SHT_PROGBITS;

        }

        if (address_adjust != 0 &&
                (strcmp(name, ".init_array") == 0 || strcmp(name, ".fini_array") == 0 ||
                 strcmp(name, "__libc_subfreeres") == 0 || strcmp(name, "__libc_atexit") == 0 ||
                 strcmp(name, "__libc_thread_subfreeres") == 0 || strcmp(name, "__libc_IO_vtables") == 0)) {
            std::vector<Offset> &relr_relocs = object->getRelrDynRelocs();
            // Every pointer-sized word in this section is shifted by address_adjust.
            // A word that is also a RELR relocation target was already shifted by the
            // earlier RELR loop, so skip to avoid offsetting it twice
            std::unordered_set<Offset> relr_addrs(relr_relocs.begin(), relr_relocs.end());
            for(std::size_t off = 0; off < newdata->d_size; off += sizeof(Elf_Addr)) {
                Offset addr = shdr->sh_addr + off;
                if (relr_addrs.count(addr)) continue;  // already adjusted as a RELR reloc

                char *loc = static_cast<char*>(newdata->d_buf) + off;
                Elf_Addr val{};
                read_memory_as(val, loc);
                if(val == 0) continue;
                val += address_adjust;
                write_memory_as(loc, val);
            }
        }
        // Change offsets of sections based on the newly added sections
        /* This special case is specific to FreeBSD but there is no harm in
         * handling it on other platforms.
         *
         * This is necessary because the INTERP header must be located within in
         * the first page of the file -- if the section is moved to the next
         * page the object file will not be parsed correctly by the kernel.
         *
         * However, the .interp section still needs to be shifted, but just
         * by the difference in size of the new PHDR segment.
         */
        if (newshdr->sh_offset > 0) {
            if (newshdr->sh_offset < offset_adjust && !strcmp(name, INTERP_NAME)) {
                newshdr->sh_addr -= offset_adjust;
                newshdr->sh_addr += oldEhdr->e_phentsize;
                newshdr->sh_offset += oldEhdr->e_phentsize;
            } else
                newshdr->sh_offset += offset_adjust;
        }

        // shift section offsets after the insertPoint that come after the insertPoint's offset
        if (scncount > insertPoint && shdr->sh_offset >= insertPointOffset)
            newshdr->sh_offset += loadSecTotalSize + extraAlignSize;

        if (newshdr->sh_offset > 0)
            newshdr->sh_offset += dirtySecsChange;

        if (foundSec->isDirty())
            dirtySecsChange += newshdr->sh_size - shdr->sh_size;
        
        secLinkMapping[sectionNumber] = shdr->sh_link;
        secInfoMapping[sectionNumber] = shdr->sh_info;

        if(updateLinkInfoSecs.find(name) != updateLinkInfoSecs.end())
            dataLinkInfo[name] = std::make_pair(shdr->sh_link, shdr->sh_info);

        rewrite_printf("section %s addr = %lx off = %lx size = %lx\n",
                       name, (long unsigned int)newshdr->sh_addr, (long unsigned int)newshdr->sh_offset, (long unsigned int)newshdr->sh_size);
        rewrite_printf(" %02u Link(%u) Info(%u) change(%d)\n",
                sectionNumber, secLinkMapping[sectionNumber], secInfoMapping[sectionNumber],
                changeMapping[sectionNumber]);

        //Insert new loadable sections after the last section of the last
        //loadable segment
        if (scncount == lastLoadedSectionIndex && !createdLoadableSections) {
            createdLoadableSections = true;
            insertPoint = scncount;
            insertPointOffset = shdr->sh_offset;
            if (shdr->sh_type != SHT_NOBITS)
                insertPointOffset += shdr->sh_size;


            if (!createLoadableSections(newshdr, extraAlignSize, newNameIndexMapping,
                       sectionNumber))
                return false;

            // Update the heap symbols, now that loadSecTotalSize is set
            updateSymbols(dynsymData, dynStrData, loadSecTotalSize);

        }

        if (0 > elf_update(newElf, ELF_C_NULL)) {
            return false;
        }
    } // end of for each elf section

    // Add non-loadable sections at the end of object file
    if (!createNonLoadableSections(newshdr))
        return false;

    if (0 > elf_update(newElf, ELF_C_NULL)) {
        return false;
    }

    //Add the section header table right at the end
    addSectionHeaderTable(newshdr);
    if (shstrtabRegion)
        regionNewIndex[shstrtabRegion] = secNames.size() - 1;   // .shstrtab is the last section

    // Second iteration to fix the link fields to point to the correct section
    scn = NULL;
    unsigned scncount;
    for (scncount = 1; (scn = elf_nextscn(newElf, scn)); scncount++) {
        shdr = ElfTypes::elf_getshdr(scn);
        if (shdr->sh_type == SHT_SYMTAB) {
            shdr->sh_link = symtabStrIndex;     // .symtab -> .strtab, wherever it ended up
            if (updateSymbolSectionIndices(scn, symtabSymRegions))
                shdr->sh_info = symtabNumLocals;    // index of first non-local symbol
        } else if (shdr->sh_type == SHT_DYNSYM) {
            updateSymbolSectionIndices(scn, dynsymSymRegions);
        }
        if(dataLinkInfo.count(secNames[scncount]))
        {
            rewrite_printf("update link info of %s\n", secNames[scncount].c_str());
            auto & data = dataLinkInfo[secNames[scncount]];
            //shdr->sh_link = data.first;
            shdr->sh_info = data.second;
        }
    }

    // libelf does not handle the extended for e_shstrndx, so manually do it here
    unsigned newShstrndx = scncount - 1;
    if (newShstrndx >= SHN_LORESERVE) {
        newEhdr->e_shstrndx = SHN_XINDEX;
        if (auto scn0 = elf_getscn(newElf, 0))  {
            if (auto shdr0 = ElfTypes::elf_getshdr(scn0))  {
                shdr0->sh_link = newShstrndx;
            }  else  {
                log_elferror(err_func_, "elf_getshdr(scn0) failed");
                return false;
            }
        }  else  {
            log_elferror(err_func_, "elf_getscn(newElf, 0) failed");
            return false;
        }
    } else {
        newEhdr->e_shstrndx = newShstrndx;
    }

    // Move the section header to the end
    newEhdr->e_shoff = shdr->sh_offset + shdr->sh_size;
    if (newEhdr->e_shoff % 8)
        newEhdr->e_shoff += 8 - (
                newEhdr->e_shoff % 8);

    //copy program headers
    oldPhdr = ElfTypes::elf_getphdr(
            oldElf);
    fixPhdrs();

    //Write the new Elf file
    if (elf_update(newElf, ELF_C_WRITE) < 0) {
        log_elferror(err_func_, "elf_update failed");
        return false;
    }
    emitElfResources.closeElf();
    fchmod(newfd, origFdMode);  // set permission to original file

    if (rename(newFName.c_str(), fName.c_str())) {
        auto msg{"rename of new elf file (" + newFName + " -> " +  fName + ") failed"};
        log_elferror(err_func_, msg.c_str());
        return false;
    }
    emitElfResources.noUnlink();

    return true;
}

template<class ElfTypes>
void emitElf<ElfTypes>::fixPhdrs() {
    // This function has to perform the addresses fix in two passes.
    // First we must update the old headers addresses, and than
    // we should look where to insert the new LOAD segment

    /*
     * If we've created a new loadable segment, we need to insert a new
     * program header amidst the other loadable segments.
     *
     * ELF format says:
     *
     * `Loadable segment entries in the program header table appear in
     * ascending order, sorted on the p_vaddr member.'
     */
    rewrite_printf("::fixPhdrs():\n");

    newEhdr->e_phnum = oldNumSegments;
    newEhdr->e_phentsize = oldEhdr->e_phentsize;

    newEhdr->e_phnum++;
    if (hasRewrittenTLS && !TLSExists) newEhdr->e_phnum++;

    // Copy old segments to vector and update contents
    Elf_Phdr *old = oldPhdr;
    vector<Elf_Phdr> segments;

    for (unsigned i = 0; i < oldNumSegments; i++)
    {
        segments.push_back(*old);

        if (old->p_type == PT_DYNAMIC) {
            segments[i].p_vaddr = dynSegAddr;
            segments[i].p_paddr = dynSegAddr;
            segments[i].p_offset = dynSegOff;
            segments[i].p_memsz = dynSegSize;
            segments[i].p_filesz = segments[i].p_memsz;
        } else if (old->p_type == PT_PHDR) {
            segments[i].p_vaddr = old->p_vaddr - offset_adjust + address_adjust;
            segments[i].p_offset = newEhdr->e_phoff;
            segments[i].p_paddr = segments[i].p_vaddr;
            segments[i].p_filesz = sizeof(Elf_Phdr) * newEhdr->e_phnum;
            segments[i].p_memsz = segments[i].p_filesz;
        } else if (hasRewrittenTLS && old->p_type == PT_TLS) {
            segments[i].p_offset = newTLSData->sh_offset;
            segments[i].p_vaddr = newTLSData->sh_addr;
            segments[i].p_paddr = newTLSData->sh_addr;
            segments[i].p_filesz = newTLSData->sh_size;
            segments[i].p_memsz = newTLSData->sh_size + old->p_memsz - old->p_filesz;
            segments[i].p_align = newTLSData->sh_addralign;
        } else if (old->p_type == PT_LOAD) {
            if (i == offset0LoadSegmentIndex) {
                if (segments[i].p_vaddr)
                    segments[i].p_vaddr = old->p_vaddr - offset_adjust;

                segments[i].p_paddr = segments[i].p_vaddr;
                segments[i].p_filesz += offset_adjust;
                segments[i].p_memsz = segments[i].p_filesz;
            } else {
                segments[i].p_offset += offset_adjust;
            }
            if (segments[i].p_vaddr) {
                segments[i].p_vaddr += address_adjust;
                segments[i].p_paddr += address_adjust;
            }
        } else if (old->p_type == PT_INTERP && old->p_offset) {
            Elf_Off addr_shift = address_adjust;
            Elf_Off offset_shift = offset_adjust;
            if (old->p_offset < offset_adjust) {
                offset_shift = oldEhdr->e_phentsize;
                addr_shift -= offset_adjust - offset_shift;
            }
            segments[i].p_offset += offset_shift;
            segments[i].p_vaddr += addr_shift;
            segments[i].p_paddr += addr_shift;
        } else if (old->p_offset) {
            segments[i].p_offset += offset_adjust;
            if (segments[i].p_vaddr) {
                segments[i].p_vaddr += address_adjust;
                segments[i].p_paddr += address_adjust;
            }
        }

        ++old;
    }

    if (firstNewLoadSec)
    {
        // Create New Segment
        Elf_Phdr newSeg;
        newSeg.p_type = PT_LOAD;
        newSeg.p_offset = firstNewLoadSec->sh_offset;
        newSeg.p_vaddr = newSegmentStart;
        newSeg.p_paddr = newSeg.p_vaddr;
        newSeg.p_filesz = loadSecTotalSize - (newSegmentStart - firstNewLoadSec->sh_addr);
        newSeg.p_memsz = (currEndAddress - firstNewLoadSec->sh_addr) -
            (newSegmentStart - firstNewLoadSec->sh_addr);
        newSeg.p_flags = PF_R + PF_W + PF_X;
        newSeg.p_align = getpagesize();

        // Insert the new segment(s) before the first segment with a vaddr greater than this one
        unsigned int insertAt = segments.size();
        for (unsigned i = 0; i < segments.size(); ++i)   {
            if (segments[i].p_type == PT_LOAD && segments[i].p_vaddr > newSegmentStart)  {
                insertAt = i;
                break;
            }
        }

        if( insertAt == segments.size() )
            segments.push_back( newSeg );
        else
            segments.insert( segments.begin() + insertAt, newSeg );
    }

    // Create newPhdr and copy segments to it
    newPhdr = ElfTypes::elf_newphdr(newElf, newEhdr->e_phnum);
    void *phdr_data = (void *) newPhdr;

    for (const auto &segment : segments)
    {
        *newPhdr = segment;
        rewrite_printf("Updated program header: type %u (%s), offset 0x%lx, addr 0x%lx\n",
                newPhdr->p_type, phdrTypeStr(newPhdr->p_type).c_str(), (long unsigned int)newPhdr->p_offset, (long unsigned int)newPhdr->p_vaddr);
        ++newPhdr;
    }

    if (hasRewrittenTLS && !TLSExists) {
        newPhdr->p_type = PT_TLS;
        newPhdr->p_offset = newTLSData->sh_offset;
        newPhdr->p_vaddr = newTLSData->sh_addr;
        newPhdr->p_filesz = newTLSData->sh_size;
        newPhdr->p_memsz = newTLSData->sh_size;
        newPhdr->p_align = newTLSData->sh_addralign;
    }

    if (!phdrs_scn)
        return;

    //We made a new section to contain the program headers--keeps
    // libelf from overwriting the program headers data when outputting
    // sections.  Fill in the new section's data with what we just wrote.
    Elf_Data *data = elf_newdata(phdrs_scn);
    size_t total_size = (size_t) newEhdr->e_phnum * (size_t) newEhdr->e_phentsize;
    data->d_buf = allocate_buffer(total_size);
    memcpy(data->d_buf, phdr_data, total_size);
    data->d_size = total_size;
    data->d_align = 0;
    data->d_off = 0;
    data->d_type = ELF_T_BYTE;
    data->d_version = 1;
}

#if !defined(DT_GNU_HASH)
#define DT_GNU_HASH 0x6ffffef5
#endif
#if !defined(DT_GNU_CONFLICT)
#define DT_GNU_CONFLICT 0x6ffffef8
#endif
#if !defined(DT_TLSDESC_PLT)
#define DT_TLSDESC_PLT 0x6ffffef6
#endif
#if !defined(DT_TLSDESC_GOT)
#define DT_TLSDESC_GOT 0x6ffffef7
#endif
// Older elf.h headers may not define RELR dynamic tag constants
#if !defined(DT_RELRSZ)
#define DT_RELRSZ 35
#endif
#if !defined(DT_RELR)
#define DT_RELR 36
#endif
#if !defined(DT_RELRENT)
#define DT_RELRENT 37
#endif
#if !defined(SHT_RELR)
#define SHT_RELR 19
#endif

//This method updates the .dynamic section to reflect the changes to the relocation section
template<class ElfTypes>
void emitElf<ElfTypes>::updateDynamic(unsigned tag, Elf_Addr val) {
    if (isStaticBinary) return;
    // This is for REL/RELA if it doesn't already exist in the original binary;
    if(dynamicSecData.find(tag) != dynamicSecData.end())
        dynamicSecData[tag][0]->d_tag = tag;
    else
        return;
    switch (dynamicSecData[tag][0]->d_tag) {
        case DT_STRSZ:
        case DT_RELSZ:
        case DT_RELASZ:
        case DT_RELRSZ:
        case DT_PLTRELSZ:
        case DT_RELACOUNT:
        case DT_RELENT:
        case DT_RELAENT:
        case DT_RELRENT:
            dynamicSecData[tag][0]->d_un.d_val = val;
            break;
        case DT_HASH:
        case DT_GNU_HASH:
        case DT_SYMTAB:
        case DT_STRTAB:
        case DT_REL:
        case DT_RELA:
        case DT_RELR:
        case DT_VERSYM:
        case DT_JMPREL:
            dynamicSecData[tag][0]->d_un.d_ptr = val;
            break;
        case DT_VERNEED:
            dynamicSecData[tag][0]->d_un.d_ptr = val;
            dynamicSecData[DT_VERNEEDNUM][0]->d_un.d_val = verneednum;
            break;
        case DT_VERDEF:
            dynamicSecData[tag][0]->d_un.d_ptr = val;
            dynamicSecData[DT_VERDEFNUM][0]->d_un.d_val = verdefnum;
            break;
        case DT_TLSDESC_PLT:
        case DT_TLSDESC_GOT:
            dynamicSecData[tag][0]->d_un.d_val += val;
            break;
    }
}

/* This method sets _end and _END_ to the starting position of the heap in the
 * new binary. 
 */
template<class ElfTypes>
void emitElf<ElfTypes>::updateSymbols(Elf_Data *symtabData, Elf_Data *strData, unsigned long loadSecsSize) {
    unsigned pgSize = (unsigned) getpagesize();
    if (symtabData && strData && loadSecsSize) {
        Elf_Sym *symPtr = (Elf_Sym *) symtabData->d_buf;
        for (unsigned int i = 0; i < symtabData->d_size / (sizeof(Elf_Sym)); i++, symPtr++) {
            if (!(strcmp("_end", (char *) strData->d_buf + symPtr->st_name))) {
                if (newSegmentStart >= symPtr->st_value) {
                    symPtr->st_value += ((newSegmentStart - symPtr->st_value) + loadSecsSize);

                    // Advance the location to the next page boundary
                    symPtr->st_value = (symPtr->st_value & ~(pgSize - 1)) + pgSize;
                }
            }
            if (!(strcmp("_END_", (char *) strData->d_buf + symPtr->st_name))) {
                if (newSegmentStart > symPtr->st_value) {
                    symPtr->st_value += (newSegmentStart - symPtr->st_value) + loadSecsSize;

                    // Advance the location to the next page boundary
                    symPtr->st_value = (symPtr->st_value & ~(pgSize - 1)) + pgSize;
                }
            }
        }
    }
}

template<class ElfTypes>
bool emitElf<ElfTypes>::createLoadableSections(Elf_Shdr *&shdr, unsigned &extraAlignSize,
                                                 std::unordered_map<std::string, unsigned> &newNameIndexMapping,
                                                 unsigned &sectionNumber) {
    rewrite_printf("createLoadableSections():\n");

    Elf_Scn *newscn;
    Elf_Data *newdata = NULL;

    Elf_Shdr *newshdr;
    std::vector<Elf_Shdr *> updateDynLinkShdr;
    std::vector<Elf_Shdr *> updateStrLinkShdr;
    firstNewLoadSec = NULL;
    unsigned pgSize = getpagesize();
    unsigned strtabIndex = 0;
    unsigned dynsymIndex = 0;
    Elf_Shdr *prevshdr = NULL;

    /*
     * Order the new sections such that those with explicit
     * memory offsets come before those without (that will be placed
     * after the non-zero sections).
     *
     * zstart is used to place the first zero-offset section if
     * no non-zero-offset sections exist.
     */
    Address zstart = emitElfUtils::orderLoadableSections(obj, newSecs);

    for (const auto newSec : newSecs) {
        if (!newSec->isLoadable()) {
            nonLoadableSecs.push_back(newSec);
            continue;
        }
        sectionNumber++;
        // Add a new loadable section
        if ((newscn = elf_newscn(newElf)) == NULL) {
            log_elferror(err_func_, "unable to create new section");
            return false;
        }
        if ((newdata = elf_newdata(newscn)) == NULL) {
            log_elferror(err_func_, "unable to create section data");
            return false;
        }
        memset(newdata, 0, sizeof(Elf_Data));

        // Fill out the new section header
        newshdr = ElfTypes::elf_getshdr(newscn);
        newshdr->sh_name = addSectionName(newSec->getRegionName());
        newshdr->sh_flags = 0;
        newshdr->sh_type = SHT_PROGBITS;
        switch (newSec->getRegionType()) {
            case Region::RT_TEXTDATA:
                newshdr->sh_flags = SHF_EXECINSTR | SHF_ALLOC | SHF_WRITE;
                break;
            case Region::RT_TEXT:
                newshdr->sh_flags = SHF_EXECINSTR | SHF_ALLOC;
                break;
            case Region::RT_BSS:
                newshdr->sh_type = SHT_NOBITS;
                //FALLTHROUGH
            case Region::RT_DATA:
                newshdr->sh_flags = SHF_WRITE | SHF_ALLOC;
                break;
            default:
                break;
        }

        auto thisSectionIndex = secNames.size() - 1;
        newNameIndexMapping[newSec->getRegionName()] = thisSectionIndex;
        regionNewIndex[newSec] = thisSectionIndex;

        if (shdr->sh_type == SHT_NOBITS) {
            newshdr->sh_offset = shdr->sh_offset;
        } else if (!firstNewLoadSec || !newSec->getDiskOffset()) {
            newshdr->sh_offset = shdr->sh_offset + shdr->sh_size;
        } else {
            // The offset can be computed by determining the difference from
            // the first new loadable section
            newshdr->sh_offset = firstNewLoadSec->sh_offset + address_adjust +
                                 (newSec->getDiskOffset() - firstNewLoadSec->sh_addr);

            // Account for inter-section spacing due to alignment constraints
            loadSecTotalSize += newshdr->sh_offset - (shdr->sh_offset + shdr->sh_size);
        }

        if (newSec->getDiskOffset())
            newshdr->sh_addr = newSec->getDiskOffset() + address_adjust;
        else if (!prevshdr)
            newshdr->sh_addr = zstart + address_adjust;
        else
            newshdr->sh_addr = prevshdr->sh_addr + prevshdr->sh_size;

        newshdr->sh_link = SHN_UNDEF;
        newshdr->sh_info = 0;
        newshdr->sh_addralign = newSec->getMemAlignment();
        newshdr->sh_entsize = 0;

        // TLS section
        if (newSec->isTLS()) {
            newTLSData = newshdr;
            newshdr->sh_flags |= SHF_TLS;
        }

        if (newSec->getRegionType() == Region::RT_REL ||
            newSec->getRegionType() == Region::RT_PLTREL)    //Relocation section
        {
            newshdr->sh_type = SHT_REL;
            newshdr->sh_flags = SHF_ALLOC;
            newshdr->sh_entsize = sizeof(Elf_Rel);
            updateDynLinkShdr.push_back(newshdr);
            newdata->d_type = ELF_T_REL;
            newdata->d_align = 4;
            if (newSec->getRegionType() == Region::RT_REL)
                updateDynamic(DT_REL, newshdr->sh_addr);
            else if (newSec->getRegionType() == Region::RT_PLTREL)
                updateDynamic(DT_JMPREL, newshdr->sh_addr);
        } else if (newSec->getRegionType() == Region::RT_RELA ||
                 newSec->getRegionType() == Region::RT_PLTRELA) //Relocation section
        {
            newshdr->sh_type = SHT_RELA;
            newshdr->sh_flags = SHF_ALLOC;
            newshdr->sh_entsize = sizeof(Elf_Rela);
            updateDynLinkShdr.push_back(newshdr);
            newdata->d_type = ELF_T_RELA;
            newdata->d_align = 4;
            if (newSec->getRegionType() == Region::RT_RELA)
                updateDynamic(DT_RELA, newshdr->sh_addr);
            else if (newSec->getRegionType() == Region::RT_PLTRELA)
                updateDynamic(DT_JMPREL, newshdr->sh_addr);
        } else if (newSec->getRegionType() == Region::RT_RELR) {
            newshdr->sh_type = SHT_RELR;
            newshdr->sh_flags = SHF_ALLOC;
            newshdr->sh_entsize = sizeof(Elf_Relr);
            // SHT_RELR entries are Elf32_Word for ELFCLASS32 and Elf64_Xword
            // for ELFCLASS64
            newdata->d_type =
                (sizeof(Elf_Relr) == sizeof(Elf32_Word)) ? ELF_T_WORD : ELF_T_XWORD;
            newdata->d_align = sizeof(Elf_Relr);
            updateDynamic(DT_RELR, newshdr->sh_addr);
            updateDynamic(DT_RELRSZ, newSec->getDiskSize());
            updateDynamic(DT_RELRENT, sizeof(Elf_Relr));
        } else if (newSec->getRegionType() == Region::RT_STRTAB) {   //String table Section
            newshdr->sh_type = SHT_STRTAB;
            newshdr->sh_entsize = 1;
            newdata->d_type = ELF_T_BYTE;
            newshdr->sh_link = SHN_UNDEF;
            newshdr->sh_flags = SHF_ALLOC;
            newdata->d_align = 1;
            dynStrData = newdata;
            strtabIndex = thisSectionIndex;
            newshdr->sh_addralign = 1;
            updateDynamic(DT_STRTAB, newshdr->sh_addr);
            updateDynamic(DT_STRSZ, newSec->getDiskSize());
        } else if (newSec->getRegionType() == Region::RT_SYMTAB) {
            newshdr->sh_type = SHT_DYNSYM;
            newshdr->sh_entsize = sizeof(Elf_Sym);
            newdata->d_type = ELF_T_SYM;
            newdata->d_align = 4;
            dynsymData = newdata;
            updateStrLinkShdr.push_back(newshdr);   // .dynsym -> .dynstr, resolved once .dynstr is created
            newshdr->sh_flags = SHF_ALLOC;
            dynsymIndex = thisSectionIndex;
            updateDynamic(DT_SYMTAB, newshdr->sh_addr);
        } else if (newSec->getRegionType() == Region::RT_DYNAMIC) {
            newDynamicRegion = newSec;
            newshdr->sh_entsize = sizeof(Elf_Dyn);
            newshdr->sh_type = SHT_DYNAMIC;
            newdata->d_type = ELF_T_DYN;
            newdata->d_align = 4;
            updateStrLinkShdr.push_back(newshdr);
            newshdr->sh_flags = SHF_ALLOC | SHF_WRITE;
            dynSegOff = newshdr->sh_offset;
            dynSegAddr = newshdr->sh_addr;
            dynSegSize = newSec->getDiskSize();
        } else if (newSec->getRegionType() == Region::RT_HASH) {
            newshdr->sh_entsize = sizeof(Elf_Word);
            newshdr->sh_type = SHT_HASH;
            newdata->d_type = ELF_T_WORD;
            newdata->d_align = 4;
            updateDynLinkShdr.push_back(newshdr);
            newshdr->sh_flags = SHF_ALLOC;
            newshdr->sh_info = 0;
            updateDynamic(DT_HASH, newshdr->sh_addr);
        } else if (newSec->getRegionType() == Region::RT_SYMVERSIONS) {
            newshdr->sh_type = SHT_GNU_versym;
            newshdr->sh_entsize = sizeof(Elf_Half);
            newshdr->sh_addralign = 2;
            newdata->d_type = ELF_T_HALF;
            newdata->d_align = 2;
            updateDynLinkShdr.push_back(newshdr);
            newshdr->sh_flags = SHF_ALLOC;
            updateDynamic(DT_VERSYM, newshdr->sh_addr);
        } else if (newSec->getRegionType() == Region::RT_SYMVERNEEDED) {
            newshdr->sh_type = SHT_GNU_verneed;
            newshdr->sh_entsize = 0;
            newshdr->sh_addralign = 4;
            newdata->d_type = ELF_T_VNEED;
            newdata->d_align = 8;
            updateStrLinkShdr.push_back(newshdr);
            newshdr->sh_flags = SHF_ALLOC;
            newshdr->sh_info = verneednum;
            updateDynamic(DT_VERNEED, newshdr->sh_addr);
        } else if (newSec->getRegionType() == Region::RT_SYMVERDEF) {
            newshdr->sh_type = SHT_GNU_verdef;
            newshdr->sh_entsize = 0;
            newdata->d_type = ELF_T_VDEF;
            newdata->d_align = 8;
            updateStrLinkShdr.push_back(newshdr);
            newshdr->sh_flags = SHF_ALLOC;
            newshdr->sh_info = verdefnum;
            updateDynamic(DT_VERDEF, newshdr->sh_addr);
        }

        // Check to make sure the (vaddr for the start of the new segment - the offset) is page aligned
        if (!firstNewLoadSec) {
            Offset newoff =
                    newshdr->sh_offset - (newshdr->sh_offset & (pgSize - 1)) + (newshdr->sh_addr & (pgSize - 1));
            if (newoff < newshdr->sh_offset)
                newoff += pgSize;
            extraAlignSize += newoff - newshdr->sh_offset;
            newshdr->sh_offset = newoff;
            newSegmentStart = newshdr->sh_addr;
        }

        //Set up the data
        newdata->d_buf = allocate_buffer(newSec->getDiskSize());
        memcpy(newdata->d_buf, newSec->getPtrToRawData(), newSec->getDiskSize());
        newdata->d_off = 0;
        newdata->d_size = newSec->getDiskSize();
        if (!newdata->d_align)
            newdata->d_align = newshdr->sh_addralign;
        newshdr->sh_size = newdata->d_size;

        if (newshdr->sh_type == SHT_NOBITS) {
            currEndOffset = newshdr->sh_offset;
        } else {
            loadSecTotalSize += newshdr->sh_size;
            currEndOffset = newshdr->sh_offset + newshdr->sh_size;
        }
        currEndAddress = newshdr->sh_addr + newshdr->sh_size;

        rewrite_printf("new section %s addr = %lx off = %lx size = %lx\n",
                       newSec->getRegionName().c_str(), (long unsigned int)newshdr->sh_addr, (long unsigned int)newshdr->sh_offset,
                       (long unsigned int)newshdr->sh_size);

        newdata->d_version = 1;
        if (newshdr->sh_addralign < newdata->d_align) {
            newshdr->sh_addralign = newdata->d_align;
        }

        if (0 > elf_update(newElf, ELF_C_NULL)) {
            return false;
        }

        shdr = newshdr;
        if (!firstNewLoadSec)
            firstNewLoadSec = shdr;
        prevshdr = newshdr;
    }

    for (const auto &s : updateDynLinkShdr) {
        s->sh_link = dynsymIndex;
    }

    for (const auto &s : updateStrLinkShdr) {
        s->sh_link = strtabIndex;
    }


    return true;
}

template<class ElfTypes>
bool emitElf<ElfTypes>::addSectionHeaderTable(Elf_Shdr *shdr) {
    Elf_Scn *newscn;
    Elf_Data *newdata = NULL;
    Elf_Shdr *newshdr;

    if ((newscn = elf_newscn(newElf)) == NULL) {
        log_elferror(err_func_, "unable to create new section");
        return false;
    }
    if ((newdata = elf_newdata(newscn)) == NULL) {
        log_elferror(err_func_, "unable to create section data");
        return false;
    }
    //Fill out the new section header
    newshdr = ElfTypes::elf_getshdr(newscn);
    newshdr->sh_name = addSectionName(".shstrtab");
    newshdr->sh_type = SHT_STRTAB;
    newshdr->sh_entsize = 1;
    newdata->d_type = ELF_T_BYTE;
    newshdr->sh_link = SHN_UNDEF;
    newshdr->sh_flags = 0;

    newshdr->sh_offset = shdr->sh_offset + shdr->sh_size;
    newshdr->sh_addr = 0;
    newshdr->sh_info = 0;
    newshdr->sh_addralign = 4;

    //Set up the data
    newdata->d_buf = allocate_buffer(secNameTableTotalBytes);
    char *ptr = (char *) newdata->d_buf;
    for (const auto &secName : secNames) {
        auto bytesToCopy(secName.length() + 1);     // string plus null byte
        memcpy(ptr, secName.c_str(), bytesToCopy);
        ptr += bytesToCopy;
    }

    newdata->d_size = secNameTableTotalBytes;
    newshdr->sh_size = newdata->d_size;

    newdata->d_align = 4;
    newdata->d_version = 1;
    return true;
}

template<class ElfTypes>
bool emitElf<ElfTypes>::createNonLoadableSections(Elf_Shdr *&shdr) {
    Elf_Scn *newscn;
    Elf_Data *newdata = NULL;
    Elf_Shdr *newshdr;

    Elf_Shdr *prevshdr = shdr;
    //All of them that are left are non-loadable. stack'em up at the end.
    for (const auto &sec : nonLoadableSecs) {
        // Add a new non-loadable section
        if ((newscn = elf_newscn(newElf)) == NULL) {
            log_elferror(err_func_, "unable to create new section");
            return false;
        }
        if ((newdata = elf_newdata(newscn)) == NULL) {
            log_elferror(err_func_, "unable to create section data");
            return false;
        }

        //Fill out the new section header
        newshdr = ElfTypes::elf_getshdr(newscn);
        newshdr->sh_name = addSectionName(sec->getRegionName());
        regionNewIndex[sec] = secNames.size() - 1;
        if (sec->getRegionType() == Region::RT_TEXT)  {        //Text Section
            newshdr->sh_type = SHT_PROGBITS;
            newshdr->sh_flags = SHF_EXECINSTR | SHF_WRITE;
            newshdr->sh_entsize = 1;
            newdata->d_type = ELF_T_BYTE;
        } else if (sec->getRegionType() == Region::RT_DATA)  { //Data Section
            newshdr->sh_type = SHT_PROGBITS;
            newshdr->sh_flags = SHF_WRITE;
            newshdr->sh_entsize = 1;
            newdata->d_type = ELF_T_BYTE;
        } else if (sec->getRegionType() == Region::RT_REL)  {  //Relocations section
            newshdr->sh_type = SHT_REL;
            newshdr->sh_flags = SHF_WRITE;
            newshdr->sh_entsize = sizeof(Elf_Rel);
            newdata->d_type = ELF_T_BYTE;
        } else if (sec->getRegionType() == Region::RT_RELA) {  //Relocations section
            newshdr->sh_type = SHT_RELA;
            newshdr->sh_flags = SHF_WRITE;
            newshdr->sh_entsize = sizeof(Elf_Rela);
            newdata->d_type = ELF_T_BYTE;
        } else if (sec->getRegionType() == Region::RT_SYMTAB) {
            newshdr->sh_type = SHT_SYMTAB;
            newshdr->sh_entsize = sizeof(Elf_Sym);
            newdata->d_type = ELF_T_SYM;
            newshdr->sh_flags = 0;   // sh_link -> .strtab is set in driver's link fixup pass
        } else if (sec->getRegionType() == Region::RT_STRTAB) {  //String table Section
            newshdr->sh_type = SHT_STRTAB;
            newshdr->sh_entsize = 1;
            newdata->d_type = ELF_T_BYTE;
            newshdr->sh_link = SHN_UNDEF;
            newshdr->sh_flags = 0;
            symtabStrIndex = secNames.size() - 1;   // index of this section
        }

        newshdr->sh_offset = prevshdr->sh_offset + prevshdr->sh_size;
        if (prevshdr->sh_type == SHT_NOBITS) {
            newshdr->sh_offset = prevshdr->sh_offset;
        } else {
            newshdr->sh_offset = prevshdr->sh_offset + prevshdr->sh_size;
        }
        if (newshdr->sh_offset < currEndOffset) {
            newshdr->sh_offset = currEndOffset;
        }
        newshdr->sh_addr = 0;
        newshdr->sh_info = 0;
        newshdr->sh_addralign = 4;

        //Set up the data
        newdata->d_buf = sec->getPtrToRawData();
        newdata->d_size = sec->getDiskSize();
        newshdr->sh_size = newdata->d_size;

        newdata->d_align = 4;
        newdata->d_off = 0;
        newdata->d_version = 1;
        currEndOffset = newshdr->sh_offset + newshdr->sh_size;

        prevshdr = newshdr;
    }
    shdr = prevshdr;
    return true;
}

/* Regenerates the .symtab, .strtab sections from the symbols
 * Add new .dynsym, .dynstr sections for the newly added dynamic symbols
 * Method - For every symbol call createElfSymbol to get a Elf_Sym corresponding
 *          to a Symbol object. Accumulate all and their names to form the sections
 *          and add them to the list of new sections
 */
template<class ElfTypes>
bool emitElf<ElfTypes>::createSymbolTables(set<Symbol *> &allSymbols) {
    rewrite_printf(" createSymbolTables for %s \n", obj->name().c_str());
    unsigned i;

    //Symbol table(.symtab) symbols
    vector<Elf_Sym *> symbols;

    //Symbol table(.dynsymtab) symbols
    vector<Elf_Sym *> dynsymbols;

    unsigned symbolNamesLength = 1, dynsymbolNamesLength = 1;
    std::unordered_map<string, unsigned long> dynSymNameMapping;
    vector<string> symbolStrs, dynsymbolStrs;
    vector<Symbol *> dynsymVector;
    vector<Symbol *> allDynSymbols;
    vector<Symbol *> allSymSymbols;

    dyn_hash_map<int, Region *> secTagRegionMapping = object->getTagRegionMapping();

    Region *sec;
    auto foundRegion = secTagRegionMapping.find(DT_STRTAB);
    if ((foundRegion != secTagRegionMapping.end()) && (foundRegion->second != NULL)) {
        // .dynstr
        sec = foundRegion->second;
        olddynStrData = (char *) (sec->getPtrToRawData());
        olddynStrSize = sec->getMemSize();
        dynsymbolNamesLength = olddynStrSize + 1;
    }

    // Copy over the previous library dependencies
    vector<string> elibs;
    obj->getObject()->getDependencies(elibs);
    for (const auto &elib : elibs) {
        addDTNeeded(elib);
    }

    //Initialize the list of new prereq libraries
    set<string> &plibs = object->prereq_libs;
    for (auto plib : plibs) {
        addDTNeeded(plib);
    }
    new_dynamic_entries = object->new_dynamic_entries;
    // recreate a "dummy symbol"
    Elf_Sym *sym = new Elf_Sym();
    symbolStrs.push_back("");
    // We should increment this here, but for reasons I don't understand we create it with a size of
    // 1.
    //symbolNamesLength++;
    sym->st_name = 0;
    sym->st_value = 0;
    sym->st_size = 0;
    sym->st_other = 0;
    sym->st_info = ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE);
    sym->st_shndx = SHN_UNDEF;

    symbols.push_back(sym);
    symtabSymRegions.push_back(nullptr);
    if (!obj->isStaticBinary()) {
        dynsymbols.push_back(sym);
        dynsymSymRegions.push_back(nullptr);
        dynsymVector.push_back(Symbol::magicEmitElfSymbol());
        versionSymTable.push_back(0);
    }

    if (obj->isStaticBinary()) {
        // Static binary case
        vector<Region *> newRegs;
        obj->getAllNewRegions(newRegs);
        if (newRegs.size()) {
            emitElfStatic linker(obj->getAddressWidth(), isStripped);

            emitElfStatic::StaticLinkError err;
            std::string errMsg;
            linkedStaticData = linker.linkStatic(obj, err, errMsg);
            if (!linkedStaticData) {
                std::string linkStaticError =
                        "Failed to link to static library code into the binary: " +
                        emitElfStatic::printStaticLinkError(err) + " = "
                        + errMsg;
                Symtab::setSymtabError(Emit_Error);
                symtab_log_perror(linkStaticError.c_str());
                fprintf(stderr, "##### %s\n", linkStaticError.c_str());
                return false;
            }

            hasRewrittenTLS = linker.hasRewrittenTLS();

            // Find the end of the new Regions
            obj->getAllNewRegions(newRegs);

            Offset lastRegionAddr = 0, lastRegionSize = 0;
            for (const auto &region : newRegs) {
                if (region->getDiskOffset() > lastRegionAddr) {
                    lastRegionAddr = region->getDiskOffset();
                    lastRegionSize = region->getDiskSize();
                }
            }

            if (!emitElfUtils::updateHeapVariables(obj, lastRegionAddr + lastRegionSize)) {
                fprintf(stderr, "updateHeapVariables returns false\n");
                return false;
            }
        }
    }

    for (const auto &s : allSymbols) {
        if (s->isInSymtab()) {
            allSymSymbols.push_back(s);
        }
        if (!obj->isStaticBinary()) {
            if (s->isInDynSymtab()) {
                allDynSymbols.push_back(s);
            }
        }
    }

    // sort allSymbols in a way that every symbol with index -1 are in order of offset
    std::sort(allDynSymbols.begin(), allDynSymbols.end(), sortByOffsetNewIndices());

    int max_index = -1;
    for (const auto &s : allDynSymbols) {
        if (max_index < s->getIndex())
            max_index = s->getIndex();
    }
    for (const auto &s : allDynSymbols) {
        if (s->getIndex() == -1) {
            max_index++;
            s->setIndex(max_index);
        }

        if (s->getStrIndex() == -1) {
            // New Symbol - append to the list of strings
            dynsymbolStrs.push_back(s->getMangledName());
            s->setStrIndex(dynsymbolNamesLength);
            dynsymbolNamesLength += s->getMangledName().length() + 1;
        }

    }
    // reorder allSymbols based on index
    std::sort(allDynSymbols.begin(), allDynSymbols.end(), sortByIndex());


    std::sort(allSymSymbols.begin(), allSymSymbols.end(), sortByOffsetNewIndices());
    max_index = -1;
    for (const auto &s : allSymSymbols) {
        if (max_index < s->getIndex())
            max_index = s->getIndex();
    }

    for (const auto &s : allSymSymbols) {
        if (s->getIndex() == -1) {
            max_index++;
            s->setIndex(max_index);
        }
    }

    std::sort(allSymSymbols.begin(), allSymSymbols.end(), sortByIndex());

    /* We regenerate symtab and symstr section. We do not
       maintain the order of the strings and symbols as it was in
       the original binary. Hence, the strings in symstr have new order and
       new index.
       On the other hand, we do not regenerate dynsym and dynstr section. We copy over
       old symbols and string in the original order as it was in the
       original binary. We preserve sh_index of Elf symbols (from Symbol's strIndex). We append
       new symbols and string that we create for the new binary (targ*, versions etc).
    */

    for (const auto &s : allSymSymbols) {
        createElfSymbol(s, symbolNamesLength, symbols);
        symtabSymRegions.push_back(s->getRegion());
        symbolStrs.push_back(s->getMangledName());
        symbolNamesLength += s->getMangledName().length() + 1;
    }
    int nTmp = dynsymVector.size();
    i = 0;
    for (const auto &s : allDynSymbols) {
        createElfSymbol(s, s->getStrIndex(), dynsymbols, true);
        dynsymSymRegions.push_back(s->getRegion());
        dynSymNameMapping[s->getMangledName()] = i + nTmp;
        ++i;
        dynsymVector.push_back(s);
    }

    //reconstruct .symtab section
    // ELF requires all STB_LOCAL symbols to precede the others, with sh_info
    // holding the index of the first non-local.  New symbols were appended
    // after the originals regardless of binding, so partition them here.
    std::vector<size_t> order(symbols.size());
    std::iota(order.begin(), order.end(), 0);
    auto firstNonLocal = std::stable_partition(order.begin(), order.end(),
        [&](size_t k) { return ELF64_ST_BIND(symbols[k]->st_info) == STB_LOCAL; });
    symtabNumLocals = firstNonLocal - order.begin();

    Elf_Sym *syms = (Elf_Sym *) malloc(symbols.size() * sizeof(Elf_Sym));
    std::vector<Region *> orderedRegions;
    orderedRegions.reserve(order.size());
    for (size_t k = 0; k < order.size(); ++k) {
        syms[k] = *symbols[order[k]];
        orderedRegions.push_back(symtabSymRegions[order[k]]);
    }
    symtabSymRegions = std::move(orderedRegions);

    char *str = (char *) malloc(symbolNamesLength);
    unsigned cur = 0;
    for (const auto &s : symbolStrs) {
        strcpy(&str[cur], s.c_str());
        cur += s.length() + 1;
    }

    if (!isStripped) {
        Region *section;
        if (obj->findRegion(section, ".symtab"))
            section->setPtrToRawData(syms, symbols.size() * sizeof(Elf_Sym));
        else
            obj->addRegion(0, syms, symbols.size() * sizeof(Elf_Sym), ".symtab", Region::RT_SYMTAB);
    } else
        obj->addRegion(0, syms, symbols.size() * sizeof(Elf_Sym), ".symtab", Region::RT_SYMTAB);

    //reconstruct .strtab section
    if (!isStripped) {
        Region *section;
        if (obj->findRegion(section, ".strtab"))
            section->setPtrToRawData(str, symbolNamesLength);
        else
            obj->addRegion(0, str, symbolNamesLength, ".strtab", Region::RT_STRTAB);
    } else
        obj->addRegion(0, str, symbolNamesLength, ".strtab", Region::RT_STRTAB);

    if (!obj->getAllNewRegions(newSecs))
        log_elferror(err_func_, "No new sections to add");

    if (dynsymbols.size() == 1)
        return true;

    if (!obj->isStaticBinary()) {
        //reconstruct .dynsym section
        Elf_Sym *dynsyms = (Elf_Sym *) malloc(dynsymbols.size() * sizeof(Elf_Sym));
        i = 0;
        for (const auto &s : dynsymbols)
            dynsyms[i++] = *s;

        Elf_Half *symVers;
        char *verneedSecData, *verdefSecData;
        unsigned verneedSecSize = 0, verdefSecSize = 0;

        createSymbolVersions(symVers, verneedSecData, verneedSecSize, verdefSecData, verdefSecSize,
                             dynsymbolNamesLength, dynsymbolStrs);
        // build new .hash section
        Elf_Word *hashsecData;
        unsigned hashsecSize = 0;
        createHashSection(hashsecData, hashsecSize, dynsymVector);
        if (hashsecSize) {
            string name;
            if (secTagRegionMapping.find(DT_HASH) != secTagRegionMapping.end()) {
                name = secTagRegionMapping[DT_HASH]->getRegionName();
                obj->addRegion(0, hashsecData, hashsecSize * sizeof(Elf_Word), name, Region::RT_HASH, true);
            } else if (secTagRegionMapping.find(0x6ffffef5) != secTagRegionMapping.end()) {
                name = secTagRegionMapping[0x6ffffef5]->getRegionName();
                obj->addRegion(0, hashsecData, hashsecSize * sizeof(Elf_Word), name, Region::RT_HASH, true);
            } else {
                name = ".hash";
                obj->addRegion(0, hashsecData, hashsecSize * sizeof(Elf_Word), name, Region::RT_HASH, true);
            }
        }

        Elf_Dyn *dynsecData = NULL;
        unsigned dynsecSize = 0;
        if (obj->findRegion(sec, ".dynamic")) {
            // Need to ensure that DT_REL and related fields added to .dynamic
            // The values of these fields will be set
            createDynamicSection(sec->getPtrToRawData(), sec->getDiskSize(), dynsecData, dynsecSize,
                                 dynsymbolNamesLength, dynsymbolStrs);
        }

        if (!dynsymbolNamesLength)
            return true;

        char *dynstr = (char *) malloc(dynsymbolNamesLength);
        memcpy((void *) dynstr, (void *) olddynStrData, olddynStrSize);
        dynstr[olddynStrSize] = '\0';
        cur = olddynStrSize + 1;
        i = 0;
        for (const auto & s : dynsymbolStrs) {
            strcpy(&dynstr[cur], s.c_str());
            cur += s.length() + 1;
            if (dynSymNameMapping.find(s) == dynSymNameMapping.end()) {
                dynSymNameMapping[s] = allDynSymbols.size() + i;
            }
            ++i;
        }

        string name;
        if (secTagRegionMapping.find(DT_SYMTAB) != secTagRegionMapping.end()) {
            name = secTagRegionMapping[DT_SYMTAB]->getRegionName();
        } else {
            name = ".dynsym";
        }
        obj->addRegion(0, dynsyms, dynsymbols.size() * sizeof(Elf_Sym), name, Region::RT_SYMTAB, true);

        if (secTagRegionMapping.find(DT_STRTAB) != secTagRegionMapping.end()) {
            name = secTagRegionMapping[DT_STRTAB]->getRegionName();
        } else {
            name = ".dynstr";
        }
        obj->addRegion(0, dynstr, dynsymbolNamesLength, name, Region::RT_STRTAB, true);

        //add .gnu.version, .gnu.version_r, and .gnu.version_d sections
        if (secTagRegionMapping.find(DT_VERSYM) != secTagRegionMapping.end()) {
            name = secTagRegionMapping[DT_VERSYM]->getRegionName();
        } else {
            name = ".gnu.version";
        }
        obj->addRegion(0, symVers, versionSymTable.size() * sizeof(Elf_Half), name, Region::RT_SYMVERSIONS, true);

        if (verneedSecSize) {
            if (secTagRegionMapping.find(DT_VERNEED) != secTagRegionMapping.end()) {
                name = secTagRegionMapping[DT_VERNEED]->getRegionName();
            } else {
                name = ".gnu.version_r";
            }
            obj->addRegion(0, verneedSecData, verneedSecSize, name, Region::RT_SYMVERNEEDED, true);
        }

        if (verdefSecSize) {
            obj->addRegion(0, verdefSecData, verdefSecSize, ".gnu.version_d", Region::RT_SYMVERDEF, true);
        }

        //Always create a dyn section, it may get our new relocations.
        //If both exist, then just try to maintain order.
        bool has_plt = object->hasRelaplt() || object->hasRelplt();
        bool has_dyn = object->hasReladyn() || object->hasReldyn();
        if (!has_plt) {
            createRelocationSections(object->getDynRelocs(), true, dynSymNameMapping);
        } else if (!has_dyn) {
            createRelocationSections(object->getPLTRelocs(), false, dynSymNameMapping);
            createRelocationSections(object->getDynRelocs(), true, dynSymNameMapping);
        } else if (object->getRelPLTAddr() < object->getRelDynAddr()) {
            createRelocationSections(object->getPLTRelocs(), false, dynSymNameMapping);
            createRelocationSections(object->getDynRelocs(), true, dynSymNameMapping);
        } else {
            createRelocationSections(object->getDynRelocs(), true, dynSymNameMapping);
            createRelocationSections(object->getPLTRelocs(), false, dynSymNameMapping);
        }
        createRelrRelocationSection(object->getRelrDynRelocs());

        //add .dynamic section
        if (dynsecSize) {
            obj->findRegion(oldDynamicRegion, ".dynamic");   // before the new one exists
            obj->addRegion(0, dynsecData, dynsecSize * sizeof(Elf_Dyn), ".dynamic", Region::RT_DYNAMIC, true);
        }
    }

    if (!obj->getAllNewRegions(newSecs))
        log_elferror(err_func_, "No new sections to add");

    return true;
}

template<class ElfTypes>
void emitElf<ElfTypes>::createRelocationSections(std::vector<relocationEntry> &relocation_table, bool isDynRelocs,
                                                   std::unordered_map<std::string, unsigned long> &dynSymNameMapping) {
    vector<relocationEntry> newRels;
    if (isDynRelocs && newSecs.size()) {
        for (const auto s : newSecs) {
            std::copy(s->getRelocations().begin(),
                      s->getRelocations().end(),
                      std::back_inserter(newRels));
        }
    }


    Elf_Rel *rels = (Elf_Rel *) malloc(sizeof(Elf_Rel) * (relocation_table.size() + newRels.size()));
    Elf_Rela *relas = (Elf_Rela *) malloc(sizeof(Elf_Rela) * (relocation_table.size() + newRels.size()));
    unsigned numRels{};
    unsigned numRelas{};

    // new dynsym index for a relocation's symbol, STN_UNDEF if none
    auto dynSymIndex = [&](const relocationEntry &reloc) -> unsigned long {
        const std::string &name{reloc.name()};
        if (name.empty())
            return STN_UNDEF;
        auto it = dynSymNameMapping.find(name);
        if (it == dynSymNameMapping.end()) {
            Symbol *sym = reloc.getDynSym();
            if (!sym)
                return STN_UNDEF;
            it = dynSymNameMapping.find(sym->getMangledName());
            if (it == dynSymNameMapping.end())
                return sym->getIndex() > 0 ? sym->getIndex() : STN_UNDEF;
        }
        return it->second;
    };

    //reconstruct .rel
    for (auto &reloc : relocation_table) {
        if (address_adjust) {
            // If we are shifting the library down in memory, we need to update
            // any relative offsets in the library. These relative offsets are
            // found via relocations
 
            // XXX ...ignore the return value
            emitElfUtils::updateRelocation(obj, reloc, address_adjust);
        }
 
        if (object->getRelType() != reloc.regionType())
            continue;

        auto r_info = ElfTypes::makeRelocInfo(dynSymIndex(reloc), reloc.getRelType());

        if (reloc.regionType() == Region::RT_REL) {
            rels[numRels].r_offset = reloc.rel_addr() + address_adjust;
            rels[numRels].r_info = r_info;
            numRels++;
        } else if (reloc.regionType() == Region::RT_RELA) {
            relas[numRelas].r_offset = reloc.rel_addr() + address_adjust;
            relas[numRelas].r_addend = reloc.addend();
            relas[numRelas].r_info = r_info;
            numRelas++;
         }
    }
    // relocations for the new sections; these grow DT_RELSZ/DT_RELASZ
    const unsigned numOldRels{numRels};
    const unsigned numOldRelas{numRelas};
    for (const auto &newRel : newRels) {
        if (object->getRelType() != newRel.regionType())
            continue;

       auto r_info = ElfTypes::makeRelocInfo(dynSymIndex(newRel), newRel.getRelType());

       if (newRel.regionType() == Region::RT_REL) {
           rels[numRels].r_offset = newRel.rel_addr() + address_adjust;
           rels[numRels].r_info = r_info;
           numRels++;
       } else if (newRel.regionType() == Region::RT_RELA) {
           relas[numRelas].r_offset = newRel.rel_addr() + address_adjust;
           relas[numRelas].r_addend = newRel.addend();
           relas[numRelas].r_info = r_info;
           numRelas++;
        }
    }

    dyn_hash_map<int, Region *> secTagRegionMapping = object->getTagRegionMapping();
    int old_reloc_size;
    const char *new_name;
    Region::RegionType rtype;
    int dtype;
    int dsize_type;
    void *buffer = NULL;

    unsigned long reloc_size{numRels * sizeof(Elf_Rel) + numRelas * sizeof(Elf_Rela)};
    if (!reloc_size) {
        return;
    }
    if (isDynRelocs
        && object->getRelType() == Region::RT_REL) {
        new_name = ".rel.dyn";
        dtype = DT_REL;
        rtype = Region::RT_REL;
        dsize_type = DT_RELSZ;
        buffer = rels;
    }
    if (isDynRelocs
        && object->getRelType() == Region::RT_RELA) {
        new_name = ".rela.dyn";
        dtype = DT_RELA;
        rtype = Region::RT_RELA;
        dsize_type = DT_RELASZ;
        buffer = relas;
        updateDynamic(DT_RELAENT, sizeof(Elf_Rela));
    }

    if (!isDynRelocs
        && object->getRelType() == Region::RT_REL) {
        new_name = ".rel.plt";
        dtype = DT_JMPREL;
        rtype = Region::RT_PLTREL;
        dsize_type = DT_PLTRELSZ;
        buffer = rels;
    }
    if (!isDynRelocs
        && object->getRelType() == Region::RT_RELA) {
        new_name = ".rela.plt";
        dtype = DT_JMPREL;
        rtype = Region::RT_PLTRELA;
        dsize_type = DT_PLTRELSZ;
        buffer = relas;
    }

    if (buffer == NULL) {
        log_elferror(err_func_, "Unknown relocation type encountered");
        return;
    }

    if (dynamicSecData.find(dsize_type) != dynamicSecData.end())
        old_reloc_size = dynamicSecData[dsize_type][0]->d_un.d_val;
    else
        old_reloc_size = 0;
    unsigned long dynamic_reloc_size{old_reloc_size + (numRels - numOldRels) * sizeof(Elf_Rel)
                                               + (numRelas - numOldRelas) * sizeof(Elf_Rela)};
    string name;
    if (secTagRegionMapping.find(dtype) != secTagRegionMapping.end())
        name = secTagRegionMapping[dtype]->getRegionName();
    else
        name = new_name;
    obj->addRegion(0, buffer, reloc_size, name, rtype, true);
    updateDynamic(dsize_type, dynamic_reloc_size);
}

template<class ElfTypes>
void emitElf<ElfTypes>::createRelrRelocationSection(std::vector<Offset> &relr_table) {
    if (relr_table.empty()) return;

    // Re-encode the decoded relr table into compact RELR format
    // Inverse of decodeRelrEntries()
    // Entries alternate by their LSB:
    //  - even = an absolute address
    //  - odd  = a bitmap whose set bits mark relocated words following the
    //           last address
    std::vector<Offset> addrs(relr_table);
    for (auto &a : addrs)
        a += address_adjust;
    std::sort(addrs.begin(), addrs.end());
    addrs.erase(std::unique(addrs.begin(), addrs.end()), addrs.end());

    const Offset word_size = sizeof(Elf_Relr);
    const Offset addrBytesPerRelrBitmap = (8 * word_size - 1) * word_size;

    std::vector<Elf_Relr> packed;
    Offset bitmapBeginAddr = 0;  // first addr covered by bitmap
    Offset bitmapEndAddr = 0;    // first addr not covered by bitmap
    Elf_Relr bitmap = 0;
    for (size_t i = 0; i < addrs.size(); ) {
        auto a = addrs[i];
        if (bitmapBeginAddr <= a && a < bitmapEndAddr) {
            auto offset = a - bitmapBeginAddr;
            if ((offset % word_size) == 0) {                 // check if aligned
                ++i;                                         // consume addr
                bitmap |= 1UL << ((offset / word_size) + 1); // set bit
            } else {
                bitmapBeginAddr = bitmapEndAddr;  // unaligned, invalidate bitmap, try again
            }
            continue;
        }
        if (bitmap) {
            bitmap |= 1;                             // set bit 0, indicating a bitmap
            packed.push_back(bitmap);                // add RELR entry
            bitmap = 0;
            if (bitmapBeginAddr != bitmapEndAddr) {  // set new bitmap range, if valid
                bitmapBeginAddr = bitmapEndAddr;
                bitmapEndAddr += addrBytesPerRelrBitmap;
            }
            continue;  // try again, the value might be in new bitmap range
        }
        ++i;                              // consume addr
        packed.push_back(a);              // add RELR entry
        bitmapBeginAddr = a + word_size;  // set new bitmap range
        bitmapEndAddr = bitmapBeginAddr + addrBytesPerRelrBitmap;
    }
    if (bitmap) {                         // if final bitmap
        bitmap |= 1;                      // set bit 0, indicating a bitmap
        packed.push_back(bitmap);         // add RELR entry
    }

    Elf_Relr *relrs = (Elf_Relr *) malloc(sizeof(Elf_Relr) * packed.size());
    memcpy(relrs, packed.data(), sizeof(Elf_Relr) * packed.size());

    dyn_hash_map<int, Region *> secTagRegionMapping = object->getTagRegionMapping();
    string name;
    if (secTagRegionMapping.find(DT_RELR) != secTagRegionMapping.end())
        name = secTagRegionMapping[DT_RELR]->getRegionName();
    else
        name = ".relr.dyn";

    obj->addRegion(0, relrs, packed.size() * sizeof(Elf_Relr), std::move(name),
                   Region::RT_RELR, true);
}

template<class ElfTypes>
void emitElf<ElfTypes>::createSymbolVersions(Elf_Half *&symVers, char *&verneedSecData, unsigned &verneedSecSize,
                                               char *&verdefSecData,
                                               unsigned &verdefSecSize, unsigned &dynSymbolNamesLength,
                                               std::vector<std::string> &dynStrs) {

    //Add all names to the new .dynstr section
    for (auto &versionName : versionNames) {
        versionName.second = dynSymbolNamesLength;
        dynStrs.push_back(versionName.first);
        dynSymbolNamesLength += versionName.first.size() + 1;
    }

    //reconstruct .gnu_version section
    symVers = (Elf_Half *) malloc(versionSymTable.size() * sizeof(Elf_Half));
    for (unsigned i = 0; i < versionSymTable.size(); i++)
        symVers[i] = versionSymTable[i];

    // Preserve original .gnu.version_r entries that were not recreated through
    // symbol version references, unless their provider library was removed
    const auto &originalVersionMapping = object->getVersionMapping();
    const auto &originalVersionFileNameMapping = object->getVersionFileNameMapping();
    const auto &removedLibraries = object->libsRMd();
    for (const auto &versionEntry : originalVersionMapping) {
        auto fileEntry = originalVersionFileNameMapping.find(versionEntry.first);
        if (fileEntry == originalVersionFileNameMapping.end()) continue;
        if (find(removedLibraries.begin(), removedLibraries.end(), fileEntry->second) !=
            removedLibraries.end()) continue;

        auto &versionEntries = verneedEntries[fileEntry->second];
        for (const auto &versionName : versionEntry.second) {
            if (versionEntries.find(versionName) != versionEntries.end()) continue;
            if (versionNames.find(versionName) == versionNames.end()) {
                versionNames[versionName] = dynSymbolNamesLength;
                dynStrs.push_back(versionName);
                dynSymbolNamesLength += versionName.size() + 1;
            }
            versionEntries[versionName] = curVersionNum++;
        }
    }

    //reconstruct .gnu.version_r section
    verneedSecSize = 0;
    for (const auto &verneedEntry : verneedEntries)
        verneedSecSize += sizeof(Elf_Verneed) + sizeof(Elf_Vernaux) * verneedEntry.second.size();

    verneedSecData = (char *) malloc(verneedSecSize);
    unsigned curpos = 0;
    verneednum = 0;
    for (const auto &unneededEntry : unversionedNeededEntries) {
        // account for any substitutions due to rewriting a shared lib
        // no need for self-references
        if (!(obj->name() == unneededEntry)) {
            versionNames[unneededEntry] = dynSymbolNamesLength;
            dynStrs.push_back(unneededEntry);
            dynSymbolNamesLength += (unneededEntry).size() + 1;
            addDTNeeded(unneededEntry);
        }
    }
    for (auto &verneedEntry : verneedEntries) {
        Elf_Verneed *verneed = reinterpret_cast<Elf_Verneed *>(verneedSecData + curpos);
        verneed->vn_version = 1;
        verneed->vn_cnt = verneedEntry.second.size();
        verneed->vn_file = dynSymbolNamesLength;
        versionNames[verneedEntry.first] = dynSymbolNamesLength;
        dynStrs.push_back(verneedEntry.first);
        dynSymbolNamesLength += verneedEntry.first.size() + 1;
        addDTNeeded(verneedEntry.first);
        verneed->vn_aux = sizeof(Elf_Verneed);
        verneed->vn_next = sizeof(Elf_Verneed) + verneedEntry.second.size() * sizeof(Elf_Vernaux);
        if (curpos + verneed->vn_next == verneedSecSize)
            verneed->vn_next = 0;
        verneednum++;
        int i = 0;
        for (const auto &ver : verneedEntry.second) {
            Elf_Vernaux *vernaux = reinterpret_cast<Elf_Vernaux *>(
                    verneedSecData + curpos + verneed->vn_aux + i * sizeof(Elf_Vernaux));
            vernaux->vna_hash = elfHash(ver.first);
            vernaux->vna_flags = 0;
            vernaux->vna_other = ver.second;
            vernaux->vna_name = versionNames[ver.first];
            if (i == verneed->vn_cnt - 1)
                vernaux->vna_next = 0;
            else
                vernaux->vna_next = sizeof(Elf_Vernaux);
            i++;
        }
        curpos += verneed->vn_next;
    }

    //reconstruct .gnu.version_d section
    verdefSecSize = 0;
    for (const auto &verdefEntry : verdefEntries)
        verdefSecSize += sizeof(Elf_Verdef) + sizeof(Elf_Verdaux) * verdauxEntries[verdefEntry.second].size();

    verdefSecData = (char *) malloc(verdefSecSize);
    curpos = 0;
    verdefnum = 0;

    for (const auto &verdefEntry : verdefEntries) {
        Elf_Verdef *verdef = reinterpret_cast<Elf_Verdef *>(verdefSecData + curpos);
        verdef->vd_version = 1;
        verdef->vd_flags = 0;
        verdef->vd_ndx = verdefEntry.second;
        verdef->vd_cnt = verdauxEntries[verdefEntry.second].size();
        verdef->vd_hash = elfHash(verdefEntry.first);
        verdef->vd_aux = sizeof(Elf_Verdef);
        verdef->vd_next = sizeof(Elf_Verdef) + verdauxEntries[verdefEntry.second].size() * sizeof(Elf_Verdaux);
        if (curpos + verdef->vd_next == verdefSecSize)
            verdef->vd_next = 0;
        verdefnum++;
        for (unsigned i = 0; i < verdauxEntries[verdefEntry.second].size(); i++) {
            Elf_Verdaux *verdaux = reinterpret_cast<Elf_Verdaux *>(
                    verdefSecData + curpos + verdef->vd_aux + i * sizeof(Elf_Verdaux));
            verdaux->vda_name = versionNames[verdauxEntries[verdefEntry.second][i]];
            if ((signed) i == verdef->vd_cnt - 1)
                verdaux->vda_next = 0;
            else
                verdaux->vda_next = sizeof(Elf_Verdaux);
        }
        curpos += verdef->vd_next;
    }
    
    return;
}

template<class ElfTypes>
void emitElf<ElfTypes>::createHashSection(Elf_Word *&hashsecData, unsigned &hashsecSize,
                                            std::vector<Symbol *> &dynSymbols) {

    /* Save the original hash table entries */
    std::vector<unsigned> originalHashEntries;
    Offset dynsymSize = object->getDynsymSize();

    Elf_Scn *scn = NULL;
    Elf_Shdr *shdr = NULL;
    while ((scn = elf_nextscn(oldElf, scn))) {
        shdr = ElfTypes::elf_getshdr(scn);
        if (object->getElfHashAddr() != 0 &&
            object->getElfHashAddr() == shdr->sh_addr) {
            Elf_Data *hashData = elf_getdata(scn, NULL);
            Elf_Word *oldHashSec = (Elf_Word *) hashData->d_buf;
            unsigned original_nbuckets, original_nchains;
            original_nbuckets = oldHashSec[0];
            original_nchains = oldHashSec[1];
            for (unsigned i = 0; i < original_nbuckets + original_nchains; i++) {
                if (oldHashSec[2 + i] != 0) {
                    originalHashEntries.push_back(oldHashSec[2 + i]);
                }
            }
        }

        if (object->getGnuHashAddr() != 0 &&
            object->getGnuHashAddr() == shdr->sh_addr) {
            Elf_Data *hashData = elf_getdata(scn, NULL);
            Elf_Word *oldHashSec = (Elf_Word *) hashData->d_buf;
            unsigned symndx = oldHashSec[1];
            if (dynsymSize != 0)
                for (unsigned i = symndx; i < dynsymSize; i++) {
                    originalHashEntries.push_back(i);
                }
        }
    }

    vector<Symbol *>::iterator iter;
    dyn_hash_map<unsigned, unsigned> lastHash; // bucket number to symbol index
    unsigned nbuckets = (unsigned) dynSymbols.size() * 2 / 3;
    if (nbuckets % 2 == 0)
        nbuckets--;
    if (nbuckets < 1)
        nbuckets = 1;
    unsigned nchains = (unsigned) dynSymbols.size();
    hashsecSize = 2 + nbuckets + nchains;
    hashsecData = (Elf_Word *) malloc(hashsecSize * sizeof(Elf_Word));
    unsigned i = 0, key;
    for (i = 0; i < hashsecSize; i++) {
        hashsecData[i] = STN_UNDEF;
    }
    hashsecData[0] = (Elf_Word) nbuckets;
    hashsecData[1] = (Elf_Word) nchains;
    i = 0;
    for (iter = dynSymbols.begin(); iter != dynSymbols.end(); iter++, i++) {
        if ((*iter)->getMangledName().empty()) continue;
        unsigned index = (*iter)->getIndex();
        if ((find(originalHashEntries.begin(), originalHashEntries.end(), index) == originalHashEntries.end()) &&
            (index < object->getDynsymSize())) {
            continue;
        }
        key = elfHash((*iter)->getMangledName()) % nbuckets;
        if (lastHash.find(key) != lastHash.end()) {
            hashsecData[2 + nbuckets + lastHash[key]] = i;
        } else {
            hashsecData[2 + key] = i;
        }
        lastHash[key] = i;
        hashsecData[2 + nbuckets + i] = STN_UNDEF;
    }
}

template<class ElfTypes>
void emitElf<ElfTypes>::createDynamicSection(void *dynData_, unsigned size, Elf_Dyn *&dynsecData, unsigned &dynsecSize,
                                               unsigned &dynSymbolNamesLength, std::vector<std::string> &dynStrs) {
    dynamicSecData.clear();
    Elf_Dyn *dyns = (Elf_Dyn *) dynData_;
    unsigned count = size / sizeof(Elf_Dyn);
    vector<string> &libs_rmd = object->libsRMd();
    dynsecSize = 2 * (count + DT_NEEDEDEntries.size() + new_dynamic_entries.size());
    dynsecData = (Elf_Dyn *) malloc(dynsecSize * sizeof(Elf_Dyn));
    unsigned curpos = 0;
    string rpathstr;
    for (unsigned i = 0; i < DT_NEEDEDEntries.size(); i++) {
        dynsecData[curpos].d_tag = DT_NEEDED;
        dynStrs.push_back(DT_NEEDEDEntries[i]);
        dynsecData[curpos].d_un.d_val = dynSymbolNamesLength;
        dynSymbolNamesLength += DT_NEEDEDEntries[i].size() + 1;
        dynamicSecData[DT_NEEDED].push_back(dynsecData + curpos);
        curpos++;
    }
    for (auto const& entry: new_dynamic_entries){
        long name = entry.first;
        long value = entry.second;
        dynsecData[curpos].d_tag = name;
        Elf_Off adjust = 0;
        switch(name)
        {
            case DT_INIT:
            case DT_FINI:
            case DT_DYNINST:
                adjust = address_adjust;
                break;
            default:
                break;
        };
        dynsecData[curpos].d_un.d_val = value + adjust;
        dynamicSecData[name].push_back(dynsecData + curpos);
        curpos++;

        if (name == DT_DYNINST) {
            // If we find the .dyninstInst section and DT_DYNINST dynamic entry, 
            // it means we are doing binary rewriting with trap springboards. 
            // If address_adjust is non-zero, then we also need to adjust springboard traps
            Region *dyninstReg = NULL;
            if (obj->findRegion(dyninstReg, ".dyninstInst") && address_adjust) {
                // The trap mapping header's in-memory offset is specified by the dynamic entry
                // We now need to get raw section data, and the raw sectiond data offset of the header
                auto header = alignas_cast<trap_mapping_header>(((char*)dyninstReg->getPtrToRawData() + value - dyninstReg->getMemOffset()));
                for (unsigned i = 0; i < header->num_entries; i++) {
                    header->traps[i].source = (void*) ((char*)header->traps[i].source + address_adjust);
                    header->traps[i].target = (void*) ((char*)header->traps[i].target + address_adjust);
                }
            }
        }
    }

    // There may be multiple HASH (ELF, GNU etc) sections in the original binary. We consolidate all of them into one.
    bool foundHashSection = false;

    for (unsigned i = 0; i < count; i++) {
        switch (dyns[i].d_tag) {
            case DT_NULL:
                break;
            case 0x6ffffef5: // DT_GNU_HASH (not defined on all platforms)
                if (!foundHashSection) {
                    dynsecData[curpos].d_tag = DT_HASH;
                    dynsecData[curpos].d_un.d_ptr = dyns[i].d_un.d_ptr;
                    dynamicSecData[DT_HASH].push_back(dynsecData + curpos);
                    curpos++;
                    foundHashSection = true;
                }
                break;
            case DT_HASH:
                if (!foundHashSection) {
                    dynsecData[curpos].d_tag = dyns[i].d_tag;
                    dynsecData[curpos].d_un.d_ptr = dyns[i].d_un.d_ptr;
                    dynamicSecData[dyns[i].d_tag].push_back(dynsecData + curpos);
                    curpos++;
                    foundHashSection = true;
                }
                break;
            case DT_NEEDED:
                rpathstr = &olddynStrData[dyns[i].d_un.d_val];
                if (find(DT_NEEDEDEntries.begin(), DT_NEEDEDEntries.end(), rpathstr) != DT_NEEDEDEntries.end()) {
                    break;
                }
                if (find(libs_rmd.begin(), libs_rmd.end(), rpathstr) != libs_rmd.end())
                    break;
                dynsecData[curpos].d_tag = dyns[i].d_tag;
                dynsecData[curpos].d_un.d_val = dynSymbolNamesLength;
                dynStrs.push_back(rpathstr);
                dynSymbolNamesLength += rpathstr.size() + 1;
                dynamicSecData[dyns[i].d_tag].push_back(dynsecData + curpos);
                curpos++;
                break;
            case DT_RPATH:
            case DT_RUNPATH:
                dynsecData[curpos].d_tag = dyns[i].d_tag;
                dynsecData[curpos].d_un.d_val = dynSymbolNamesLength;
                rpathstr = &olddynStrData[dyns[i].d_un.d_val];
                dynStrs.push_back(rpathstr);
                dynSymbolNamesLength += rpathstr.size() + 1;
                dynamicSecData[dyns[i].d_tag].push_back(dynsecData + curpos);
                curpos++;
                break;
            case DT_INIT:
            case DT_FINI:
            case DT_GNU_CONFLICT:
            case DT_JMPREL:
            case DT_PLTGOT:
            case DT_PREINIT_ARRAY:
            case DT_INIT_ARRAY:
            case DT_FINI_ARRAY:
#if defined(DYNINST_CODEGEN_ARCH_POWER) && defined(DYNINST_CODEGEN_ARCH_64BIT)
            // DT_PPC64_GLINK specifies the addres of the
            // PLT resolver in Power ABI V2.
            //
            // DT_PPC64_GLINK may not be defined in elf.h
            // on other platforms and has the same value as
            // other processor sepcific entries
            case DT_PPC64_GLINK:
#endif
                /**
                 * List every dynamic entry that references an address and isn't already
                 * updated here.  address_adjust will be a page size if
                 * we're dealing with a library without a fixed load address.  We'll be shifting
                 * the addresses of that library by a page.
                 **/
                dynsecData[curpos] = dyns[i];
                dynsecData[curpos].d_un.d_ptr += address_adjust;
                dynamicSecData[dyns[i].d_tag].push_back(dynsecData + curpos);
                curpos++;
                break;
            default:
                dynsecData[curpos] = dyns[i];
                dynamicSecData[dyns[i].d_tag].push_back(dynsecData + curpos);
                curpos++;
                break;
        }
    }
    // Need to ensure that DT_REL and related fields added to .dynamic
    // The values of these fields will be set

    if (!object->hasReldyn() && !object->hasReladyn()) {
        if (object->getRelType() == Region::RT_REL) {
            new_dynamic_entries.push_back(pair<long,long>(DT_REL, 0));
            new_dynamic_entries.push_back(pair<long,long>(DT_RELSZ, 0));

            dynamicSecData[DT_REL].push_back(dynsecData + curpos);
            dynsecData[curpos].d_tag = DT_NULL;
            dynsecData[curpos].d_un.d_val = 0;
            curpos++;
            dynamicSecData[DT_RELSZ].push_back(dynsecData + curpos);
            dynsecData[curpos].d_tag = DT_NULL;
            dynsecData[curpos].d_un.d_val = 0;
            curpos++;
            dynamicSecData[DT_RELENT].push_back(dynsecData + curpos);
            dynsecData[curpos].d_tag = DT_NULL;
            dynsecData[curpos].d_un.d_val = 0;
            curpos++;
        } else if (object->getRelType() == Region::RT_RELA) {
            dynamicSecData[DT_RELA].push_back(dynsecData + curpos);
            dynsecData[curpos].d_tag = DT_NULL;
            dynsecData[curpos].d_un.d_val = 0;
            curpos++;
            dynamicSecData[DT_RELASZ].push_back(dynsecData + curpos);
            dynsecData[curpos].d_tag = DT_NULL;
            dynsecData[curpos].d_un.d_val = 0;
            curpos++;
            dynamicSecData[DT_RELAENT].push_back(dynsecData + curpos);
            dynsecData[curpos].d_tag = DT_NULL;
            dynsecData[curpos].d_un.d_val = 0;
            curpos++;
        }
    }

    dynsecData[curpos].d_tag = DT_NULL;
    dynsecData[curpos].d_un.d_val = 0;
    curpos++;
    dynsecSize = curpos;
}


// Symbols were emitted with st_shndx = the Region's number in the original
// file.  Inserting the new sections renumbers everything after the insertion
// point, so point each symbol at its Region's index in the new file, and give
// section symbols the section's new address.  Returns false if the table is
// not one this emitter generated (left untouched).
template<class ElfTypes>
bool emitElf<ElfTypes>::updateSymbolSectionIndices(Elf_Scn *scn, const std::vector<Region *> &symRegions) {
    Elf_Data *data = elf_getdata(scn, NULL);
    if (!data || !data->d_buf)
        return false;
    Elf_Sym *syms = static_cast<Elf_Sym *>(data->d_buf);
    size_t numSyms = data->d_size / sizeof(Elf_Sym);
    if (numSyms != symRegions.size()) {
        // not a table we generated (e.g. .dynsym copied unchanged), leave it alone
        rewrite_printf("symbol table has %zu entries, %zu regions recorded; st_shndx not updated\n",
                       numSyms, symRegions.size());
        return false;
    }
    for (size_t k = 0; k < numSyms; ++k) {
        Region *region = symRegions[k];
        if (!region)
            continue;
        bool isSectionSym = ELF64_ST_TYPE(syms[k].st_info) == STT_SECTION;
        // .dynamic was regenerated elsewhere: symbols into the old one must
        // follow it, keeping their offset within the section.  In particular
        // _DYNAMIC, which the ELF standard defines as labeling the .dynamic
        // section and which glibc's r_debug support relies on.
        Offset offsetInRegion = 0;
        if (region == oldDynamicRegion && newDynamicRegion && !isSectionSym) {
            offsetInRegion = syms[k].st_value - address_adjust - region->getMemOffset();
            region = newDynamicRegion;
        }
        auto it = regionNewIndex.find(region);
        if (it == regionNewIndex.end())
            continue;
        syms[k].st_shndx = it->second;
        if (isSectionSym || region == newDynamicRegion) {
            if (Elf_Scn *target = elf_getscn(newElf, it->second))
                if (Elf_Shdr *targetShdr = ElfTypes::elf_getshdr(target))
                    syms[k].st_value = targetShdr->sh_addr + offsetInRegion;
        }
    }
    return true;
}

template<class ElfTypes>
void emitElf<ElfTypes>::log_elferror(void (*err_func)(const char *), const char *msg) {
    const char *err = elf_errmsg(elf_errno());
    std::string str{std::string{err ? err : "(bad elf error)"} + " " + msg};
    err_func(str.c_str());
}

template<class ElfTypes>
void emitElf<ElfTypes>::addDTNeeded(string s) {
    if (find(DT_NEEDEDEntries.begin(), DT_NEEDEDEntries.end(), s) != DT_NEEDEDEntries.end())
        return;
    vector<string> &libs_rmd = object->libsRMd();
    if (find(libs_rmd.begin(), libs_rmd.end(), s) != libs_rmd.end())
        return;
    DT_NEEDEDEntries.push_back(s);
}

template<class ElfType>
char* emitElf<ElfType>::allocate_buffer(size_t size) {
    buffers.push_back(malloc(size));
    return static_cast<char*>(buffers.back());
}


namespace Dyninst {
    namespace SymtabAPI {
        template class emitElf<ElfTypes32>;
        template class emitElf<ElfTypes64>;
    } // namespace SymtabAPI
} // namespace Dyninst

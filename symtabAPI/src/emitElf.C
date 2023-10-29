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
#include "emitElf.h"
#include "emitElfStatic.h"
#include "common/src/pathName.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "unaligned_memory_access.h"


#if defined(os_freebsd)
#include "common/src/freebsdKludges.h"
#endif


#if defined(os_linux)

#include "common/src/linuxKludges.h"

#endif

extern void symtab_log_perror(const char *msg);
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;


unsigned int elfHash(const char *name) {
    unsigned int h = 0, g;

    while (*name) {
        h = (h << 4) + *name++;
        if ((g = h & 0xf0000000))
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

template<class ElfTypes>
emitElf<ElfTypes>::emitElf(Elf_X *oldElfHandle_, bool isStripped_, Object *obj_, void (*err_func)(const char *),
                               Symtab *st) :
        oldElfHandle(oldElfHandle_), newElf(NULL), oldElf(NULL),
        obj(st),
        newEhdr(NULL), oldEhdr(NULL),
        newPhdr(NULL), oldPhdr(NULL), phdr_offset(0),
        textData(NULL), symStrData(NULL), dynStrData(NULL),
        olddynStrData(NULL), olddynStrSize(0),
        symTabData(NULL), dynsymData(NULL), dynData(NULL),
        phdrs_scn(NULL), verneednum(0), verdefnum(0),
        newSegmentStart(0), firstNewLoadSec(NULL),
        dataSegEnd(0), dynSegOff(0), dynSegAddr(0),
        phdrSegOff(0), phdrSegAddr(0), dynSegSize(0),
        secNameIndex(0), currEndOffset(0), currEndAddress(0),
        linkedStaticData(NULL), loadSecTotalSize(0),
        isStripped(isStripped_), library_adjust(0),
        object(obj_), err_func_(err_func),
        hasRewrittenTLS(false), TLSExists(false), newTLSData(NULL) {
    oldElf = oldElfHandle->e_elfp();
    curVersionNum = 2;

    //Set variable based on the mechanism to add new load segment
    // 1) createNewPhdr (Total program headers + 1) - default
    //	(a) movePHdrsFirst
    //    (b) create new section called dynphdrs and change pointers (createNewPhdrRegion)
    //    (c) library_adjust - create room for a new program header in a position-indepdent library
    //                         by increasing all virtual addresses for the library
    // 2) Use existing Phdr (used in bleugene - will be handled in function fixPhdrs)
    //    (a) replaceNOTE section - if NOTE exists
    //    (b) BSSExpandFlag - expand BSS section - default option

    // default
    createNewPhdr = true;
    BSSExpandFlag = false;
    replaceNOTE = false;

    //If we're dealing with a library that can be loaded anywhere,
    // then load the program headers into the later part of the binary,
    // this may trigger a kernel bug that was fixed in Summer 2007,
    // but is the only reliable way to modify these libraries.
    //If we're dealing with a library/executable that loads at a specific
    // address we'll put the phdrs into the page before that address.  This
    // works and will avoid the kernel bug.

    isStaticBinary = obj_->isStaticBinary();
    movePHdrsFirst = createNewPhdr && object && object->getLoadAddress();

    //If we want to try a mode where we add the program headers to a library
    // that can be loaded anywhere, and put the program headers in the first
    // page (avoiding the kernel bug), then set library_adjust to getpagesize().
    // This will shift all addresses in the library down by a page, accounting
    // for the extra page for program headers.  This causes some significant
    // changes to the binary, and isn't well tested.

    library_adjust = 0;
    if (cannotRelocatePhdrs() && !movePHdrsFirst) {
        movePHdrsFirst = true;
        library_adjust = getpagesize();
    }
}

template<typename ElfTypes>
bool emitElf<ElfTypes>::cannotRelocatePhdrs()
{
//#if defined(bug_phdrs_first_page)
    return true;
//#else
    //  return false;
//#endif
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
#if defined(STT_GNU_IFUNC)
     case Symbol::ST_INDIRECT: return STT_GNU_IFUNC;
#endif
     default: return STT_SECTION;
  }
}

static int elfSymBind(Symbol::SymbolLinkage sLinkage)
{
  switch (sLinkage) {
  case Symbol::SL_LOCAL: return STB_LOCAL;
  case Symbol::SL_WEAK: return STB_WEAK;
  case Symbol::SL_GLOBAL: return STB_GLOBAL;
#if defined(STB_GNU_UNIQUE)
  case Symbol::SL_UNIQUE: return STB_GNU_UNIQUE;
#endif
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

    int offset_adjust;
    if(elfSymType(symbol) == STT_TLS) {
        offset_adjust = 0; // offset relative to TLS section, any new entries go at end
    } else {
        offset_adjust = library_adjust;
    }
    // OPD-based systems
    if (symbol->getPtrOffset()) {
        sym->st_value = symbol->getPtrOffset() + offset_adjust;
    }
    else if (symbol->getOffset()) {
        sym->st_value = symbol->getOffset() + offset_adjust;
    }

    sym->st_size = symbol->getSize();
    sym->st_other = ELF64_ST_VISIBILITY(elfSymVisibility(symbol->getVisibility()));
    sym->st_info = (unsigned char) ELF64_ST_INFO(elfSymBind(symbol->getLinkage()), elfSymType(symbol));

    if (symbol->getRegion()) {
#if defined(os_freebsd)
        sym->st_shndx = (Elf_Half) symbol->getRegion()->getRegionNumber();
#else
        sym->st_shndx = (Elf_Section) symbol->getRegion()->getRegionNumber();
#endif
    }
    else if (symbol->isAbsolute()) {
        sym->st_shndx = SHN_ABS;
    }
    else {
        sym->st_shndx = 0;
    }

    symbols.push_back(sym);

    if (dynSymFlag) {
        //printf("dynamic symbol: %s\n", symbol->getMangledName().c_str());

        char msg[2048];
        char *mpos = msg;
        msg[0] = '\0';
        string fileName;

        if (!symbol->getVersionFileName(fileName)) {
            //verdef entry
            vector<string> *vers;
            if (!symbol->getVersions(vers)) {
                if (symbol->getLinkage() == Symbol::SL_GLOBAL) {
                    versionSymTable.push_back(1);

                }
                else {
                    versionSymTable.push_back(0);
                    mpos += sprintf(mpos, "  local\n");
                }
            }
            else {
                if (vers->size() > 0) {
                    // new verdef entry
                    mpos += sprintf(mpos, "verdef: symbol=%s  version=%s ", symbol->getMangledName().c_str(),
                                    (*vers)[0].c_str());
                    if (verdefEntries.find((*vers)[0]) != verdefEntries.end()) {
                        unsigned short index = verdefEntries[(*vers)[0]];
                        if (symbol->getVersionHidden()) index += 0x8000;
                        versionSymTable.push_back(index);
                    }
                    else {
                        unsigned short index = curVersionNum;
                        if (symbol->getVersionHidden()) index += 0x8000;
                        versionSymTable.push_back(index);

                        verdefEntries[(*vers)[0]] = curVersionNum;
                        curVersionNum++;
                    }
                }
                // add all versions to the verdef entry
                for (unsigned i = 0; i < vers->size(); i++) {
                    mpos += sprintf(mpos, "  {%s}", (*vers)[i].c_str());
                    if (versionNames.find((*vers)[i]) == versionNames.end()) {
                        versionNames[(*vers)[i]] = 0;
                    }

                    if (find(verdauxEntries[verdefEntries[(*vers)[0]]].begin(),
                             verdauxEntries[verdefEntries[(*vers)[0]]].end(),
                             (*vers)[i]) == verdauxEntries[verdefEntries[(*vers)[0]]].end()) {
                        verdauxEntries[verdefEntries[(*vers)[0]]].push_back((*vers)[i]);
                    }
                }
                mpos += sprintf(mpos, "\n");
            }
        }
        else {
            //verneed entry
            mpos += sprintf(mpos, "need: symbol=%s    filename=%s\n",
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
                            mpos += sprintf(mpos, "  new unversioned: %s\n", fileName.c_str());
                            unversionedNeededEntries.push_back(fileName);
                        }
                    }

                    if (symbol->getLinkage() == Symbol::SL_GLOBAL) {
                        mpos += sprintf(mpos, "  global (w/ filename)\n");
                        versionSymTable.push_back(1);
                    }
                    else {

                        versionSymTable.push_back(0);
                    }
                }
            }
            else {
                if (vers) {
                    // There should only be one version string by this time
                    //If the version name already exists then add the same version number to the version symbol table
                    //Else give a new number and add it to the mapping.
                    if (versionNames.find((*vers)[0]) == versionNames.end()) {
                        mpos += sprintf(mpos, "  new version name: %s\n", (*vers)[0].c_str());
                        versionNames[(*vers)[0]] = 0;
                    }

                    if (verneedEntries.find(fileName) != verneedEntries.end()) {
                        if (verneedEntries[fileName].find((*vers)[0]) != verneedEntries[fileName].end()) {
                            mpos += sprintf(mpos, "  vernum: %u\n", verneedEntries[fileName][(*vers)[0]]);
                            versionSymTable.push_back((unsigned short) verneedEntries[fileName][(*vers)[0]]);
                        }
                        else {
                            mpos += sprintf(mpos, "  new entry #%d: %s [%s]\n",
                                            curVersionNum, (*vers)[0].c_str(), fileName.c_str());
                            versionSymTable.push_back((unsigned short) curVersionNum);
                            verneedEntries[fileName][(*vers)[0]] = curVersionNum;
                            curVersionNum++;
                        }
                    }
                    else {
                        mpos += sprintf(mpos, "  new entry #%d: %s [%s]\n",
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

// Find the end of data/text segment
template<class ElfTypes>
void emitElf<ElfTypes>::findSegmentEnds() {
    Elf_Phdr *tmp = ElfTypes::elf_getphdr(oldElf);
    // Find the offset of the start of the text & the data segment
    // The first LOAD segment is the text & the second LOAD segment
    // is the data
    dataSegEnd = 0;
    for (unsigned i = 0; i < oldEhdr->e_phnum; i++) {
        if (tmp->p_type == PT_LOAD) {
            if (dataSegEnd < tmp->p_vaddr + tmp->p_memsz)
                dataSegEnd = tmp->p_vaddr + tmp->p_memsz;
        } else if (PT_TLS == tmp->p_type) {
            TLSExists = true;
        }
        tmp++;
    }
}

// Rename an old section. Lengths of old and new names must match.
// Only renames the FIRST matching section encountered.
template<class ElfTypes>
void emitElf<ElfTypes>::renameSection(const std::string &oldStr, const std::string &newStr, bool renameAll) {
    assert(oldStr.length() == newStr.length());
    for (unsigned k = 0; k < secNames.size(); k++) {
        if (secNames[k] == oldStr) {
            secNames[k].replace(0, oldStr.length(), newStr);
            if (!renameAll)
                break;
        }
    }
}

template<class ElfTypes>
bool emitElf<ElfTypes>::driver(std::string fName) {
    vector<ExceptionBlock *> exceptions;
    obj->getAllExceptions(exceptions);
    //  cerr << "Dumping exception info: " << endl;

    /*for (auto eb = exceptions.begin();
         eb != exceptions.end();
         ++eb) {
        //cerr << **eb << endl;
    }*/


    int newfd;
    Region *foundSec = NULL;
    unsigned pgSize = getpagesize();
    rewrite_printf("::driver for emitElf\n");

    string strtmpl = fName + "XXXXXX";
    auto buf = std::unique_ptr<char[]>(new char[strtmpl.length() + 1]);
    strncpy(buf.get(), strtmpl.c_str(), strtmpl.length() + 1);

    newfd = mkstemp(buf.get());

    if (newfd == -1) {
        log_elferror(err_func_, "error opening file to write symbols");
        return false;
    }
    strtmpl = buf.get();

    fchmod(newfd, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP);
    rewrite_printf("Emitting to temporary file %s\n", buf.get());

#if 0
    //open ELF File for writing
  if((newfd = (open(fName.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP)))==-1){
    log_elferror(err_func_, "error opening file to write symbols");
    return false;
  }
#endif

    if ((newElf = elf_begin(newfd, ELF_C_WRITE, NULL)) == NULL) {
        log_elferror(err_func_, "NEWELF_BEGIN_FAIL");
        fflush(stdout);
        cerr << "Failed to elf_begin" << endl;
        return false;
    }

    //Section name index for all sections
    secNames.push_back("");
    secNameIndex = 1;
    //Section name index for new sections
    loadSecTotalSize = 0;
    unsigned NOBITStotalsize = 0;
    int dirtySecsChange = 0;
    unsigned extraAlignSize = 0;

    // ".shstrtab" section: string table for section header names
    const char *shnames = pdelf_get_shnames(oldElfHandle);
    if (shnames == NULL) {
        log_elferror(err_func_, ".shstrtab section");
        return false;
    }

    dynsym_info = 0;
    // Write the Elf header first!
    newEhdr = ElfTypes::elf_newehdr(
            newElf);
    if (!newEhdr) {
        log_elferror(err_func_, "newEhdr failed\n");
        return false;
    }
    oldEhdr = ElfTypes::elf_getehdr(
            oldElf);
    memcpy(newEhdr, oldEhdr, sizeof(Elf_Ehdr));

    newEhdr->e_shnum += newSecs.size();

    // Find the end of text and data segments
    findSegmentEnds();
    unsigned insertPoint = oldEhdr->e_shnum;
    unsigned insertPointOffset = 0;
    unsigned NOBITSstartPoint = oldEhdr->e_shnum;

    if (movePHdrsFirst) {
        newEhdr->e_phoff = sizeof(Elf_Ehdr);
    }
    newEhdr->e_entry += library_adjust;

    /* flag the file for no auto-layout */
    elf_flagelf(newElf, ELF_C_SET, ELF_F_LAYOUT);

    Elf_Scn *scn = NULL, *newscn = NULL;
    Elf_Data *newdata = NULL, *olddata = NULL;
    Elf_Shdr *newshdr = NULL, *shdr = NULL;
    std::unordered_map<unsigned, unsigned> secLinkMapping;
    std::unordered_map<unsigned, unsigned> secInfoMapping;
    std::unordered_map<unsigned, unsigned> changeMapping;
    std::unordered_map<string, unsigned> newNameIndexMapping;
    std::unordered_map<unsigned, string> oldIndexNameMapping;

    std::unordered_set<string> updateLinkInfoSecs = {
        ".dynsym", /*".dynstr",*/ ".rela.dyn", ".rela.plt", ".dynamic", ".symtab"};
    std::unordered_map<string, pair<unsigned, unsigned>> dataLinkInfo;

    bool createdLoadableSections = false;
    unsigned scncount;
    unsigned sectionNumber = 0;

    for (scncount = 0; (scn = elf_nextscn(oldElf, scn)); scncount++) {
        //copy sections from oldElf to newElf
        shdr = ElfTypes::elf_getshdr(scn);

        // resolve section name
        const char *name = &shnames[shdr->sh_name];
        bool result = obj->findRegion(foundSec, shdr->sh_addr, shdr->sh_size);
        if (!result || foundSec->isDirty()) {
            result = obj->findRegion(foundSec, name);
        }

        // write the shstrtabsection at the end
        if (!strcmp(name, ".shstrtab"))
            continue;

        sectionNumber++;
        changeMapping[sectionNumber] = 0;
        oldIndexNameMapping[scncount + 1] = string(name);
        newNameIndexMapping[string(name)] = sectionNumber;

        newscn = elf_newscn(newElf);
        newshdr = ElfTypes::elf_getshdr(newscn);
        newdata = elf_newdata(newscn);
        olddata = elf_getdata(scn, NULL);
        memcpy(newshdr, shdr, sizeof(Elf_Shdr));
        memcpy(newdata, olddata, sizeof(Elf_Data));

        secNames.push_back(name);
        newshdr->sh_name = secNameIndex;
        secNameIndex += strlen(name) + 1;

        if (newshdr->sh_addr) {
            newshdr->sh_addr += library_adjust;

#if defined(arch_aarch64)
            if (strcmp(name, ".plt")==0)
                updateDynamic(DT_TLSDESC_PLT, library_adjust);
            if (strcmp(name, ".got")==0)
                updateDynamic(DT_TLSDESC_GOT, library_adjust);
#endif
        }

        if (foundSec->isDirty()) {
            newdata->d_buf = allocate_buffer(foundSec->getDiskSize());
            memcpy(newdata->d_buf, foundSec->getPtrToRawData(), foundSec->getDiskSize());
            newdata->d_size = foundSec->getDiskSize();
            newshdr->sh_size = foundSec->getDiskSize();
        }
        else if (olddata->d_buf)     //copy the data buffer from oldElf
        {
            newdata->d_buf = allocate_buffer(olddata->d_size);
            memcpy(newdata->d_buf, olddata->d_buf, olddata->d_size);
        }

        if (newshdr->sh_entsize && (newshdr->sh_size % newshdr->sh_entsize != 0)) {
            newshdr->sh_entsize = 0x0;
        }

        if (BSSExpandFlag) {
            // Add the expanded SHT_NOBITS section size if the section comes after those sections
            if (scncount > NOBITSstartPoint)
                newshdr->sh_offset += NOBITStotalsize;

            // Expand the NOBITS sections in file & and change the type from SHT_NOBITS to SHT_PROGBITS
            if (shdr->sh_type == SHT_NOBITS) {
                newshdr->sh_type = SHT_PROGBITS;
                newdata->d_buf = allocate_buffer(shdr->sh_size);
                memset(newdata->d_buf, '\0', shdr->sh_size);
                newdata->d_size = shdr->sh_size;
                if (NOBITSstartPoint == oldEhdr->e_shnum)
                    NOBITSstartPoint = scncount;
                NOBITStotalsize += shdr->sh_size;
            }
        }

        vector<vector<unsigned long> > moveSecAddrRange = obj->getObject()->getMoveSecAddrRange();

        for (unsigned i = 0; i != moveSecAddrRange.size(); i++) {
            if ((moveSecAddrRange[i][0] == shdr->sh_addr) ||
                (shdr->sh_addr >= moveSecAddrRange[i][0] && shdr->sh_addr < moveSecAddrRange[i][1])) {
                newshdr->sh_type = SHT_PROGBITS;
                changeMapping[sectionNumber] = 1;
                string newName = ".o";
                newName.append(name, 2, strlen(name));
                renameSection((string) name, newName, false);
            }
        }

        if ((obj->getObject()->getStrtabAddr() != 0 &&
             obj->getObject()->getStrtabAddr() == shdr->sh_addr) ||
            !strcmp(name, STRTAB_NAME)) {
            symStrData = newdata;
            updateSymbols(symTabData, symStrData, loadSecTotalSize);
        }

        //Change sh_link for .symtab to point to .strtab
        if ((obj->getObject()->getSymtabAddr() != 0 &&
             obj->getObject()->getSymtabAddr() == shdr->sh_addr) ||
            !strcmp(name, SYMTAB_NAME)) {
            newshdr->sh_link = secNames.size();
            changeMapping[sectionNumber] = 1;
            symTabData = newdata;
        }


        if (obj->getObject()->getTextAddr() != 0 &&
            obj->getObject()->getTextAddr() == shdr->sh_addr) {
            textData = newdata;
        }

        if (obj->getObject()->getDynamicAddr() != 0 &&
            obj->getObject()->getDynamicAddr() == shdr->sh_addr) {
            dynData = newdata;
            dynSegOff = newshdr->sh_offset;
            dynSegAddr = newshdr->sh_addr;
            // Change the data to update the relocation addr
            newshdr->sh_type = SHT_PROGBITS;
            changeMapping[sectionNumber] = 1;
            string newName = ".o";
            newName.append(name, 2, strlen(name));
            renameSection((string) name, newName, false);
        }

        // Only need to rewrite data section
        if (hasRewrittenTLS && foundSec->isTLS()
            && foundSec->getRegionType() == Region::RT_DATA) {
            // Clear TLS flag
            newshdr->sh_flags &= ~SHF_TLS;

            string newName = ".o";
            newName.append(name, 2, strlen(name));
            renameSection((string) name, newName, false);
        }

#if defined(arch_power) && defined(arch_64bit)
        // DISABLED
    if (0 && isStaticBinary && strcmp(name, ".got") == 0) {
      string newName = ".o";
      newName.append(name, 2, strlen(name));
      renameSection((string) name, newName, false);
    }
#endif

        if (isStaticBinary && (strcmp(name, ".rela.plt") == 0)) {
            string newName = ".o";
            newName.append(name, 2, strlen(name));
            renameSection(name, newName, false);
        }

        if (isStaticBinary && (strcmp(name, ".rela.plt") == 0)) {
            string newName = ".o";
            newName.append(name, 2, strlen(name));
            renameSection(name, newName, false);
            // Clear the PLT type; use PROGBITS
            newshdr->sh_type = SHT_PROGBITS;
        }
        if (library_adjust > 0 &&
                (strcmp(name, ".init_array") == 0 || strcmp(name, ".fini_array") == 0 ||
                 strcmp(name, "__libc_subfreeres") == 0 || strcmp(name, "__libc_atexit") == 0 ||
                 strcmp(name, "__libc_thread_subfreeres") == 0 || strcmp(name, "__libc_IO_vtables") == 0)) {
            for(std::size_t off = 0; off < newdata->d_size; off += sizeof(void*)) {
                char *loc = static_cast<char*>(newdata->d_buf) + off;
                size_t val{};
                // The calls to memcpy are required to not break the aliasing rules.
                memcpy(&val, loc, sizeof(val));
                if(val == 0) continue;
                val += library_adjust;
                memcpy(loc, &val, sizeof(val));
            }
        }
        // Change offsets of sections based on the newly added sections
        if (movePHdrsFirst) {
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
                if (newshdr->sh_offset < pgSize && !strcmp(name, INTERP_NAME)) {
                    newshdr->sh_addr -= pgSize;
                    if (createNewPhdr) {
                        newshdr->sh_addr += oldEhdr->e_phentsize;
                        newshdr->sh_offset += oldEhdr->e_phentsize;
                    }
                } else
                    newshdr->sh_offset += pgSize;
            }
        }

        if (scncount > insertPoint && newshdr->sh_offset >= insertPointOffset)
            newshdr->sh_offset += loadSecTotalSize;

        if (newshdr->sh_offset > 0)
            newshdr->sh_offset += dirtySecsChange;

        if (BSSExpandFlag) {
            if (newshdr->sh_offset > 0) {
                newshdr->sh_offset += extraAlignSize;
            }
        } else if (newshdr->sh_offset >= insertPointOffset) {
            newshdr->sh_offset += extraAlignSize;
        }

        if (foundSec->isDirty())
            dirtySecsChange += newshdr->sh_size - shdr->sh_size;

        if (BSSExpandFlag && newshdr->sh_addr) {
            unsigned newOff =
                    newshdr->sh_offset - (newshdr->sh_offset & (pgSize - 1)) + (newshdr->sh_addr & (pgSize - 1));
            if (newOff < newshdr->sh_offset)
                newOff += pgSize;
            extraAlignSize += newOff - newshdr->sh_offset;
            newshdr->sh_offset = newOff;
        }

        secLinkMapping[sectionNumber] = shdr->sh_link;
        secInfoMapping[sectionNumber] = shdr->sh_info;

        if(updateLinkInfoSecs.find(string(name)) != updateLinkInfoSecs.end())
            dataLinkInfo[string(name)] = std::make_pair(shdr->sh_link, shdr->sh_info);

        rewrite_printf("section %s addr = %lx off = %lx size = %lx\n",
                       name, (long unsigned int)newshdr->sh_addr, (long unsigned int)newshdr->sh_offset, (long unsigned int)newshdr->sh_size);
        rewrite_printf(" %02u Link(%u) Info(%u) change(%u)\n",
                sectionNumber, secLinkMapping[sectionNumber], secInfoMapping[sectionNumber],
                changeMapping[sectionNumber]);

        //Insert new loadable sections at the end of data segment
        if (shdr->sh_addr + shdr->sh_size == dataSegEnd && !createdLoadableSections) {
            createdLoadableSections = true;
            insertPoint = scncount;
            if (SHT_NOBITS == shdr->sh_type) {
                insertPointOffset = shdr->sh_offset;
            } else {
                insertPointOffset = shdr->sh_offset + shdr->sh_size;
            }

            if (!createLoadableSections(newshdr, extraAlignSize, newNameIndexMapping,
                       sectionNumber))
                return false;
            if (createNewPhdr && !movePHdrsFirst) {
                sectionNumber++;
                createNewPhdrRegion(newNameIndexMapping);
            }

            // Update the heap symbols, now that loadSecTotalSize is set
            updateSymbols(dynsymData, dynStrData, loadSecTotalSize);

        }

        if (0 > elf_update(newElf, ELF_C_NULL)) {
            return false;
        }

        // code to change interpreter for test
        /*if (strcmp(name, INTERP_NAME)==0)
        {
            cerr << "interpreter: ";
            cerr << (char *) newdata->d_buf << endl;
            const char* libc_path = "/tmp/libc/ld-2.29.so\0";
            strcpy((char*)newdata->d_buf, libc_path);
            cerr << "new interpreter: ";
            cerr << (char *) newdata->d_buf << endl;
        }*/

    } // end of for each elf section

    // Add non-loadable sections at the end of object file
    if (!createNonLoadableSections(newshdr))
        return false;

    if (0 > elf_update(newElf, ELF_C_NULL)) {
        return false;
    }

    //Add the section header table right at the end
    addSectionHeaderTable(newshdr);

    // Second iteration to fix the link fields to point to the correct section
    scn = NULL;
    for (scncount = 1; (scn = elf_nextscn(newElf, scn)); scncount++) {
        shdr = ElfTypes::elf_getshdr(scn);
        if(dataLinkInfo.count(secNames[scncount]))
        {
            rewrite_printf("update link info of %s\n", secNames[scncount].c_str());
            auto & data = dataLinkInfo[secNames[scncount]];
            //shdr->sh_link = data.first;
            shdr->sh_info = data.second;
        }
    }

    newEhdr->e_shstrndx = scncount - 1;

    // Move the section header to the end
    newEhdr->e_shoff = shdr->sh_offset + shdr->sh_size;
    if (newEhdr->e_shoff % 8)
        newEhdr->e_shoff += 8 - (
                newEhdr->e_shoff % 8);

    //copy program headers
    oldPhdr = ElfTypes::elf_getphdr(
            oldElf);
    fixPhdrs(extraAlignSize);

    //Write the new Elf file
    if (elf_update(newElf, ELF_C_WRITE) < 0) {
        log_elferror(err_func_, "elf_update failed");
        return false;
    }
    elf_end(newElf);
    close(newfd);

    if (rename(strtmpl.c_str(), fName.c_str())) {
        return false;
    }

    return true;
}


template<class ElfTypes>
void emitElf<ElfTypes>::createNewPhdrRegion(std::unordered_map<std::string, unsigned> &newNameIndexMapping) {
    assert(!movePHdrsFirst);

    unsigned phdr_size = oldEhdr->e_phnum * oldEhdr->e_phentsize;
    if (createNewPhdr)
        phdr_size += oldEhdr->e_phentsize;

    unsigned align = 0;
    if (currEndOffset % 8)
        align = 8 - (currEndOffset % 8);

    newEhdr->e_phoff = currEndOffset + align;
    phdr_offset = newEhdr->e_phoff;

    Address endaddr = currEndAddress;
    currEndAddress += phdr_size + align;
    currEndOffset += phdr_size + align;
    loadSecTotalSize += phdr_size + align;

    //libelf.so.1 is annoying.  It'll overwrite the data
    // between sections with 0's, even if we've stuck the
    // program headers in there.  Create a dummy section
    // to contain the program headers.
    phdrs_scn = elf_newscn(newElf);
    Elf_Shdr *newshdr = ElfTypes::elf_getshdr(phdrs_scn);
    const char *newname = ".dynphdrs";

    secNames.push_back(newname);
    newNameIndexMapping[newname] = secNames.size() - 1;
    newshdr->sh_name = secNameIndex;
    secNameIndex += strlen(newname) + 1;
    newshdr->sh_flags = SHF_ALLOC;
    newshdr->sh_type = SHT_PROGBITS;
    newshdr->sh_offset = newEhdr->e_phoff;
    newshdr->sh_addr = endaddr + align;
    newshdr->sh_size = phdr_size;
    newshdr->sh_link = SHN_UNDEF;
    newshdr->sh_info = 0;
    newshdr->sh_addralign = 4;
    newshdr->sh_entsize = newEhdr->e_phentsize;
    phdrSegOff = newshdr->sh_offset;
    phdrSegAddr = newshdr->sh_addr;

}


template<class ElfTypes>
void emitElf<ElfTypes>::fixPhdrs(unsigned &extraAlignSize) {
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
    unsigned pgSize = getpagesize();

    newEhdr->e_phnum = oldEhdr->e_phnum;
    newEhdr->e_phentsize = oldEhdr->e_phentsize;

    if (createNewPhdr) {
        newEhdr->e_phnum++;
        if (hasRewrittenTLS && !TLSExists) newEhdr->e_phnum++;
    }

    // Copy old segments to vector and update contents
    bool replaced = false;
    Elf_Phdr *old = oldPhdr;
    vector<Elf_Phdr> segments;

    for (unsigned i = 0; i < oldEhdr->e_phnum; i++)
    {
        segments.push_back(*old);

        if (old->p_type == PT_DYNAMIC) {
            segments[i].p_vaddr = dynSegAddr;
            segments[i].p_paddr = dynSegAddr;
            segments[i].p_offset = dynSegOff;
            segments[i].p_memsz = dynSegSize;
            segments[i].p_filesz = segments[i].p_memsz;
        }
        else if (old->p_type == PT_PHDR) {
            if (createNewPhdr && !movePHdrsFirst)
                segments[i].p_vaddr = phdrSegAddr;
            else if (createNewPhdr && movePHdrsFirst)
                segments[i].p_vaddr = old->p_vaddr - pgSize + library_adjust;
            else
                segments[i].p_vaddr = old->p_vaddr;
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
            if (!createNewPhdr && segments[i].p_align > pgSize) {
                segments[i].p_align = pgSize;
            }
            if (BSSExpandFlag) {
                if (old->p_flags == 6 || old->p_flags == 7) {
                    segments[i].p_memsz += loadSecTotalSize + extraAlignSize;
                    segments[i].p_filesz = segments[i].p_memsz;
                    segments[i].p_flags = 7;
                }
            }

            if (movePHdrsFirst) {
                if (!old->p_offset) {
                    if (segments[i].p_vaddr) {
                        segments[i].p_vaddr = old->p_vaddr - pgSize;
                        segments[i].p_align = pgSize;
                    }

                    segments[i].p_paddr = segments[i].p_vaddr;
                    segments[i].p_filesz += pgSize;
                    segments[i].p_memsz = segments[i].p_filesz;
                } else {
                    segments[i].p_offset += pgSize;
                    segments[i].p_align = pgSize;
                }
                if (segments[i].p_vaddr) {
                    segments[i].p_vaddr += library_adjust;
                    segments[i].p_paddr += library_adjust;
                }
            }
        } else if (replaceNOTE && old->p_type == PT_NOTE && !replaced) {
            replaced = true;
            segments[i].p_type = PT_LOAD;
            segments[i].p_offset = firstNewLoadSec->sh_offset;
            segments[i].p_vaddr = newSegmentStart;
            segments[i].p_paddr = segments[i].p_vaddr;
            segments[i].p_filesz = loadSecTotalSize - (newSegmentStart - firstNewLoadSec->sh_addr);
            segments[i].p_memsz =
                    (currEndAddress - firstNewLoadSec->sh_addr) - (newSegmentStart - firstNewLoadSec->sh_addr);
            segments[i].p_flags = PF_R + PF_W + PF_X;
            segments[i].p_align = pgSize;
        }
        else if (old->p_type == PT_INTERP && movePHdrsFirst && old->p_offset) {
            Elf_Off addr_shift = library_adjust;
            Elf_Off offset_shift = pgSize;
            if (old->p_offset < pgSize) {
                offset_shift = createNewPhdr ? oldEhdr->e_phentsize : 0;
                addr_shift -= pgSize - offset_shift;
            }
            segments[i].p_offset += offset_shift;
            segments[i].p_vaddr += addr_shift;
            segments[i].p_paddr += addr_shift;
        }
        else if (movePHdrsFirst && old->p_offset) {
            segments[i].p_offset += pgSize;
            if (segments[i].p_vaddr) {
                segments[i].p_vaddr += library_adjust;
                segments[i].p_paddr += library_adjust;
            }
        }

        ++old;
    }

    if (createNewPhdr && firstNewLoadSec)
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
        newSeg.p_align = pgSize;

        // Search position to insert new segment
        unsigned int position = -1;
        for( unsigned i = 0; i < segments.size(); i++ )
        {
            if (i + 1 == segments.size()) {
                // it's the last, so add after
                position = i + 1;
                break;
            }
            else if (segments[i].p_type == PT_LOAD && segments[i+1].p_type != PT_LOAD) {
                // insert at end of loadable phdrs
                position = i + 1;
                break;
            }
            else if (segments[i].p_type != PT_LOAD &&
                    segments[i+1].p_type == PT_LOAD &&
                    newSegmentStart < segments[i+1].p_vaddr) {
                // insert at beginning of loadable list (after the
                // current phdr)
                position = i + 1;
                break;
            }
            else if (segments[i].p_type == PT_LOAD &&
                    segments[i+1].p_type == PT_LOAD &&
                    newSegmentStart >= segments[i].p_vaddr &&
                    newSegmentStart < segments[i+1].p_vaddr) {
                // insert in middle of loadable list, after current
                position = i + 1;
                break;
            }
            else if (i == 0 &&
                    segments[i].p_type == PT_LOAD &&
                    newSegmentStart < segments[i].p_vaddr) {
                // insert BEFORE current phdr
                position = i;
                break;
            }
        }
        assert(position!=-1U);

        // Insert new Segment at position
        if( position == segments.size() )
            segments.push_back( newSeg );
        else
            segments.insert( segments.begin() + position, newSeg );
    }

    // Create newPhdr and copy segments to it
    newPhdr = ElfTypes::elf_newphdr(newElf, newEhdr->e_phnum);
    void *phdr_data = (void *) newPhdr;

    for (unsigned i = 0; i < segments.size(); i++)
    {
        memcpy(newPhdr, &segments[i], oldEhdr->e_phentsize);
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

//This method updates the .dynamic section to reflect the changes to the relocation section
template<class ElfTypes>
void emitElf<ElfTypes>::updateDynamic(unsigned tag, Elf_Addr val) {
    if (isStaticBinary) return;
    // This is for REL/RELA if it doesn't already exist in the original binary;
    if(dynamicSecData.find(tag) != dynamicSecData.end())
        dynamicSecData[tag][0]->d_tag = tag;
    else return;
    switch (dynamicSecData[tag][0]->d_tag) {
        case DT_STRSZ:
        case DT_RELSZ:
        case DT_RELASZ:
        case DT_PLTRELSZ:
        case DT_RELACOUNT:
        case DT_RELENT:
        case DT_RELAENT:
            dynamicSecData[tag][0]->d_un.d_val = val;
            break;
        case DT_HASH:
        case DT_GNU_HASH:
        case DT_SYMTAB:
        case DT_STRTAB:
        case DT_REL:
        case DT_RELA:
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

    for (unsigned i = 0; i < newSecs.size(); i++) {
        if (!newSecs[i]->isLoadable()) {
            nonLoadableSecs.push_back(newSecs[i]);
            continue;
        }
        secNames.push_back(newSecs[i]->getRegionName());
        newNameIndexMapping[newSecs[i]->getRegionName()] = secNames.size() - 1;
        sectionNumber++;
        // Add a new loadable section
        if ((newscn = elf_newscn(newElf)) == NULL) {
            log_elferror(err_func_, "unable to create new function");
            return false;
        }
        if ((newdata = elf_newdata(newscn)) == NULL) {
            log_elferror(err_func_, "unable to create section data");
            return false;
        }
        memset(newdata, 0, sizeof(Elf_Data));

        // Fill out the new section header
        newshdr = ElfTypes::elf_getshdr(newscn);
        newshdr->sh_name = secNameIndex;
        newshdr->sh_flags = 0;
        newshdr->sh_type = SHT_PROGBITS;
        switch (newSecs[i]->getRegionType()) {
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

        if (shdr->sh_type == SHT_NOBITS) {
            newshdr->sh_offset = shdr->sh_offset;
        } else if (!firstNewLoadSec || !newSecs[i]->getDiskOffset()) {
            newshdr->sh_offset = shdr->sh_offset + shdr->sh_size;
        } else {
            // The offset can be computed by determining the difference from
            // the first new loadable section
            newshdr->sh_offset = firstNewLoadSec->sh_offset + library_adjust +
                                 (newSecs[i]->getDiskOffset() - firstNewLoadSec->sh_addr);

            // Account for inter-section spacing due to alignment constraints
            loadSecTotalSize += newshdr->sh_offset - (shdr->sh_offset + shdr->sh_size);
        }

        if (newSecs[i]->getDiskOffset())
            newshdr->sh_addr = newSecs[i]->getDiskOffset() + library_adjust;
        else if (!prevshdr) {
            newshdr->sh_addr = zstart + library_adjust;
        }
        else {
            newshdr->sh_addr = prevshdr->sh_addr + prevshdr->sh_size;
        }

        newshdr->sh_link = SHN_UNDEF;
        newshdr->sh_info = 0;
        newshdr->sh_addralign = newSecs[i]->getMemAlignment();
        newshdr->sh_entsize = 0;

        // TLS section
        if (newSecs[i]->isTLS()) {
            newTLSData = newshdr;
            newshdr->sh_flags |= SHF_TLS;
        }

        if (newSecs[i]->getRegionType() == Region::RT_REL ||
            newSecs[i]->getRegionType() == Region::RT_PLTREL)    //Relocation section
        {
            newshdr->sh_type = SHT_REL;
            newshdr->sh_flags = SHF_ALLOC;
            newshdr->sh_entsize = sizeof(Elf_Rel);
            updateDynLinkShdr.push_back(newshdr);
            newdata->d_type = ELF_T_REL;
            newdata->d_align = 4;
            if (newSecs[i]->getRegionType() == Region::RT_REL)
                updateDynamic(DT_REL, newshdr->sh_addr);
            else if (newSecs[i]->getRegionType() == Region::RT_PLTREL)
                updateDynamic(DT_JMPREL, newshdr->sh_addr);
        }
        else if (newSecs[i]->getRegionType() == Region::RT_RELA ||
                 newSecs[i]->getRegionType() == Region::RT_PLTRELA) //Relocation section
        {
            newshdr->sh_type = SHT_RELA;
            newshdr->sh_flags = SHF_ALLOC;
            newshdr->sh_entsize = sizeof(Elf_Rela);
            updateDynLinkShdr.push_back(newshdr);
            newdata->d_type = ELF_T_RELA;
            newdata->d_align = 4;
            if (newSecs[i]->getRegionType() == Region::RT_RELA)
                updateDynamic(DT_RELA, newshdr->sh_addr);
            else if (newSecs[i]->getRegionType() == Region::RT_PLTRELA)
                updateDynamic(DT_JMPREL, newshdr->sh_addr);
        }
        else if (newSecs[i]->getRegionType() == Region::RT_STRTAB)    //String table Section
        {
            newshdr->sh_type = SHT_STRTAB;
            newshdr->sh_entsize = 1;
            newdata->d_type = ELF_T_BYTE;
            newshdr->sh_link = SHN_UNDEF;
            newshdr->sh_flags = SHF_ALLOC;
            newdata->d_align = 1;
            dynStrData = newdata;
            strtabIndex = secNames.size() - 1;
            newshdr->sh_addralign = 1;
            updateDynamic(DT_STRTAB, newshdr->sh_addr);
            updateDynamic(DT_STRSZ, newSecs[i]->getDiskSize());
        }
        else if (newSecs[i]->getRegionType() == Region::RT_SYMTAB) {
            newshdr->sh_type = SHT_DYNSYM;
            newshdr->sh_entsize = sizeof(Elf_Sym);
            newdata->d_type = ELF_T_SYM;
            newdata->d_align = 4;
            dynsymData = newdata;
            newshdr->sh_link = secNames.size();   //.symtab section should have sh_link = index of .strtab for .dynsym
            newshdr->sh_flags = SHF_ALLOC;
            dynsymIndex = secNames.size() - 1;
            updateDynamic(DT_SYMTAB, newshdr->sh_addr);
        }
        else if (newSecs[i]->getRegionType() == Region::RT_DYNAMIC) {
            newshdr->sh_entsize = sizeof(Elf_Dyn);
            newshdr->sh_type = SHT_DYNAMIC;
            newdata->d_type = ELF_T_DYN;
            newdata->d_align = 4;
            updateStrLinkShdr.push_back(newshdr);
            newshdr->sh_flags = SHF_ALLOC | SHF_WRITE;
            dynSegOff = newshdr->sh_offset;
            dynSegAddr = newshdr->sh_addr;
            dynSegSize = newSecs[i]->getDiskSize();
        }
        else if (newSecs[i]->getRegionType() == Region::RT_HASH) {
            newshdr->sh_entsize = sizeof(Elf_Word);
            newshdr->sh_type = SHT_HASH;
            newdata->d_type = ELF_T_WORD;
            newdata->d_align = 4;
            updateDynLinkShdr.push_back(newshdr);
            newshdr->sh_flags = SHF_ALLOC;
            newshdr->sh_info = 0;
            updateDynamic(DT_HASH, newshdr->sh_addr);
        }
        else if (newSecs[i]->getRegionType() == Region::RT_SYMVERSIONS) {
            newshdr->sh_type = SHT_GNU_versym;
            newshdr->sh_entsize = sizeof(Elf_Half);
            newshdr->sh_addralign = 2;
            newdata->d_type = ELF_T_HALF;
            newdata->d_align = 2;
            updateDynLinkShdr.push_back(newshdr);
            newshdr->sh_flags = SHF_ALLOC;
            updateDynamic(DT_VERSYM, newshdr->sh_addr);
        }
        else if (newSecs[i]->getRegionType() == Region::RT_SYMVERNEEDED) {
            newshdr->sh_type = SHT_GNU_verneed;
            newshdr->sh_entsize = 0;
            newshdr->sh_addralign = 4;
            newdata->d_type = ELF_T_VNEED;
            newdata->d_align = 8;
            updateStrLinkShdr.push_back(newshdr);
            newshdr->sh_flags = SHF_ALLOC;
            newshdr->sh_info = verneednum;
            updateDynamic(DT_VERNEED, newshdr->sh_addr);
        }
        else if (newSecs[i]->getRegionType() == Region::RT_SYMVERDEF) {
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
        newdata->d_buf = allocate_buffer(newSecs[i]->getDiskSize());
        memcpy(newdata->d_buf, newSecs[i]->getPtrToRawData(), newSecs[i]->getDiskSize());
        newdata->d_off = 0;
        newdata->d_size = newSecs[i]->getDiskSize();
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
                       newSecs[i]->getRegionName().c_str(), (long unsigned int)newshdr->sh_addr, (long unsigned int)newshdr->sh_offset,
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
        secNameIndex += newSecs[i]->getRegionName().size() + 1;
        prevshdr = newshdr;
    }

    for (unsigned i = 0; i < updateDynLinkShdr.size(); i++) {
        newshdr = updateDynLinkShdr[i];
        newshdr->sh_link = dynsymIndex;
    }

    for (unsigned i = 0; i < updateStrLinkShdr.size(); i++) {
        newshdr = updateStrLinkShdr[i];
        newshdr->sh_link = strtabIndex;
    }


    return true;
}

template<class ElfTypes>
bool emitElf<ElfTypes>::addSectionHeaderTable(Elf_Shdr *shdr) {
    Elf_Scn *newscn;
    Elf_Data *newdata = NULL;
    Elf_Shdr *newshdr;

    if ((newscn = elf_newscn(newElf)) == NULL) {
        log_elferror(err_func_, "unable to create new function");
        return false;
    }
    if ((newdata = elf_newdata(newscn)) == NULL) {
        log_elferror(err_func_, "unable to create section data");
        return false;
    }
    //Fill out the new section header
    newshdr = ElfTypes::elf_getshdr(newscn);
    newshdr->sh_name = secNameIndex;
    secNames.push_back(".shstrtab");
    secNameIndex += 10;
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
    newdata->d_buf = allocate_buffer(secNameIndex);
    char *ptr = (char *) newdata->d_buf;
    for (unsigned i = 0; i < secNames.size(); i++) {
        memcpy(ptr, secNames[i].c_str(), secNames[i].length());
        memcpy(ptr + secNames[i].length(), "\0", 1);
        ptr += secNames[i].length() + 1;
    }

    newdata->d_size = secNameIndex;
    newshdr->sh_size = newdata->d_size;
    //elf_update(newElf, ELF_C_NULL);

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
    for (unsigned i = 0; i < nonLoadableSecs.size(); i++) {
        secNames.push_back(nonLoadableSecs[i]->getRegionName());
        // Add a new non-loadable section
        if ((newscn = elf_newscn(newElf)) == NULL) {
            log_elferror(err_func_, "unable to create new function");
            return false;
        }
        if ((newdata = elf_newdata(newscn)) == NULL) {
            log_elferror(err_func_, "unable to create section data");
            return false;
        }

        //Fill out the new section header
        newshdr = ElfTypes::elf_getshdr(newscn);
        newshdr->sh_name = secNameIndex;
        secNameIndex += nonLoadableSecs[i]->getRegionName().length() + 1;
        if (nonLoadableSecs[i]->getRegionType() == Region::RT_TEXT)        //Text Section
        {
            newshdr->sh_type = SHT_PROGBITS;
            newshdr->sh_flags = SHF_EXECINSTR | SHF_WRITE;
            newshdr->sh_entsize = 1;
            newdata->d_type = ELF_T_BYTE;
        }
        else if (nonLoadableSecs[i]->getRegionType() == Region::RT_DATA)    //Data Section
        {
            newshdr->sh_type = SHT_PROGBITS;
            newshdr->sh_flags = SHF_WRITE;
            newshdr->sh_entsize = 1;
            newdata->d_type = ELF_T_BYTE;
        }
        else if (nonLoadableSecs[i]->getRegionType() == Region::RT_REL)    //Relocations section
        {
            newshdr->sh_type = SHT_REL;
            newshdr->sh_flags = SHF_WRITE;
            newshdr->sh_entsize = sizeof(Elf_Rel);
            newdata->d_type = ELF_T_BYTE;
        }
        else if (nonLoadableSecs[i]->getRegionType() == Region::RT_RELA)    //Relocations section
        {
            newshdr->sh_type = SHT_RELA;
            newshdr->sh_flags = SHF_WRITE;
            newshdr->sh_entsize = sizeof(Elf_Rela);
            newdata->d_type = ELF_T_BYTE;
        }
        else if (nonLoadableSecs[i]->getRegionType() == Region::RT_SYMTAB) {
            newshdr->sh_type = SHT_SYMTAB;
            newshdr->sh_entsize = sizeof(Elf_Sym);
            newdata->d_type = ELF_T_SYM;
            newshdr->sh_link = secNames.size();   //.symtab section should have sh_link = index of .strtab
            newshdr->sh_flags = 0;
        }
        else if (nonLoadableSecs[i]->getRegionType() == Region::RT_STRTAB)    //String table Section
        {
            newshdr->sh_type = SHT_STRTAB;
            newshdr->sh_entsize = 1;
            newdata->d_type = ELF_T_BYTE;
            newshdr->sh_link = SHN_UNDEF;
            newshdr->sh_flags = 0;
        }
        /*
          else if(nonLoadableSecs[i]->getFlags() & Section::dynsymtabSection)
          {
      newshdr->sh_type = SHT_DYNSYM;
      newshdr->sh_entsize = sizeof(Elf_Sym);
      newdata->d_type = ELF_T_SYM;
      //newshdr->sh_link = newSecSize+i+1;   //.symtab section should have sh_link = index of .strtab
      newshdr->sh_flags=  SHF_ALLOC | SHF_WRITE;
          }*/
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
        newdata->d_buf = nonLoadableSecs[i]->getPtrToRawData();
        newdata->d_size = nonLoadableSecs[i]->getDiskSize();
        newshdr->sh_size = newdata->d_size;
        //elf_update(newElf, ELF_C_NULL);

        newdata->d_align = 4;
        newdata->d_off = 0;
        newdata->d_version = 1;
        currEndOffset = newshdr->sh_offset + newshdr->sh_size;
        //currEndAddress = newshdr->sh_addr + newshdr->sh_size;
        /* DEBUG */

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

    dyn_hash_map<int, Region *> secTagRegionMapping = obj->getObject()->getTagRegionMapping();

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
    for (auto iter = elibs.begin();
         iter != elibs.end(); ++iter) {
        addDTNeeded(*iter);
    }

    //Initialize the list of new prereq libraries
    set<string> &plibs = obj->getObject()->prereq_libs;
    for (auto plib : plibs) {
        addDTNeeded(plib);
    }
    new_dynamic_entries = obj->getObject()->new_dynamic_entries;
    Object *object_ = obj->getObject();
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
    if (!obj->isStaticBinary()) {
        dynsymbols.push_back(sym);
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
                        std::string("Failed to link to static library code into the binary: ") +
                        emitElfStatic::printStaticLinkError(err) + std::string(" = ")
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
            vector<Region *>::iterator newRegIter;
            for (newRegIter = newRegs.begin(); newRegIter != newRegs.end();
                 ++newRegIter) {
                if ((*newRegIter)->getDiskOffset() > lastRegionAddr) {
                    lastRegionAddr = (*newRegIter)->getDiskOffset();
                    lastRegionSize = (*newRegIter)->getDiskSize();
                }
            }

            if (!emitElfUtils::updateHeapVariables(obj, lastRegionAddr + lastRegionSize)) {
		fprintf(stderr, "updateHeapVariables returns false\n");
                return false;
            }
        }
    }

    for (auto sym_iter = allSymbols.begin(); sym_iter != allSymbols.end(); ++sym_iter) {
        if ((*sym_iter)->isInSymtab()) {
            allSymSymbols.push_back(*sym_iter);
        }
        if (!obj->isStaticBinary()) {
            if ((*sym_iter)->isInDynSymtab()) {
                allDynSymbols.push_back(*sym_iter);
            }
        }
    }

    // sort allSymbols in a way that every symbol with index -1 are in order of offset
    std::sort(allDynSymbols.begin(), allDynSymbols.end(), sortByOffsetNewIndices());

    int max_index = -1;
    for (i = 0; i < allDynSymbols.size(); i++) {
        if (max_index < allDynSymbols[i]->getIndex())
            max_index = allDynSymbols[i]->getIndex();
    }
    for (i = 0; i < allDynSymbols.size(); i++) {
        if (allDynSymbols[i]->getIndex() == -1) {
            max_index++;
            allDynSymbols[i]->setIndex(max_index);
        }

        if (allDynSymbols[i]->getStrIndex() == -1) {
            // New Symbol - append to the list of strings
            dynsymbolStrs.push_back(allDynSymbols[i]->getMangledName().c_str());
            allDynSymbols[i]->setStrIndex(dynsymbolNamesLength);
            dynsymbolNamesLength += allDynSymbols[i]->getMangledName().length() + 1;
        }

    }
    // reorder allSymbols based on index
    std::sort(allDynSymbols.begin(), allDynSymbols.end(), sortByIndex());


    std::sort(allSymSymbols.begin(), allSymSymbols.end(), sortByOffsetNewIndices());
    max_index = -1;
    for (i = 0; i < allSymSymbols.size(); i++) {
        if (max_index < allSymSymbols[i]->getIndex())
            max_index = allSymSymbols[i]->getIndex();
    }

    for (i = 0; i < allSymSymbols.size(); i++) {
        if (allSymSymbols[i]->getIndex() == -1) {
            max_index++;
            allSymSymbols[i]->setIndex(max_index);
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

    for (i = 0; i < allSymSymbols.size(); i++) {
        //allSymSymbols[i]->setStrIndex(symbolNamesLength);
        createElfSymbol(allSymSymbols[i], symbolNamesLength, symbols);
        symbolStrs.push_back(allSymSymbols[i]->getMangledName());
        symbolNamesLength += allSymSymbols[i]->getMangledName().length() + 1;
    }
    int nTmp = dynsymVector.size();
    for (i = 0; i < allDynSymbols.size(); i++) {
        createElfSymbol(allDynSymbols[i], allDynSymbols[i]->getStrIndex(), dynsymbols, true);
        dynSymNameMapping[allDynSymbols[i]->getMangledName().c_str()] = i + nTmp; //allDynSymbols[i]->getIndex();
        dynsymVector.push_back(allDynSymbols[i]);
    }

    //reconstruct .symtab section
    Elf_Sym *syms = (Elf_Sym *) malloc(symbols.size() * sizeof(Elf_Sym));
    for (i = 0; i < symbols.size(); i++)
        syms[i] = *(symbols[i]);

    char *str = (char *) malloc(symbolNamesLength);
    unsigned cur = 0;
    for (i = 0; i < symbolStrs.size(); i++) {
        strcpy(&str[cur], symbolStrs[i].c_str());
        cur += symbolStrs[i].length() + 1;
    }

    if (!isStripped) {
        Region *section;
        if (obj->findRegion(section, ".symtab"))
            section->setPtrToRawData(syms, symbols.size() * sizeof(Elf_Sym));
        else
            obj->addRegion(0, syms, symbols.size() * sizeof(Elf_Sym), ".symtab", Region::RT_SYMTAB);
    }
    else
        obj->addRegion(0, syms, symbols.size() * sizeof(Elf_Sym), ".symtab", Region::RT_SYMTAB);

    //reconstruct .strtab section
    if (!isStripped) {
        Region *section;
        if (obj->findRegion(section, ".strtab"))
            section->setPtrToRawData(str, symbolNamesLength);
        else
            obj->addRegion(0, str, symbolNamesLength, ".strtab", Region::RT_STRTAB);
    }
    else
        obj->addRegion(0, str, symbolNamesLength, ".strtab", Region::RT_STRTAB);

    if (!obj->getAllNewRegions(newSecs))
        log_elferror(err_func_, "No new sections to add");

    if (dynsymbols.size() == 1)
        return true;

    if (!obj->isStaticBinary()) {
        //reconstruct .dynsym section
        Elf_Sym *dynsyms = (Elf_Sym *) malloc(dynsymbols.size() * sizeof(Elf_Sym));
        for (i = 0; i < dynsymbols.size(); i++)
            dynsyms[i] = *(dynsymbols[i]);

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
/*
        if( !object_->hasReldyn() && !object_->hasReladyn() ) {
            if( object_->getRelType() == Region::RT_REL ) {
                new_dynamic_entries.push_back(make_pair(DT_REL,0));
                new_dynamic_entries.push_back(make_pair(DT_RELSZ,0));
            }else if( object_->getRelType() == Region::RT_RELA ) {
                new_dynamic_entries.push_back(make_pair(DT_RELA,0));
                new_dynamic_entries.push_back(make_pair(DT_RELASZ,0));
            }else{
                assert(!"Relocation type not set to known RT_REL or RT_RELA.");
            }
        }
*/
            createDynamicSection(sec->getPtrToRawData(), sec->getDiskSize(), dynsecData, dynsecSize,
                                 dynsymbolNamesLength, dynsymbolStrs);
        }

        if (!dynsymbolNamesLength)
            return true;

        char *dynstr = (char *) malloc(dynsymbolNamesLength);
        memcpy((void *) dynstr, (void *) olddynStrData, olddynStrSize);
        dynstr[olddynStrSize] = '\0';
        cur = olddynStrSize + 1;
        for (i = 0; i < dynsymbolStrs.size(); i++) {
            strcpy(&dynstr[cur], dynsymbolStrs[i].c_str());
            cur += dynsymbolStrs[i].length() + 1;
            if (dynSymNameMapping.find(dynsymbolStrs[i]) == dynSymNameMapping.end()) {
                dynSymNameMapping[dynsymbolStrs[i]] = allDynSymbols.size() + i;
            }
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
        bool has_plt = object_->hasRelaplt() || object_->hasRelplt();
        bool has_dyn = object_->hasReladyn() || object_->hasReldyn();
        if (!has_plt) {
            createRelocationSections(object_->getDynRelocs(), true, dynSymNameMapping);
        }
        else if (!has_dyn) {
            createRelocationSections(object_->getPLTRelocs(), false, dynSymNameMapping);
            createRelocationSections(object_->getDynRelocs(), true, dynSymNameMapping);
        }
        else if (object_->getRelPLTAddr() < object_->getRelDynAddr()) {
            createRelocationSections(object_->getPLTRelocs(), false, dynSymNameMapping);
            createRelocationSections(object_->getDynRelocs(), true, dynSymNameMapping);
        }
        else {
            createRelocationSections(object_->getDynRelocs(), true, dynSymNameMapping);
            createRelocationSections(object_->getPLTRelocs(), false, dynSymNameMapping);
        }

        //add .dynamic section
        if (dynsecSize)
            obj->addRegion(0, dynsecData, dynsecSize * sizeof(Elf_Dyn), ".dynamic", Region::RT_DYNAMIC, true);
    }

    if (!obj->getAllNewRegions(newSecs))
        log_elferror(err_func_, "No new sections to add");

    unsigned int prev_size = 0;
    unsigned long sec_addr = 0;
    for (unsigned long nsi = 0; nsi < newSecs.size(); nsi++) {
	// Update the _DYNAMIC symbol; described in the elf standard as:
	//  The program header table will have an element of type PT_DYNAMIC.
	//  This "segment" contains the .dynamic section. A special symbol,
	//  _DYNAMIC, labels the section
	if (newSecs[nsi]->getDiskOffset())
	  sec_addr = newSecs[nsi]->getDiskOffset() + library_adjust;
	else
	  sec_addr += prev_size;
	prev_size = newSecs[nsi]->getDiskSize();
	if (".dynamic" == newSecs[nsi]->getRegionName()) {
	    // Found the .dynamic section
	    for (unsigned long symi = 0; symi < symbolStrs.size(); symi++)
	      if ("_DYNAMIC" == symbolStrs[symi]) {
		  // Found the _DYNAMIC symbol
		  rewrite_printf("update _DYNAMIC symbol from %#lx to %#lx\n",
				 (unsigned long) syms[symi].st_value, (unsigned long) sec_addr);
		  syms[symi].st_value = sec_addr;
		  break;
	      }
	    break;
	}
    }

    return true;
}

template<class ElfTypes>
void emitElf<ElfTypes>::createRelocationSections(std::vector<relocationEntry> &relocation_table, bool isDynRelocs,
                                                   std::unordered_map<std::string, unsigned long> &dynSymNameMapping) {
    vector<relocationEntry> newRels;
    if (isDynRelocs && newSecs.size()) {
        std::vector<Region *>::iterator i;
        for (i = newSecs.begin(); i != newSecs.end(); i++) {
            std::copy((*i)->getRelocations().begin(),
                      (*i)->getRelocations().end(),
                      std::back_inserter(newRels));
        }
    }

    unsigned i, j, k, l, m;

    Elf_Rel *rels = (Elf_Rel *) malloc(sizeof(Elf_Rel) * (relocation_table.size() + newRels.size()));
    Elf_Rela *relas = (Elf_Rela *) malloc(sizeof(Elf_Rela) * (relocation_table.size() + newRels.size()));
    j = 0;
    k = 0;
    l = 0;
    m = 0;
    //reconstruct .rel
    for (i = 0; i < relocation_table.size(); i++) {

        if (library_adjust) {
            // If we are shifting the library down in memory, we need to update
            // any relative offsets in the library. These relative offsets are 
            // found via relocations

            // XXX ...ignore the return value
            emitElfUtils::updateRelocation(obj, relocation_table[i], library_adjust);
        }

        if ((object->getRelType() == Region::RT_REL) && (relocation_table[i].regionType() == Region::RT_REL)) {
            rels[j].r_offset = relocation_table[i].rel_addr() + library_adjust;
            unsigned long sym_offset = 0;
            std::string sym_name = relocation_table[i].name();
            if (!sym_name.empty()) {
                std::unordered_map<string, unsigned long>::iterator it = dynSymNameMapping.find(sym_name);
                if (it != dynSymNameMapping.end())
                    sym_offset = it->second;
                else {
                    Symbol *sym = relocation_table[i].getDynSym();
                    if (sym)
                        sym_offset = sym->getIndex();
                }
            }

            if (sym_offset) {
                rels[j].r_info = ElfTypes::makeRelocInfo(sym_offset, relocation_table[i].getRelType());
            } else {
                rels[j].r_info = ElfTypes::makeRelocInfo((unsigned long) STN_UNDEF, relocation_table[i].getRelType());
            }
            j++;
        } else if ((object->getRelType() == Region::RT_RELA) && (relocation_table[i].regionType() == Region::RT_RELA)) {
            relas[k].r_offset = relocation_table[i].rel_addr() + library_adjust;
            relas[k].r_addend = relocation_table[i].addend();
            //if (relas[k].r_addend)
            //   relas[k].r_addend += library_adjust;
            unsigned long sym_offset = 0;
            std::string sym_name = relocation_table[i].name();
            if (!sym_name.empty()) {
                std::unordered_map<string, unsigned long>::iterator it = dynSymNameMapping.find(sym_name);
                if (it != dynSymNameMapping.end()) {
                    sym_offset = it->second;
                }
                else {
                    Symbol *sym = relocation_table[i].getDynSym();
                    if (sym) {
                        it = dynSymNameMapping.find(sym->getMangledName());
                        if (it != dynSymNameMapping.end())
                            sym_offset = it->second;
                    }
                }
            }
            if (sym_offset) {
                relas[k].r_info = ElfTypes::makeRelocInfo(sym_offset, relocation_table[i].getRelType());
            } else {
                relas[k].r_info = ElfTypes::makeRelocInfo((unsigned long) STN_UNDEF, relocation_table[i].getRelType());
            }
            k++;
        }
    }
    for (i = 0; i < newRels.size(); i++) {
        if ((object->getRelType() == Region::RT_REL) && (newRels[i].regionType() == Region::RT_REL)) {
            rels[j].r_offset = newRels[i].rel_addr() + library_adjust;
            if (dynSymNameMapping.find(newRels[i].name()) != dynSymNameMapping.end()) {
                rels[j].r_info = ElfTypes::makeRelocInfo(dynSymNameMapping[newRels[i].name()],
                                                         newRels[i].getRelType());
            } else {
                rels[j].r_info = ElfTypes::makeRelocInfo((unsigned long) (STN_UNDEF),
                                                         newRels[i].getRelType());
            }
            j++;
            l++;
        } else if ((object->getRelType() == Region::RT_RELA) && (newRels[i].regionType() == Region::RT_RELA)) {
            relas[k].r_offset = newRels[i].rel_addr() + library_adjust;
            relas[k].r_addend = newRels[i].addend();
            //if( relas[k].r_addend ) relas[k].r_addend += library_adjust;
            if (dynSymNameMapping.find(newRels[i].name()) != dynSymNameMapping.end()) {
                relas[k].r_info = ElfTypes::makeRelocInfo(dynSymNameMapping[newRels[i].name()],
                                                          newRels[i].getRelType());
            } else {
                relas[k].r_info = ElfTypes::makeRelocInfo((unsigned long) (STN_UNDEF),
                                                          newRels[i].getRelType());
            }
            k++;
            m++;
        }
    }

    dyn_hash_map<int, Region *> secTagRegionMapping = obj->getObject()->getTagRegionMapping();
    int reloc_size, old_reloc_size, dynamic_reloc_size;
    const char *new_name;
    Region::RegionType rtype;
    int dtype;
    int dsize_type;
    void *buffer = NULL;

    reloc_size = j * sizeof(Elf_Rel) + k * sizeof(Elf_Rela);
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
    dynamic_reloc_size = old_reloc_size + l * sizeof(Elf64_Rel) + m * sizeof(Elf64_Rela);
    string name;
    if (secTagRegionMapping.find(dtype) != secTagRegionMapping.end())
        name = secTagRegionMapping[dtype]->getRegionName();
    else
        name = std::string(new_name);
    obj->addRegion(0, buffer, reloc_size, name, rtype, true);
    updateDynamic(dsize_type, dynamic_reloc_size);
}

template<class ElfTypes>
void emitElf<ElfTypes>::createSymbolVersions(Elf_Half *&symVers, char *&verneedSecData, unsigned &verneedSecSize,
                                               char *&verdefSecData,
                                               unsigned &verdefSecSize, unsigned &dynSymbolNamesLength,
                                               std::vector<std::string> &dynStrs) {

    //Add all names to the new .dynstr section
    map<string, unsigned>::iterator iter = versionNames.begin();
    for (; iter != versionNames.end(); iter++) {
        iter->second = dynSymbolNamesLength;
        dynStrs.push_back(iter->first);
        dynSymbolNamesLength += iter->first.size() + 1;
    }

    //reconstruct .gnu_version section
    symVers = (Elf_Half *) malloc(versionSymTable.size() * sizeof(Elf_Half));
    for (unsigned i = 0; i < versionSymTable.size(); i++)
        symVers[i] = versionSymTable[i];

    //reconstruct .gnu.version_r section
    verneedSecSize = 0;
    map<string, map<string, unsigned> >::iterator it = verneedEntries.begin();
    for (; it != verneedEntries.end(); it++)
        verneedSecSize += sizeof(Elf_Verneed) + sizeof(Elf_Vernaux) * it->second.size();

    verneedSecData = (char *) malloc(verneedSecSize);
    unsigned curpos = 0;
    verneednum = 0;
    std::vector<std::string>::iterator dit;
    for (dit = unversionedNeededEntries.begin(); dit != unversionedNeededEntries.end(); dit++) {
        // account for any substitutions due to rewriting a shared lib
        std::string name = obj->getDynLibSubstitution(*dit);
        // no need for self-references
        if (!(obj->name() == name)) {
            versionNames[name] = dynSymbolNamesLength;
            dynStrs.push_back(name);
            dynSymbolNamesLength += (name).size() + 1;
            addDTNeeded(name);
        }
    }
    for (it = verneedEntries.begin(); it != verneedEntries.end(); it++) {
        Elf_Verneed *verneed = reinterpret_cast<Elf_Verneed *>(verneedSecData + curpos);
        verneed->vn_version = 1;
        verneed->vn_cnt = it->second.size();
        verneed->vn_file = dynSymbolNamesLength;
        versionNames[it->first] = dynSymbolNamesLength;
        dynStrs.push_back(it->first);
        dynSymbolNamesLength += it->first.size() + 1;
        addDTNeeded(it->first);
        verneed->vn_aux = sizeof(Elf_Verneed);
        verneed->vn_next = sizeof(Elf_Verneed) + it->second.size() * sizeof(Elf_Vernaux);
        if (curpos + verneed->vn_next == verneedSecSize)
            verneed->vn_next = 0;
        verneednum++;
        int i = 0;
        for (iter = it->second.begin(); iter != it->second.end(); iter++) {
            Elf_Vernaux *vernaux = reinterpret_cast<Elf_Vernaux *>(
                    verneedSecData + curpos + verneed->vn_aux + i * sizeof(Elf_Vernaux));
            vernaux->vna_hash = elfHash(iter->first.c_str());
            vernaux->vna_flags = 0;
            vernaux->vna_other = iter->second;
            vernaux->vna_name = versionNames[iter->first];
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
    for (iter = verdefEntries.begin(); iter != verdefEntries.end(); iter++)
        verdefSecSize += sizeof(Elf_Verdef) + sizeof(Elf_Verdaux) * verdauxEntries[iter->second].size();

    verdefSecData = (char *) malloc(verdefSecSize);
    curpos = 0;
    verdefnum = 0;

    for (iter = verdefEntries.begin(); iter != verdefEntries.end(); iter++) {
        Elf_Verdef *verdef = reinterpret_cast<Elf_Verdef *>(verdefSecData + curpos);
        verdef->vd_version = 1;
        verdef->vd_flags = 0;
        verdef->vd_ndx = iter->second;
        verdef->vd_cnt = verdauxEntries[iter->second].size();
        verdef->vd_hash = elfHash(iter->first.c_str());
        verdef->vd_aux = sizeof(Elf_Verdef);
        verdef->vd_next = sizeof(Elf_Verdef) + verdauxEntries[iter->second].size() * sizeof(Elf_Verdaux);
        if (curpos + verdef->vd_next == verdefSecSize)
            verdef->vd_next = 0;
        verdefnum++;
        for (unsigned i = 0; i < verdauxEntries[iter->second].size(); i++) {
            Elf_Verdaux *verdaux = reinterpret_cast<Elf_Verdaux *>(
                    verdefSecData + curpos + verdef->vd_aux + i * sizeof(Elf_Verdaux));
            verdaux->vda_name = versionNames[verdauxEntries[iter->second][i]];
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
    Offset dynsymSize = obj->getObject()->getDynsymSize();

    Elf_Scn *scn = NULL;
    Elf_Shdr *shdr = NULL;
    while ((scn = elf_nextscn(oldElf, scn))) {
        shdr = ElfTypes::elf_getshdr(scn);
        if (obj->getObject()->getElfHashAddr() != 0 &&
            obj->getObject()->getElfHashAddr() == shdr->sh_addr) {
            Elf_Data *hashData = elf_getdata(scn, NULL);
            Elf_Word *oldHashSec = (Elf_Word *) hashData->d_buf;
            unsigned original_nbuckets, original_nchains;
            original_nbuckets = oldHashSec[0];
            original_nchains = oldHashSec[1];
            for (unsigned i = 0; i < original_nbuckets + original_nchains; i++) {
                if (oldHashSec[2 + i] != 0) {
                    originalHashEntries.push_back(oldHashSec[2 + i]);
                    //printf(" ELF HASH pushing hash entry %d \n", oldHashSec[2+i] );
                }
            }
        }

        if (obj->getObject()->getGnuHashAddr() != 0 &&
            obj->getObject()->getGnuHashAddr() == shdr->sh_addr) {
            Elf_Data *hashData = elf_getdata(scn, NULL);
            Elf_Word *oldHashSec = (Elf_Word *) hashData->d_buf;
            unsigned symndx = oldHashSec[1];
            if (dynsymSize != 0)
                for (unsigned i = symndx; i < dynsymSize; i++) {
                    originalHashEntries.push_back(i);
                    //printf(" GNU HASH pushing hash entry %d \n", i);
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
            (index < obj->getObject()->getDynsymSize())) {
            continue;
        }
        key = elfHash((*iter)->getMangledName().c_str()) % nbuckets;
        if (lastHash.find(key) != lastHash.end()) {
            hashsecData[2 + nbuckets + lastHash[key]] = i;
        }
        else {
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
        long adjust = 0;
        switch(name)
        {


            case DT_INIT:
            case DT_FINI:
            case DT_DYNINST:
                adjust = library_adjust;
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
            // If library_adjust is non-zero, then we also need to adjust springboard traps
            Region *dyninstReg = NULL;
            if (obj->findRegion(dyninstReg, ".dyninstInst") && library_adjust) {
                // The trap mapping header's in-memory offset is specified by the dynamic entry
                // We now need to get raw section data, and the raw sectiond data offset of the header
                auto header = alignas_cast<trap_mapping_header>(((char*)dyninstReg->getPtrToRawData() + value - dyninstReg->getMemOffset()));
                for (unsigned i = 0; i < header->num_entries; i++) {
                    header->traps[i].source = (void*) ((char*)header->traps[i].source + library_adjust);
                    header->traps[i].target = (void*) ((char*)header->traps[i].target + library_adjust);
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
            case DT_INIT_ARRAY:
            case DT_FINI_ARRAY:
#if defined(arch_power) && defined(arch_64bit)
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
                 * updated here.  library_adjust will be a page size if
                 * we're dealing with a library without a fixed load address.  We'll be shifting
                 * the addresses of that library by a page.
                 **/
                memcpy(dynsecData + curpos, dyns + i, sizeof(Elf_Dyn));
                dynsecData[curpos].d_un.d_ptr += library_adjust;
                dynamicSecData[dyns[i].d_tag].push_back(dynsecData + curpos);
                curpos++;
                break;
            default:
                memcpy(dynsecData + curpos, dyns + i, sizeof(Elf_Dyn));
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


template<class ElfTypes>
void emitElf<ElfTypes>::log_elferror(void (*err_func)(const char *), const char *msg) {
    const char *err = elf_errmsg(elf_errno());
    err = err ? err : "(bad elf error)";
    string str = string(err) + string(msg);
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

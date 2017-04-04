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
 * $Id: Object-elf.C,v 1.54 2008/11/03 15:19:25 jaw Exp $
 * Object-elf.C: Object class for ELF file format
 ************************************************************************/

#include "Type.h"
#include "Variable.h"
#include "Object.h"

#include "Function.h"

#include "debug.h"

#include "emitElf.h"

#include "dwarfWalker.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace Dyninst::Dwarf;
using namespace std;

#if !defined(_Object_elf_h_)
#error "Object-elf.h not #included"
#endif

#if defined(cap_dwarf)
#include "dwarf.h"

#endif

//#include "symutil.h"
#include "common/src/pathName.h"
#include "Collections.h"
#if defined(TIMED_PARSE)
#include <sys/time.h>
#endif

#include <iomanip>

#include <fstream>

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/set.hpp>

#include "SymReader.h"

using namespace boost::assign;

// add some space to avoid looking for functions in data regions
#define EXTRA_SPACE 8

bool Object::truncateLineFilenames = false;

string symt_current_func_name;
string symt_current_mangled_func_name;
Symbol *symt_current_func = NULL;

std::vector<Symbol *> opdsymbols_;

extern void print_symbols( std::vector< Symbol *>& allsymbols );
extern void print_symbol_map( dyn_hash_map< std::string, std::vector< Symbol *> > *symbols);

void (*dwarf_err_func)(const char *);   // error callback for dwarf errors

static bool pdelf_check_ehdr(Elf_X &elf)
{
    // Elf header integrity check

    // This implies a valid header is a header for an executable, a shared object
    // or a relocatable file (e.g .o) and it either has section headers or program headers

    return ( (elf.e_type() == ET_EXEC || elf.e_type() == ET_DYN || elf.e_type() == ET_REL ) &&
             (   ((elf.e_shoff() != 0) && (elf.e_shnum() != 0))
                 || ((elf.e_phoff() != 0) && (elf.e_phnum() != 0))
             )
    );
}

const char *pdelf_get_shnames(Elf_X *elf)
{
    const char *result = NULL;
    size_t shstrndx = elf->e_shstrndx();

    Elf_X_Shdr &shstrscn = elf->get_shdr(shstrndx);
    if (shstrscn.isValid()) {
        Elf_X_Data shstrdata = shstrscn.get_data();
        if (shstrdata.isValid())
            result = shstrdata.get_string();
    }
    return result;
}

//
// SectionHeaderSortFunction
//
// Compare function for use with the Vector<T> sort method.
//
struct  SectionHeaderSortFunction: public binary_function<Elf_X_Shdr *, Elf_X_Shdr *, bool>
{
    bool operator()(Elf_X_Shdr *hdr1, Elf_X_Shdr *hdr2) {
        return (hdr1->sh_addr() < hdr2->sh_addr());
    }
};

Region::perm_t getSegmentPerms(unsigned long flags){
    if(flags == 7)
        return Region::RP_RWX;
    else if(flags == 6)
        return Region::RP_RW;
    else if(flags == 5)
        return Region::RP_RX;
    else
        return Region::RP_R;
}

Region::RegionType getSegmentType(unsigned long type, unsigned long flags)
{
    if(type == PT_DYNAMIC)
        return Region::RT_DYNAMIC;
    if(flags == 7)
        return Region::RT_TEXTDATA;
    if(flags == 5)
        return Region::RT_TEXT;
    if(flags == 6)
        return Region::RT_DATA;
    return Region::RT_OTHER;
}

/* binary search to find the section starting at a particular address */
Elf_X_Shdr *Object::getRegionHdrByAddr(Offset addr)
{
    unsigned end = allRegionHdrs.size() - 1, start = 0;
    unsigned mid = 0;
    while(start < end) {
        mid = start + (end-start)/2;
        if(allRegionHdrs[mid]->sh_addr() == addr)
            return allRegionHdrs[mid];
        else if(allRegionHdrs[mid]->sh_addr() < addr)
            start = mid + 1;
        else
            end = mid;
    }
    if(allRegionHdrs[start]->sh_addr() == addr)
        return allRegionHdrs[start];
    return NULL;
}

/* binary search to find the index into the RegionHdrs vector
   of the section starting at a partidular address*/
int Object::getRegionHdrIndexByAddr(Offset addr)
{
    int end = allRegionHdrs.size() - 1, start = 0;
    int mid = 0;
    while(start < end) {
        mid = start + (end-start)/2;
        if(allRegionHdrs[mid]->sh_addr() == addr)
            return mid;
        else if(allRegionHdrs[mid]->sh_addr() < addr)
            start = mid + 1;
        else
            end = mid;
    }
    if(allRegionHdrs[start]->sh_addr() == addr)
        return start;
    return -1;
}

Elf_X_Shdr *Object::getRegionHdrByIndex(unsigned index)
{
    if (index >= allRegionHdrs.size())
        return NULL;
    return allRegionHdrs[index];
}

/* Check if there is a section which belongs to the segment and update permissions of that section.
 * Return value indicates whether the segment has to be added to the list of regions*/
bool Object::isRegionPresent(Offset segmentStart, Offset segmentSize, unsigned segPerms){
    bool present = false;
    for(unsigned i = 0; i < regions_.size() ;i++){
        if((regions_[i]->getDiskOffset() >= segmentStart) &&
           ((regions_[i]->getDiskOffset()+regions_[i]->getDiskSize()) <= (segmentStart+segmentSize))){
            present = true;
            regions_[i]->setRegionPermissions(getSegmentPerms(segPerms));
        }

    }
    return present;
}

Region::perm_t getRegionPerms(unsigned long flags){
    if((flags & SHF_WRITE) && !(flags & SHF_EXECINSTR))
        return Region::RP_RW;
    else if(!(flags & SHF_WRITE) && (flags & SHF_EXECINSTR))
        return Region::RP_RX;
    else if((flags & SHF_WRITE) && (flags & SHF_EXECINSTR))
        return Region::RP_RWX;
    else
        return Region::RP_R;
}

Region::RegionType getRegionType(unsigned long type, unsigned long flags, const char *reg_name){
    switch(type){
        case SHT_SYMTAB:
        case SHT_DYNSYM:
            return Region::RT_SYMTAB;
        case SHT_STRTAB:
            return Region::RT_STRTAB;
        case SHT_REL:
            return Region::RT_REL;
        case SHT_RELA:
            return Region::RT_RELA;
        case SHT_NOBITS:
            //Darn it, Linux/PPC has the PLT as a NOBITS.  Can't just default
            // call this bss
            if (strcmp(reg_name, ".plt") == 0)
                return Region::RT_OTHER;
            else
                return Region::RT_BSS;
        case SHT_PROGBITS:
            if((flags & SHF_EXECINSTR) && (flags & SHF_WRITE))
                return Region::RT_TEXTDATA;
            else if(flags & SHF_EXECINSTR)
                return Region::RT_TEXT;
            else
                return Region::RT_DATA;
        case SHT_DYNAMIC:
            return Region::RT_DYNAMIC;
        case SHT_HASH:
            return Region::RT_HASH;
        case SHT_GNU_versym:
            return Region::RT_SYMVERSIONS;
        case SHT_GNU_verdef:
            return Region::RT_SYMVERDEF;
        case SHT_GNU_verneed:
            return Region::RT_SYMVERNEEDED;
        default:
            return Region::RT_OTHER;
    }
}

static Region::RegionType getRelTypeByElfMachine(Elf_X *localHdr) {
    Region::RegionType ret;
    switch(localHdr->e_machine()) {
        case EM_SPARC:
        case EM_SPARC32PLUS:
        case EM_SPARCV9:
        case EM_PPC:
        case EM_PPC64:
        case EM_X86_64:
        case EM_IA_64:
            ret = Region::RT_RELA;
            break;
        default:
            ret = Region::RT_REL;
            break;
    }
    return ret;
}

const char* EDITED_TEXT_NAME = ".edited.text";
// const char* INIT_NAME        = ".init";
const char *INTERP_NAME      = ".interp";
const char* FINI_NAME        = ".fini";
const char* TEXT_NAME        = ".text";
const char* BSS_NAME         = ".bss";
const char* SYMTAB_NAME      = ".symtab";
const char* STRTAB_NAME      = ".strtab";
const char* STAB_NAME        = ".stab";
const char* STABSTR_NAME     = ".stabstr";
const char* STAB_INDX_NAME   = ".stab.index";
const char* STABSTR_INDX_NAME= ".stab.indexstr";
const char* COMMENT_NAME= ".comment";
const char* OPD_NAME         = ".opd"; // PPC64 Official Procedure Descriptors
// sections from dynamic executables and shared objects
const char* PLT_NAME         = ".plt";
#if defined(os_vxworks)
const char* REL_PLT_NAME     = ".rela.text";
#else
const char* REL_PLT_NAME     = ".rela.plt"; // sparc-solaris
#endif
const char* REL_PLT_NAME2    = ".rel.plt";  // x86-solaris
const char* GOT_NAME         = ".got";
const char* DYNSYM_NAME      = ".dynsym";
const char* DYNSTR_NAME      = ".dynstr";
const char* DATA_NAME        = ".data";
const char* RO_DATA_NAME     = ".ro_data";  // mips
const char* DYNAMIC_NAME     = ".dynamic";
const char* EH_FRAME_NAME    = ".eh_frame";
const char* EXCEPT_NAME      = ".gcc_except_table";
const char* EXCEPT_NAME_ALT  = ".except_table";


extern template
class Dyninst::SymtabAPI::emitElf<ElfTypes32>;

extern template
class Dyninst::SymtabAPI::emitElf<ElfTypes64>;

set<string> debugInfoSections = list_of(string(SYMTAB_NAME))
        (string(STRTAB_NAME));

// loaded_elf(): populate elf section pointers
// for EEL rewritten code, also populate "code_*_" members
bool Object::loaded_elf(Offset& txtaddr, Offset& dataddr,
                        Elf_X_Shdr* &bssscnp,
                        Elf_X_Shdr*& symscnp, Elf_X_Shdr*& strscnp,
                        Elf_X_Shdr*& stabscnp, Elf_X_Shdr*& stabstrscnp,
                        Elf_X_Shdr*& stabs_indxcnp, Elf_X_Shdr*& stabstrs_indxcnp,
                        Elf_X_Shdr*& rel_plt_scnp, Elf_X_Shdr*& plt_scnp,
                        Elf_X_Shdr*& got_scnp, Elf_X_Shdr*& dynsym_scnp,
                        Elf_X_Shdr*& dynstr_scnp, Elf_X_Shdr* &dynamic_scnp,
                        Elf_X_Shdr*& eh_frame, Elf_X_Shdr*& gcc_except,
                        Elf_X_Shdr *& interp_scnp, Elf_X_Shdr *& opd_scnp,
                        bool)
{
    std::map<std::string, int> secnNameMap;
    dwarf_err_func  = err_func_;
    entryAddress_ = elfHdr->e_entry();

    no_of_sections_ = elfHdr->e_shnum();

    // ".shstrtab" section: string table for section header names
    const char *shnames = pdelf_get_shnames(elfHdr);
    if (shnames == NULL) {
        log_elferror(err_func_, ".shstrtab section");
        //return false;
    }

    // initialize Object members

    text_addr_ = 0; //ccw 23 jan 2002
    text_size_ = 0; //for determining if a mutation
    //falls within the text section
    //of a shared library

    elf_hash_addr_ = 0;
    gnu_hash_addr_ = 0;

    dynamic_offset_ = 0;
    dynamic_addr_ = 0;
    dynsym_addr_ = 0;
    dynsym_size_ = 0;
    dynstr_addr_ = 0;
    init_addr_ = 0;
    fini_addr_ = 0;
    got_addr_ = 0;
    got_size_ = 0;
    plt_addr_ = 0;
    plt_size_ = 0;
    symtab_addr_ = 0;
    strtab_addr_ = 0;
#if defined (ppc32_linux) || defined(ppc32_bgp)
    plt_entry_size_ = 8;
  rel_plt_entry_size_ = 8;
#else
    plt_entry_size_ = 0;
    rel_plt_entry_size_ = 0;
#endif
    rel_plt_addr_ = 0;
    rel_plt_size_ = 0;
    rel_addr_ = 0;
    rel_size_ = 0;
    rel_entry_size_ = 0;
    stab_off_ = 0;
    stab_size_ = 0;
    stabstr_off_ = 0;
    stab_indx_off_ = 0;
    stab_indx_size_ = 0;
    stabstr_indx_off_ = 0;
    dwarvenDebugInfo = false;

    txtaddr = 0;

    set<string> sectionsInOriginalBinary;

#if defined(TIMED_PARSE)
    struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

    // Find pointer to dynamic section and interpreter section
    bool foundInterp = false;
    unsigned phdr_max_count = elfHdr->e_phnum();

    for (unsigned i = 0; i < phdr_max_count; i++) {
        Elf_X_Phdr &elfPhdr = elfHdr->get_phdr(i);
        if(elfPhdr.p_type() == PT_DYNAMIC){
            dynamic_offset_ = elfPhdr.p_offset();
            dynamic_size_ = elfPhdr.p_memsz();
        } else if (elfPhdr.p_type() == PT_INTERP) {
            foundInterp = true;
        }
    }

    if (elfHdr->e_type() == ET_DYN || foundInterp || elfHdr->e_type() == ET_REL) {
        is_static_binary_ = false;
    } else {
        is_static_binary_ = true;
    }

    bool foundDynamicSection = false;
    int dynamic_section_index = -1;
    unsigned int dynamic_section_type = 0;
    size_t dynamic_section_size = 0;
    for (int i = 0; i < elfHdr->e_shnum();++i) {
        Elf_X_Shdr &scn = elfHdr->get_shdr(i);
        if (! scn.isValid()) {  // section is malformed
            continue;
        }
        if ((dynamic_offset_ !=0) && (scn.sh_offset() == dynamic_offset_)) {
            if (!foundDynamicSection) {
                dynamic_section_index = i;
                dynamic_section_size = scn.sh_size();
                dynamic_section_type = scn.sh_type();
                foundDynamicSection = true;
            } else {
                // If there are two or more sections with the same offset as the dynamic_offset,
                // use other fields to chose which one the the dynamic section
                if (dynamic_section_size == dynamic_size_ && dynamic_section_type == SHT_DYNAMIC) {
                    // We already have the right section
                } else if ((scn.sh_size() == dynamic_size_ && scn.sh_type() == SHT_DYNAMIC) ||
                           (scn.sh_size() == dynamic_size_)) {
                    // This section is a better match to be the dynamic section
                    // than the previous one!
                    dynamic_section_index = i;
                    dynamic_section_size = scn.sh_size();
                    dynamic_section_type = scn.sh_type();
                }
            }
        }
    }

    if (dynamic_section_index != -1) {
        Elf_X_Shdr &scn = elfHdr->get_shdr(dynamic_section_index);
        Elf_X_Data data = scn.get_data();
        Elf_X_Dyn dynsecData = data.get_dyn();
        // Ubuntu 12.04 - we have debug files with NOBITS dynamic sections.
        if (dynsecData.isValid()) {
            for (unsigned j = 0; j < dynsecData.count(); ++j) {
                switch (dynsecData.d_tag(j)) {
                    case DT_REL:
                        hasReldyn_ = true;
                        secAddrTagMapping[dynsecData.d_ptr(j)] = dynsecData.d_tag(j);
                        break;
                    case DT_RELA:
                        hasReladyn_ = true;
                        secAddrTagMapping[dynsecData.d_ptr(j)] = dynsecData.d_tag(j);
                        break;
                    case DT_JMPREL:
                        secAddrTagMapping[dynsecData.d_ptr(j)] = dynsecData.d_tag(j);
                        break;
                    case DT_SYMTAB:
                    case DT_STRTAB:
                    case DT_VERSYM:
                    case DT_VERNEED:
                        secAddrTagMapping[dynsecData.d_ptr(j)] = dynsecData.d_tag(j);
                        break;
                    case DT_HASH:
                        secAddrTagMapping[dynsecData.d_ptr(j)] = dynsecData.d_tag(j);
                        elf_hash_addr_ = dynsecData.d_ptr(j);
                        break;
                    case  0x6ffffef5: //DT_GNU_HASH (not defined on all platforms)
                        secAddrTagMapping[dynsecData.d_ptr(j)] = dynsecData.d_tag(j);
                        gnu_hash_addr_ = dynsecData.d_ptr(j);
                        break;
                    case DT_PLTGOT:
                        secAddrTagMapping[dynsecData.d_ptr(j)] = dynsecData.d_tag(j);
                        break;
                    case DT_RELSZ:
                        secTagSizeMapping[DT_REL] = dynsecData.d_val(j);
                        break;
                    case DT_RELASZ:
                        secTagSizeMapping[DT_RELA] = dynsecData.d_val(j);
                        break;
                    case DT_PLTRELSZ:
                        secTagSizeMapping[DT_JMPREL] = dynsecData.d_val(j);
                        break;
                    case DT_STRSZ:
                        secTagSizeMapping[DT_STRTAB] = dynsecData.d_val(j);
                        break;
                    case DT_PLTREL:
                        if (dynsecData.d_val(j) == DT_REL)
                            hasRelplt_ = true;
                        else if (dynsecData.d_val(j) == DT_RELA)
                            hasRelaplt_ = true;
                        break;

                }
            }
        }
        dyn_hash_map<Offset, int>::iterator it = secAddrTagMapping.begin();
        while (it != secAddrTagMapping.end()) {
            int tag = it->second;
            switch (tag) {
                // Only sections with these tags are moved in the new rewritten binary
                case DT_REL:
                case DT_RELA:
                case DT_JMPREL:
                case DT_SYMTAB:
                case DT_STRTAB:
                case DT_VERSYM:
                case DT_VERNEED:
                case DT_HASH:
                case  0x6ffffef5: // DT_GNU_HASH (not defined on all platforms)

                    if (secTagSizeMapping.find(tag) != secTagSizeMapping.end()) {
                        vector<Offset> row;
                        row.push_back(it->first);
                        row.push_back(it->first+ secTagSizeMapping[tag]);
                        moveSecAddrRange.push_back(row);
                    } else {
                        vector<Offset> row;
                        row.push_back(it->first);
                        row.push_back(it->first);
                        moveSecAddrRange.push_back(row);
                    }
                    break;
            }
            it++;
        }
    }

    isBlueGeneP_ = false;
    isBlueGeneQ_ = false;

    hasNoteSection_ = false;

    const char *shnamesForDebugInfo = NULL;
    unsigned int elfHdrDbg_numSections = 0;
    unsigned int elfHdr_numSections = elfHdr->e_shnum();
    Elf_X *elfHdrDbg = dwarf->debugLinkFile();
    if (elfHdrDbg) {
        shnamesForDebugInfo = pdelf_get_shnames(elfHdrDbg);
        if (shnamesForDebugInfo == NULL) {
            log_elferror(err_func_, ".shstrtab section");
        }
        elfHdrDbg_numSections = elfHdrDbg->e_shnum();
    }

    for (unsigned int i = 0; i < elfHdr_numSections + elfHdrDbg_numSections; i++) {
        const char *name;

        bool isFromDebugFile = (i >= elfHdr_numSections);
        Elf_X_Shdr &scn = (!isFromDebugFile) ? elfHdr->get_shdr(i) :
                          elfHdrDbg->get_shdr(i - elfHdr_numSections);
        if (! scn.isValid()) { // section is malformed
            continue;
        }
        Elf_X_Shdr *scnp = &scn;

        if (!isFromDebugFile) {
            name = &shnames[scn.sh_name()];
            sectionsInOriginalBinary.insert(string(name));

            if(scn.sh_flags() & SHF_ALLOC) {
                DbgAddrConversion_t orig;
                orig.name = std::string(name);
                orig.orig_offset = scn.sh_addr();
                secnNameMap[orig.name] = DebugSectionMap.size();
                DebugSectionMap.push_back(orig);
            }
        }
        else {
            if (!shnamesForDebugInfo)
                break;
            name = &shnamesForDebugInfo[scn.sh_name()];

            std::string sname(name);
            std::map<std::string, int>::iterator s = secnNameMap.find(sname);
            if (s != secnNameMap.end()) {
                int i = (*s).second;
                DebugSectionMap[i].dbg_offset = scn.sh_addr();
                DebugSectionMap[i].dbg_size = scn.sh_size();
            }
            scn.setDebugFile(true);

            if (scn.sh_type() == SHT_NOBITS) {
                continue;
            }
        }

        if(scn.sh_type() == SHT_NOTE) {
            hasNoteSection_ = true;
        }


        //If the section appears in the memory image of a process address is given by
        // sh_addr()
        //otherwise this is zero and sh_offset() gives the offset to the first byte
        // in section.
        if (!scn.isFromDebugFile()) {
            allRegionHdrs.push_back(&scn);
            Elf_X_Data data = scn.get_data();
            if(strcmp(name, OPD_NAME) == 0)
            {
                data.d_type(ELF_T_XWORD);
                data.xlatetom(elfHdr->e_endian() ? ELFDATA2MSB : ELFDATA2LSB);
            }
            if(scn.sh_flags() & SHF_ALLOC) {
                // .bss, etc. have a disk size of 0
                unsigned long diskSize  = (scn.sh_type() == SHT_NOBITS) ? 0 : scn.sh_size();

                Region *reg = new Region(i, name, scn.sh_addr(), diskSize,
                                         scn.sh_addr(), scn.sh_size(),
                                         ((char*)data.d_buf()),
                                         getRegionPerms(scn.sh_flags()),
                                         getRegionType(scn.sh_type(),
                                                       scn.sh_flags(),
                                                       name),
                                         true, ((scn.sh_flags() & SHF_TLS) != 0),
                                         scn.sh_addralign());
                reg->setFileOffset(scn.sh_offset());
                regions_.push_back(reg);
            }
            else {
                Region *reg = new Region(i, name, scn.sh_addr(), scn.sh_size(), 0, 0,
                                         ((char*)data.d_buf()),
                                         getRegionPerms(scn.sh_flags()),
                                         getRegionType(scn.sh_type(),
                                                       scn.sh_flags(),
                                                       name),
                                         false, ((scn.sh_flags() & SHF_TLS) != 0),
                                         scn.sh_addralign());

                reg->setFileOffset(scn.sh_offset());
                regions_.push_back(reg);
            }
        }

        // section-specific processing
        if (P_strcmp(name, EDITED_TEXT_NAME) == 0) {
            // EEL rewritten executable
            EEL = true;
            if (txtaddr == 0)
                txtaddr = scn.sh_addr();
            char *file_ptr = (char *)mf->base_addr();
            code_ptr_ = (char *)(void*)&file_ptr[scn.sh_offset() - EXTRA_SPACE];
            code_off_ = scn.sh_addr() - EXTRA_SPACE;
            code_len_ = scn.sh_size() + EXTRA_SPACE;
        }

        if (strcmp(name, TEXT_NAME) == 0) {
            text_addr_ = scn.sh_addr();
            text_size_ = scn.sh_size();

            if (txtaddr == 0)
                txtaddr = scn.sh_addr();

            // .o's don't have program headers, so these members need
            // to be populated here
            if( !elfHdr->e_phnum() && !code_len_) {
                // Populate code members
                code_ptr_ = reinterpret_cast<char *>(scn.get_data().d_buf());
                code_off_ = scn.sh_offset();
                code_len_ = scn.sh_size();
            }
        }
            /* The following sections help us find the PH entry that
       encompasses the data region. */
        else if (strcmp(name, DATA_NAME) == 0) {
            dataddr = scn.sh_addr();

            // .o's don't have program headers, so these members need
            // to be populated here
            if( !elfHdr->e_phnum() && !data_len_) {
                // Populate data members
                data_ptr_ = reinterpret_cast<char *>(scn.get_data().d_buf());
                data_off_ = scn.sh_offset();
                data_len_ = scn.sh_size();
            }
        }
        else if (strcmp(name, RO_DATA_NAME) == 0) {
            if (!dataddr) dataddr = scn.sh_addr();
        }
        else if (strcmp(name, GOT_NAME) == 0) {
            got_scnp = scnp;
            got_addr_ = scn.sh_addr();
            got_size_ = scn.sh_size();
            if (!dataddr) dataddr = scn.sh_addr();
        }
        else if (strcmp(name, BSS_NAME) == 0) {
            bssscnp = scnp;
            if (!dataddr) dataddr = scn.sh_addr();
        }
            /* End data region search */
            /*else if (strcmp( name, FINI_NAME) == 0) {
      fini_addr_ = scn.sh_addr();
      }*/
        else if (strcmp(name, SYMTAB_NAME) == 0) {
            if (!symscnp) {
                symscnp = scnp;
                symtab_addr_ = scn.sh_addr();
            }
        }
        else if (strcmp(name, STRTAB_NAME) == 0) {
            if (!strscnp) {
                strscnp = scnp;
                strtab_addr_ = scn.sh_addr();
            }
        } else if (strcmp(name, STAB_INDX_NAME) == 0) {
            stabs_indxcnp = scnp;
            stab_indx_off_ = scn.sh_offset();
            stab_indx_size_ = scn.sh_size();
        } else if (strcmp(name, STABSTR_INDX_NAME) == 0) {
            stabstrs_indxcnp = scnp;
            stabstr_indx_off_ = scn.sh_offset();
        } else if (strcmp(name, STAB_NAME) == 0) {
            stabscnp = scnp;
            stab_off_ = scn.sh_offset();
            stab_size_ = scn.sh_size();
        } else if (strcmp(name, STABSTR_NAME) == 0) {
            stabstrscnp = scnp;
            stabstr_off_ = scn.sh_offset();
        }
#if defined(os_vxworks)
            else if ((strcmp(name, REL_PLT_NAME) == 0) ||
	     (strcmp(name, REL_PLT_NAME2) == 0)) {
      rel_plt_scnp = scnp;
      rel_plt_addr_ = scn.sh_addr();
      rel_plt_size_ = scn.sh_size();
      rel_plt_entry_size_ = scn.sh_entsize();
    }
#else
        else if ((secAddrTagMapping.find(scn.sh_addr()) != secAddrTagMapping.end() ) &&
                 secAddrTagMapping[scn.sh_addr()] == DT_JMPREL ) {
            rel_plt_scnp = scnp;
            rel_plt_addr_ = scn.sh_addr();
            rel_plt_size_ = scn.sh_size();
            rel_plt_entry_size_ = scn.sh_entsize();
        }
#endif
        else if (strcmp(name, OPD_NAME) == 0) {
            opd_scnp = scnp;
            opd_addr_ = scn.sh_addr();
            opd_size_ = scn.sh_size();
        }
        else if (strcmp(name, PLT_NAME) == 0) {
            plt_scnp = scnp;
            plt_addr_ = scn.sh_addr();
            plt_size_ = scn.sh_size();
            if(getArch() == Dyninst::Arch_x86 || getArch() == Dyninst::Arch_x86_64) {
                //
                // On x86, the GNU linker purposefully sets the PLT
                // table entry size to an incorrect value to be
                // compatible with the UnixWare linker.  (See the comment
                // in the elf_i386_finish_dynamic_sections function of
                // the BFD library.)  The GNU linker sets this value to 4,
                // when it should be 16.
                //
                // I see no good way to determine this value from the
                // ELF section header information.  We can either (a) hard-code
                // the value that is used in the BFD library, or (b) compute
                // it by dividing the size of the PLT by the number of entries
                // we think should be in the PLT.  I'm not certain, but I
                // believe the PLT and the .rel.plt section should have the
                // same number of "real" entries (the x86 PLT has one extra entry
                // at the beginning).
                //
                // This code is applicable to any x86 system that uses the
                // GNU linker.  We currently only support Linux on x86 - if
                // we start supporting some other x86 OS that uses the GNU
                // linker in the future, it should be enabled for that platform as well.
                // Note that this problem does not affect the non-x86 platforms
                // that might use the GNU linker.  For example, programs linked
                // with gld on SPARC Solaris have the correct PLT entry size.
                //
                // Another potential headache in the future is if we support
                // some other x86 platform that has both the GNU linker and
                // some other linker.  (Does BSD fall into this category?)
                // If the two linkers set the entry size differently, we may
                // need to re-evaluate this code.
                //
                //plt_entry_size_ = plt_size_ / ((rel_plt_size_ / rel_plt_entry_size_) + 1);
                plt_entry_size_ = 16;
            }
            else
            {
                plt_entry_size_ = scn.sh_entsize();
                if(getArch() == Dyninst::Arch_ppc32)
                {
                    if (scn.sh_flags() & SHF_EXECINSTR) {
                        // Old style executable PLT
                        if (!plt_entry_size_)
                            plt_entry_size_ = 8;
                        else {
                            if (plt_entry_size_ != 8)
                                create_printf("%s[%d]:  weird plt_entry_size_ is %d, not 8\n",
                                              FILE__, __LINE__, plt_entry_size_);
                        }

                    } else {
                        // New style secure PLT
                        plt_entry_size_ = 16;
                    }
                }
            }
        } else if (strcmp(name, COMMENT_NAME) == 0) {
            /* comment section is a sequence of NULL-terminated strings.
	 We want to concatenate them and search for BGP to determine
	 if the binary is built for BGP compute nodes */

            Elf_X_Data data = scn.get_data();

            unsigned int index = 0;
            size_t size = data.d_size();
            char *buf = (char *) data.d_buf();
            while (buf && (index < size))
            {
                string comment = buf+index;
                size_t pos_p = comment.find("BGP");
                size_t pos_q = comment.find("BGQ");
                if (pos_p !=string::npos) {
                    isBlueGeneP_ = true;
                    break;
                } else if (pos_q !=string::npos) {
                    isBlueGeneQ_ = true;
                    break;
                }
                index += comment.size();
                if (comment.size() == 0) { // Skip NULL characters in the comment section
                    index ++;
                }
            }
        }

        else if ((secAddrTagMapping.find(scn.sh_addr()) != secAddrTagMapping.end() ) &&
                 secAddrTagMapping[scn.sh_addr()] == DT_SYMTAB ) {
            is_dynamic_ = true;
            dynsym_scnp = scnp;
            dynsym_addr_ = scn.sh_addr();
            dynsym_size_ = scn.sh_size()/scn.sh_entsize();
        }
        else if ((secAddrTagMapping.find(scn.sh_addr()) != secAddrTagMapping.end() ) &&
                 secAddrTagMapping[scn.sh_addr()] == DT_STRTAB ) {
            dynstr_scnp = scnp;
            dynstr_addr_ = scn.sh_addr();
        }
        else if (strcmp(name, ".debug_info") == 0) {
            dwarvenDebugInfo = true;
        }
        else if (strcmp(name, EH_FRAME_NAME) == 0) {
            eh_frame = scnp;
        }
        else if ((strcmp(name, EXCEPT_NAME) == 0) ||
                 (strcmp(name, EXCEPT_NAME_ALT) == 0)) {
            gcc_except = scnp;
        }
        else if (strcmp(name, INTERP_NAME) == 0) {
            interp_scnp = scnp;
        }
        else if ((int) i == dynamic_section_index) {
            dynamic_scnp = scnp;
            dynamic_addr_ = scn.sh_addr();
        }
    }

    if(!symscnp || !strscnp) {
        isStripped = true;
        if(dynsym_scnp && dynstr_scnp){
            symscnp = dynsym_scnp;
            strscnp = dynstr_scnp;
        }
    }

    loadAddress_ = 0x0;
#if defined(os_linux) || defined(os_freebsd)
    /**
   * If the virtual address of the first PT_LOAD element in the
   * program table is 0, Linux loads the shared object into any
   * free spot into the address space.  If the virtual address is
   * non-zero, it gets loaded only at that address.
   **/
    for (unsigned i = 0; i < elfHdr->e_phnum() && !loadAddress_; ++i) {
        Elf_X_Phdr &phdr = elfHdr->get_phdr(i);

        if (phdr.p_type() == PT_LOAD) {
            loadAddress_ = phdr.p_vaddr();
            break;
        }
    }
#endif

    // save a copy of region headers, maintaining order in section header table
    allRegionHdrsByShndx = allRegionHdrs;

    // sort the section headers by base address
    sort(allRegionHdrs.begin(), allRegionHdrs.end(), SectionHeaderSortFunction());

    for (unsigned j = 0 ; j < regions_.size() ; j++) {
        if (secAddrTagMapping.find(regions_[j]->getDiskOffset()) != secAddrTagMapping.end()) {
            secTagRegionMapping[secAddrTagMapping[regions_[j]->getDiskOffset()]] = regions_[j];
        }
    }

#if defined(TIMED_PARSE)
    struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << "main loop of loaded elf took "<<dursecs <<" msecs" << endl;
#endif

    if (!dataddr || !symscnp || !strscnp) {
        //log_elferror(err_func_, "no text/bss/symbol/string section");
    }
    //if (addressWidth_nbytes == 8) bperr( ">>> 64-bit loaded_elf() successful\n");
    return true;
}

bool Object::is_offset_in_plt(Offset offset) const
{
    return (offset > plt_addr_ && offset < plt_addr_ + plt_size_);
}

void Object::parseDynamic(Elf_X_Shdr *&dyn_scnp, Elf_X_Shdr *&dynsym_scnp,
                          Elf_X_Shdr *&dynstr_scnp)
{
    Elf_X_Data data = dyn_scnp->get_data();
    Elf_X_Dyn dyns = data.get_dyn();
    int rel_scnp_index = -1;

    for (unsigned i = 0; i < dyns.count(); ++i) {
        switch(dyns.d_tag(i)) {
            case DT_REL:
            case DT_RELA:
                /*found Relocation section*/
                rel_addr_ = (Offset) dyns.d_ptr(i);
                rel_scnp_index = getRegionHdrIndexByAddr(dyns.d_ptr(i));
                break;
            case DT_JMPREL:
                rel_plt_addr_ = (Offset) dyns.d_ptr(i);
                break;
            case DT_PLTRELSZ:
                rel_plt_size_ = dyns.d_val(i) ;
                break;
            case DT_RELSZ:
            case DT_RELASZ:
                rel_size_ = dyns.d_val(i) ;
                break;
            case DT_RELENT:
            case DT_RELAENT:
                rel_entry_size_ = dyns.d_val(i);
                /* Maybe */
                //rel_plt_entry_size_ = dyns.d_val(i);
                break;
            case DT_INIT:
                init_addr_ = dyns.d_val(i);
                break;
            case DT_FINI:
                fini_addr_ = dyns.d_val(i);
                break;
            default:
                break;
        }
    }
    if (rel_scnp_index  != -1)
        get_relocationDyn_entries(rel_scnp_index, dynsym_scnp, dynstr_scnp);
}

/* parse relocations for the sections represented by DT_REL/DT_RELA in
 * the dynamic section. This section is the one we would want to emit
 */
bool Object::get_relocationDyn_entries( unsigned rel_scnp_index,
                                        Elf_X_Shdr *&dynsym_scnp,
                                        Elf_X_Shdr *&dynstr_scnp )
{
    Elf_X_Data symdata = dynsym_scnp->get_data();
    Elf_X_Data strdata = dynstr_scnp->get_data();
    Elf_X_Shdr* rel_scnp = NULL;
    if( !symdata.isValid() || !strdata.isValid())
        return false;
    const char *strs = strdata.get_string();
    Elf_X_Sym sym = symdata.get_sym();

    unsigned num_rel_entries_found = 0;
    unsigned num_rel_entries = rel_size_/rel_entry_size_;

    if (rel_addr_ + rel_size_ > rel_plt_addr_){
        // REL/RELA section overlaps with REL PLT section
        num_rel_entries = (rel_plt_addr_-rel_addr_)/rel_entry_size_;
    }
    while (num_rel_entries_found != num_rel_entries) {
        rel_scnp = getRegionHdrByIndex(rel_scnp_index++);
        Elf_X_Data reldata = rel_scnp->get_data();

        if( ! reldata.isValid()) return false;
        Elf_X_Rel rel = reldata.get_rel();
        Elf_X_Rela rela = reldata.get_rela();

        if (sym.isValid() && (rel.isValid() || rela.isValid()) && strs) {
            /* Iterate over the entries. */
            for( u_int i = 0; i < (reldata.d_size()/rel_entry_size_); ++i ) {
                num_rel_entries_found++;
                long offset;
                long addend;
                long index;
                unsigned long type;
                Region::RegionType rtype = Region::RT_REL;

                switch (reldata.d_type()) {
                    case ELF_T_REL:
                        offset = rel.r_offset(i);
                        addend = 0;
                        index = rel.R_SYM(i);
                        type = rel.R_TYPE(i);
                        break;

                    case ELF_T_RELA:
                        offset = rela.r_offset(i);
                        addend = rela.r_addend(i);
                        index = rela.R_SYM(i);
                        type = rela.R_TYPE(i);
                        rtype = Region::RT_RELA;
                        break;

                    default:
                        // We should never reach this case.
                        return false;
                };
                relocationEntry re( offset, string( &strs[ sym.st_name(index) ] ), NULL, type );
                re.setAddend(addend);
                re.setRegionType(rtype);
                if(symbols_.find(&strs[ sym.st_name(index)]) != symbols_.end()){
                    vector<Symbol *> &syms = symbols_[&strs[ sym.st_name(index)]];
                    for (vector<Symbol *>::iterator i = syms.begin(); i != syms.end(); i++) {
                        if (!(*i)->isInDynSymtab())
                            continue;
                        re.addDynSym(*i);
                        break;
                    }
                }

                relocation_table_.push_back(re);
            }
        } else {
            return false;
        }

    }
    return true;
}

bool Object::get_relocation_entries( Elf_X_Shdr *&rel_plt_scnp,
                                     Elf_X_Shdr *&dynsym_scnp,
                                     Elf_X_Shdr *&dynstr_scnp )
{
    if (rel_plt_size_ && rel_plt_addr_) {
        Elf_X_Data reldata = rel_plt_scnp->get_data();
        Elf_X_Data symdata = dynsym_scnp->get_data();
        Elf_X_Data strdata = dynstr_scnp->get_data();

        if( reldata.isValid() && symdata.isValid() && strdata.isValid() ) {
            Offset next_plt_entry_addr = plt_addr_;
        if(getArch() == Dyninst::Arch_x86 || getArch() == Dyninst::Arch_x86_64)
        {
            next_plt_entry_addr += plt_entry_size_;  // 1st PLT entry is special
        }
        else if(getArch()==Dyninst::Arch_ppc32)
        {
            bool extraStubs = false;

            // Sanity check.
            if (!plt_entry_size_) {
                create_printf("%s[%d]:  FIXME:  plt_entry_size not established\n", FILE__, __LINE__);
                plt_entry_size_ = 8;
            }

            if (plt_entry_size_ == 8) {
                // Old style executable PLT section
                next_plt_entry_addr += 9*plt_entry_size_;  // 1st 9 PLT entries are special

            } else if (plt_entry_size_ == 16) {
                // New style secure PLT
                Region *plt = NULL, *dynamic = NULL,
                        *got = NULL, *glink = NULL;
                unsigned int glink_addr = 0;
                unsigned int stub_addr = 0;

                // Find the GLINK section.  See ppc_elf_get_synthetic_symtab() in
                // bfd/elf32-ppc.c of GNU's binutils for more information.

                for (unsigned iter = 0; iter < regions_.size(); ++iter) {
                    std::string name = regions_[iter]->getRegionName();
                    if (name == PLT_NAME) plt = regions_[iter];
                    // else if (name == REL_PLT_NAME) relplt = regions_[iter];
                    else if (name == DYNAMIC_NAME) dynamic = regions_[iter];
                    else if (name == GOT_NAME) got = regions_[iter];
                }

                // Rely on .dynamic section for prelinked binaries.
                if (dynamic != NULL) {
                    Elf32_Dyn *dyn = (Elf32_Dyn *)dynamic->getPtrToRawData();
                    unsigned int count = dynamic->getMemSize() / sizeof(Elf32_Dyn);

                    for (unsigned int i = 0; i < count; ++i) {
                        // Use DT_LOPROC instead of DT_PPC_GOT to circumvent problems
                        // caused by early versions of libelf where DT_PPC_GOT has
                        // yet to be defined.
                        if (dyn[i].d_tag == DT_LOPROC) {
                            unsigned int g_o_t = dyn[i].d_un.d_val;
                            if (got != NULL) {
                                unsigned char *data =
                                        (unsigned char *)got->getPtrToRawData();
                                glink_addr = *(unsigned int *)
                                        (data + (g_o_t - got->getMemOffset() + 4));
                                break;
                            }
                        }
                    }
                }

                // Otherwise, first entry in .plt section holds the glink address
                if (glink_addr == 0) {
                    unsigned char *data = (unsigned char *)plt->getPtrToRawData();
                    glink_addr = *(unsigned int *)(data);
                }

                // Search for region that contains glink address
                for (unsigned iter = 0; iter < regions_.size(); ++iter) {
                    unsigned int start = regions_[iter]->getMemOffset();
                    unsigned int end = start + regions_[iter]->getMemSize();
                    if (start <= glink_addr && glink_addr < end) {
                        glink = regions_[iter];
                        break;
                    }
                }

                if (!glink) {
                    return false;
                }

                // Find PLT function stubs.  They preceed the glink section.
                stub_addr = glink_addr - (rel_plt_size_/rel_plt_entry_size_) * 16;

                const unsigned int LWZ_11_30   = 0x817e0000;
                const unsigned int ADDIS_11_30 = 0x3d7e0000;
                const unsigned int LWZ_11_11   = 0x816b0000;
                const unsigned int MTCTR_11    = 0x7d6903a6;
                const unsigned int BCTR        = 0x4e800420;

                unsigned char *sec_data = (unsigned char *)glink->getPtrToRawData();
                unsigned int *insn = (unsigned int *)
                        (sec_data + (stub_addr - glink->getMemOffset()));

                // Keep moving pointer back if more -fPIC stubs are found.
                while (sec_data < (unsigned char *)insn) {
                    unsigned int *back = insn - 4;

                    if ((  (back[0] & 0xffff0000) == LWZ_11_30
                           && back[1] == MTCTR_11
                           && back[2] == BCTR)

                        || (   (back[0] & 0xffff0000) == ADDIS_11_30
                               && (back[1] & 0xffff0000) == LWZ_11_11
                               &&  back[2] == MTCTR_11
                               &&  back[3] == BCTR))
                    {
                        extraStubs = true;
                        stub_addr -= 16;
                        insn = back;
                    } else {
                        break;
                    }
                }

                // Okay, this is where things get hairy.  If we have a one to one
                // relationship between the glink stubs and plt entries (meaning
                // extraStubs == false), then we can generate our relocationEntry
                // objects normally below.

                // However, if we have extra glink stubs, then we must generate
                // relocations with unknown destinations for *all* stubs.  Then,
                // we use the loop below to store additional information about
                // the data plt entry keyed by plt entry address.

                // Finally, if a symbol with any of the following forms:
                //     [hex_addr].got2.plt_pic32.[sym_name]
                //     [hex_addr].plt_pic32.[sym_name]
                //
                // matches the glink stub address, then stub symbols exist and we
                // can rely on these tell us where the stub will eventually go.

                if (extraStubs == true) {
                    std::string name;
                    relocationEntry re;

                    while (stub_addr < glink_addr) {
                        if (symsByOffset_.find(stub_addr) != symsByOffset_.end()) {
                            name = (symsByOffset_[stub_addr])[0]->getMangledName();
                            name = name.substr( name.rfind("plt_pic32.") + 10 );
                        }

                        if (!name.empty()) {
                            re = relocationEntry( stub_addr, 0, name, NULL, 0 );
                        } else {
                            re = relocationEntry( stub_addr, 0, "@plt", NULL, 0 );
                        }
                        fbt_.push_back(re);
                        stub_addr += 16;
                    }

                    // Now prepare to iterate over plt below.
                    next_plt_entry_addr = plt_addr_;
                    plt_entry_size_ = 4;

                } else {
                    next_plt_entry_addr = stub_addr;
                }

            } else {
                create_printf("ERROR: Can't handle %d PLT entry size\n",
                              plt_entry_size_);
                return false;
            }

            //  actually this is just fudged to make the offset value 72, which is what binutils uses
            //  Note that binutils makes the distinction between PLT_SLOT_SIZE (8),
            //  and PLT_ENTRY_SIZE (12).  PLT_SLOT_SIZE seems to be what we want, even though we also
            //  have PLT_INITIAL_ENTRY_SIZE (72)
            //  see binutils/bfd/elf32-ppc.c/h if more info is needed
            //next_plt_entry_addr += 72;  // 1st 6 PLT entries art special


        } else if(getArch() == Dyninst::Arch_ppc64)
        {
            // Unlike PPC32 Linux, we don't have a deterministic way of finding
            // PPC64 Linux linker stubs.  So, we'll wait until the CFG is built
            // inside Dyninst, and code read at that point.  To find them at this
            // point would require a scan of the entire .text section.
            //
            // If we're lucky, symbols may exist for these linker stubs.  They will
            // come in the following forms:
            //     [hex_addr].plt_call.[sym_name]
            //     [hex_addr].plt_branch.[sym_name]
            //     [hex_addr].long_branch.[sym_name]
            //     [hex_addr].plt_branch_r2off.[sym_name]
            //     [hex_addr].long_branch_r2off.[sym_name]
            //
            // Again unlike PPC32 above, we have no glink stub address to compare
            // against, so we must search through all symbols to find these names.
            //

            // First, build a map of the .rela.plt symbol name -> .rela.plt offset:
            dyn_hash_map<std::string, Offset> plt_rel_map;

            // I'm not a fan of code duplication, but merging this into the
            // loop below would be ugly and difficult to maintain.
            Elf_X_Sym  _sym  = symdata.get_sym();
            Elf_X_Rel  _rel  = reldata.get_rel();
            Elf_X_Rela _rela = reldata.get_rela();
            const char *_strs = strdata.get_string();

            for( u_int i = 0; i < (rel_plt_size_/rel_plt_entry_size_); ++i ) {
                long _offset;
                long _index;

                switch (reldata.d_type()) {
                    case ELF_T_REL:
                        _offset = _rel.r_offset(i);
                        _index  = _rel.R_SYM(i);
                        break;

                    case ELF_T_RELA:
                        _offset = _rela.r_offset(i);
                        _index  = _rela.R_SYM(i);
                        break;

                    default:
                        // We should never reach this case.
                        return false;
                };

                std::string _name = &_strs[ _sym.st_name(_index) ];
                // I'm interested to see if this assert will ever fail.
                if(!_name.length())
                {
                    create_printf("Empty name for REL/RELA entry found, ignoring\n");
                    continue;
                }

                plt_rel_map[_name] = _offset;
            }
            // End code duplication.

            dyn_hash_map<std::string, std::vector<Symbol *> >::iterator iter;
            for (iter = symbols_.begin(); iter != symbols_.end(); ++iter) {
                std::string name = iter->first;
                if (name.length() > 8) {
                    if (name.substr(8, 10) == ".plt_call.")
                        name = name.substr(8 + 10);
                    else if (name.substr(8, 12) == ".plt_branch.")
                        name = name.substr(8 + 12);
                    else if (name.substr(8, 13) == ".long_branch.")
                        name = name.substr(8 + 13);
                    else if (name.substr(8, 18) == ".plt_branch_r2off.")
                        name = name.substr(8 + 18);
                    else if (name.substr(8, 19) == ".long_branch_r2off.")
                        name = name.substr(8 + 19);
                    else
                        continue;

                    // Remove possible trailing addend value.
                    std::string::size_type pos = name.rfind('+');
                    if (pos != std::string::npos) name.erase(pos);

                    // Remove possible version number.
                    pos = name.find('@');
                    if (pos != std::string::npos) name.erase(pos);

                    // Find the dynamic symbol this linker stub branches to.
                    Symbol *targ_sym = NULL;
                    if (symbols_.find(name) != symbols_.end())
                        for (unsigned i = 0; i < symbols_[name].size(); ++i)
                            if ( (symbols_[name])[i]->isInDynSymtab())
                                targ_sym = (symbols_[name])[i];

                    // If a corresponding target symbol cannot be found for a
                    // named linker stub, then ignore it.  We'll find it during
                    // parsing.
                    if (!targ_sym) continue;

                    if (iter->second.size() != 1)
                        continue;
                    dyn_hash_map<string, Offset>::iterator pltrel_iter = plt_rel_map.find(name);
                    if (pltrel_iter == plt_rel_map.end())
                        continue;

                    Symbol *stub_sym = iter->second[0];
                    relocationEntry re(stub_sym->getOffset(),
                                       pltrel_iter->second,
                                       name,
                                       targ_sym);
                    fbt_.push_back(re);
                }
            }

            // 1st plt entry is special.
            next_plt_entry_addr += plt_entry_size_;

        } else if (getArch() == Dyninst::Arch_aarch64)
        {
            next_plt_entry_addr += 2 * plt_entry_size_;
        } else {
            next_plt_entry_addr += 4*(plt_entry_size_); //1st 4 entries are special
        }


        Elf_X_Sym sym = symdata.get_sym();
        Elf_X_Rel rel = reldata.get_rel();
        Elf_X_Rela rela = reldata.get_rela();
        const char *strs = strdata.get_string();

        if (sym.isValid() && (rel.isValid() || rela.isValid()) && strs) {

            // Sometimes, PPC32 Linux may use this loop to update fbt entries.
            // Should stay -1 for all other platforms.  See notes above.
            int fbt_iter = -1;
            if (fbt_.size() > 0 && !fbt_[0].rel_addr() && fbt_[0].name() != "@plt")
                fbt_iter = 0;

            for( u_int i = 0; i < (rel_plt_size_/rel_plt_entry_size_); ++i ) {
                long offset;
                long addend;
                long index;
                unsigned long type;
                Region::RegionType rtype;

                switch (reldata.d_type()) {
                    case ELF_T_REL:
                        offset = rel.r_offset(i);
                        addend = 0;
                        index = rel.R_SYM(i);
                        type = rel.R_TYPE(i);
                        rtype = Region::RT_REL;
                        break;

                    case ELF_T_RELA:
                        offset = rela.r_offset(i);
                        addend = rela.r_addend(i);
                        index = rela.R_SYM(i);
                        type = rela.R_TYPE(i);
                        rtype = Region::RT_RELA;
                        break;

                    default:
                        // We should never reach this case.
                        return false;
                };

                std::string targ_name = &strs[ sym.st_name(index) ];
                vector<Symbol *> dynsym_list;
                if (symbols_.find(targ_name) != symbols_.end())
                {
                    vector<Symbol *> &syms = symbols_[&strs[ sym.st_name(index)]];
                    for (vector<Symbol *>::iterator i = syms.begin(); i != syms.end(); i++) {
                        if (!(*i)->isInDynSymtab())
                            continue;
                        dynsym_list.push_back(*i);
                    }
                }
                else {
                    dynsym_list.clear();
                }

#if defined(os_vxworks)
                    // VxWorks Kernel Images don't use PLT's, but we'll use the fbt to
          // note all function call relocations, and we'll fix these up later
          // in Symtab::fixup_RegionAddr()
          next_plt_entry_addr = sym.st_value(index);
#endif

                    if (fbt_iter == -1) { // Create new relocation entry.
                        relocationEntry re( next_plt_entry_addr, offset, targ_name,
                                            NULL, type );
                        re.setAddend(addend);
                        re.setRegionType(rtype);
                        if (dynsym_list.size() > 0)
                            re.addDynSym(dynsym_list[0]);
                        fbt_.push_back(re);

                    } else { // Update existing relocation entry.
                        while ((unsigned)fbt_iter < fbt_.size() &&
                               fbt_[fbt_iter].name() == targ_name) {

                            fbt_[fbt_iter].setRelAddr(offset);
                            fbt_[fbt_iter].setAddend(addend);
                            fbt_[fbt_iter].setRegionType(rtype);
                            if (dynsym_list.size() > 0)
                                fbt_[fbt_iter].addDynSym(dynsym_list[0]);

                            ++fbt_iter;
                        }
                    }

#if defined(os_vxworks)
                    // Nothing to increment here.
#else
                    next_plt_entry_addr += plt_entry_size_;
#endif
                }
                return true;
            }
        }
    }
    return false;
}

void Object::load_object(bool alloc_syms)
{
    Elf_X_Shdr *bssscnp = 0;
    Elf_X_Shdr *symscnp = 0;
    Elf_X_Shdr *strscnp = 0;
    Elf_X_Shdr *stabscnp = 0;
    Elf_X_Shdr *stabstrscnp = 0;
    Elf_X_Shdr *stabs_indxcnp = 0;
    Elf_X_Shdr *stabstrs_indxcnp = 0;
    Offset txtaddr = 0;
    Offset dataddr = 0;
    Elf_X_Shdr *rel_plt_scnp = 0;
    Elf_X_Shdr *plt_scnp = 0;
    Elf_X_Shdr *got_scnp = 0;
    Elf_X_Shdr *dynsym_scnp = 0;
    Elf_X_Shdr *dynstr_scnp = 0;
    Elf_X_Shdr *dynamic_scnp = 0;
    Elf_X_Shdr *eh_frame_scnp = 0;
    Elf_X_Shdr *gcc_except = 0;
    Elf_X_Shdr *interp_scnp = 0;
    Elf_X_Shdr *opd_scnp = NULL;

    { // binding contour (for "goto cleanup")

        // initialize object (for failure detection)
        code_ptr_ = 0;
        code_off_ = 0;
        code_len_ = 0;
        data_ptr_ = 0;
        data_off_ = 0;
        data_len_ = 0;

        // initialize "valid" regions of code and data segments
        code_vldS_ = (Offset) -1;
        code_vldE_ = 0;
        data_vldS_ = (Offset) -1;
        data_vldE_ = 0;

        // And attempt to parse the ELF data structures in the file....
        // EEL, added one more parameter

        if (!loaded_elf(txtaddr, dataddr, bssscnp, symscnp, strscnp,
                        stabscnp, stabstrscnp, stabs_indxcnp, stabstrs_indxcnp,
                        rel_plt_scnp, plt_scnp, got_scnp, dynsym_scnp, dynstr_scnp,
                        dynamic_scnp, eh_frame_scnp,gcc_except, interp_scnp,
                        opd_scnp, true))
        {
            goto cleanup;
        }

        addressWidth_nbytes = elfHdr->wordSize();

        // find code and data segments....
        find_code_and_data(*elfHdr, txtaddr, dataddr);

        if (elfHdr->e_type() != ET_REL)
        {
            if (!code_ptr_ || !code_len_)
            {
                //bpfatal( "no text segment\n");
                goto cleanup;
            }
        }
        get_valid_memory_areas(*elfHdr);

#if (defined(os_linux) || defined(os_freebsd))
        if(getArch() == Dyninst::Arch_x86 || getArch() == Dyninst::Arch_x86_64)
        {
            if (eh_frame_scnp != 0 && gcc_except != 0)
            {
                find_catch_blocks(eh_frame_scnp, gcc_except,
                                  txtaddr, dataddr, catch_addrs_);
            }
        }
#endif
        if (interp_scnp) {
            interpreter_name_ = (char *) interp_scnp->get_data().d_buf();
        }

        // global symbols are put in global_symbols. Later we read the
        // stab section to find the module to where they belong.
        // Experiment : lets try to be a bit more intelligent about
        // how we initially size the global_symbols table.
        // dictionary_lite takes an initial # of bins (2nd param),
        // a max bin load (3rd param), and a grow factor (4th param).
        // Leaving aside the grow factor, lets allocate an initial #
        // of bins = nsyms / max bin load.

#if defined(TIMED_PARSE)
        struct timeval starttime;
    gettimeofday(&starttime, NULL);
#endif
        if (alloc_syms)
        {
            // find symbol and string data
#if defined(os_vxworks)
            // Avoid assigning symbols to DEFAULT_MODULE on VxWorks
      string module = mf->pathname();
#else
            string module = "DEFAULT_MODULE";
#endif
            string name   = "DEFAULT_NAME";
            Elf_X_Data symdata, strdata;

            if (symscnp && strscnp)
            {
                symdata = symscnp->get_data();
                strdata = strscnp->get_data();
                parse_symbols(symdata, strdata, bssscnp, symscnp, false, module);
            }

            no_of_symbols_ = nsymbols();

            // try to resolve the module names of global symbols
            // Sun compiler stab.index section
            fix_global_symbol_modules_static_stab(stabs_indxcnp, stabstrs_indxcnp);

            // STABS format (.stab section)
            fix_global_symbol_modules_static_stab(stabscnp, stabstrscnp);

            // DWARF format (.debug_info section)
            fix_global_symbol_modules_static_dwarf();

            if (dynamic_addr_ && dynsym_scnp && dynstr_scnp)
            {
                symdata = dynsym_scnp->get_data();
                strdata = dynstr_scnp->get_data();
                parse_dynamicSymbols(dynamic_scnp, symdata, strdata, false, module);
            }

            //TODO
            //Have a hash on the symbol table. Iterate over dynamic symbol table to check if it exists
            //If yes set dynamic for the symbol ( How to distinguish between symbols only in symtab,
            //         symbols only in dynsymtab & symbols present in both).
            // Else add dynamic symbol to dictionary
            // (or) Have two sepearate dictionaries. Makes life easy!
            // Think about it today!!

            //allsymbols = merge(allsymbols, alldynSymbols);

#if defined(TIMED_PARSE)
            struct timeval endtime;
      gettimeofday(&endtime, NULL);
      unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
      unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
      unsigned long difftime = lendtime - lstarttime;
      double dursecs = difftime/(1000);
      cout << "parsing/fixing/overriding elf took "<<dursecs <<" msecs" << endl;
#endif

            if (dynamic_addr_ && dynsym_scnp && dynstr_scnp)
            {
                parseDynamic(dynamic_scnp, dynsym_scnp, dynstr_scnp);
            }

#if defined(os_vxworks)
            // Load relocations like they are PLT entries.
      // Use the non-dynamic symbol tables.
      if (rel_plt_scnp && symscnp && strscnp) {
	if (!get_relocation_entries(rel_plt_scnp, symscnp, strscnp))
	  goto cleanup;
      }
#endif

            // populate "fbt_"
            if (rel_plt_scnp && dynsym_scnp && dynstr_scnp)
            {
                if (!get_relocation_entries(rel_plt_scnp,dynsym_scnp,dynstr_scnp))
                {
                    goto cleanup;
                }
            }
            parse_all_relocations(*elfHdr, dynsym_scnp, dynstr_scnp,
                                  symscnp, strscnp);

            handle_opd_relocations();
        }

        //Set object type
        int e_type = elfHdr->e_type();

        if (e_type == ET_DYN) {
            obj_type_ = obj_SharedLib;
        }else if (e_type == ET_EXEC) {
            obj_type_ = obj_Executable;
        }else if (e_type == ET_REL) {
            obj_type_ = obj_RelocatableFile;
        }

        // Set rel type based on the ELF machine type
        relType_ = getRelTypeByElfMachine(elfHdr);

        if (opd_scnp) {
            parse_opd(opd_scnp);
        } else if (got_scnp) {
	    // Add a single global TOC value...
	}

        return;
    } // end binding contour (for "goto cleanup2")

    cleanup:
    {
        /* NOTE: The file should NOT be munmap()ed.  The mapped file is
       used for function parsing (see dyninstAPI/src/symtab.C) */

        create_printf("%s[%d]:  failed to load elf object\n", FILE__, __LINE__);
    }
}

void Object::load_shared_object(bool alloc_syms)
{
    Elf_X_Shdr *bssscnp = 0;
    Elf_X_Shdr *symscnp = 0;
    Elf_X_Shdr *strscnp = 0;
    Elf_X_Shdr *stabscnp = 0;
    Elf_X_Shdr *stabstrscnp = 0;
    Elf_X_Shdr *stabs_indxcnp = 0;
    Elf_X_Shdr *stabstrs_indxcnp = 0;
    Offset txtaddr = 0;
    Offset dataddr = 0;
    Elf_X_Shdr *rel_plt_scnp = 0;
    Elf_X_Shdr *plt_scnp = 0;
    Elf_X_Shdr *got_scnp = 0;
    Elf_X_Shdr *dynsym_scnp = 0;
    Elf_X_Shdr *dynstr_scnp = 0;
    Elf_X_Shdr *dynamic_scnp = 0;
    Elf_X_Shdr *eh_frame_scnp = 0;
    Elf_X_Shdr *gcc_except = 0;
    Elf_X_Shdr *interp_scnp = 0;
    Elf_X_Shdr *opd_scnp = NULL;

    { // binding contour (for "goto cleanup2")

        // initialize "valid" regions of code and data segments
        code_vldS_ = (Offset) -1;
        code_vldE_ = 0;
        data_vldS_ = (Offset) -1;
        data_vldE_ = 0;

        if (!loaded_elf(txtaddr, dataddr, bssscnp, symscnp, strscnp,
                        stabscnp, stabstrscnp, stabs_indxcnp, stabstrs_indxcnp,
                        rel_plt_scnp, plt_scnp, got_scnp, dynsym_scnp, dynstr_scnp,
                        dynamic_scnp, eh_frame_scnp, gcc_except, interp_scnp, opd_scnp))
            goto cleanup2;

        if (interp_scnp)
            interpreter_name_ = (char *) interp_scnp->get_data().d_buf();

        addressWidth_nbytes = elfHdr->wordSize();

        // find code and data segments....
        find_code_and_data(*elfHdr, txtaddr, dataddr);

        get_valid_memory_areas(*elfHdr);

#if (defined(os_linux) || defined(os_freebsd))
        if(getArch() == Dyninst::Arch_x86 || getArch() == Dyninst::Arch_x86_64) {
            if (eh_frame_scnp != 0 && gcc_except != 0) {
                find_catch_blocks(eh_frame_scnp, gcc_except,
                                  txtaddr, dataddr, catch_addrs_);
            }
        }
#endif

#if defined(TIMED_PARSE)
        struct timeval starttime;
    gettimeofday(&starttime, NULL);
#endif

        if (alloc_syms) {
            // build symbol dictionary
            string module = mf->pathname();
            string name   = "DEFAULT_NAME";

            Elf_X_Data symdata, strdata;
            if (symscnp && strscnp)
            {
                symdata = symscnp->get_data();
                strdata = strscnp->get_data();
                if (!symdata.isValid() || !strdata.isValid()) {
                    log_elferror(err_func_, "locating symbol/string data");
                    goto cleanup2;
                }
                bool result = parse_symbols(symdata, strdata, bssscnp, symscnp, false, module);
                if (!result) {
                    log_elferror(err_func_, "locating symbol/string data");
                    goto cleanup2;
                }
            }

            no_of_symbols_ = nsymbols();
            // try to resolve the module names of global symbols
            // Sun compiler stab.index section
            fix_global_symbol_modules_static_stab(stabs_indxcnp, stabstrs_indxcnp);

            // STABS format (.stab section)
            fix_global_symbol_modules_static_stab(stabscnp, stabstrscnp);

            // DWARF format (.debug_info section)
            fix_global_symbol_modules_static_dwarf();

            if (dynamic_addr_ && dynsym_scnp && dynstr_scnp)
            {
                symdata = dynsym_scnp->get_data();
                strdata = dynstr_scnp->get_data();
                parse_dynamicSymbols(dynamic_scnp, symdata, strdata, false, module);
            }

#if defined(TIMED_PARSE)
            struct timeval endtime;
      gettimeofday(&endtime, NULL);
      unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
      unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
      unsigned long difftime = lendtime - lstarttime;
      double dursecs = difftime/(1000 * 1000);
      cout << "*INSERT SYMBOLS* elf took "<<dursecs <<" msecs" << endl;
      //cout << "parsing/fixing/overriding/insertion elf took "<<dursecs <<" msecs" << endl;
#endif
            if(dynamic_addr_ && dynsym_scnp && dynstr_scnp) {
                parseDynamic(dynamic_scnp, dynsym_scnp, dynstr_scnp);
            }

#if defined(os_vxworks)
            // Load relocations like they are PLT entries.
      // Use the non-dynamic symbol tables.
      if (rel_plt_scnp && symscnp && strscnp) {
	if (!get_relocation_entries(rel_plt_scnp, symscnp, strscnp))
	  goto cleanup2;
      }
#endif

            if (rel_plt_scnp && dynsym_scnp && dynstr_scnp) {
                if (!get_relocation_entries(rel_plt_scnp,dynsym_scnp,dynstr_scnp)) {
                    goto cleanup2;
                }
            }

            parse_all_relocations(*elfHdr, dynsym_scnp, dynstr_scnp,
                                  symscnp, strscnp);
            // Apply relocations to opd
            handle_opd_relocations();
        }

        //Set object type
        int e_type = elfHdr->e_type();
        if (e_type == ET_DYN) {
            obj_type_ = obj_SharedLib;
        }
        else if (e_type == ET_EXEC) {
            obj_type_ = obj_Executable;
        }else if( e_type == ET_REL ) {
            obj_type_ = obj_RelocatableFile;
        }


        if (opd_scnp) {
            parse_opd(opd_scnp);
        }

        // Set rel type based on the ELF machine type
        relType_ = getRelTypeByElfMachine(elfHdr);

    } // end binding contour (for "goto cleanup2")

    cleanup2:
    {
    }
}

static Symbol::SymbolType pdelf_type(int elf_type)
{
    switch (elf_type) {
        case STT_FILE:   return Symbol::ST_MODULE;
        case STT_SECTION:return Symbol::ST_SECTION;
        case STT_OBJECT: return Symbol::ST_OBJECT;
        case STT_TLS:    return Symbol::ST_TLS;
        case STT_FUNC:   return Symbol::ST_FUNCTION;
        case STT_NOTYPE: return Symbol::ST_NOTYPE;
#if defined(STT_GNU_IFUNC)
        case STT_GNU_IFUNC: return Symbol::ST_INDIRECT;
#endif
        default: return Symbol::ST_UNKNOWN;
    }
}

static Symbol::SymbolLinkage pdelf_linkage(int elf_binding)
{
    switch (elf_binding) {
        case STB_LOCAL:  return Symbol::SL_LOCAL;
        case STB_WEAK:   return Symbol::SL_WEAK;
        case STB_GLOBAL: return Symbol::SL_GLOBAL;
#if defined(STB_GNU_UNIQUE)        
        case STB_GNU_UNIQUE: return Symbol::SL_UNIQUE;
#endif
    }
    return Symbol::SL_UNKNOWN;
}

static Symbol::SymbolVisibility pdelf_visibility(int elf_visibility)
{
    switch (elf_visibility) {
        case STV_DEFAULT:    return Symbol::SV_DEFAULT;
        case STV_INTERNAL:   return Symbol::SV_INTERNAL;
        case STV_HIDDEN:     return Symbol::SV_HIDDEN;
        case STV_PROTECTED:  return Symbol::SV_PROTECTED;
    }
    return Symbol::SV_UNKNOWN;
}

//============================================================================

//#include "dyninstAPI/src/arch.h"
//#include "dyninstAPI/src/inst.h"
//#include "dyninstAPI/src/instPoint.h" // includes instPoint-x86.h
//#include "dyninstAPI/src/instP.h" // class returnInstance
//#include "dyninstAPI/src/rpcMgr.h"

//linear search
bool lookUpSymbol( std::vector< Symbol *>& allsymbols, Offset& addr )
{
    for( unsigned i = 0; i < allsymbols.size(); i++ )
    {
        if( allsymbols[ i ]->getOffset() == addr )
        {
            return true;
        }
    }
    return false;
}

bool lookUpAddress( std::vector< Offset >& jumpTargets, Offset& addr )
{
    for( unsigned i = 0; i < jumpTargets.size(); i++ )
    {
        if( jumpTargets[ i ] == addr )
        {
            return true;
        }
    }
    return false;
}

//utitility function to print std::vector of symbols
void printSyms( std::vector< Symbol *>& allsymbols )
{
    for( unsigned i = 0; i < allsymbols.size(); i++ )
    {
        if( allsymbols[ i ]->getType() != Symbol::ST_FUNCTION )
        {
            continue;
        }
        cout << allsymbols[ i ] << endl;
    }
}

// Official Procedure Descriptor handler
//
// Some platforms (PPC64 Linux libc v2.1) only produce function
// symbols which point into the .opd section.  Find the actual
// function address in .text, and fix the symbol.
//
// Additionally, large binaries may contain multiple TOC values.
// Use the .opd section to determine the correct per-function TOC.
// See binutils' bfd/elf64-ppc.c for all the gory details.
//
// Symbol altering routines should be minimized to prevent problems
// in the rewrite case.  Instead, return a new symbol the encapsulates
// the correct information.

// In statically linked binaries, there are relocations against
// .opd section. We need to apply the relocations before using
// opd symbols.

void Object::parse_opd(Elf_X_Shdr *opd_hdr) {
    // If the OPD is filled in, parse it and fill in our TOC table
    if(!opd_hdr) return;

    Elf_X_Data data = opd_hdr->get_data();
    if(!(data.isValid())) return;

    // Let's read this puppy
    unsigned long *buf = (unsigned long *)data.d_buf();
    // In some cases, the OPD is a set of 3-tuples: <func offset, TOC, environment ptr>.
    // In others, it's a set of 2-tuples. Since we can't tell the difference, we
    // instead look for function offsets.
    // Heck, it can even change within the opd!

    // In almost all cases, the TOC is the same. So let's store it at the
    // special location 0 and only record differences.
    Offset baseTOC = buf[1];
    TOC_table_[0] = baseTOC;
    create_printf("Set base TOC to %p\n", baseTOC);

    // Note the lack of 32/64 here: okay because the only platform with an OPD
    // is 64-bit elf.
    unsigned i = 0;
    while (i < (data.d_size() / sizeof(unsigned long))) {
        Offset func = buf[i];
        Offset toc = buf[i+1];

        if (func == 0 && i != 0) break;

        if (symsByOffset_.find(func) == symsByOffset_.end()) {
            i++;
            continue;
        }

        if (toc != baseTOC) {
            TOC_table_[func] = toc;
	    create_printf("Set TOC for %p to %p\n", func, toc);
        }
        i += 2;
    }
}

void Object::handle_opd_relocations(){

    unsigned int i = 0, opdregion = 0;
    while (i < regions_.size()) {
        if(regions_[i]->getRegionName().compare(".opd") == 0){
            opdregion = i;
            break;
        }
        i++;
    }

    vector<relocationEntry> region_rels = (regions_[opdregion])->getRelocations();
    vector<relocationEntry>::iterator rel_it;
    vector<Symbol *>::iterator sym_it;
    for(sym_it = opdsymbols_.begin(); sym_it != opdsymbols_.end(); ++sym_it) {
        for(rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {
            if((*sym_it)->getPtrOffset() == (*rel_it).rel_addr()) {
                i = 0;
                while (i < regions_.size()) {
                    if(regions_[i]->getRegionName().compare((*rel_it).getDynSym()->getMangledName()) == 0){
                        Region *targetRegion = regions_[i];
                        Offset regionOffset = targetRegion->getDiskOffset()+(*rel_it).addend();
                        (*sym_it)->setRegion(targetRegion);
                        (*sym_it)->setOffset(regionOffset);   // Store code address for the function.
                        break;
                    }
                    ++i;
                }
            }
        }
    }
    opdsymbols_.clear();
}

Symbol *Object::handle_opd_symbol(Region *opd, Symbol *sym)
{
    if (!sym) return NULL;

    Offset soffset = sym->getOffset();
    if(!opd->isOffsetInRegion(soffset)) return NULL;  // Symbol must be in .opd section.

    Offset* opd_entry = (Offset*)opd->getPtrToRawData();
    opd_entry += (soffset - opd->getDiskOffset()) / sizeof(Offset); // table of offsets;
    Symbol *retval = new Symbol(*sym); // Copy the .opd symbol.
    retval->setOffset(opd_entry[0]);   // Store code address for the function.
    retval->setLocalTOC(opd_entry[1]); // Store TOC address for this function.
    retval->setPtrOffset(soffset);     // Original address is the ptr address.

    // Find the appropriate region for the new symbol.
    unsigned int i = 0;
    while (i < regions_.size()) {
        if (regions_[i]->isOffsetInRegion(opd_entry[0])) {
            retval->setRegion(regions_[i]);
            break;
        }
        ++i;
    }
    retval->setSymbolType(Symbol::ST_FUNCTION);
#if 0
    retval->tag_ = Symbol::TAG_INTERNAL;  // Not sure if this is an appropriate
                                        // use of the tag field, but we need
                                        // to mark this symbol somehow as a
                                        // fake.
#endif
    return retval;
}

// parse_symbols(): populate "allsymbols"
bool Object::parse_symbols(Elf_X_Data &symdata, Elf_X_Data &strdata,
                           Elf_X_Shdr* bssscnp,
                           Elf_X_Shdr* symscnp,
                           bool /*shared*/, string smodule)
{
#if defined(TIMED_PARSE)
    struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

    if (!symdata.isValid() || !strdata.isValid()) {
        return false;
    }

    Elf_X_Sym syms = symdata.get_sym();
    const char *strs = strdata.get_string();
    if(syms.isValid()){
        for (unsigned i = 0; i < syms.count(); i++) {
            //If it is not a dynamic executable then we need undefined symbols
            //in symtab section so that we can resolve symbol references. So
            //we parse & store undefined symbols only if there is no dynamic
            //symbol table
            //1/09: not so anymore--we want to preserve all symbols,
            //regardless of file type

            int etype = syms.ST_TYPE(i);
            int ebinding = syms.ST_BIND(i);
            int evisibility = syms.ST_VISIBILITY(i);

            // resolve symbol elements
            string sname = &strs[ syms.st_name(i) ];
            Symbol::SymbolType stype = pdelf_type(etype);
            Symbol::SymbolLinkage slinkage = pdelf_linkage(ebinding);
            Symbol::SymbolVisibility svisibility = pdelf_visibility(evisibility);
            unsigned ssize = syms.st_size(i);
            unsigned secNumber = syms.st_shndx(i);

            Offset soffset;
            if (symscnp->isFromDebugFile()) {
                Offset soffset_dbg = syms.st_value(i);
                soffset = soffset_dbg;
                if (soffset_dbg) {
                    bool result = convertDebugOffset(soffset_dbg, soffset);
                    if (!result) {
                        //Symbol does not match any section, can't convert
                        continue;
                    }
                }
            }
            else {
                soffset = syms.st_value(i);
            }

            /* icc BUG: Variables in BSS are categorized as ST_NOTYPE instead of
	 ST_OBJECT.  To fix this, we check if the symbol is in BSS and has
	 size > 0. If so, we can almost always say it is a variable and hence,
	 change the type from ST_NOTYPE to ST_OBJECT.
      */
            if (bssscnp) {
                Offset bssStart = Offset(bssscnp->sh_addr());
                Offset bssEnd = Offset (bssStart + bssscnp->sh_size()) ;

                if(( bssStart <= soffset) && ( soffset < bssEnd ) && (ssize > 0) &&
                   (stype == Symbol::ST_NOTYPE))
                {
                    stype = Symbol::ST_OBJECT;
                }
            }

            // discard "dummy" symbol at beginning of file
            if (i==0 && sname == "" && soffset == (Offset)0)
                continue;


            Region *sec;
            if(secNumber >= 1 && secNumber < regions_.size()) {
                sec = regions_[secNumber];
            } else {
                sec = NULL;
            }
            int ind = int (i);
            int strindex = syms.st_name(i);

            if(stype == Symbol::ST_SECTION && sec != NULL) {
                sname = sec->getRegionName();
                soffset = sec->getDiskOffset();
            }

            if (stype == Symbol::ST_MODULE) {
                smodule = sname;
            }
            Symbol *newsym = new Symbol(sname,
                                        stype,
                                        slinkage,
                                        svisibility,
                                        soffset,
                                        NULL,
                                        sec,
                                        ssize,
                                        false,
                                        (secNumber == SHN_ABS),
                                        ind,
                                        strindex,
                                        (secNumber == SHN_COMMON));

            if (stype == Symbol::ST_UNKNOWN)
                newsym->setInternalType(etype);

            if (sec && sec->getRegionName() == OPD_NAME && stype == Symbol::ST_FUNCTION ) {
                newsym = handle_opd_symbol(sec, newsym);

                opdsymbols_.push_back(newsym);
                symbols_[sname].push_back(newsym);
                symsByOffset_[newsym->getOffset()].push_back(newsym);
                symsToModules_[newsym] = smodule;
            } else {
                symbols_[sname].push_back(newsym);
                symsByOffset_[newsym->getOffset()].push_back(newsym);
                symsToModules_[newsym] = smodule;
            }

        }
    } // syms.isValid()
#if defined(TIMED_PARSE)
    struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 * 1000);
  cout << "parsing elf took "<<dursecs <<" secs" << endl;
#endif
    return true;
}

// parse_symbols(): populate "allsymbols"
// Lazy parsing of dynamic symbol  & string tables
// Parsing the dynamic symbols lazily would certainly
// not increase the overhead of the entire parse
void Object::parse_dynamicSymbols (Elf_X_Shdr *&
dyn_scnp
        , Elf_X_Data &symdata,
                                   Elf_X_Data &strdata,
                                   bool /*shared*/,
                                   std::string smodule)
{
#if defined(TIMED_PARSE)
    struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

    Elf_X_Sym syms = symdata.get_sym();
    const char *strs = strdata.get_string();
    Elf_X_Shdr *versymSec = NULL, *verneedSec = NULL, *verdefSec = NULL;
    Elf_X_Data data = dyn_scnp->get_data();
    Elf_X_Dyn dyns = data.get_dyn();
    unsigned verneednum = 0, verdefnum = 0;
    Elf_X_Versym symVersions;
    Elf_X_Verdef *symVersionDefs = NULL;
    Elf_X_Verneed *symVersionNeeds = NULL;
    for (unsigned i = 0; i < dyns.count(); ++i) {
        switch(dyns.d_tag(i)) {
            case DT_NEEDED:
                deps_.push_back(&strs[dyns.d_ptr(i)]);
                break;
            case DT_VERSYM:
                versymSec = getRegionHdrByAddr(dyns.d_ptr(i));
                break;
            case DT_VERNEED:
                verneedSec = getRegionHdrByAddr(dyns.d_ptr(i));
                break;
            case DT_VERDEF:
                verdefSec = getRegionHdrByAddr(dyns.d_ptr(i));
                break;
            case DT_VERNEEDNUM:
                verneednum = dyns.d_ptr(i);
                break;
            case DT_VERDEFNUM:
                verdefnum = dyns.d_ptr(i);
                break;
            case DT_SONAME:
                soname_ = &strs[dyns.d_ptr(i)];
            default:
                break;
        }
    }
    if(versymSec)
        symVersions = versymSec->get_data().get_versyms();
    if(verdefSec)
        symVersionDefs = verdefSec->get_data().get_verDefSym();
    if(verneedSec)
        symVersionNeeds = verneedSec->get_data().get_verNeedSym();

    for(unsigned i = 0; i < verdefnum ;i++) {
        Elf_X_Verdaux *aux = symVersionDefs->get_aux();
        for(unsigned j=0; j< symVersionDefs->vd_cnt(); j++){
            versionMapping[symVersionDefs->vd_ndx()].push_back(&strs[aux->vda_name()]);
            Elf_X_Verdaux *auxnext = aux->get_next();
            delete aux;
            aux = auxnext;
        }
        Elf_X_Verdef *symVersionDefsnext = symVersionDefs->get_next();
        delete symVersionDefs;
        symVersionDefs = symVersionDefsnext;
    }

    for(unsigned i = 0; i < verneednum; i++){
        Elf_X_Vernaux *aux = symVersionNeeds->get_aux();
        for(unsigned j=0; j< symVersionNeeds->vn_cnt(); j++){
            versionMapping[aux->vna_other()].push_back(&strs[aux->vna_name()]);
            versionFileNameMapping[aux->vna_other()] = &strs[symVersionNeeds->vn_file()];
            Elf_X_Vernaux *auxnext = aux->get_next();
            delete aux;
            aux = auxnext;
        }
        Elf_X_Verneed *symVersionNeedsnext = symVersionNeeds->get_next();
        delete symVersionNeeds;
        symVersionNeeds = symVersionNeedsnext;
    }

    if(syms.isValid()) {
        for (unsigned i = 0; i < syms.count(); i++) {
            int etype = syms.ST_TYPE(i);
            int ebinding = syms.ST_BIND(i);
            int evisibility = syms.ST_VISIBILITY(i);

            // resolve symbol elements
            string sname = &strs[ syms.st_name(i) ];
            Symbol::SymbolType stype = pdelf_type(etype);
            Symbol::SymbolLinkage slinkage = pdelf_linkage(ebinding);
            Symbol::SymbolVisibility svisibility = pdelf_visibility(evisibility);
            unsigned ssize = syms.st_size(i);
            Offset soffset = syms.st_value(i);
            unsigned secNumber = syms.st_shndx(i);

            // discard "dummy" symbol at beginning of file
            if (i==0 && sname == "" && soffset == 0)
                continue;

            Region *sec;
            if(secNumber >= 1 && secNumber < regions_.size()) {
                sec = regions_[secNumber];
            } else {
                sec = NULL;
            }

            int ind = int (i);
            int strindex = syms.st_name(i);

            if (stype == Symbol::ST_MODULE) {
                smodule = sname;
            }

            Symbol *newsym = new Symbol(sname,
                                        stype,
                                        slinkage,
                                        svisibility,
                                        soffset,
                                        NULL,
                                        sec,
                                        ssize,
                                        true,  // is dynamic
                                        (secNumber == SHN_ABS),
                                        ind,
                                        strindex,
                                        (secNumber == SHN_COMMON));

            if (stype == Symbol::ST_UNKNOWN)
                newsym->setInternalType(etype);

            if(versymSec) {
                unsigned short raw = symVersions.get(i);
                bool hidden = raw >> 15;
                int index = raw & 0x7fff;
                if(versionFileNameMapping.find(index) != versionFileNameMapping.end()) {
                    //printf("version filename for %s: %s\n", sname.c_str(),
                    //versionFileNameMapping[index].c_str());
                    newsym->setVersionFileName(versionFileNameMapping[index]);
                }
                if(versionMapping.find(index) != versionMapping.end()) {
                    //printf("versions for %s: ", sname.c_str());
                    //for (unsigned k=0; k < versionMapping[index].size(); k++)
                    //printf(" %s", versionMapping[index][k].c_str());
                    newsym->setVersions(versionMapping[index]);
                    //printf("\n");
                }
                if (hidden) {
                    newsym->setVersionHidden();
                }
            }
            // register symbol in dictionary

            if (sec && sec->getRegionName() == OPD_NAME && stype == Symbol::ST_FUNCTION ) {
                newsym = handle_opd_symbol(sec, newsym);
                opdsymbols_.push_back(newsym);

                symbols_[sname].push_back(newsym);
                symsByOffset_[newsym->getOffset()].push_back(newsym);
                symsToModules_[newsym] = smodule;
            } else {
                symbols_[sname].push_back(newsym);
                symsByOffset_[newsym->getOffset()].push_back(newsym);
                symsToModules_[newsym] = smodule;
            }
        }
    }

#if defined(TIMED_PARSE)
    struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 * 1000);
  cout << "parsing elf took "<<dursecs <<" secs" << endl;
#endif
}

#if defined(cap_dwarf)

string Object::find_symbol(string name)
{
    string name2;

    // pass #1: unmodified
    name2 = name;
    if (symbols_.find(name2)!=symbols_.end()) return name2;

    // pass #2: leading underscore (C)
    name2 = "_" + name;
    if (symbols_.find(name2)!=symbols_.end()) return name2;

    // pass #3: trailing underscore (Fortran)
    name2 = name + "_";
    if (symbols_.find(name2)!=symbols_.end())
        return name2;

    return "";
}

#endif


/********************************************************
 *
 * For object files only....
 *   read the .debug_info section to find the module of global symbols
 *   see documents...
 *   - "DWARF Debugging Information Format"
 *   - "A Consumer Libary Interface to DWARF"
 *
 ********************************************************/

#if defined(cap_dwarf)

void pd_dwarf_handler(Dwarf_Error error, Dwarf_Ptr /*userData*/)
{
    if (error == NULL)
        return;

    char *dwarf_msg = dwarf_errmsg(error);
    string str = string("DWARF Error: ")+ dwarf_msg;
    dwarf_err_func(str.c_str());

    //bperr( "DWARF error: %s\n", dwarf_msg);
}

Dwarf_Signed declFileNo = 0;
char ** declFileNoToName = NULL;

bool Object::dwarf_parse_aranges(Dwarf_Debug dbg, std::set<Dwarf_Off>& dies_seen)
{
    Dwarf_Arange* ranges;
    Dwarf_Signed num_ranges;
    int status = dwarf_get_aranges(dbg, &ranges, &num_ranges, NULL);
    if(status != DW_DLV_OK) return false;
    Dwarf_Off cu_die_off;
    Dwarf_Die cu_die;
//    cout << "Processing " << num_ranges << "DWARF ranges" << endl;
    for(int i = 0; i < num_ranges; i++)
    {
        Dwarf_Addr start;
        Dwarf_Unsigned len, segment, segment_size;
        // TODO: info_b has segment info from DWARF4
        status = dwarf_get_arange_info_b(ranges[i], &segment, &segment_size, &start, &len, &cu_die_off, NULL);
        if(status == DW_DLV_OK)
        {
            if(segment_size > 0)
            {
                cout << "WARNING: ignoring segment info" << endl;
            }
            if(dies_seen.find(cu_die_off) != dies_seen.end()) continue;
            if(len == 0) continue;
            status = dwarf_offdie_b(dbg, cu_die_off, Dwarf_Bool(true), &cu_die, NULL);
            assert(status == DW_DLV_OK);
            std::string modname;
            if(!DwarfWalker::findDieName(dbg, cu_die, modname))
            {
                modname = associated_symtab->file(); // default module
            }
            Offset actual_start, actual_end;
            convertDebugOffset(start, actual_start);
            convertDebugOffset(start + len, actual_end);
            Module* m = associated_symtab->getOrCreateModule(modname, actual_start);
            m->addRange(actual_start, actual_end);
            m->addDebugInfo(cu_die);
            DwarfWalker::buildSrcFiles(dbg, cu_die, m->getStrings());
            dies_seen.insert(cu_die_off);
            dwarf_dealloc(dbg, ranges[i], DW_DLA_ARANGE);
        }
    }
    dwarf_dealloc(dbg, ranges, DW_DLA_LIST);
    return true;
}

bool Object::fix_global_symbol_modules_static_dwarf()
{
    /* Initialize libdwarf. */
    Dwarf_Debug *dbg_ptr = dwarf->type_dbg();
    if (!dbg_ptr)
        return false;
    Dwarf_Debug dbg = *dbg_ptr;
    std::set<Dwarf_Off> dies_seen;
    Dwarf_Off cu_die_off;
    Dwarf_Die cu_die;
    dwarf_parse_aranges(dbg, dies_seen);
    /* Iterate over the compilation-unit headers. */
    while (dwarf_next_cu_header_c(dbg, Dwarf_Bool(true),
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &cu_die_off, NULL) == DW_DLV_OK )
    {
        int status = dwarf_siblingof_b(dbg, NULL, Dwarf_Bool(true), &cu_die, NULL);
        if(status == DW_DLV_OK)
        {

            if(dies_seen.find(cu_die_off) != dies_seen.end()) continue;
            std::string modname;
            if(!DwarfWalker::findDieName(dbg, cu_die, modname))
            {
                modname = associated_symtab->file(); // default module
            }
//        cout << "Processing CU DIE for " << modname << endl;
            Address tempModLow;
            Address modLow = 0;
            if (DwarfWalker::findConstant(DW_AT_low_pc, tempModLow, cu_die, dbg)) {
                convertDebugOffset(tempModLow, modLow);
            }
            std::vector<AddressRange> mod_ranges = DwarfWalker::getDieRanges(dbg, cu_die, modLow);
            Module* m = associated_symtab->getOrCreateModule(modname, modLow);
            for(auto r = mod_ranges.begin();
                r != mod_ranges.end();
                ++r)
            {
                m->addRange(r->first, r->second);
            }
            if(!m->hasRanges())
            {
                Dwarf_Line* lines;
                Dwarf_Signed num_lines;
                if(dwarf_srclines(cu_die, &lines, &num_lines, NULL) == DW_DLV_OK)
                {
                    Dwarf_Addr low;
                    for(int i = 0; i < num_lines; ++i)
                    {
                        if((dwarf_lineaddr(lines[i], &low, NULL) == DW_DLV_OK) && low)
                        {
                            Dwarf_Bool is_end = false;
                            Dwarf_Addr high = low;
                            int result = DW_DLV_OK;
                            for(; (i < num_lines) &&
                                  (is_end == false) &&
                                  (result == DW_DLV_OK); ++i)
                            {
                                result = dwarf_lineendsequence(lines[i], &is_end, NULL);
                                if(result == DW_DLV_OK && is_end) {
                                    result = dwarf_lineaddr(lines[i], &high, NULL);
                                }

                            }
                            m->addRange(low, high);
                        }
                    }
                    dwarf_srclines_dealloc(dbg, lines, num_lines);
                }
            }
            m->addDebugInfo(cu_die);
            DwarfWalker::buildSrcFiles(dbg, cu_die, m->getStrings());
            dies_seen.insert(cu_die_off);
        }

    }

    return true;
}

#else

// dummy definition for non-DWARF platforms
bool Object::fix_global_symbol_modules_static_dwarf()
{ return false; }

#endif // cap_dwarf

/********************************************************
 *
 * For object files only....
 *  read the .stab section to find the module of global symbols
 *
 ********************************************************/

bool Object::fix_global_symbol_modules_static_stab(Elf_X_Shdr* stabscnp, Elf_X_Shdr* stabstrscnp)
{
    // Read the stab section to find the module of global symbols.
    // The symbols appear in the stab section by module. A module begins
    // with a symbol of type N_UNDF and ends with a symbol of type N_ENDM.
    // All the symbols in between those two symbols belong to the module.

    if (!stabscnp || !stabstrscnp) return false;

    Elf_X_Data stabdata = stabscnp->get_data();
    Elf_X_Data stabstrdata = stabstrscnp->get_data();
    stab_entry *stabptr = NULL;

    if (!stabdata.isValid() || !stabstrdata.isValid()) return false;

    switch (addressWidth_nbytes)
    {
        case 4:
            stabptr = new stab_entry_32(stabdata.d_buf(),
                                        stabstrdata.get_string(),
                                        stabscnp->sh_size() / sizeof(stab32));
            break;

        case 8:
            stabptr = new stab_entry_64(stabdata.d_buf(),
                                        stabstrdata.get_string(),
                                        stabscnp->sh_size() / sizeof(stab64));
            break;
    };

    const char *next_stabstr = stabptr->getStringBase();
    string module = "DEFAULT_MODULE";

    // the stabstr contains one string table for each module.
    // stabstr_offset gives the offset from the begining of stabstr of the
    // string table for the current module.

    bool is_fortran = false;  // is the current module fortran code?

    for (unsigned i = 0; i < stabptr->count(); i++)
    {
        switch(stabptr->type(i))
        {
            case N_UNDF: /* start of object file */
                stabptr->setStringBase(next_stabstr);
                next_stabstr = stabptr->getStringBase() + stabptr->val(i);
                break;

            case N_ENDM: /* end of object file */
                is_fortran = false;
                module = "DEFAULT_MODULE";
                break;

            case N_SO: /* compilation source or file name */
                if ((stabptr->desc(i) == N_SO_FORTRAN) || (stabptr->desc(i) == N_SO_F90))
                    is_fortran = true;

                module = string(stabptr->name(i));
                break;

            case N_ENTRY: /* fortran alternate subroutine entry point */
            case N_GSYM: /* global symbol */
                // the name string of a function or object appears in the stab
                // string table as <symbol name>:<symbol descriptor><other stuff>
                // where <symbol descriptor> is a one char code.
                // we must extract the name and descriptor from the string
            {
                const char *p = stabptr->name(i);
                // bperr("got %d type, str = %s\n", stabptr->type(i), p);
                // if (stabptr->type(i) == N_FUN && strlen(p) == 0) {

                if (strlen(p) == 0)
                {
                    // GNU CC 2.8 and higher associate a null-named function
                    // entry with the end of a function.  Just skip it.
                    break;
                }

                const char *q = strchr(p,':');
                unsigned len;

                if (q)
                {
                    len = q - p;
                }
                else
                {
                    len = strlen(p);
                }

                if (len == 0)
                {
                    // symbol name is empty.Skip it.- 02/12/07 -Giri
                    break;
                }

                char *sname = new char[len+1];
                strncpy(sname, p, len);
                sname[len] = 0;

                string SymName = string(sname);

                // q points to the ':' in the name string, so
                // q[1] is the symbol descriptor. We must check the symbol descriptor
                // here to skip things we are not interested in, such as prototypes.

                bool res = (symbols_.find(SymName)!=symbols_.end());

                if (!res && is_fortran)
                {
                    // Fortran symbols usually appear with an '_' appended in .symtab,
                    // but not on .stab
                    SymName += "_";
                    res = (symbols_.find(SymName)!=symbols_.end());
                }

                if (res && (q == 0 || q[1] != SD_PROTOTYPE))
                {
                    unsigned int count = 0;
                    std::vector< Symbol *> & syms = symbols_[SymName];

                    /* If there's only one, apply regardless. */
                    if ( syms.size() == 1 )
                    {
                        // TODO: set module
                        //		    symbols_[SymName][0]->setModuleName(module);
                    }
                    else
                    {
                        for ( unsigned int i = 0; i < syms.size(); i++ )
                        {
                            if ( syms[i]->getLinkage() == Symbol::SL_GLOBAL )
                            {
                                // TODO: set module
                                //			    symbols_[SymName][i]->setModuleName(module);
                                count++;
                            }
                        }
                    }
                }
                break;
            }
            case N_FUN:
                /* function */
            {
                const char *p = stabptr->name(i);

                if (strlen(p) == 0)
                {
                    // Rumours are that GNU CC 2.8 and higher associate a
                    // null-named function entry with the end of a
                    // function. Just skip it.
                    break;
                }

                const char *q = strchr(p,':');

                if (q == 0)
                {
                    // bperr( "Unrecognized stab format: %s\n", p);
                    // Happens with the Solaris native compiler (.xstabs entries?)
                    break;
                }

                if (q[1] == SD_PROTOTYPE)
                {
                    // We see a prototype, skip it
                    break;
                }

                unsigned long entryAddr = stabptr->val(i);

                if (entryAddr == 0)
                {
                    // The function stab doesn't contain a function address
                    // (happens with the Solaris native compiler). We have to
                    // look up the symbol by its name. That's unfortunate, since
                    // names may not be unique and we may end up assigning a wrong
                    // module name to the symbol.
                    unsigned len = q - p;
                    if (len == 0)
                    {
                        // symbol name is empty.Skip it.- 02/12/07 -Giri
                        break;
                    }

                    char *sname = new char[len+1];
                    strncpy(sname, p, len);
                    sname[len] = 0;
                    string nameFromStab = string(sname);
                    delete[] sname;

                    for (unsigned i = 0; i < symbols_[nameFromStab].size(); i++) {
                        symsToModules_[symbols_[nameFromStab][i]] = module;
                    }
                }
                else
                {
                    if (symsByOffset_.find(entryAddr)==symsByOffset_.end()) {
                        //bperr( "fix_global_symbol_modules_static_stab "
                        //	   "can't find address 0x%lx of STABS entry %s\n", entryAddr, p);
                        break;
                    }
                    for (unsigned i = 0; i < symsByOffset_[entryAddr].size(); i++) {
                        symsToModules_[symsByOffset_[entryAddr][i]] = module;
                    }
                }
                break;
            }

            default:
                /* ignore other entries */
                break;
        }
    }

    delete stabptr;

    return true;
}


// find_code_and_data(): populates the following members:
//   code_ptr_, code_off_, code_len_
//   data_ptr_, data_off_, data_len_
void Object::find_code_and_data(Elf_X &elf,
                                Offset txtaddr,
                                Offset dataddr)
{
    /* Note:
   * .o's don't have program headers, so these fields are populated earlier
   * when the sections are processed -> see loaded_elf()
   */

    for (int i = 0; i < elf.e_phnum(); ++i) {
        Elf_X_Phdr &phdr = elf.get_phdr(i);

        char *file_ptr = (char *)mf->base_addr();
        /*
      if(!isRegionPresent(phdr.p_paddr(), phdr.p_filesz(), phdr.p_flags())) {
      Region *reg = new Region(i, "", phdr.p_paddr(), phdr.p_filesz(),
      phdr.p_vaddr(), phdr.p_memsz(),
      &file_ptr[phdr.p_offset()],
      getSegmentPerms(phdr.p_flags()),
      getSegmentType(phdr.p_type(), phdr.p_flags()));
      reg->setFileOffset(phdr.p_offset());
      regions_.push_back(reg);
      }
    */
        // The code pointer, offset, & length should be set even if
        // txtaddr=0, so in this case we set these values by
        // identifying the segment that contains the entryAddress
        if (((phdr.p_vaddr() <= txtaddr) &&
             (phdr.p_vaddr() + phdr.p_filesz() >= txtaddr)) ||
            (!txtaddr && ((phdr.p_vaddr() <= entryAddress_) &&
                          (phdr.p_vaddr() + phdr.p_filesz() >= entryAddress_)))) {

            if (code_ptr_ == 0 && code_off_ == 0 && code_len_ == 0) {
                code_ptr_ = (char *)(void*)&file_ptr[phdr.p_offset()];
                code_off_ = (Offset)phdr.p_vaddr();
                code_len_ = (unsigned)phdr.p_filesz();
            }

        } else if (((phdr.p_vaddr() <= dataddr) &&
                    (phdr.p_vaddr() + phdr.p_filesz() >= dataddr)) ||
                   (!dataddr && (phdr.p_type() == PT_LOAD))) {
            if (data_ptr_ == 0 && data_off_ == 0 && data_len_ == 0) {
                data_ptr_ = (char *)(void *)&file_ptr[phdr.p_offset()];
                data_off_ = (Offset)phdr.p_vaddr();
                data_len_ = (unsigned)phdr.p_filesz();
            }
        }
    }
    //if (addressWidth_nbytes == 8) bperr( ">>> 64-bit find_code_and_data() successful\n");
}

const char *Object::elf_vaddr_to_ptr(Offset vaddr) const
{
    const char *ret = NULL;
    unsigned code_size_ = code_len_;
    unsigned data_size_ = data_len_;

    if (vaddr >= code_off_ && vaddr < code_off_ + code_size_) {
        ret = ((char *)code_ptr_) + (vaddr - code_off_);
    } else if (vaddr >= data_off_ && vaddr < data_off_ + data_size_) {
        ret = ((char *)data_ptr_) + (vaddr - data_off_);
    }

    return ret;
}

stab_entry *Object::get_stab_info() const
{
    char *file_ptr = (char *)mf->base_addr();

    // check that file has .stab info
    if (stab_off_ && stab_size_ && stabstr_off_) {
        switch (addressWidth_nbytes) {
            case 4: // 32-bit object
                return new stab_entry_32(file_ptr + stab_off_,
                                         file_ptr + stabstr_off_,
                                         stab_size_ / sizeof(stab32));
                break;
            case 8: // 64-bit object
                return new stab_entry_64(file_ptr + stab_off_,
                                         file_ptr + stabstr_off_,
                                         stab_size_ / sizeof(stab64));
                break;
        };
    }

    return new stab_entry_64();
}

Object::Object(MappedFile *mf_, bool, void (*err_func)(const char *),
               bool alloc_syms, Symtab* st) :
        AObject(mf_, err_func, st),
        elfHdr(NULL),
        hasReldyn_(false),
        hasReladyn_(false),
        hasRelplt_(false),
        hasRelaplt_(false),
        relType_(Region::RT_REL),
        isBlueGeneP_(false), isBlueGeneQ_(false),
        hasNoteSection_(false),
        elf_hash_addr_(0), gnu_hash_addr_(0),
        dynamic_offset_(0), dynamic_size_(0), dynsym_size_(0),
        init_addr_(0), fini_addr_(0),
        text_addr_(0), text_size_(0),
        symtab_addr_(0), strtab_addr_(0),
        dynamic_addr_(0), dynsym_addr_(0), dynstr_addr_(0),
        got_addr_(0), got_size_(0),
        plt_addr_(0), plt_size_(0), plt_entry_size_(0),
        rel_plt_addr_(0), rel_plt_size_(0), rel_plt_entry_size_(0),
        rel_addr_(0), rel_size_(0), rel_entry_size_(0),
        opd_addr_(0), opd_size_(0),
        stab_off_(0), stab_size_(0), stabstr_off_(0),
        stab_indx_off_(0), stab_indx_size_(0), stabstr_indx_off_(0),
        dwarvenDebugInfo(false),
        loadAddress_(0), entryAddress_(0),
        interpreter_name_(NULL),
        isStripped(false),
        dwarf(NULL),
        EEL(false), did_open(false),
        obj_type_(obj_Unknown),
        DbgSectionMapSorted(false),
        soname_(NULL)
{

#if defined(TIMED_PARSE)
    struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif
    is_aout_ = false;

    if(mf->getFD() != -1) {
        elfHdr = Elf_X::newElf_X(mf->getFD(), ELF_C_READ, NULL, mf_->pathname());
    }
    else {
        elfHdr = Elf_X::newElf_X((char *)mf->base_addr(), mf->size(), mf_->pathname());
    }

    // ELF header: sanity check
    //if (!elfHdr->isValid()|| !pdelf_check_ehdr(elfHdr))
    if (!elfHdr->isValid())  {
        log_elferror(err_func_, "ELF header");
        has_error = true;
        return;
    }
    else if (!pdelf_check_ehdr(*elfHdr)) {
        log_elferror(err_func_, "ELF header failed integrity check");
        has_error = true;
        return;
    }

    dwarf = DwarfHandle::createDwarfHandle(mf_->pathname(), elfHdr);
    if( elfHdr->e_type() == ET_DYN ) {
//        load_shared_object(alloc_syms);
            load_object(alloc_syms);
    }
    else if( elfHdr->e_type() == ET_REL || elfHdr->e_type() == ET_EXEC ) {
        // Differentiate between an executable and an object file
        if( elfHdr->e_phnum() ) is_aout_ = true;
        else is_aout_ = false;

        load_object(alloc_syms);
    }
    else {
        log_perror(err_func_,"Invalid filetype in Elf header");
        has_error = true;
        return;
    }

#ifdef BINEDIT_DEBUG
    print_symbol_map(&symbols_);
#endif
#if defined(TIMED_PARSE)
    struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << "obj parsing in Object-elf took "<<dursecs <<" msecs" << endl;
#endif
}

Object::~Object()
{
    relocation_table_.clear();
    fbt_.clear();
    allRegionHdrs.clear();
    versionMapping.clear();
    versionFileNameMapping.clear();
    deps_.clear();
}

void Object::log_elferror(void (*err_func)(const char *), const char* msg)
{
    const char* err = elf_errmsg(elf_errno());
    err = err ? err: "(bad elf error)";
    string str = string(err)+string(msg);
    err_func(str.c_str());
}

bool Object::get_func_binding_table(std::vector<relocationEntry> &fbt) const
{
#if !defined(os_vxworks)
    if(!plt_addr_ || (!fbt_.size())) return false;
#endif
    fbt = fbt_;
    return true;
}

bool Object::get_func_binding_table_ptr(const std::vector<relocationEntry> *&fbt) const
{
    if(!plt_addr_ || (!fbt_.size())) return false;
    fbt = &fbt_;
    return true;
}

void Object::getDependencies(std::vector<std::string> &deps){
    deps = deps_;
}

bool Object::addRelocationEntry(relocationEntry &re)
{
    relocation_table_.push_back(re);
    return true;
}

#ifdef DEBUG

// stream-based debug output
const ostream &Object::dump_state_info(ostream &s)
{
  s << "Debugging Information for Object (address) : " << this << endl;

  s << " <<begin debugging info for base object>>" << endl;
  AObject::dump_state_info(s);
  s << " <<end debuggingo info for base object>>" << endl;

  s << " dynsym_addr_ = " << dynsym_addr_ << endl;
  s << " dynstr_addr_ = " << dynstr_addr_ << endl;
  s << " got_addr_ = " << got_addr_ << endl;
  s << " plt_addr_ = " << plt_addr_ << endl;
  s << " plt_size_ = " << plt_size_ << endl;
  s << " plt_entry_size_ = " << plt_entry_size_ << endl;
  s << " rel_plt_addr_ = " << rel_plt_addr_ << endl;
  s << " rel_plt_size_ = " << rel_plt_size_ << endl;
  s << " rel_plt_entry_size_ = " << rel_plt_entry_size_ << endl;
  s << " rel_size_ = " << rel_size_ << endl;
  s << " rel_entry_size_ = " << rel_entry_size_ << endl;
  s << " stab_off_ = " << stab_off_ << endl;
  s << " stab_size_ = " << stab_size_ << endl;
  s << " stabstr_off_ = " << stabstr_off_ << endl;
  s << " dwarvenDebugInfo = " << dwarvenDebugInfo << endl;

  // and dump the relocation table....
  s << " fbt_ = (field seperator :: )" << endl;
  for (unsigned i=0; i < fbt_.size(); i++) {
    s << fbt_[i] << " :: ";
  }
  s << endl;

  return s;
}

#endif


Offset Object::getPltSlot(string funcName) const
{
    relocationEntry re;
    Offset offset=0;

    for ( unsigned int i = 0; i < fbt_.size(); i++ ){
        if (funcName == fbt_[i].name() ){
            offset =  fbt_[i].rel_addr();
        }
    }

    return offset;
}

//
// get_valid_memory_areas - get ranges of code/data segments that have
//                       sections mapped to them
//

void Object::get_valid_memory_areas(Elf_X &elf)
{
    for (unsigned i = 0; i < elf.e_shnum(); ++i) {
        Elf_X_Shdr &shdr = elf.get_shdr(i);
        if ( !shdr.isValid()) {
            break;
        }
        if (shdr.sh_flags() & SHF_ALLOC) { // This section is in memory
            if (code_off_ <= shdr.sh_addr() &&
                shdr.sh_addr() <= code_off_ + code_len_) {
                if (shdr.sh_addr() < code_vldS_)
                    code_vldS_ = shdr.sh_addr();
                if (shdr.sh_addr() + shdr.sh_size() > code_vldE_)
                    code_vldE_ = shdr.sh_addr() + shdr.sh_size();

            } else if (data_off_ <= shdr.sh_addr() &&
                       shdr.sh_addr() <= data_off_ + data_len_) {
                if (shdr.sh_addr() < data_vldS_)
                    data_vldS_ = shdr.sh_addr();
                if (shdr.sh_addr() + shdr.sh_size() > data_vldE_)
                    data_vldE_ = shdr.sh_addr() + shdr.sh_size();
            }
        }
    }
}

//
// parseCompilerType - parse for compiler that was used to generate object
//
//
//
#if defined(os_linux)
// Differentiating between g++ and pgCC by stabs info (as in the solaris/
// aix case, below) will not work; the gcc-compiled object files that
// get included at link time will fill in the N_OPT stabs line. Instead,
// look for "pgCC_compiled." symbols.
bool parseCompilerType(Object *objPtr)
{
    dyn_hash_map<string, std::vector<Symbol *> >*syms = objPtr->getAllSymbols();
    if(syms->find("pgCC_compiled.") != syms->end())
        return true;
    return false;
}
#else
bool parseCompilerType(Object *objPtr)
{
  stab_entry *stabptr = objPtr->get_stab_info();
  const char *next_stabstr = stabptr->getStringBase();

  for (unsigned int i=0; i < stabptr->count(); ++i) {
    // if (stabstrs) bperr("parsing #%d, %s\n", stabptr->type(i), stabptr->name(i));
    switch (stabptr->type(i)) {

    case N_UNDF: /* start of object file */
      /* value contains offset of the next string table for next module */
      // assert(stabptr.nameIdx(i) == 1);
      stabptr->setStringBase(next_stabstr);
      next_stabstr = stabptr->getStringBase() + stabptr->val(i);
      break;

    case N_OPT: /* Compiler options */
      delete stabptr;
      return false;
    }
  }
  delete stabptr;
  return false; // Shouldn't happen - maybe N_OPT stripped
}
#endif


#if (defined(os_linux) || defined(os_freebsd))

static unsigned long read_uleb128(const unsigned char *data, unsigned *bytes_read)
{
    unsigned long result = 0;
    unsigned shift = 0;
    *bytes_read = 0;
    while (1)
    {
        result |= (data[*bytes_read] & 0x7f) << shift;
        if ((data[(*bytes_read)++] & 0x80) == 0)
            break;
        shift += 7;
    }
    return result;
}

static signed long read_sleb128(const unsigned char *data, unsigned *bytes_read)
{
    unsigned long result = 0;
    unsigned shift = 0;
    *bytes_read = 0;
    while (1)
    {
        result |= (data[*bytes_read] & 0x7f) << shift;
        shift += 7;
        if ((data[*bytes_read] & 0x80) == 0)
            break;
        (*bytes_read)++;
    }
    if (shift < sizeof(int) && (data[*bytes_read] & 0x40))
        result |= -(1 << shift);
    (*bytes_read)++;

    return result;
}

#define DW_EH_PE_absptr      0x00
#define DW_EH_PE_uleb128     0x01
#define DW_EH_PE_udata2      0x02
#define DW_EH_PE_udata4      0x03
#define DW_EH_PE_udata8      0x04
#define DW_EH_PE_sleb128     0x09
#define DW_EH_PE_sdata2      0x0A
#define DW_EH_PE_sdata4      0x0B
#define DW_EH_PE_sdata8      0x0C
#define DW_EH_PE_pcrel       0x10
#define DW_EH_PE_textrel     0x20
#define DW_EH_PE_datarel     0x30
#define DW_EH_PE_funcrel     0x40
#define DW_EH_PE_aligned     0x50
#define DW_EH_PE_indirect    0x80
#define DW_EH_PE_omit        0xff

typedef struct {
    int word_size;
    unsigned long pc;
    unsigned long text;
    unsigned long data;
    unsigned long func;
} mach_relative_info;

static int read_val_of_type(int type, unsigned long *value, const unsigned char *addr,
                            const mach_relative_info &mi)
{
    unsigned size = 0;
    if (type == DW_EH_PE_omit)
        return 0;

    unsigned long base = 0x0;
    /**
   * LSB Standard says that the upper four bits (0xf0) encode the base.
   * Except that none of these values should their upper bits set,
   * and I'm finding the upper bit seems to sometimes contain garbage.
   * gcc uses the 0x70 bits in its exception parsing, so that's what we'll do.
   **/
    switch (type & 0x70)
    {
        case DW_EH_PE_pcrel:
            base = mi.pc;
            break;
        case DW_EH_PE_textrel:
            base = mi.text;
            break;
        case DW_EH_PE_datarel:
            base = mi.data;
            break;
        case DW_EH_PE_funcrel:
            base = mi.func;
            break;
    }

    if ((type & 0x70) == DW_EH_PE_aligned)
    {
        if (mi.word_size == 4) {
            addr = (const unsigned char*)(((unsigned long)addr + 3) & (~0x3l));
        }
        else if (mi.word_size == 8) {
            addr = (const unsigned char*)(((unsigned long)addr + 7) & (~0x7l));
        }
    }


    switch (type & 0x0f)
    {
        case DW_EH_PE_absptr:
            if (mi.word_size == 4) {
                *value = (unsigned long) *((const uint32_t *) addr);
                size = 4;
            }
            else if (mi.word_size == 8) {
                *value = (unsigned long) *((const uint64_t *) addr);
                size = 8;
            }
            break;
        case DW_EH_PE_uleb128:
            *value = read_uleb128(addr, &size);
            break;
        case DW_EH_PE_sleb128:
            *value = read_sleb128(addr, &size);
            break;
        case DW_EH_PE_udata2:
            *value = *((const uint16_t *) addr);
            size = 2;
            break;
        case DW_EH_PE_sdata2:
            *value = *((const int16_t *) addr);
            size = 2;
            break;
        case DW_EH_PE_udata4:
            *value = *((const uint32_t *) addr);
            size = 4;
            break;
        case DW_EH_PE_sdata4:
            *value = *((const int32_t *) addr);
            size = 4;
            break;
        case DW_EH_PE_udata8:
            *value = *((const uint64_t *) addr);
            size = 8;
            break;
        case DW_EH_PE_sdata8:
            *value = *((const int64_t *) addr);
            size = 8;
            break;
        default:
            fprintf(stderr, "Unhandled type %d\n", type & 0x0f);
            return -1;
    }

    if (*value) {
        *value += base;
        if (type & DW_EH_PE_indirect) {
            // When there is a indirect catch block,
            // it is often the case that the indirect pointer would point
            // to a dynamic section, not resolvable at static time.
            // We currently ignore this dynamic catch block.
            *value = 0;
        }
    }
    return size;
}

/**
 * On GCC 3.x/x86 we find catch blocks as follows:
 *   1. We start with a list of FDE entries in the .eh_frame
 *      table.
 *   2. Each FDE entry has a pointer to a  CIE entry.  The CIE
 *      tells us whether the FDE has any 'Augmentations', and
 *      the types of those augmentations.  The FDE also
 *      contains a pointer to a function.
 *   3. If one of the FDE augmentations is a 'Language Specific Data Area'
 *      then we have a pointer to one or more entires in the gcc_except_table.
 *   4. The gcc_except_table contains entries that point
 *      to try and catch blocks, all encoded as offsets
 *      from the function start (it doesn't tell you which
 *      function, however).
 *   5. We can add the function offsets from the gcc_except_table
 *      to the function pointer from the FDE to get all of
 *      the try/catch blocks.
 **/
#define SHORT_FDE_HLEN 4
#define LONG_FDE_HLEN 12
static
int read_except_table_gcc3(Dwarf_Fde *fde_data, Dwarf_Signed fde_count,
                           mach_relative_info &mi,
                           Elf_X_Shdr *eh_frame, Elf_X_Shdr *except_scn,
                           std::vector<ExceptionBlock> &addresses)
{
    Dwarf_Error err = (Dwarf_Error) NULL;
    Dwarf_Addr low_pc;
    Dwarf_Unsigned bytes_in_cie;
    Dwarf_Off fde_offset, cie_offset;
    Dwarf_Fde fde;
    Dwarf_Cie cie;
    int status, result, ptr_size;
    char *augmentor;
    unsigned char lpstart_format, ttype_format, table_format;
    unsigned long value, table_end, region_start, region_size, landingpad_base;
    unsigned long catch_block, action, augmentor_len;
    Dwarf_Small *fde_augdata, *cie_augdata;
    Dwarf_Unsigned fde_augdata_len, cie_augdata_len;

    //For each FDE
    for (int i = 0; i < fde_count; i++) {
        unsigned int j;
        unsigned char lsda_encoding = 0xff, personality_encoding = 0xff;
        unsigned char *lsda_ptr = NULL;
        unsigned char *cur_augdata;
        unsigned long except_off;
        unsigned long fde_addr, cie_addr;
        unsigned char *fde_bytes, *cie_bytes;

        //Get the FDE
        status = dwarf_get_fde_n(fde_data, (Dwarf_Unsigned) i, &fde, &err);
        if (status != DW_DLV_OK) {
            pd_dwarf_handler(err, NULL);
            return false;
        }

        //After this set of computations we should have:
        // low_pc = mi.func = the address of the function that contains this FDE
        // fde_bytes = the start of the FDE in our memory space
        // cie_bytes = the start of the CIE in our memory space
        status = dwarf_get_fde_range(fde, &low_pc, NULL, (void **) &fde_bytes,
                                     NULL, &cie_offset, NULL,
                                     &fde_offset, &err);
        if (status != DW_DLV_OK) {
            pd_dwarf_handler(err, NULL);
            return false;
        }
        //The LSB strays from the DWARF here, when parsing the except_eh section
        // the cie_offset is relative to the FDE rather than the start of the
        // except_eh section.
        cie_offset = fde_offset - cie_offset +
                     (*(uint32_t*)fde_bytes == 0xffffffff ? LONG_FDE_HLEN : SHORT_FDE_HLEN);
        cie_bytes = (unsigned char *)eh_frame->get_data().d_buf() + cie_offset;

        //Get the CIE for the FDE
        status = dwarf_get_cie_of_fde(fde, &cie, &err);
        if (status != DW_DLV_OK) {
            pd_dwarf_handler(err, NULL);
            return false;
        }

        //Get the Augmentation string for the CIE
        status = dwarf_get_cie_info(cie, &bytes_in_cie, NULL, &augmentor,
                                    NULL, NULL, NULL, NULL, NULL, &err);
        if (status != DW_DLV_OK) {
            pd_dwarf_handler(err, NULL);
            return false;
        }

        //Check that the string pointed to by augmentor has a 'L',
        // meaning we have a LSDA
        augmentor_len = (augmentor == NULL) ? 0 : strlen(augmentor);
        for (j = 0; j < augmentor_len; j++) {
            if (augmentor[j] == 'L') {
                break;
            }
        }
        if (j == augmentor_len)
            //If we don't have a language specific data area, then
            // we don't care about this FDE.
            continue;

        //Some ptr encodings may be of type DW_EH_PE_pcrel, which means
        // that they're relative to their own location in the binary.
        // We'll figure out where the FDE and CIE original load addresses
        // were and use those in pcrel computations.
        fde_addr = eh_frame->sh_addr() + fde_offset;
        cie_addr = eh_frame->sh_addr() + cie_offset;

        //Extract encoding information from the CIE.
        // The CIE may have augmentation data, specified in the
        // Linux Standard Base. The augmentation string tells us how
        // which augmentation data is present.  We only care about one
        // field, a byte telling how the LSDA pointer is encoded.
        status = dwarf_get_cie_augmentation_data(cie,
                                                 &cie_augdata,
                                                 &cie_augdata_len,
                                                 &err);
        if (status != DW_DLV_OK) {
            pd_dwarf_handler(err, NULL);
            return false;
        }

        cur_augdata = (unsigned char *) cie_augdata;
        lsda_encoding = DW_EH_PE_omit;
        for (j=0; j<augmentor_len; j++)
        {
            if (augmentor[j] == 'L')
            {
                lsda_encoding = *cur_augdata;
                cur_augdata++;
            }
            else if (augmentor[j] == 'P')
            {
                //We don't actually need the personality info, but we extract it
                // anyways to make sure we properly extract the LSDA.
                personality_encoding = *cur_augdata;
                cur_augdata++;
                unsigned long personality_val;
                mi.pc = cie_addr + (unsigned long) (cur_augdata - cie_bytes);
                cur_augdata += read_val_of_type(personality_encoding,
                                                &personality_val, cur_augdata, mi);
            }
            else if (augmentor[j] == 'z' || augmentor[j] == 'R')
            {
                //Do nothing, these don't affect the CIE encoding.
            }
            else
            {
                //Fruit, Someone needs to check the Linux Standard Base,
                // section 11.6 (as of v3.1), to see what new encodings
                // exist and how we should decode them in the CIE.
                dwarf_printf("WARNING: Unhandled augmentation %c\n", augmentor[j]);
                break;
            }
        }
        if (lsda_encoding == DW_EH_PE_omit)
            continue;


        //Read the LSDA pointer out of the FDE.
        // The FDE has an augmentation area, similar to the above one in the CIE.
        // Where-as the CIE augmentation tends to contain things like bytes describing
        // pointer encodings, the FDE contains the actual pointers.
        status = dwarf_get_fde_augmentation_data(fde,
                                                 &fde_augdata,
                                                 &fde_augdata_len,
                                                 &err);
        if (status != DW_DLV_OK) {
            pd_dwarf_handler(err, NULL);
            return false;
        }
        cur_augdata = (unsigned char *) fde_augdata;
        for (j=0; j<augmentor_len; j++)
        {
            if (augmentor[j] == 'L')
            {
                unsigned long lsda_val;
                mi.pc = fde_addr + (unsigned long) (cur_augdata - fde_bytes);
                ptr_size = read_val_of_type(lsda_encoding, &lsda_val, cur_augdata, mi);
                if (ptr_size == -1)
                    break;
                lsda_ptr = (unsigned char *) lsda_val;
                cur_augdata += ptr_size;
            }
            else if (augmentor[j] == 'P' ||
                     augmentor[j] == 'z' ||
                     augmentor[j] == 'R')
            {
                //These don't affect the FDE augmentation data, do nothing
            }
            else
            {
                //See the comment for the 'else' case in the above CIE parsing
                break;
            }
        }
        if (!lsda_ptr)
            //Many FDE's have an LSDA area, but then have a NULL LSDA ptr.
            // Just means there's no exception info here.
            continue;

        // Get the exception data from the section.
        Elf_X_Data data = except_scn->get_data();
        if (!data.isValid()) {
            return false;
        }

        const unsigned char *datap = (const unsigned char *) data.get_string();
        unsigned long int except_size = data.d_size();

        except_off = (unsigned long) (lsda_ptr - except_scn->sh_addr());
        if (except_off >= except_size) {
            continue;
        }

        // Read the landing pad base address.
        // If it is omitted, the base address should be
        // the entry address of the function involved
        // in the exception handling
        lpstart_format = datap[except_off++];
        if (lpstart_format != DW_EH_PE_omit) {
            except_off += read_val_of_type(DW_EH_PE_uleb128, &landingpad_base, datap + except_off, mi);
        } else {
            landingpad_base = low_pc;
        }
        ttype_format = datap[except_off++];
        if (ttype_format != DW_EH_PE_omit)
            except_off += read_val_of_type(DW_EH_PE_uleb128, &value, datap + except_off, mi);

        // This 'type' byte describes the data format of the entries in the
        // table and the format of the table_size field.
        table_format = datap[except_off++];
        mi.pc = except_scn->sh_addr() + except_off;

//  This assertion would fail:
//  assert(table_format == DW_EH_PE_uleb128);
//  result = read_val_of_type(table_format, &table_end, datap + except_off, mi);
//  This read should always be a ULEB128 read
        result = read_val_of_type(DW_EH_PE_uleb128, &table_end, datap + except_off, mi);
        if (result == -1) {
            continue;
        }
        except_off += result;
        table_end += except_off;

        while (except_off < table_end && except_off < except_size) {
            Offset tryStart;
            Offset tryEnd;
            Offset catchStart;

            //The entries in the gcc_except_table are the following format:
            //   <type>   region start
            //   <type>   region length
            //   <type>   landing pad
            //  uleb128   action
            //The 'region' is the try block, the 'landing pad' is the catch.
            mi.pc = except_scn->sh_addr() + except_off;
            tryStart = except_off;
            //cerr << "Reading tryStart at " <<  except_off;

            except_off += read_val_of_type(table_format, &region_start,
                                           datap + except_off, mi);
            mi.pc = except_scn->sh_addr() + except_off;
            tryEnd = except_off;
            //cerr << "Reading tryEnd at " << except_off;
            except_off += read_val_of_type(table_format, &region_size,
                                           datap + except_off, mi);
            mi.pc = except_scn->sh_addr() + except_off;
            catchStart = except_off;
            //cerr << "Reading catchStart at " <<  except_off;
            except_off += read_val_of_type(table_format, &catch_block,
                                           datap + except_off, mi);
            //cerr << "Reading action (uleb128) at " << except_off;
            except_off += read_val_of_type(DW_EH_PE_uleb128, &action,
                                           datap + except_off, mi);

            if (catch_block == 0)
                continue;
            ExceptionBlock eb(region_start + landingpad_base, (unsigned) region_size,
                              catch_block + landingpad_base);
            eb.setTryStart(tryStart);
            eb.setTryEnd(tryEnd);
            eb.setCatchStart(catchStart);

            addresses.push_back(eb);
        }
    }

    return true;
}

/**
 * Things were much simpler in the old days.  On gcc 2.x
 * the gcc_except_table looks like:
 *   <long> try_start
 *   <long> try_end
 *   <long> catch_start
 * Where everything is an absolute address, even when compiled
 * with PIC.  All we got to do is read the catch_start entries
 * out of it.
 **/
static bool read_except_table_gcc2(Elf_X_Shdr *except_table,
                                   std::vector<ExceptionBlock> &addresses,
                                   const mach_relative_info &mi)
{
    Offset try_start = (Offset) -1;
    Offset try_end = (Offset) -1;
    Offset catch_start = 0;

    Elf_X_Data data = except_table->get_data();
    const unsigned char *datap = (const unsigned char *)data.get_string();
    unsigned long except_size = data.d_size();

    unsigned i = 0;
    while (i < except_size) {
        ExceptionBlock eb;
        if (mi.word_size == 4) {
            i += read_val_of_type(DW_EH_PE_udata4, &try_start, datap + i, mi);
            i += read_val_of_type(DW_EH_PE_udata4, &try_end, datap + i, mi);
            i += read_val_of_type(DW_EH_PE_udata4, &catch_start, datap + i, mi);
        }
        else if (mi.word_size == 8) {
            i += read_val_of_type(DW_EH_PE_udata8, &try_start, datap + i, mi);
            i += read_val_of_type(DW_EH_PE_udata8, &try_end, datap + i, mi);
            i += read_val_of_type(DW_EH_PE_udata8, &catch_start, datap + i, mi);
        }

        if (try_start != (Offset) -1 && try_end != (Offset) -1) {
            ExceptionBlock eb(try_start, (unsigned) (try_end - try_start), catch_start);
            addresses.push_back(eb);
        }
    }
    return true;
}

struct  exception_compare: public binary_function<const ExceptionBlock &, const ExceptionBlock &, bool>
{
    bool operator()(const ExceptionBlock &e1, const ExceptionBlock &e2) {
        if (e1.tryStart() < e2.tryStart())
            return true;
        return false;
    }
};

/**
 * Finds the addresses of catch blocks in a g++ generated elf file.
 *  'except_scn' should point to the .gcc_except_table section
 *  'eh_frame' should point to the .eh_frame section
 *  the addresses will be pushed into 'addresses'
 **/
bool Object::find_catch_blocks(Elf_X_Shdr *eh_frame,
                               Elf_X_Shdr *except_scn,
                               Address txtaddr, Address dataaddr,
                               std::vector<ExceptionBlock> &catch_addrs)
{
    Dwarf_Cie *cie_data;
    Dwarf_Fde *fde_data;
    Dwarf_Signed cie_count, fde_count;
    Dwarf_Error err = (Dwarf_Error) NULL;
    Dwarf_Unsigned bytes_in_cie;
    char *augmentor;
    int status, gcc_ver = 3;
    unsigned i;
    bool result = false;

    if (except_scn == NULL) {
        //likely to happen if we're not using gcc
        return true;
    }

    Dwarf_Debug *dbg_ptr = dwarf->frame_dbg();
    if (!dbg_ptr) {
        pd_dwarf_handler(err, NULL);
        return false;
    }
    Dwarf_Debug &dbg = *dbg_ptr;

    //Read the FDE and CIE information
    status = dwarf_get_fde_list_eh(dbg, &cie_data, &cie_count,
                                   &fde_data, &fde_count, &err);
    if (status != DW_DLV_OK) {
        //No actual stackwalk info in this object
        return false;
    }

    mach_relative_info mi;
    mi.text = txtaddr;
    mi.data = dataaddr;
    mi.pc = 0x0;
    mi.func = 0x0;
    mi.word_size = eh_frame->wordSize();


    //GCC 2.x has "eh" as its augmentor string in the CIEs
    for (i = 0; i < cie_count; i++) {
        status = dwarf_get_cie_info(cie_data[i], &bytes_in_cie, NULL,
                                    &augmentor, NULL, NULL, NULL, NULL, NULL, &err);
        if (status != DW_DLV_OK) {
            pd_dwarf_handler(err, NULL);
            goto cleanup;
        }
        if (augmentor[0] == 'e' && augmentor[1] == 'h') {
            gcc_ver = 2;
        }
    }

    //Parse the gcc_except_table
    if (gcc_ver == 2) {
        result = read_except_table_gcc2(except_scn, catch_addrs, mi);

    } else if (gcc_ver == 3) {
        result = read_except_table_gcc3(fde_data, fde_count, mi,
                                        eh_frame, except_scn,
                                        catch_addrs);
    }
    sort(catch_addrs.begin(),catch_addrs.end(),exception_compare());
    //VECTOR_SORT(catch_addrs, exception_compare);

    cleanup:
    //Unallocate fde and cie information
    for (i = 0; i < cie_count; i++)
        dwarf_dealloc(dbg, cie_data[i], DW_DLA_CIE);
    for (i = 0; i < fde_count; i++)
        dwarf_dealloc(dbg, fde_data[i], DW_DLA_FDE);
    dwarf_dealloc(dbg, cie_data, DW_DLA_LIST);
    dwarf_dealloc(dbg, fde_data, DW_DLA_LIST);

    return result;
}

#endif

ObjectType Object::objType() const
{
    return obj_type_;
}

void Object::getModuleLanguageInfo(dyn_hash_map<string, supportedLanguages> *mod_langs)
{
    string working_module;
    const char *ptr;
    // check .stabs section to get language info for modules:
    //   int stab_nsyms;
    //   char *stabstr_nextoffset;
    //   const char *stabstrs = 0;

    string mod_string;

    // This ugly flag is set when certain (sun) fortran compilers are detected.
    // If it is set at any point during the following iteration, this routine
    // ends with "backtrack mode" and reiterates through all chosen languages, changing
    // lang_Fortran to lang_Fortran_with_pretty_debug.
    //
    // This may be ugly, but it is set up this way since the information that is used
    // to determine whether this flag is set comes from the N_OPT field, which
    // seems to come only once per image.  The kludge is that we assume that all
    // fortran sources in the module have this property (they probably do, but
    // could conceivably be mixed (???)).
    int fortran_kludge_flag = 0;

    // "state variables" we use to accumulate potentially useful information
    //  A final module<->language decision is not made until we have arrived at the
    //  next module entry, at which point we use any and all info we have to
    //  make the most sensible guess
    supportedLanguages working_lang = lang_Unknown;
    char *working_options = NULL;
    const char *working_name = NULL;

    stab_entry *stabptr = NULL;
    const char *next_stabstr = NULL;
#if defined(TIMED_PARSE)
    struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

    //Using the Object to get the pointers to the .stab and .stabstr
    // XXX - Elf32 specific needs to be in seperate file -- jkh 3/18/99
    stabptr = get_stab_info();
    next_stabstr = stabptr->getStringBase();

    for( unsigned int i = 0; i < stabptr->count(); i++ )
    {
        if (stabptr->type(i) == N_UNDF)
        {/* start of object file */
            /* value contains offset of the next string table for next module */
            // assert(stabptr->nameIdx(i) == 1);
            stabptr->setStringBase(next_stabstr);
            next_stabstr = stabptr->getStringBase() + stabptr->val(i);
        }
        else if (stabptr->type(i) == N_OPT)
        {
            //  We can use the compiler option string (in a pinch) to guess at the source file language
            //  There is possibly more useful information encoded somewhere around here, but I lack
            //  an immediate reference....
            if (working_name)
                working_options = const_cast<char *>(stabptr->name(i));
        }
        else if ((stabptr->type(i) == N_SO)  || (stabptr->type(i) == N_ENDM))
        { /* compilation source or file name */
            // We have arrived at the next source file, finish up with the last one and reset state
            // before starting next


            //   XXXXXXXXXXX  This block is mirrored near the end of routine, if you edit it,
            //   XXXXXXXXXXX  change it there too.
            if  (working_name)
            {
                working_lang = pickLanguage(working_module, working_options, working_lang);
                if (working_lang == lang_Fortran_with_pretty_debug)
                    fortran_kludge_flag = 1;
                (*mod_langs)[working_module] = working_lang;

            }
            //   XXXXXXXXXXX

            // reset "state" here
            working_lang = lang_Unknown;
            working_options = NULL;

            //  Now:  out with the old, in with the new

            if (stabptr->type(i) == N_ENDM)
            {
                // special case:
                // which is most likely both broken (and ignorable ???)
                working_name = "DEFAULT_MODULE";
            }
            else
            {
                working_name = stabptr->name(i);
                ptr = strrchr(working_name, '/');
                if (ptr)
                {
                    ptr++;
                    working_name = ptr;
                }
            }
            working_module = string(working_name);

            if ((mod_langs->find(working_module) != mod_langs->end()) && (*mod_langs)[working_module] != lang_Unknown)
            {
                //  we already have a module with this name in the map.  If it has been given
                //  a language assignment (not lang_Unknown), we can just skip ahead
                working_name = NULL;
                working_options = NULL;
                continue;
            }
            else
            {
                //cerr << __FILE__ << __LINE__ << ":  Module: " <<working_module<< " has language "<< stabptr->desc(i) << endl;
                switch (stabptr->desc(i))
                {
                    case N_SO_FORTRAN:
                        working_lang = lang_Fortran;
                        break;
                    case N_SO_F90:
                        working_lang = lang_Fortran;  // not sure if this should be different from N_SO_FORTRAN
                        break;
                    case N_SO_AS:
                        working_lang = lang_Assembly;
                        break;
                    case N_SO_ANSI_C:
                    case N_SO_C:
                        working_lang = lang_C;
                        break;
                    case N_SO_CC:
                        working_lang = lang_CPlusPlus;
                        break;
                    default:
                        //  currently uncovered options are lang_CMFortran, and lang_GnuCPlusPlus
                        //  do we need to make this kind of distinction here?
                        working_lang = lang_Unknown;
                        break;
                }

            }
        } // end N_SO section
    } // for loop

    //  Need to make sure we finish up with the module we were last collecting information
    //  about

    //   XXXXXXXXXXX  see note above (find the X's)
    if  (working_name)
    {
        working_lang = pickLanguage(working_module, working_options, working_lang);
        if (working_lang == lang_Fortran_with_pretty_debug)
            fortran_kludge_flag = 1;
        (*mod_langs)[working_module] = working_lang;
    }
    //   XXXXXXXXXXX

    if (fortran_kludge_flag)
    {
        //  XXX  This code does not appear to be used anymore??
        // go through map and change all lang_Fortran to lang_Fortran_with_pretty_symtab
        dyn_hash_map<string, supportedLanguages>::iterator iter = (*mod_langs).begin();
        string aname;
        supportedLanguages alang;
        for(;iter!=(*mod_langs).end();iter++)
        {
            aname = iter->first;
            alang = iter->second;
            if(lang_Fortran == alang)
            {
                (*mod_langs)[aname] = lang_Fortran_with_pretty_debug;
            }
        }
    }
#if defined(TIMED_PARSE)
    struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": getModuleLanguageInfo took "<<dursecs <<" msecs" << endl;
#endif
    delete stabptr;

#if defined(cap_dwarf)
    if (hasDwarfInfo())
    {
        int status;
        Dwarf_Debug *dbg_ptr = dwarf->type_dbg();
        if (!dbg_ptr)
            return;
        Dwarf_Debug &dbg = *dbg_ptr;

        Dwarf_Unsigned hdr;
        char * moduleName = NULL;
        Dwarf_Die moduleDIE = NULL;
        Dwarf_Attribute languageAttribute = NULL;
        bool done = false;

        /* Only .debug_info for now, not .debug_types */
        Dwarf_Bool is_info = 1;

        while( !done &&
               dwarf_next_cu_header_c( dbg, is_info,
                                       NULL, NULL, NULL, // len, stamp, abbrev
                                       NULL, NULL, NULL, // address, offset, extension
                                       NULL, NULL, // signature, typeoffset
                                       & hdr, NULL ) == DW_DLV_OK )
        {
            Dwarf_Half moduleTag;
            Dwarf_Unsigned languageConstant;

            status = dwarf_siblingof_b(dbg, NULL, is_info, &moduleDIE, NULL);
            if (status != DW_DLV_OK) {
                done = true;
                goto cleanup_dwarf;
            }

            status = dwarf_tag( moduleDIE, & moduleTag, NULL);
            if (status != DW_DLV_OK || moduleTag != DW_TAG_compile_unit) {
                done = true;
                goto cleanup_dwarf;
            }

            /* Extract the name of this module. */
            status = dwarf_diename( moduleDIE, & moduleName, NULL );
            if (status != DW_DLV_OK || !moduleName) {
                done = true;
                goto cleanup_dwarf;
            }
            ptr = strrchr(moduleName, '/');
            if (ptr)
                ptr++;
            else
                ptr = moduleName;

            working_module = string(ptr);

            status = dwarf_attr( moduleDIE, DW_AT_language, & languageAttribute, NULL );
            if (status == DW_DLV_ERROR) {
                done = true;
                goto cleanup_dwarf;
            }

            status = dwarf_formudata( languageAttribute, & languageConstant, NULL );
            if (status != DW_DLV_OK) {
                done = true;
                goto cleanup_dwarf;
            }

            switch( languageConstant )
            {
                case DW_LANG_C:
                case DW_LANG_C89:
                case DW_LANG_C99:
#ifdef DW_LANG_C11
                    case DW_LANG_C11:
#endif
                    (*mod_langs)[working_module] = lang_C;
                    break;
                case DW_LANG_C_plus_plus:
#ifdef DW_LANG_C_plus_plus_03
                    case DW_LANG_C_plus_plus_03:
#endif
#ifdef DW_LANG_C_plus_plus_11
                    case DW_LANG_C_plus_plus_11:
#endif
                    (*mod_langs)[working_module] = lang_CPlusPlus;
                    break;
                case DW_LANG_Fortran77:
                case DW_LANG_Fortran90:
                case DW_LANG_Fortran95:
                    (*mod_langs)[working_module] = lang_Fortran;
                    break;
                default:
                    /* We know what the language is but don't care. */
                    break;
            } /* end languageConstant switch */
            cleanup_dwarf:
            if (languageAttribute)
                dwarf_dealloc( dbg, languageAttribute, DW_DLA_ATTR );
            if (moduleName)
                dwarf_dealloc( dbg, moduleName, DW_DLA_STRING );
            if (moduleDIE)
                dwarf_dealloc( dbg, moduleDIE, DW_DLA_DIE );
            languageAttribute = NULL;
            moduleName = NULL;
            moduleDIE = NULL;
        }

    }
#endif

}

bool AObject::getSegments(vector<Segment> &segs) const
{
    unsigned i;
    for(i=0;i<regions_.size();i++)
    {
        if((regions_[i]->getRegionName() == ".text") || (regions_[i]->getRegionName() == ".init") || (regions_[i]->getRegionName() == ".fini") ||
           (regions_[i]->getRegionName() == ".rodata") || (regions_[i]->getRegionName() == ".plt") || (regions_[i]->getRegionName() == ".data"))
        {
            Segment seg;
            seg.data = regions_[i]->getPtrToRawData();
            seg.loadaddr = regions_[i]->getDiskOffset();
            seg.size = regions_[i]->getDiskSize();
            seg.name = regions_[i]->getRegionName();
            //            seg.segFlags = regions_[i]->getFlags();
            segs.push_back(seg);
        }
    }
    return true;
}


bool Object::emitDriver(string fName, std::vector<Symbol *> &allSymbols, unsigned)
{
#ifdef BINEDIT_DEBUG
    printf("emitting...\n");
  //print_symbol_map(&symbols_);
  print_symbols(allSymbols);
  printf("%d total symbol(s)\n", allSymbols.size());
#endif
    if (elfHdr->e_ident()[EI_CLASS] == ELFCLASS32)
    {
        Dyninst::SymtabAPI::emitElf<Dyninst::SymtabAPI::ElfTypes32> *em =
                new Dyninst::SymtabAPI::emitElf<Dyninst::SymtabAPI::ElfTypes32>(elfHdr, isStripped, this, err_func_, associated_symtab);
        bool ok = em->createSymbolTables(allSymbols);
        if(ok)
        {
            ok = em->driver(fName);
        }
        delete em;
        return ok;
    }
    else if (elfHdr->e_ident()[EI_CLASS] == ELFCLASS64)
    {
        Dyninst::SymtabAPI::emitElf<Dyninst::SymtabAPI::ElfTypes64> *em =
                new Dyninst::SymtabAPI::emitElf<Dyninst::SymtabAPI::ElfTypes64>(elfHdr, isStripped, this, err_func_, associated_symtab);
        bool ok = em->createSymbolTables(allSymbols);
        if(ok)
        {
            ok = em->driver(fName);
        }
        delete em;
        return ok;
    }
    return false;
}

const char *Object::interpreter_name() const
{
    return interpreter_name_;
}

/* Parse everything in the file on disk, and cache that we've done so,
   because our modules may not bear any relation to the name source files. */
void Object::parseStabFileLineInfo()
{
    static dyn_hash_map< string, bool > haveParsedFileMap;

    /* We haven't parsed this file already, so iterate over its stab entries. */

    stab_entry * stabEntry = get_stab_info();
    if( stabEntry == NULL ) return;
    const char * nextStabString = stabEntry->getStringBase();

    const char * currentSourceFile = NULL;
    const char * moduleName = NULL;
    Function *currentFunction = NULL;
    Offset currentAddress = 0;
    unsigned currentLineBase = 0;
    unsigned functionLineToPossiblyAdd = 0;

    //Offset baseAddress = getBaseAddress();
    LineInformation* li_for_module = NULL;

    for ( unsigned int i = 0; i < stabEntry->count(); i++ )
    {
        switch (stabEntry->type( i ))
        {
            case N_UNDF: /* start of an object file */
            {
                stabEntry->setStringBase( nextStabString );
                nextStabString = stabEntry->getStringBase() + stabEntry->val( i );

                currentSourceFile = NULL;
            }
                break;

            case N_SO: /* compilation source or file name */
            {
                const char * sourceFile = stabEntry->name( i );
                currentSourceFile = strrchr( sourceFile, '/' );

                if ( currentSourceFile == NULL )
                {
                    currentSourceFile = sourceFile;
                }
                else
                {
                    ++currentSourceFile;
                }
                Module* mod;

                moduleName = currentSourceFile;
                if(!associated_symtab->findModuleByName(mod, moduleName)) {
                    mod = associated_symtab->getDefaultModule();
                }
                li_for_module = mod->getLineInformation();
                if(!li_for_module)
                {
                    li_for_module = new LineInformation;
                    mod->setLineInfo(li_for_module);
                }

            }
                break;

            case N_SOL: /* file name (possibly an include file) */
            {
                const char * sourceFile = stabEntry->name( i );
                currentSourceFile = strrchr( sourceFile, '/' );
                if ( currentSourceFile == NULL )
                {
                    currentSourceFile = sourceFile;
                }
                else
                {
                    ++currentSourceFile;
                }

            }
                break;

            case N_FUN: /* a function */
            {
                if ( *stabEntry->name( i ) == 0 )
                {
                    currentFunction = NULL;
                    currentLineBase = 0;
                    break;
                } /* end if the N_FUN is an end-of-function-marker. */

                std::vector<Function *> funcs;
                char stringbuf[2048];
                const char *stabstr = stabEntry->name(i);
                unsigned iter = 0;

                while (iter < 2048)
                {
                    char c = stabstr[iter];

                    if ( (c == ':')  || (c == '\0'))
                    {
                        //stabstrs use ':' as delimiter
                        stringbuf[iter] = '\0';
                        break;
                    }

                    stringbuf[iter] = c;

                    iter++;
                }

                if (iter >= 2047)
                {
                    create_printf("%s[%d]:  something went horribly awry\n", FILE__, __LINE__);
                    continue;
                }
                else
                {
                    switch (stabstr[iter+1])
                    {
                        case 'F':
                        case 'f':
                            //  A "good" function
                            break;
                        case 'P':
                        case 'p':
                            //  A prototype function? need to discard
                            continue;
                            break;
                        default:
                            continue;
                            break;
                    };
                }

                if (! associated_symtab->findFunctionsByName(funcs, std::string(stringbuf))
                    || !funcs.size())
                {
                    continue;
                }

                currentFunction = funcs[0];
                currentLineBase = stabEntry->desc(i);
                functionLineToPossiblyAdd = currentLineBase;

                if(!currentFunction) continue;
                currentAddress = currentFunction->getOffset();

            }
                break;

            case N_SLINE:
            {
                unsigned current_col = 0;

                if (!currentLineBase)
                {
                    continue;
                }

                unsigned newLineSpec = stabEntry->desc(i);

                //  Addresses specified in SLINEs are relative to the beginning of the fn
                Offset newLineAddress = stabEntry->val(i) + currentFunction->getOffset();

                if (newLineAddress <= currentAddress)
                {
                    continue;
                }

                //  If we just got our first N_SLINE after a function definition
                //  its possible that the line number specified in the function
                //  definition was less than the line number that we are currently on
                //  If so, add an additional line number entry that encompasses
                //  the line number of the original function definition in addition
                //  to this SLINE ( use the same address range)

                if (functionLineToPossiblyAdd)
                {
                    if (functionLineToPossiblyAdd < newLineSpec)
                    {
                        if(li_for_module)
                            li_for_module->addLine(currentSourceFile,
                                                   functionLineToPossiblyAdd,
                                                   current_col, currentAddress,
                                                   newLineAddress );
                    }

                    functionLineToPossiblyAdd = 0;
                }

                if(li_for_module)
                    li_for_module->addLine(currentSourceFile, newLineSpec,
                                           current_col, currentAddress,
                                           newLineAddress );

                currentAddress = newLineAddress;
                currentLineBase = newLineSpec + 1;

            }
                break;

        } /* end switch on the ith stab entry's type */

    } /* end iteration over stab entries. */

    //  haveParsedFileMap[ key ] = true;
} /* end parseStabFileLineInfo() */

struct open_statement {
    Dwarf_Unsigned string_table_index;
    Dwarf_Addr start_addr;
    Dwarf_Addr end_addr;
    Dwarf_Unsigned line_number;
    Dwarf_Signed column_number;
};


void Object::parseLineInfoForCU(Dwarf_Die cuDIE, LineInformation* li_for_module)
{
    std::vector<open_statement> open_statements;
    Dwarf_Debug *dbg_ptr = dwarf->line_dbg();
    if (!dbg_ptr)
        return;
    if(!cuDIE) return;
    Dwarf_Debug dbg = *dbg_ptr;
    /* Acquire this CU's source lines. */
    Dwarf_Line * lineBuffer;
    Dwarf_Signed lineCount;
    Dwarf_Error ignored;
    int status = dwarf_srclines( cuDIE, & lineBuffer, & lineCount, &ignored );

    /* It's OK for a CU not to have line information. */
    if(status != DW_DLV_OK)
    {
        return;
    }

    StringTablePtr strings(li_for_module->getStrings());
    char** files;
    size_t offset = strings->size();
    Dwarf_Signed filecount;
    status = dwarf_srcfiles(cuDIE, &files, &filecount, &ignored);
    if (status != DW_DLV_OK ) 
    {
        // It could happen the line table is present,
	// but there is no line in the table
        return;
    }
    // dwarf_line_srcfileno == 0 means unknown; 1...n means files[0...n-1]
    // so we ensure that we're adding a block of unknown, 1...n to the string table
    // and that offset + dwarf_line_srcfileno points to the correct string
    strings->push_back("<Unknown file>");
    for(int i = 0; i < filecount; i++)
    {
        char* tmp = NULL;
        if(truncateLineFilenames && (tmp = strrchr(files[i], '/')))
        {
            strings->push_back(tmp);
        }
        else
        {
            strings->push_back(files[i]);
        }
        dwarf_dealloc(dbg, files[i], DW_DLA_STRING);
    }
    dwarf_dealloc(dbg, files, DW_DLA_LIST);
    li_for_module->setStrings(strings);

    /* The 'lines' returned are actually interval markers; the code
     generated from lineNo runs from lineAddr up to but not including
     the lineAddr of the next line. */

    Offset baseAddr = getBaseAddress();

    Dwarf_Addr cu_high_pc = 0;
    dwarf_highpc(cuDIE, &cu_high_pc, NULL);
    /* Iterate over this CU's source lines. */
    open_statement current_statement;
    for ( int i = 0; i < lineCount; i++ )
    {
        /* Acquire the line number, address, source, and end of sequence flag. */
        status = dwarf_lineno( lineBuffer[i], & current_statement.line_number, NULL );
        if ( status != DW_DLV_OK ) {
            cout << "dwarf_lineno failed" << endl;
            continue;
        }

        status = dwarf_lineoff( lineBuffer[i], & current_statement.column_number, NULL );
        if ( status != DW_DLV_OK ) { current_statement.column_number = 0; }

        status = dwarf_lineaddr( lineBuffer[i], & current_statement.start_addr, NULL );
        if ( status != DW_DLV_OK )
        {
            cout << "dwarf_lineaddr failed" << endl;
            continue;

        }

        current_statement.start_addr += baseAddr;


        if (dwarf->debugLinkFile()) {
            Offset new_lineAddr;
            bool result = convertDebugOffset(current_statement.start_addr, new_lineAddr);
            if (result)
                current_statement.start_addr = new_lineAddr;
        }
        status = dwarf_line_srcfileno( lineBuffer[i], & current_statement.string_table_index, NULL );
        if ( status != DW_DLV_OK ) {
            cout << "dwarf_line_srcfileno failed" << endl;
            continue;
        }
        current_statement.string_table_index += offset;

        Dwarf_Bool isEndOfSequence;
        status = dwarf_lineendsequence( lineBuffer[i], & isEndOfSequence, NULL );
        if ( status != DW_DLV_OK ) {
            cout << "dwarf_lineendsequence failed" << endl;
            continue;
        }
        if(i == lineCount - 1) {
            isEndOfSequence = true;
        }
        Dwarf_Bool isStatement;
        status = dwarf_linebeginstatement(lineBuffer[i], &isStatement, NULL);
        if(status != DW_DLV_OK) {
            cout << "dwarf_linebeginstatement failed" << endl;
            continue;
        }
        std::vector<open_statement> tmp;
        for(auto stmt = open_statements.begin();
                stmt != open_statements.end();
                ++stmt)
        {
            stmt->end_addr = current_statement.start_addr;
            if(stmt->string_table_index != current_statement.string_table_index ||
                    stmt->line_number != current_statement.line_number ||
                    isEndOfSequence)
            {
                li_for_module->addLine((unsigned int)(stmt->string_table_index),
                                       (unsigned int)(stmt->line_number),
                                       (unsigned int)(stmt->column_number),
                                       stmt->start_addr,
                                       stmt->end_addr);
            }
            else
            {
                tmp.push_back(*stmt);
            }
        }
        open_statements.swap(tmp);
        if(isEndOfSequence) {
            open_statements.clear();
        } else
        if(isStatement) {
            open_statements.push_back(current_statement);
        }
    } /* end iteration over source line entries. */


/* Free this CU's source lines. */
    dwarf_srclines_dealloc(dbg, lineBuffer, lineCount);
}



void Object::parseLineInfoForAddr(Offset addr_to_find)
{
    Dwarf_Debug *dbg_ptr = dwarf->line_dbg();
    if (!dbg_ptr)
        return;
    std::set<Module*> mod_for_offset;
    associated_symtab->findModuleByOffset(mod_for_offset, addr_to_find);
    for(auto mod = mod_for_offset.begin();
            mod != mod_for_offset.end();
            ++mod)
    {
        (*mod)->parseLineInformation();
    }
    // no mod for offset means no line info for sure if we've parsed all ranges...
}




// Dwarf Debug Format parsing
void Object::parseDwarfFileLineInfo()
{
    vector<Module*> mods;
    associated_symtab->getAllModules(mods);
    for(auto mod = mods.begin();
            mod != mods.end();
            ++mod)
    {
        (*mod)->parseLineInformation();
    }
} /* end parseDwarfFileLineInfo() */

void Object::parseFileLineInfo()
{
    if(parsedAllLineInfo) return;

    parseStabFileLineInfo();
    parseDwarfFileLineInfo();
    parsedAllLineInfo = true;

}

void Object::parseTypeInfo()
{
#if defined(TIMED_PARSE)
    struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

    parseStabTypes();
    Dwarf_Debug* typeInfo = dwarf->type_dbg();
    if(!typeInfo) return;
    DwarfWalker walker(associated_symtab, *typeInfo);
    walker.parse();
#if defined(TIMED_PARSE)
    struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": parseTypes("<< obj->file()
       <<") took "<< dursecs <<" msecs" << endl;
#endif
}

void Object::parseStabTypes()
{
    types_printf("Entry to parseStabTypes for %s\n", associated_symtab->name().c_str());
    stab_entry *stabptr = NULL;
    const char *next_stabstr = NULL;

    unsigned i;
    char *modName = NULL;
    string temp;
    char *ptr = NULL, *ptr2 = NULL, *ptr3 = NULL;
    bool parseActive = false;

    std::string *currentFunctionName = NULL;
    Symbol *commonBlockVar = NULL;
    string *commonBlockName = NULL;
    typeCommon *commonBlock = NULL;
    int mostRecentLinenum = 0;

    Module *mod;
    typeCollection *tc = NULL;

#if defined(TIMED_PARSE)
    struct timeval starttime;
  gettimeofday(&starttime, NULL);
  unsigned int pss_count = 0;
  double pss_dur = 0;
  unsigned int src_count = 0;
  double src_dur = 0;
  unsigned int fun_count = 0;
  double fun_dur = 0;
  struct timeval t1, t2;
#endif


    stabptr = get_stab_info();
    if (!stabptr) {
        types_printf("\tWarning: no stab ptr, returning immediately\n");
        return;
    }

    //Using the Object to get the pointers to the .stab and .stabstr
    // XXX - Elf32 specific needs to be in seperate file -- jkh 3/18/99
    next_stabstr = stabptr->getStringBase();
    types_printf("\t Parsing %d stab entries\n", stabptr->count());
    for (i=0; i<stabptr->count(); i++) {
        switch(stabptr->type(i)){
            case N_UNDF: /* start of object file */
                /* value contains offset of the next string table for next module */
                // assert(stabptr->nameIdx(i) == 1);
                stabptr->setStringBase(next_stabstr);
                next_stabstr = stabptr->getStringBase() + stabptr->val(i);

                //N_UNDF is the start of object file. It is time to
                //clean source file name at this moment.
                /*
	if(currentSourceFile){
	delete currentSourceFile;
	currentSourceFile = NULL;
	delete absoluteDirectory;
	absoluteDirectory = NULL;
	delete currentFunctionName;
	currentFunctionName = NULL;
	currentFileInfo = NULL;
	currentFuncInfo = NULL;
	}
      */
                break;

            case N_ENDM: /* end of object file */
                break;

            case N_SO: /* compilation source or file name */
                /* bperr("Resetting CURRENT FUNCTION NAME FOR NEXT OBJECT FILE\n");*/
#ifdef TIMED_PARSE
                src_count++;
      gettimeofday(&t1, NULL);
#endif
                symt_current_func_name = ""; // reset for next object file
                symt_current_mangled_func_name = ""; // reset for next object file
                symt_current_func = NULL;

                modName = const_cast<char*>(stabptr->name(i));
                // cerr << "checkpoint B" << endl;
                ptr = strrchr(modName, '/');
                //  cerr << "checkpoint C" << endl;
                if (ptr) {
                    ptr++;
                    modName = ptr;
                }
                if (associated_symtab->findModuleByName(mod, modName)) {
                    tc = typeCollection::getModTypeCollection(mod);
                    parseActive = true;
                    if (!mod) {
                        create_printf("%s[%d]:  FIXME\n", FILE__, __LINE__);
                    }
                    else if (!tc)
                    {
                        create_printf("%s[%d]:  FIXME\n", FILE__, __LINE__);
                    }
                    else
                        tc->clearNumberedTypes();
                }
                else {
                    //parseActive = false;
                    mod = associated_symtab->getDefaultModule();
                    tc = typeCollection::getModTypeCollection(mod);
                    types_printf("\t Warning: failed to find module name matching %s, using %s\n", modName, mod->fileName().c_str());
                }

#ifdef TIMED_PARSE
            gettimeofday(&t2, NULL);
      src_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
      //src_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000) ;
#endif
                break;
            case N_SLINE:
                mostRecentLinenum = stabptr->desc(i);
                break;
            default:
                break;
        }
        if(parseActive || !is_aout()) {
            std::vector<Symbol *> bpfv;
            switch(stabptr->type(i)){
                case N_FUN:
#ifdef TIMED_PARSE
                    fun_count++;
	gettimeofday(&t1, NULL);
#endif
                    //all we have to do with function stabs at this point is to assure that we have
                    //properly set the var currentFunctionName for the later case of (parseActive)
                    symt_current_func = NULL;
                    int currentEntry = i;
                    int funlen = strlen(stabptr->name(currentEntry));
                    ptr = new char[funlen+1];
                    strcpy(ptr, stabptr->name(currentEntry));
                    while(strlen(ptr) != 0 && ptr[strlen(ptr)-1] == '\\'){
                        ptr[strlen(ptr)-1] = '\0';
                        currentEntry++;
                        strcat(ptr,stabptr->name(currentEntry));
                    }
                    char* colonPtr = NULL;
                    if(currentFunctionName) delete currentFunctionName;
                    if(!ptr || !(colonPtr = strchr(ptr,':')))
                        currentFunctionName = NULL;
                    else {
                        char* tmp = new char[colonPtr-ptr+1];
                        strncpy(tmp,ptr,colonPtr-ptr);
                        tmp[colonPtr-ptr] = '\0';
                        currentFunctionName = new string(tmp);
                        // Shouldn't this be a function name lookup?
                        std::vector<Symbol *>syms;
                        if(!associated_symtab->findSymbol(syms,
                                            *currentFunctionName,
                                            Symbol::ST_FUNCTION,
                                            mangledName)) {
                            if(!associated_symtab->findSymbol(syms,
                                                "_"+*currentFunctionName,
                                                Symbol::ST_FUNCTION,
                                                mangledName)) {
                                string fortranName = *currentFunctionName + string("_");
                                if (associated_symtab->findSymbol(syms,
                                                    fortranName,
                                                    Symbol::ST_FUNCTION,
                                                    mangledName)) {
                                    delete currentFunctionName;
                                    currentFunctionName = new string(fortranName);
                                }
                            }
                        }
                        syms.clear();
                        delete[] tmp;
                    }
                    delete[] ptr;
#ifdef TIMED_PARSE
                gettimeofday(&t2, NULL);
	fun_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	//fun_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000);
#endif
                    break;
            }
            if (!parseActive) continue;
            switch(stabptr->type(i)){
                case N_BCOMM:     {
                    // begin Fortran named common block
                    string tmp = string(stabptr->name(i));
                    commonBlockName = &tmp;
                    // find the variable for the common block

                    //TODO? change this. findLocalVar will cause an infinite loop
                    std::vector<Symbol *>vars;
                    if(!associated_symtab->findSymbol(vars,
                                        *commonBlockName,
                                        Symbol::ST_OBJECT,
                                        mangledName)) {
                        if(!associated_symtab->findSymbol(vars,
                                            *commonBlockName,
                                            Symbol::ST_OBJECT,
                                            mangledName,
                                            true))
                            commonBlockVar = NULL;
                        else
                            commonBlockVar = vars[0];
                    }
                    else
                        commonBlockVar = vars[0];
                    if (!commonBlockVar) {
                        // //bperr("unable to find variable %s\n", commonBlockName);
                    } else {
                        commonBlock = dynamic_cast<typeCommon *>(tc->findVariableType(*commonBlockName));
                        if (commonBlock == NULL) {
                            // its still the null type, create a new one for it
                            commonBlock = new typeCommon(*commonBlockName);
                            tc->addGlobalVariable(*commonBlockName, commonBlock);
                        }
                        // reset field list
                        commonBlock->beginCommonBlock();
                    }
                    break;
                }
                case N_ECOMM: {
                    //copy this set of fields
                    if(!currentFunctionName) break;
                    if(!associated_symtab->findSymbol(bpfv,
                                        *currentFunctionName,
                                        Symbol::ST_FUNCTION,
                                        mangledName)) {
                        if(!associated_symtab->findSymbol(bpfv,
                                            *currentFunctionName,
                                            Symbol::ST_FUNCTION,
                                            mangledName,
                                            true)){
                            // //bperr("unable to locate current function %s\n", currentFunctionName->c_str());
                        }
                        else{
                            Symbol *func = bpfv[0];
                            commonBlock->endCommonBlock(func, (void *)commonBlockVar->getOffset());
                        }
                    } else {
                        if (bpfv.size() > 1) {
                            // warn if we find more than one function with this name
                            // //bperr("%s[%d]:  WARNING: found %d funcs matching name %s, using the first\n",
                            //                     __FILE__, __LINE__, bpfv.size(), currentFunctionName->c_str());
                        }
                        Symbol *func = bpfv[0];
                        commonBlock->endCommonBlock(func, (void *)commonBlockVar->getOffset());
                    }
                    //TODO?? size for local variables??
                    //       // update size if needed
                    //       if (commonBlockVar)
                    //           commonBlockVar->setSize(commonBlock->getSize());
                    commonBlockVar = NULL;
                    commonBlock = NULL;
                    break;
                }
                    // case C_BINCL: -- what is the elf version of this jkh 8/21/01
                    // case C_EINCL: -- what is the elf version of this jkh 8/21/01
                case 32:    // Global symbols -- N_GYSM
                case 38:    // Global Static -- N_STSYM
                case N_FUN:
                case 128:   // typedefs and variables -- N_LSYM
                case 160:   // parameter variable -- N_PSYM
                case 0xc6:  // position-independant local typedefs -- N_ISYM
                case 0xc8: // position-independant external typedefs -- N_ESYM
#ifdef TIMED_PARSE
                    pss_count++;
	gettimeofday(&t1, NULL);
#endif
                    if (stabptr->type(i) == N_FUN) symt_current_func = NULL;
                    ptr = const_cast<char *>(stabptr->name(i));
                    while (ptr[strlen(ptr)-1] == '\\') {
                        //ptr[strlen(ptr)-1] = '\0';
                        ptr2 =  const_cast<char *>(stabptr->name(i+1));
                        ptr3 = (char *) malloc(strlen(ptr) + strlen(ptr2) + 1);
                        strcpy(ptr3, ptr);
                        ptr3[strlen(ptr)-1] = '\0';
                        strcat(ptr3, ptr2);
                        ptr = ptr3;
                        i++;
                        // XXX - memory leak on multiple cont. lines
                    }
                    // bperr("stab #%d = %s\n", i, ptr);
                    // may be nothing to parse - XXX  jdd 5/13/99

                    if (parseCompilerType(this))
                        temp = parseStabString(mod, mostRecentLinenum, (char *)ptr, stabptr->val(i), commonBlock);
                    else
                        temp = parseStabString(mod, stabptr->desc(i), (char *)ptr, stabptr->val(i), commonBlock);
                    if (temp.length()) {
                        //Error parsing the stabstr, return should be \0
                        // //bperr( "Stab string parsing ERROR!! More to parse: %s\n",
                        //				                  temp.c_str());
                        // //bperr( "  symbol: %s\n", ptr);
                    }
#ifdef TIMED_PARSE
                gettimeofday(&t2, NULL);
	pss_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	//      pss_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000);
#endif
                    break;
                default:
                    break;
            }
        }
    }
#if defined(TIMED_PARSE)
    struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": parseTypes("<< mod->fileName()
       <<") took "<<dursecs <<" msecs" << endl;
  cout << "Breakdown:" << endl;
  cout << "     Functions: " << fun_count << " took " << fun_dur << "msec" << endl;
  cout << "     Sources: " << src_count << " took " << src_dur << "msec" << endl;
  cout << "     parseStabString: " << pss_count << " took " << pss_dur << "msec" << endl;
  cout << "     Total: " << pss_dur + fun_dur + src_dur
       << " msec" << endl;
#endif
}

bool sort_dbg_map(const Object::DbgAddrConversion_t &a,
                  const Object::DbgAddrConversion_t &b)
{
    return (a.dbg_offset < b.dbg_offset);
}

bool Object::convertDebugOffset(Offset off, Offset &new_off)
{
    if (!DbgSectionMapSorted) {
        std::sort(DebugSectionMap.begin(), DebugSectionMap.end(), sort_dbg_map);
        DbgSectionMapSorted = true;
    }

    int hi = DebugSectionMap.size();
    int low = 0;
    int cur = -1;

    for (;;) {
        int last_cur = cur;
        cur = (low + hi) / 2;
        if (cur == last_cur) {
            new_off = off;
            return true;
        }

        const DbgAddrConversion_t &cur_d = DebugSectionMap[cur];

        if (off >= cur_d.dbg_offset && off < cur_d.dbg_offset+cur_d.dbg_size) {
            new_off = off - cur_d.dbg_offset + cur_d.orig_offset;
            return true;
        }
        else if (off > cur_d.dbg_offset)
        {
            low = cur;
        }
        else if (off < cur_d.dbg_offset)
        {
            hi = cur;
        }
    }

}

void Object::insertPrereqLibrary(std::string libname)
{
    prereq_libs.insert(libname);
}

bool Object::removePrereqLibrary(std::string libname)
{
    rmd_deps.push_back(libname);
    return true;
}

std::vector<std::string> &Object::libsRMd()
{
    return rmd_deps;
}

void Object::insertDynamicEntry(long name, long value)
{
    new_dynamic_entries.push_back(std::pair<long, long>(name, value));
}

// Parses sections with relocations and links these relocations to
// existing symbols
bool Object::parse_all_relocations(Elf_X &elf, Elf_X_Shdr *dynsym_scnp,
                                   Elf_X_Shdr *dynstr_scnp, Elf_X_Shdr *symtab_scnp,
                                   Elf_X_Shdr *strtab_scnp) {

    //const char *shnames = pdelf_get_shnames(*elfHdr);
    // Setup symbol table access
    Offset dynsym_offset = 0;
    Elf_X_Data dynsym_data, dynstr_data;
    Elf_X_Sym dynsym;
    const char *dynstr = NULL;
    if( dynsym_scnp && dynstr_scnp ) {
        dynsym_offset = dynsym_scnp->sh_offset();
        dynsym_data = dynsym_scnp->get_data();
        dynstr_data = dynstr_scnp->get_data();
        if( !(dynsym_data.isValid() && dynstr_data.isValid()) ) {
            return false;
        }
        dynsym = dynsym_data.get_sym();
        dynstr = dynstr_data.get_string();
    }

    Offset symtab_offset = 0;
    Elf_X_Data symtab_data, strtab_data;
    Elf_X_Sym symtab;
    const char *strtab = NULL;
    if( symtab_scnp && strtab_scnp) {
        symtab_offset = symtab_scnp->sh_offset();
        symtab_data = symtab_scnp->get_data();
        strtab_data = strtab_scnp->get_data();
        if( !(symtab_data.isValid() && strtab_data.isValid()) ) {
            return false;
        }
        symtab = symtab_data.get_sym();
        strtab = strtab_data.get_string();
    }

    if( dynstr == NULL && strtab == NULL ) return false;

    // Symbols are only truly uniquely idenfitied by their index in their
    // respective symbol table (this really applies to the symbols associated
    // with sections) So, a map for each symbol table needs to be built so a
    // relocation can be associated with the correct symbol
    dyn_hash_map<int, Symbol *> symtabByIndex;
    dyn_hash_map<int, Symbol *> dynsymByIndex;

    dyn_hash_map<std::string, std::vector<Symbol *> >::iterator symVec_it;
    for(symVec_it = symbols_.begin(); symVec_it != symbols_.end(); ++symVec_it) {
        std::vector<Symbol *>::iterator sym_it;
        for(sym_it = symVec_it->second.begin(); sym_it != symVec_it->second.end(); ++sym_it) {
            // Skip any symbols pointing to the undefined symbol entry
            if( (*sym_it)->getIndex() == STN_UNDEF ) {
                continue;
            }
            if( (*sym_it)->tag() == Symbol::TAG_INTERNAL ) {
                continue;
            }
            if( (*sym_it)->isDebug() ) {
                continue;
            }

            std::pair<dyn_hash_map<int, Symbol *>::iterator, bool> result;
            if( (*sym_it)->isInDynSymtab() ) {
                result = dynsymByIndex.insert(std::make_pair((*sym_it)->getIndex(), (*sym_it)));
            }else{
                result = symtabByIndex.insert(std::make_pair((*sym_it)->getIndex(), (*sym_it)));
            }

            // A symbol should be uniquely identified by its index in the symbol table
            if(!result.second) continue;
        }
    }

    // Build mapping from section headers to Regions
    std::vector<Region *>::iterator reg_it;
    dyn_hash_map<unsigned, Region *> shToRegion;
    for(reg_it = regions_.begin(); reg_it != regions_.end(); ++reg_it) {
        std::pair<dyn_hash_map<unsigned, Region *>::iterator, bool> result;
        result = shToRegion.insert(std::make_pair((*reg_it)->getRegionNumber(), (*reg_it)));
    }

    for(unsigned i = 0; i < elf.e_shnum(); ++i) {
        Elf_X_Shdr *shdr = allRegionHdrsByShndx[i];
        if( shdr->sh_type() != SHT_REL && shdr->sh_type() != SHT_RELA ) continue;

        Elf_X_Data reldata = shdr->get_data();
        Elf_X_Rel rel = reldata.get_rel();
        Elf_X_Rela rela = reldata.get_rela();

        Elf_X_Shdr *curSymHdr = allRegionHdrsByShndx[shdr->sh_link()];

        // Apparently, relocation entries may not have associated symbols.

        for(unsigned j = 0; j < (shdr->sh_size() / shdr->sh_entsize()); ++j) {
            // Relocation entry fields - need to be populated

            Offset relOff, addend = 0;
            std::string name;
            unsigned long relType;
            Region::RegionType regType;

            long symbol_index;
            switch(shdr->sh_type()) {
                case SHT_REL:
                    relType = rel.R_TYPE(j);
                    relOff = rel.r_offset(j);
                    symbol_index = rel.R_SYM(j);
                    regType = Region::RT_REL;
                    break;
                case SHT_RELA:
                    relType = rela.R_TYPE(j);
                    relOff = rela.r_offset(j);
                    symbol_index = rela.R_SYM(j);
                    regType = Region::RT_RELA;
                    addend = rela.r_addend(j);
                    break;
                default:
                    continue;
            }

            // Determine which symbol table to use
            Symbol *sym = NULL;
            // Use dynstr to ensure we've initialized dynsym...
            if( dynstr && curSymHdr->sh_offset() == dynsym_offset ) {
                name = string( &dynstr[dynsym.st_name(symbol_index)] );

                dyn_hash_map<int, Symbol *>::iterator sym_it;
                sym_it = dynsymByIndex.find(symbol_index);
                if( sym_it != dynsymByIndex.end() ) {
                    sym = sym_it->second;
                    if(sym->getType() == Symbol::ST_SECTION) {
                        name = sym->getRegion()->getRegionName().c_str();
                    }
                }
            }else if( strtab && curSymHdr->sh_offset() == symtab_offset ) {
                name = string( &strtab[symtab.st_name(symbol_index)] );
                dyn_hash_map<int, Symbol *>::iterator sym_it;
                sym_it = symtabByIndex.find(symbol_index);
                if( sym_it != symtabByIndex.end() ) {
                    sym = sym_it->second;
                    if(sym->getType() == Symbol::ST_SECTION) {
                        name = sym->getRegion()->getRegionName().c_str();
                    }
                }
            }

            Region *region = NULL;
            dyn_hash_map<unsigned, Region *>::iterator shToReg_it;
            shToReg_it = shToRegion.find(i);
            if( shToReg_it != shToRegion.end() ) {
                region = shToReg_it->second;
            }

            if(region != NULL)
            {
                relocationEntry newrel(0, relOff, addend, name, sym, relType, regType);
                region->addRelocationEntry(newrel);
                // relocations are also stored with their targets
                // Need to find target region
                if (sym) {
                    if (shdr->sh_info() != 0) {
                        Region *targetRegion = NULL;
                        shToReg_it = shToRegion.find(shdr->sh_info());
                        if( shToReg_it != shToRegion.end() ) {
                            targetRegion = shToReg_it->second;
                        }
                        assert(targetRegion != NULL);
                        targetRegion->addRelocationEntry(newrel);
                    }
                }
            }
        }
    }

    return true;
}

bool Region::isStandardCode()
{
    return ((getRegionPermissions() == RP_RX || getRegionPermissions() == RP_RWX) &&
            ((name_ == std::string(".text")) ||
             (name_ == std::string(".init")) ||
             (name_ == std::string(".fini"))));
}

void Object::setTruncateLinePaths(bool value)
{
    truncateLineFilenames = value;
}

bool Object::getTruncateLinePaths()
{
    return truncateLineFilenames;
}

Dyninst::Architecture Object::getArch() const
{
    return elfHdr->getArch();
}

bool Object::getABIVersion(int &major, int &minor) const
{
   if (elfHdr->e_machine() == EM_PPC64 && elfHdr->e_flags() == 0x2) {
      major = elfHdr->e_flags();
      minor = 0;
      return true;
   }
   else {
      return false;
   }
}

bool Object::isBigEndianDataEncoding() const
{
   return (elfHdr->e_endian() != 0);
}

Offset Object::getTOCoffset(Offset off) const {
    if (TOC_table_.empty()) return 0;
    Offset baseTOC = TOC_table_.find(0)->second;
    // We only store exceptions to the base TOC, so if we can't find it
    // return the base

    std::map<Offset, Offset>::const_iterator iter = TOC_table_.find(off);
    if (iter == TOC_table_.end()) {
        return baseTOC;
    }
    return iter->second;
}

void Object::setTOCoffset(Offset off) {
    TOC_table_.clear();
    TOC_table_[0] = off;
}

void Object::getSegmentsSymReader(vector<SymSegment> &segs) {
    for (unsigned i = 0; i < elfHdr->e_phnum(); ++i) {
        Elf_X_Phdr &phdr = elfHdr->get_phdr(i);

        SymSegment seg;
        seg.file_offset = phdr.p_offset();
        seg.mem_addr = phdr.p_vaddr();
        seg.file_size = phdr.p_filesz();
        seg.mem_size = phdr.p_memsz();
        seg.type = phdr.p_type();
        seg.perms = phdr.p_flags() & 0x7;

        segs.push_back(seg);
    }
}

std::string Object::getFileName() const
{
    if(soname_) {
        return soname_;
    }

    return mf->filename();
}


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
#include "symtabAPI/src/Object-pe.h"
#include <pe-parse/parse.h>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

Region::perm_t ObjectPE::getRegionPerms(std::uint32_t flags)
{
    if((flags & peparse::IMAGE_SCN_MEM_EXECUTE) && (flags & peparse::IMAGE_SCN_MEM_WRITE))
        return Region::RP_RWX;
    else if(flags & peparse::IMAGE_SCN_MEM_EXECUTE)
        return Region::RP_RX;
    else if(flags & peparse::IMAGE_SCN_MEM_WRITE)
        return Region::RP_RW;
    else
        return Region::RP_R;
}

Region::RegionType ObjectPE::getRegionType(std::uint32_t flags){
    /**
     * It's possible for a PE sections to contain an actually executed code
     * without being labeled as a code section. So we check for both permission and section type
     */
    std::uint32_t exec_flags = ( peparse::IMAGE_SCN_CNT_CODE | peparse::IMAGE_SCN_MEM_EXECUTE );
    std::uint32_t data_flags = ( peparse::IMAGE_SCN_CNT_INITIALIZED_DATA | peparse::IMAGE_SCN_CNT_UNINITIALIZED_DATA );
    if((flags & exec_flags) && (flags & peparse::IMAGE_SCN_CNT_INITIALIZED_DATA))
        return Region::RT_TEXTDATA;
    else if(flags & exec_flags)
        return Region::RT_TEXT;
    else if (flags & data_flags)
        return Region::RT_DATA;
    else
        return Region::RT_OTHER;
}

void ObjectPE::log_error(const std::string &msg)
{
    if (err_func_) {
        std::string full_msg = msg + std::string(": ") + peparse::GetPEErrString() + std::string("\n");
        err_func_(full_msg.c_str());
    }
}

ObjectPE::ObjectPE(MappedFile *mf_,
                   bool,
                   void (*err_func)(const char *),
                   bool,
                   Symtab *st) :
    Object(mf_, err_func, st)
{
    parsed_object_ = peparse::ParsePEFromPointer((unsigned char *)mf_->base_addr(), mf_->size());
    if (!parsed_object_) {
        has_error = true;
        log_error("Error while parsing PE file");
        return;
    }

    file_format_ = FileFormat::PE;

    parse_object();
}

ObjectPE::~ObjectPE()
{
    if (parsed_object_) {
        peparse::DestructParsedPE((peparse::parsed_pe *)parsed_object_);
    }
}

void ObjectPE::parse_object()
{
    peparse::parsed_pe *parsed_pe_ = (peparse::parsed_pe *)parsed_object_;
    no_of_sections_ = parsed_pe_->peHeader.nt.FileHeader.NumberOfSections;
    no_of_symbols_ = parsed_pe_->peHeader.nt.FileHeader.NumberOfSymbols;
    addressWidth_nbytes = getArchWidth();

    peparse::VA entry_addr = -1;
    if (!peparse::GetEntryPoint(parsed_pe_, entry_addr)) {
        log_error("Error while getting entry point");
    }
    entryAddress_ = entry_addr;
    if (parsed_pe_->peHeader.nt.FileHeader.Machine == peparse::IMAGE_FILE_MACHINE_AMD64) {
        preferedBase_ = loadAddress_ = parsed_pe_->peHeader.nt.OptionalHeader64.ImageBase;
    } else {
        preferedBase_ = loadAddress_ = parsed_pe_->peHeader.nt.OptionalHeader.ImageBase;
    }

    if (parsed_pe_->peHeader.nt.FileHeader.Characteristics & peparse::IMAGE_FILE_DLL) {
        is_aout_ = false;
    } else {
        is_aout_ = true;
    }

    /* Iterate over section */
    peparse::iterSec section_iterator =
        [](void *N, const peparse::VA &, const std::string &name,
           const peparse::image_section_header &header,
           const peparse::bounded_buffer *) -> int {
            ObjectPE *obj = static_cast<ObjectPE*>(N);
            int reg_num = obj->regions_.size();
            char *sec_ptr = (char *)obj->mf->base_addr() + header.PointerToRawData;
            Offset sec_off = header.PointerToRawData;
            Offset sec_len = std::max(header.SizeOfRawData, header.Misc.VirtualSize);
            if (header.Characteristics & peparse::IMAGE_SCN_CNT_CODE) {
                obj->code_ptr_ = sec_ptr;
                obj->code_off_ = sec_off;
                obj->code_len_ = sec_len;
            } else if (header.Characteristics & peparse::IMAGE_SCN_CNT_INITIALIZED_DATA) {
                obj->data_ptr_ = sec_ptr;
                obj->data_off_ = sec_off;
                obj->data_len_ = sec_len;
            }
            obj->regions_.push_back(new Region(reg_num, name, header.PointerToRawData,
                                    header.SizeOfRawData, obj->loadAddress_ + header.VirtualAddress, sec_len,
                                    sec_ptr, getRegionPerms(header.Characteristics),
                                    getRegionType(header.Characteristics)));
            return 0;
        };
    peparse::IterSec(parsed_pe_, section_iterator, this);

    struct region_less {
        bool operator()(const Region *r1, const Region *r2)
        {
            return r1->memOff_ < r2->memOff_;
        }
    } cmp;
    std::sort(regions_.begin(), regions_.end(), cmp);

    no_of_symbols_ = 0;
}

Offset ObjectPE::getPreferedBase() const
{
    return preferedBase_;
}

void ObjectPE::getDependencies(std::vector<std::string> &deps)
{
    deps.clear();
    /* TODO */
}

Offset ObjectPE::getBaseAddress() const
{
    return 0;
}

Offset ObjectPE::getEntryAddress() const
{
    return entryAddress_;
}

Offset ObjectPE::getLoadAddress() const
{
    return loadAddress_;
}

bool ObjectPE::isOnlyExecutable() const
{
    return is_aout_;
}

bool ObjectPE::isExecutable() const
{
    /**
     * This method is used only to check whether we should consider an entry point
     * as a function hint. A DLL may have a valid entry point that is not covered by
     * a symbol, so we want to always consider it as a hint.
     */
    return true;
}

bool ObjectPE::isOnlySharedLibrary() const
{
    return !is_aout_;
}

bool ObjectPE::isSharedLibrary() const
{
    return !is_aout_;
}

Dyninst::Architecture ObjectPE::getArch() const
{
    if (!parsed_object_) return Arch_none;
    switch (((peparse::parsed_pe *)parsed_object_)->peHeader.nt.FileHeader.Machine) {
        case peparse::IMAGE_FILE_MACHINE_AMD64:
        case peparse::IMAGE_FILE_MACHINE_EBC:
            return Arch_x86_64;
        case peparse::IMAGE_FILE_MACHINE_I386:
            return Arch_x86;
        case peparse::IMAGE_FILE_MACHINE_ARM:
        case peparse::IMAGE_FILE_MACHINE_ARMNT:
            return Arch_aarch32;
        case peparse::IMAGE_FILE_MACHINE_ARM64:
            return Arch_aarch64;
        case peparse::IMAGE_FILE_MACHINE_POWERPC:
        case peparse::IMAGE_FILE_MACHINE_POWERPCFP:
            return Arch_ppc64;
        case peparse::IMAGE_FILE_MACHINE_UNKNOWN:
        case peparse::IMAGE_FILE_MACHINE_ALPHA:
        case peparse::IMAGE_FILE_MACHINE_ALPHA64:
        case peparse::IMAGE_FILE_MACHINE_CEE:
        case peparse::IMAGE_FILE_MACHINE_CEF:
        case peparse::IMAGE_FILE_MACHINE_IA64:
        case peparse::IMAGE_FILE_MACHINE_M32R:
        case peparse::IMAGE_FILE_MACHINE_MIPS16:
        case peparse::IMAGE_FILE_MACHINE_MIPSFPU:
        case peparse::IMAGE_FILE_MACHINE_MIPSFPU16:
        case peparse::IMAGE_FILE_MACHINE_R10000:
        case peparse::IMAGE_FILE_MACHINE_RISCV32:
        case peparse::IMAGE_FILE_MACHINE_RISCV64:
        case peparse::IMAGE_FILE_MACHINE_RISCV128:
        case peparse::IMAGE_FILE_MACHINE_SH3:
        case peparse::IMAGE_FILE_MACHINE_SH3DSP:
        case peparse::IMAGE_FILE_MACHINE_SH4:
        case peparse::IMAGE_FILE_MACHINE_SH5:
        case peparse::IMAGE_FILE_MACHINE_THUMB:
        case peparse::IMAGE_FILE_MACHINE_TRICORE:
        case peparse::IMAGE_FILE_MACHINE_WCEMIPSV2:
        default:
            return Arch_none;
    }
}

int ObjectPE::getArchWidth() const
{
    switch (getArch()) {
        case Arch_x86:
        case Arch_aarch32:
            return 4;
        case Arch_x86_64:
        case Arch_aarch64:
            return 8;
        case Arch_none:
        default:
            return -1;
    }
}

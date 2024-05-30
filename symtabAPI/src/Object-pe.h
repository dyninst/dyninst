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

#if !defined(_Object_pe_h_)
#define _Object_pe_h_

#include "symtabAPI/src/Object.h"
#include <pe-parse/parse.h>

namespace Dyninst {
namespace SymtabAPI {

class ObjectPE : public Object {
    friend class Symtab;

public:
    ObjectPE(MappedFile *, bool, void(*)(const char *) = log_msg, bool = true, Symtab * = NULL);

    Offset getPreferedBase() const;
    void getDependencies(std::vector<std::string> &deps);
    Offset getEntryAddress() const;
    Offset getBaseAddress() const;
    Offset getLoadAddress() const;
    ObjectType objType() const;

    DYNINST_EXPORT bool isOnlyExecutable() const;
    DYNINST_EXPORT bool isExecutable() const;
    DYNINST_EXPORT bool isSharedLibrary() const;
    DYNINST_EXPORT bool isOnlySharedLibrary() const;
    DYNINST_EXPORT Dyninst::Architecture getArch() const;

    void insertPrereqLibrary(std::string) {} // TODO
    bool emitDriver(std::string, std::set<Symbol *> &, unsigned ) { return false; } // TODO
    void getModuleLanguageInfo(dyn_hash_map<std::string, supportedLanguages> *) {} // TODO

private:
    ObjectPE(const ObjectPE &);
    const ObjectPE& operator=(const ObjectPE &);

    void log_error(const std::string &);
    void parse_object();

    static Region::perm_t getRegionPerms(std::uint32_t flags);
    static Region::RegionType getRegionType(std::uint32_t flags);

    static Offset readRelocTarget(const void *ptr, peparse::reloc_type type);
    static void writeRelocTarget(void *ptr, peparse::reloc_type type, Offset val);

    void rebase(Offset new_base);

    int getArchWidth() const;

    peparse::parsed_pe *parsed_pe_;

    Offset entryAddress_;

    Offset preferedBase_;
    Offset loadAddress_;
};

} // namespace SymtabAPI
} // namespace Dyninst

#endif /* !defined(_Object_pe_h_) */

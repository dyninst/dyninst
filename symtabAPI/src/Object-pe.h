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

namespace Dyninst {
namespace SymtabAPI {

class ObjectPE : public Object {
    friend class Symtab;

public:
    ObjectPE(MappedFile *, bool, void(*)(const char *) = log_msg, bool = true, Symtab * = NULL);
    ~ObjectPE();

    Offset getPreferedBase() const override;
    void getDependencies(std::vector<std::string> &deps) override;
    Offset getEntryAddress() const override;
    Offset getBaseAddress() const override;
    Offset getLoadAddress() const override;

    DYNINST_EXPORT bool isOnlyExecutable() const override;
    DYNINST_EXPORT bool isExecutable() const override;
    DYNINST_EXPORT bool isSharedLibrary() const override;
    DYNINST_EXPORT bool isOnlySharedLibrary() const override;
    DYNINST_EXPORT Dyninst::Architecture getArch() const override;

    void insertPrereqLibrary(std::string) override {} // TODO
    bool emitDriver(std::string, std::set<Symbol *> &, unsigned ) override { return false; } // TODO
    void getModuleLanguageInfo(dyn_hash_map<std::string, supportedLanguages> *) override {} // TODO

    Region *findEnclosingRegion(const Offset off)
    {
      auto rgn = std::lower_bound(regions_.begin(), regions_.end(), off,
          [](const Region *r, const Offset &value)
          {
            return r->getMemOffset() <= value;
          });
      if (rgn != regions_.begin()) {
        rgn--;
      } else {
        rgn = regions_.end();
      }
      return rgn != regions_.end() ? *rgn : NULL;
    }

    void rebase(Offset new_base) override;

private:
    ObjectPE(const ObjectPE &);
    const ObjectPE& operator=(const ObjectPE &);

    void log_error(const std::string &);
    void parse_object();

    static Region::perm_t getRegionPerms(std::uint32_t flags);
    static Region::RegionType getRegionType(std::uint32_t flags);

    int getArchWidth() const;

    // Internal pe-parse structure. The pointer is kept as "void"
    // to support compilation both with and without pr-parse library.
    void *parsed_object_;

    Offset entryAddress_;

    Offset preferedBase_;
    Offset loadAddress_;
};

} // namespace SymtabAPI
} // namespace Dyninst

#endif /* !defined(_Object_pe_h_) */

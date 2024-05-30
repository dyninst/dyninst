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

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

void ObjectPE::log_error(const std::string &msg)
{
    if (err_func_) {
        std::string full_msg = msg + std::string("\n");
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
    file_format_ = FileFormat::PE;

    has_error = true;
    log_error("Parsing of PE files is not implemented yet");
}

ObjectPE::~ObjectPE()
{
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

void ObjectPE::rebase(Offset)
{
}

Dyninst::Architecture ObjectPE::getArch() const
{
    return Arch_none;
}

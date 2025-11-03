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

#include "Operation_impl.h"
#include "common/src/vgannotations.h"
#include "concurrent.h"
#include "entryIDs.h"

using namespace NS_x86;

namespace Dyninst { namespace InstructionAPI {

  Operation::Operation(entryID id, std::string m, Architecture arch)
      : operationID(id), archDecodedFrom(arch), mnemonic{std::move(m)} {}

  std::string Operation::format() const {
    if(mnemonic != "") {
      return mnemonic;
    }
    dyn_hash_map<prefixEntryID, std::string>::const_iterator foundPrefix =
        prefixEntryNames_IAPI.find(prefixID);
    dyn_hash_map<entryID, std::string>::const_iterator found = entryNames_IAPI.find(operationID);
    std::string result;
    if(foundPrefix != prefixEntryNames_IAPI.end()) {
      result += (foundPrefix->second + " ");
    }
    if(found != entryNames_IAPI.end()) {
      result += found->second;
    } else {
      result += "[INVALID]";
    }
    return result;
  }

  entryID Operation::getID() const { return operationID; }

  prefixEntryID Operation::getPrefixID() const { return prefixID; }

}}

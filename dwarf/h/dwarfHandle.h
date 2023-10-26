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

#if !defined(DWARF_HANDLE_H_)
#define DWARF_HANDLE_H_

#include "elfutils/libdw.h"
#include "dyntypes.h"
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>

namespace Dyninst {
class Elf_X;

namespace DwarfDyninst {
class DwarfFrameParser;

typedef boost::shared_ptr<DwarfFrameParser> DwarfFrameParserPtr;

class DYNDWARF_EXPORT DwarfHandle {
  public:
   typedef DwarfHandle* ptr;
  private:
   DwarfFrameParserPtr sw;
   typedef enum {
      dwarf_status_uninitialized,
      dwarf_status_error,
      dwarf_status_ok
   } dwarf_status_t;
   dwarf_status_t init_dwarf_status;

   Dwarf *dbg_file_data;
   Dwarf *file_data;
   Dwarf **line_data;
   Dwarf **type_data;
   Dwarf **frame_data;

   Elf_X *file;
   Elf_X *dbg_file;
   /*Dwarf_Handler err_func;*/
   bool init_dbg();
   void locate_dbg_file();
   bool hasFrameData(Elf_X *elfx);
   std::string filename;
   std::string debug_filename;
   static std::map<std::string, DwarfHandle::ptr> all_dwarf_handles;
   /*static Dwarf_Handler defaultErrFunc;
   static void defaultDwarfError(Dwarf_Error err, Dwarf_Ptr arg);*/

   DwarfHandle(std::string filename_, Elf_X *file_,
           void* /*, Dwarf_Handler err_func_*/);
  public:
   ~DwarfHandle();

   static DwarfHandle::ptr createDwarfHandle(
           std::string filename_, Elf_X *file_,
           void *e=NULL /*, Dwarf_Handler err_func_ = defaultErrFunc*/);

   Elf_X *origFile();
   Elf_X *debugLinkFile();
   Dwarf **line_dbg();
   Dwarf **type_dbg();
   Dwarf **frame_dbg();
   DwarfFrameParserPtr frameParser();
   const std::string& getDebugFilename() { return debug_filename; }
};

}
}

#endif

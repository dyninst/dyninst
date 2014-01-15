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

#include "libdwarf.h"
#include "elf/h/Elf_X.h"
#include "dwarf/h/dwarfHandle.h"
#include "dwarf/h/dwarfFrameParser.h"
#include "common/src/debug_common.h"
#include <cstring>

using namespace Dyninst;
using namespace Dwarf;
using namespace std;

void DwarfHandle::defaultDwarfError(Dwarf_Error , Dwarf_Ptr) {

}

Dwarf_Handler DwarfHandle::defaultErrFunc = DwarfHandle::defaultDwarfError;

DwarfHandle::DwarfHandle(string filename_, Elf_X *file_,
                         Dwarf_Handler err_func_, Dwarf_Ptr err_data_) :
   init_dwarf_status(dwarf_status_uninitialized),
   dbg_file_data(NULL),
   file_data(NULL),
   line_data(NULL),
   type_data(NULL),
   frame_data(NULL),
   file(file_),
   dbg_file(NULL),
   err_func(err_func_),
   err_data(err_data_),
   filename(filename_)
{

   locate_dbg_file();
}

void DwarfHandle::locate_dbg_file()
{
   char *buffer;
   unsigned long buffer_size;
   bool result = file->findDebugFile(filename, debug_filename, buffer, buffer_size);
   if (!result)
      return;
   dbg_file = Elf_X::newElf_X(buffer, buffer_size, debug_filename);
   if (!dbg_file->isValid()) {
      dwarf_printf("Invalid ELF file for debug info: %s\n", debug_filename.c_str());
      dbg_file->end();
      dbg_file = NULL;
   }
}

bool DwarfHandle::init_dbg()
{
   int status;
   Dwarf_Error err;
   if (init_dwarf_status == dwarf_status_ok) {
      return true;
   }

   if (init_dwarf_status == dwarf_status_error) {
      return false;
   }

   status = dwarf_elf_init(file->e_elfp(), DW_DLC_READ, 
                           err_func, err_data, &file_data, &err);
   if (status != DW_DLV_OK) {
      init_dwarf_status = dwarf_status_error;
      return false;
   }

   if (dbg_file) {
      status = dwarf_elf_init(dbg_file->e_elfp(), DW_DLC_READ, 
                              err_func, err_data, &dbg_file_data, &err);
      if (status != DW_DLV_OK) {
         init_dwarf_status = dwarf_status_error;
         return false;
      }

      //Have a debug file, choose which file to use for different lookups.
      line_data = &dbg_file_data;
      type_data = &dbg_file_data;

      if (hasFrameData(dbg_file))
         frame_data = &dbg_file_data;
      else
         frame_data = &file_data;
   }
   else {
      //No debug file, take everything from file
      line_data = &file_data;
      type_data = &file_data;
      frame_data = &file_data;
   }
   
   Dyninst::Architecture arch;
   switch (file->e_machine()) {
      case EM_386:
         arch = Arch_x86;
         break;
      case EM_X86_64:
         arch = Arch_x86_64;
         break;
      case EM_PPC:
         arch = Arch_ppc32;
         break;
      case EM_PPC64:
         arch = Arch_ppc64;
         break;
      default:
         assert(0 && "Unsupported archiecture in ELF file.");
	 return false;
   }
   sw = Dwarf::DwarfFrameParser::create(*frame_data, arch);

   init_dwarf_status = dwarf_status_ok;
   return true;
}

const char* frame_section_names[] = { ".debug_frame", ".eh_frame", NULL };
bool DwarfHandle::hasFrameData(Elf_X *e)
{
   unsigned short shstrtab_idx = e->e_shstrndx();
   Elf_X_Shdr &shstrtab = e->get_shdr(shstrtab_idx);
   if (!shstrtab.isValid())
      return false;
   Elf_X_Data data = shstrtab.get_data();
   if (!data.isValid())
      return false;
   const char *shnames = data.get_string();

   unsigned short num_sections = e->e_shnum();
   for (unsigned i = 0; i < num_sections; i++) {
      Elf_X_Shdr &shdr = e->get_shdr(i);
      if (!shdr.isValid())
         continue;
      if (shdr.sh_type() == SHT_NOBITS)
         continue;
      unsigned long name_idx = shdr.sh_name();
      for (const char **s = frame_section_names; *s; s++) {
         if (strcmp(*s, shnames + name_idx) == 0) {
            return true;
         }
      }
   }
   return false;
}

Elf_X *DwarfHandle::origFile()
{
   return file;
}

Elf_X *DwarfHandle::debugLinkFile()
{
   return dbg_file;
}

Dwarf_Debug *DwarfHandle::line_dbg()
{
   if (!init_dbg()) 
      return NULL;
   return line_data;
}

Dwarf_Debug *DwarfHandle::type_dbg()
{
   if (!init_dbg()) 
      return NULL;
   return type_data;
}

Dwarf_Debug *DwarfHandle::frame_dbg()
{
   if (!init_dbg()) 
      return NULL;
   return frame_data;
}


DwarfHandle::~DwarfHandle()
{
   if (init_dwarf_status != dwarf_status_ok) 
      return;

   Dwarf_Error err;
   if (dbg_file_data)
      dwarf_finish(dbg_file_data, &err);
   if (file_data)
      dwarf_finish(file_data, &err);
}

map<std::string, DwarfHandle::ptr> DwarfHandle::all_dwarf_handles;
DwarfHandle::ptr DwarfHandle::createDwarfHandle(string filename_, Elf_X *file_, 
                                                Dwarf_Handler err_func_, Dwarf_Ptr err_data_)
{   
   map<string, DwarfHandle::ptr>::iterator i;
   i = all_dwarf_handles.find(filename_);
   if (i != all_dwarf_handles.end()) {
      return i->second;
   }
   
   DwarfHandle::ptr ret = DwarfHandle::ptr(new DwarfHandle(filename_, file_, err_func_, err_data_));
   all_dwarf_handles.insert(make_pair(filename_, ret));
   return ret;
}

DwarfFrameParserPtr DwarfHandle::frameParser() { 
   return sw;
}

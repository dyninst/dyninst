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

//Hashing function for std::unordered_mapes

#if !defined(_symtab_util_h_)
#define _symtab_util_h_

#include "dyntypes.h"
#include "util.h"
#include <string>

#if defined(_MSC_VER)	
#include <set>
#else
#include <regex.h>
#include <string>
#endif

namespace Dyninst{
namespace SymtabAPI{


typedef enum {
    mangledName = 1,
    prettyName = 2,
    typedName = 4,
    anyName = 7 } NameType;

typedef enum { 
   lang_Unknown,
   lang_Assembly,
   lang_C,
   lang_CPlusPlus,
   lang_GnuCPlusPlus,
   lang_Fortran,
   lang_CMFortran
} supportedLanguages;

SYMTAB_EXPORT const char *supportedLanguages2Str(supportedLanguages s);

typedef enum {
   obj_Unknown,
   obj_SharedLib,
   obj_Executable,
   obj_RelocatableFile
} ObjectType;

typedef enum { 
   Obj_Parsing = 0,
   Syms_To_Functions,
   Build_Function_Lists,
   No_Such_Function,
   No_Such_Variable,
   No_Such_Module,
   No_Such_Region,
   No_Such_Symbol,
   No_Such_Member,
   Not_A_File,
   Not_An_Archive,
   Duplicate_Symbol,
   Export_Error,
   Emit_Error,
   Invalid_Flags,
   Bad_Frame_Data,      /* 15 */
   No_Frame_Entry,
   Frame_Read_Error,
   Multiple_Region_Matches,
   No_Error
} SymtabError;

typedef struct{
    void *data;
    Offset loadaddr;
    unsigned long size;
    std::string name; 
    unsigned segFlags;
}Segment;

}//namespace SymtabAPI
}//namespace Dyninst


#endif

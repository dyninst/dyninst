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


#if !defined(_emit_Win_h_)
#define _emit_Win_h_

#include "symtabAPI/src/Object.h"
#include "symtabAPI/src/Object-nt.h"
#include <string>
#include <cstdio>
#include <cstdlib>
#include <windows.h>

namespace Dyninst{
namespace SymtabAPI{

class emitWin{
  public:
    emitWin(PCHAR baseaddress, Object* o_nt, void (*)(const char *) = log_msg);
    ~emitWin();
    bool driver(Symtab *obj, std::string fName);
 
  private:

    const static unsigned int SizeOfSecHeader = 40; //size of section header entry is 40 bytes
    PCHAR base_addr; //the base address of the mapped image file
    Offset bit_addr; //the offset of bound import table
    unsigned int bit_size{}; //the size of bound import table
    Object* obj_nt;
    Offset PEAlign(Offset dwAddr,Offset dwAlign);
    unsigned int NumOfTotalAllowedSec();
    unsigned int NumOfAllowedSecInSectionTable();
    unsigned int NumOfAllowedSecInDosHeader();
    PIMAGE_SECTION_HEADER CreateSecHeader(unsigned int size,PIMAGE_SECTION_HEADER preSecHdr);
    bool AlignSection(PIMAGE_SECTION_HEADER p);
    bool writeImpTable(Symtab*);
    bool isMoveAhead{false};//variable indicating whether or not we need to move things ahead to Dos Stub Area

	void (*err_func_)(const char*);
    void log_winerror(void (*err_func)(const char *), const char* msg);
};

} // namespace SymtabAPI
} // namespace Dyninst

#endif

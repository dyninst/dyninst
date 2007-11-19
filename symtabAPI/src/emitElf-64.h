/*
 * Copyright (c) 1996-2007 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if !defined(_emit_Elf64_h_)
#define _emit_Elf64_h_

#include "Object.h"

namespace Dyninst{
namespace SymtabAPI{

class emitElf64{
  public:
    emitElf64(Elf_X &oldElfHandle_, bool isStripped_ = false, int BSSexpandflag = false, void (*)(const char *) = log_msg);
    ~emitElf64();

    bool checkIfStripped(Symtab *obj, vector<Symbol *>&functions, vector<Symbol *>&variables, vector<Symbol *>&mods, vector<Symbol *>&notypes);
    bool driver(Symtab *obj, std::string fName);
 
  private:
    Elf_X oldElfHandle;
    Elf *newElf;
    Elf *oldElf;
    
    //New Section & Program Headers
    Elf64_Ehdr *newEhdr;
    Elf64_Ehdr *oldEhdr;
    
    Elf64_Phdr *newPhdr;
    Elf64_Phdr *oldPhdr;

    //important data sections in the
    //new Elf that need updated
    Elf_Data *textData;
    Elf_Data *symStrData;
    Elf_Data *dynStrData;
    Elf_Data *symTabData;
    Elf_Data *hashData;
    Elf_Data *dynsymData;
    Elf_Data *rodata;
    Elf_Data *dataData;
    
    Elf64_Shdr *textSh;
    Elf64_Shdr *rodataSh;
    
    //Symbol table(.symtab) symbols
    std::vector<Elf64_Sym *> symbols;
    std::vector<std::string> symbolStrs; //names of symbols
    unsigned symbolNamesLength; //Total size of all the names

    std::vector<Section *>nonLoadableSecs;

    // Needed when adding a new segment
    Elf64_Off newSegmentStart;
    Elf64_Shdr *firstNewLoadSec;// initialize to NULL
 
    //text & data segment ends
    Elf64_Off dataSegEnd, textSegEnd;

    //flags
    // Expand NOBITS sections within the object file to their size
    bool BSSExpandFlag;
    bool addNewSegmentFlag;
    
    bool isStripped;

    void (*err_func_)(const char*);

    bool getBackSymbol(Symbol *symbol);
    void findSegmentEnds();
    void fixPhdrs(unsigned);
    void addSectionNames(Elf_Data *&, Elf_Data *, unsigned , unsigned , vector<std::string> &);
    bool createNonLoadableSections(Elf64_Shdr *shdr, unsigned shstrtabDataSize, unsigned);
    void addSectionNames(Elf_Data *&, Elf_Data *, unsigned , unsigned , std::vector<std::string> &, std::vector<Section*>&);
    bool createLoadableSections( Elf64_Shdr *shdr, std::vector<Section *>&newSecs, std::vector<std::string> &loadSecNames, unsigned &shstrtabDataSize, unsigned &nonLoadableNamesSize, unsigned &loadSecTotalSize);

    void updateSymbols(Elf_Data* symtabData,Elf_Data* strData, unsigned long loadSecsSize);
    
    void log_elferror(void (*err_func)(const char *), const char* msg);
};

} // namespace SymtabAPI
} // namespace Dyninst

#endif

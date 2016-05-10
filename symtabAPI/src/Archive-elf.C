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

#include <ar.h>

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"
#include "symtabAPI/src/Object.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

Archive::Archive(std::string& filename, bool& err)
    : basePtr(NULL), symbolTableParsed(false)
{
    mf = MappedFile::createMappedFile(filename);

    if( mf == NULL ) {
        serr = Not_A_File;
        errMsg = "Failed to locate file";
        err = false;
        return;
    }

    Elf_Cmd cmd = ELF_C_READ;
    Elf_Arhdr *archdr;
    Elf_X *arf = Elf_X::newElf_X(mf->getFD(), cmd, NULL, filename);
    if (elf_kind(arf->e_elfp()) != ELF_K_AR) {
        /* Don't close mf, because this file will most
         * likely be opened again as a normal Symtab
         * object
         */
	serr = Not_An_Archive;
	err = false;
	return;
    }

    basePtr = (void *) arf;
    Elf_X *newelf = Elf_X::newElf_X(mf->getFD(), cmd, arf);

    while( newelf->e_elfp() ) {
	archdr = elf_getarhdr(newelf->e_elfp());
	string member_name = archdr->ar_name;

        if (elf_kind(newelf->e_elfp()) == ELF_K_ELF) {
            /* The offset is to the beginning of the arhdr for the member, not
             * to the beginning of the elfhdr for the member. This allows the
             * the lazy construction of the object to use the elf_* methods to
             * get pointers to the ar and elf headers. This offset also matches
             * up with the offset in the archive global symbol table
             */
            Offset tmpOffset = elf_getbase(newelf->e_elfp()) - sizeof(struct ar_hdr);

            // Member is parsed lazily
            ArchiveMember *newMember = new ArchiveMember(member_name, tmpOffset);

            membersByName[member_name] = newMember;
            membersByOffset[tmpOffset] = newMember;
	}

	Elf_X *elfhandle = arf->e_next(newelf);
	newelf->end();
	newelf = elfhandle;
    }

    newelf->end();
    err = true;
}

Archive::Archive(char *, size_t, bool &err) 
    : mf(NULL), basePtr(NULL), symbolTableParsed(false)
{
    err = false;
    serr = Obj_Parsing;
    errMsg = "current version of libelf doesn't fully support in memory archives";
}

bool Archive::parseMember(Symtab *&img, ArchiveMember *member) 
{
    // Locate the member based on the stored offset
    Elf_X* elfX_Hdr = ((Elf_X *)basePtr)->e_rand(member->getOffset());
    Elf* elfHdr = elfX_Hdr->e_elfp();

    Elf_Arhdr *arhdr = elf_getarhdr(elfHdr);
    if( arhdr == NULL ) {
        serr = Obj_Parsing;
        errMsg = elf_errmsg(elf_errno());
        return false;
    }

    // Sanity check
    assert(member->getName() == string(arhdr->ar_name));

    size_t rawSize;
    char * rawMember = elf_rawfile(elfHdr, &rawSize);

    if( 0 == rawSize ) {
        rawSize = arhdr->ar_size;
    }

    if( rawMember == NULL || rawSize == 0 ) {
        serr = Obj_Parsing;
        errMsg = elf_errmsg(elf_errno());
        return false;
    }

    bool success = Symtab::openFile(img, (void *)rawMember, rawSize, member->getName());
    if( !success ) {
        serr = Obj_Parsing;
        errMsg = "problem creating underlying Symtab object";
        return false;
    }

    img->member_name_ = member->getName();
    img->member_offset_ = member->getOffset();

    img->parentArchive_ = this;
    member->setSymtab(img);

    elfX_Hdr->end();

    return true;
}

bool Archive::parseSymbolTable() {
    if( symbolTableParsed ) return true;

    Elf_Arsym *ar_syms;
    size_t numSyms;
    if( (ar_syms = elf_getarsym(static_cast<Elf_X *>(basePtr)->e_elfp(), &numSyms)) == NULL ) {
        serr = Obj_Parsing;
        errMsg = string("No symbol table found: ") + string(elf_errmsg(elf_errno()));
        return false;
    }

    // The last element is always a null element
    for(unsigned i = 0; i < (numSyms - 1); i++) {
        string symbol_name(ar_syms[i].as_name);

        // Duplicate symbols are okay here, they should be treated as errors
        // when necessary
        membersBySymbol.insert(make_pair(symbol_name, membersByOffset[ar_syms[i].as_off]));
    }

    symbolTableParsed = true;

    return true;
}

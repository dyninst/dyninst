/*
 * Copyright (c) 1996-2009 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 *
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 *
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 *
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * Exercises the public interface of the Archive class
 */

#include "symtab_comp.h"
#include "test_lib.h"

#include "Archive.h"
#include "Symtab.h"
#include "Symbol.h"
#include "MappedFile.h"

// XXX remove
#include "ParameterDict.h"
#include "error.h"
#include "ResumeLog.h"
#include "TestOutputDriver.h"
#include "StdOutputDriver.h"
#include "comptester.h"
#include "help.h"

#include <ar.h>
#if !defined(os_aix)
#include <elf.h>
#include <libelf.h>
#endif

#define log_estr(msg) logerror("[%s:%u] - %s\n", __FILE__, __LINE__, msg)

using namespace Dyninst;
using namespace SymtabAPI;

class test_archive_Mutator : public ArchiveMutator {
    public:
        virtual test_results_t executeTest();

    private:
        bool findGlobalSymbol(vector<Symtab *> &members, Symtab * &member,
                Symbol * &global);
};

extern "C" DLLEXPORT TestMutator* test_archive_factory()
{
    return new test_archive_Mutator();
}

bool test_archive_Mutator::findGlobalSymbol(vector<Symtab *> &members, 
        Symtab * &member, Symbol * &global) 
{
    vector<Symtab *>::iterator mem_it;
    for(mem_it = members.begin(); mem_it != members.end(); ++mem_it) {
        vector<Symbol *> syms;
        if( !(*mem_it)->getAllSymbols(syms) ) {
            logerror("[%s:%u] - Failed to get all symbols for member '%s': %s\n", __FILE__, __LINE__,
                (*mem_it)->memberName().c_str(), Archive::printError(Archive::getLastError()).c_str());
            return false;
        }

        vector<Symbol *>::iterator sym_it;
        for(sym_it = syms.begin(); sym_it != syms.end(); ++sym_it) {
            if( (*sym_it)->getLinkage() == Symbol::SL_GLOBAL ) {
                global = *sym_it;
                member = *mem_it;
                break;
            }
        }
    }

    return true;
}

test_results_t test_archive_Mutator::executeTest()
{
    const unsigned ELF_AR_SYM_EMPTY = 1;

    if( archiveFile.length() == 0 ) {
        log_estr("Archive used for test unspecified");
        return FAILED;
    }

    Symtab *passedMember;
    string memberName;
    vector<Symtab *> members;

    MappedFile *testMf = MappedFile::createMappedFile(archiveFile);
    if( testMf == NULL ) {
        logerror("[%s:%u] - Failed to open mapped file %s: %s\n", __FILE__, __LINE__,
                archiveFile.c_str(), Archive::printError(Archive::getLastError()).c_str());
        return FAILED;
    }

#if defined(os_aix)
    // Currently only AIX supports this
    /********************************************
               openArchive: valid memory image
    ********************************************/
    Archive *memAr;
    if( !Archive::openArchive(memAr, static_cast<char *>(testMf->base_addr()), testMf->size()) ) {
        logerror("[%s:%u] - Failed to open file %s in memory: %s\n", __FILE__, __LINE__,
                archiveFile.c_str(), Archive::printError(Archive::getLastError()).c_str());
        return FAILED;
    }

    /*TODO there isn't an easy way to get an offset into the archive, so
     * getMemberByOffset isn't tested. Additionally, the logic for retrieving a
     * member by offset isn't architecture-dependent.
     *
     * Instead, just test parsing of in memory members
     */
    if( !memAr->getAllMembers(members) || members.size() == 0) {
        logerror("[%s:%u] - Failed to get members of archive: %s\n", __FILE__,
                __LINE__, Archive::printError(Archive::getLastError()).c_str());
        return FAILED;
    }

    // Don't do this, it closes the MappedFile for 'archive' member
    // delete memAr;
    // memAr = NULL

    /********************************************
            openArchive: valid file
    ********************************************/
    // Handled by ArchiveMutator
#else
    /********************************************
            openArchive: valid file
    ********************************************/
    // Handled by ArchiveMutator

    // First, find a valid offset
    Elf *elfobj;
    if( (elfobj = elf_begin(testMf->getFD(), ELF_C_READ, NULL)) == NULL ) {
        logerror("[%s:%u] - Failed to open raw archive: %s\n", __FILE__,
                __LINE__, elf_errmsg(-1));
        return FAILED;
    }

    Elf_Arsym *ar_syms;
    size_t numSyms;
    if( (ar_syms = elf_getarsym(elfobj, &numSyms)) == NULL ) {
        logerror("[%s:%u] - Failed to retrieve archive symbol table: %s\n",
                __FILE__, __LINE__, elf_errmsg(-1));
        return FAILED;
    }

    // The array is always terminated by a NULL-symbol
    if( numSyms <= ELF_AR_SYM_EMPTY ) {
        log_estr("Archive contains empty symbol table");
        return FAILED;
    }

    Offset firstOffset = ar_syms[0].as_off;

    // Obtain the name for member 
    if( elf_rand(elfobj, firstOffset) != firstOffset) { 
        logerror("[%s:%u] - Failed to locate member in archive: %s\n",
                __FILE__, __LINE__, elf_errmsg(-1));
        return FAILED;
    }

    Elf *member;
    if( (member = elf_begin(testMf->getFD(), ELF_C_READ, elfobj)) == NULL ) {
        logerror("[%s:%u] - Failed to locate member in archive: %s\n",
                __FILE__, __LINE__, elf_errmsg(-1));
        return FAILED;
    }

    Elf_Arhdr *ar_hdr;
    if( (ar_hdr = elf_getarhdr(member)) == NULL ) {
        logerror("[%s:%u] - Failed to locate member name: %s\n",
                __FILE__, __LINE__, elf_errmsg(-1));
        return FAILED;
    }
    memberName = ar_hdr->ar_name;
    /********************************************
               getMemberByOffset:
    ********************************************/
    if( !archive->getMemberByOffset(passedMember, firstOffset)
        || passedMember->memberName() != memberName) {
        logerror("[%s:%u] - Failed to locate member by offset: %s\n",
                __FILE__, __LINE__, 
                Archive::printError(Archive::getLastError()).c_str());
        return FAILED;
    }
#endif

    /********************************************
            openArchive: invalid file
    ********************************************/
    string badFile = "foobar.a";
    Archive *badAr;
    if( Archive::openArchive(badAr, badFile) ) {
        log_estr("Opening invalid file succeeded");
        return FAILED;
    }

   /********************************************
              getAllMembers: 
    ********************************************/
    if( !archive->getAllMembers(members) || members.size() == 0) {
        logerror("[%s:%u] - Failed to get members of archive: %s\n", __FILE__,
                __LINE__, Archive::printError(Archive::getLastError()).c_str());
        return FAILED;
    }

    Symbol *global = NULL;
    Symtab *testMember = NULL;
    if( !findGlobalSymbol(members, testMember, global) ) {
        log_estr("Failed to locate global symbol in archive");
        return FAILED;
    }

    if( global == NULL || testMember == NULL ) {
        log_estr("Failed to locate global symbol in archive");
        return FAILED;
    }

    /********************************************
               getMember: 
    ********************************************/
    memberName = testMember->memberName();
    if( !archive->getMember(passedMember, memberName)
        || passedMember != testMember ) {
        logerror("[%s:%u] - Failed to open file '%s': %s\n", __FILE__,
                __LINE__, testMember->memberName().c_str(),
                Archive::printError(Archive::getLastError()).c_str());
        return FAILED;
    }

    /********************************************
               isMemberInArchive:
    ********************************************/
    memberName = testMember->memberName();
    if( !archive->isMemberInArchive(memberName) ) {
        logerror("[%s:%u] - Failed to locate member '%s': %s\n", __FILE__,
                __LINE__, testMember->memberName().c_str(),
                Archive::printError(Archive::getLastError()).c_str());
        return FAILED;
    }

#if !defined(os_aix)
    // Parsing of symbol tables is currently unimplemented on AIX

    /********************************************
               getMemberByGlobalSymbol:
    ********************************************/
    if( !archive->getMemberByGlobalSymbol(passedMember, 
                const_cast<std::string&>(global->getPrettyName())) 
                || passedMember != testMember ) 
    {
        logerror("[%s:%u] - Failed to locate member '%s': %s\n", __FILE__,
                __LINE__, testMember->memberName().c_str(),
                Archive::printError(Archive::getLastError()).c_str());
        return FAILED;
    }
#endif

    /********************************************
               findMemberWithDefinition
    ********************************************/
    if( !archive->findMemberWithDefinition(passedMember, 
                const_cast<std::string&>(global->getPrettyName()))
                || passedMember != testMember) {
        logerror("[%s:%u] - Failed to locate member '%s': %s\n", __FILE__,
                __LINE__, testMember->memberName().c_str(),
                Archive::printError(Archive::getLastError()).c_str());
        return FAILED;
    }

    // Test dtor
    delete archive;
    archive = NULL;

    if( !Archive::openArchive(archive, archiveFile) ) {
        logerror("[%s:%u] - Failed to open archive '%s': %s\n", __FILE__,
                __LINE__, archiveFile.c_str(),
                Archive::printError(Archive::getLastError()).c_str());
        return FAILED;
    }

    delete archive;
    archive = NULL;

    return PASSED;
}

const string USAGE = " <archivefile>";

int main(int argc, char **argv)
{
    if( argc < 2 ) {
        cerr << "Usage: " << argv[0] << USAGE << endl;
        return 1;
    }

    setOutput(new StdOutputDriver(NULL));

    ParameterDict params;
    ArchiveComponent archiveComp;
    RunGroup *rg = new RunGroup(argv[1], STOPPED, DISK, false, argv[1], "gcc", "-O0", "sysvabi");
    if( archiveComp.group_setup(rg, params) == FAILED ) {
        cerr << "Failed to setup test" << endl;
        return 1;
    }

    test_archive_Mutator test;
    if( test.setup(params) == FAILED ) {
        cerr << "Failed to setup test" << endl;
        return 1;
    }

    if( test.executeTest() == PASSED ) {
        cerr << "Tests passed" << endl;
        return 0;
    }else{
        cerr << "Tests failed" << endl;
        return 1;
    }
}

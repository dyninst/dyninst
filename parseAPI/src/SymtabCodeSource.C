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
#include <vector>
#include <map>

#include <boost/assign/list_of.hpp>

#include "common/src/stats.h"
#include "dyntypes.h"

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Function.h"

#include "CodeSource.h"
#include "debug_parse.h"
#include "util.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;


/** SymtabCodeRegion **/

SymtabCodeRegion::~SymtabCodeRegion()
{

}

SymtabCodeRegion::SymtabCodeRegion(
        SymtabAPI::Symtab * st,
        SymtabAPI::Region * reg) :
    _symtab(st),
    _region(reg)
{
}

void
SymtabCodeRegion::names(Address entry, vector<string> & names)
{
    // FIXME SymtabAPI currently doesn't handle ambiguous
    //       functions at the same linear address within
    //       two address spaces. That error is reflected
    //       here.
    SymtabAPI::Function * f = NULL;
    bool found = _symtab->findFuncByEntryOffset(f,entry);
    if(found) {
        // just pretty names?
        const vector<string> & pretty = f->getAllPrettyNames();
        names.insert(names.begin(),pretty.begin(),pretty.end());
    }
	else {
		cerr << "\t Failed to find name" << endl;
	}
}

bool
SymtabCodeRegion::findCatchBlock(Address addr, Address & catchStart)
{
    if(!contains(addr)) return false;

    // FIXME SymtabAPI doesn't handle per-region catch block
    //       lookups either, so faking it for now.
    SymtabAPI::ExceptionBlock e;
    if(_symtab->findCatchBlock(e,addr)) {
        catchStart = e.catchStart();
        return true;
    }
    return false;
}

bool
SymtabCodeRegion::isValidAddress(const Address addr) const
{
    if(!contains(addr)) return false;

    return isAligned(addr) && (isCode(addr) || isData(addr));
}

void *
SymtabCodeRegion::getPtrToInstruction(const Address addr) const
{
    if(!contains(addr)) return NULL;

    if(isCode(addr))
        return (void*)((Address)_region->getPtrToRawData() + 
                       addr - _region->getMemOffset());
    else if(isData(addr))
        return getPtrToData(addr);
    else
        return NULL;
}

void *
SymtabCodeRegion::getPtrToData(const Address addr) const
{
    if(!contains(addr)) return NULL;

    if(isData(addr))
        return (void*)((Address)_region->getPtrToRawData() +
                        addr - _region->getMemOffset());
    else
        return NULL;
}

unsigned int
SymtabCodeRegion::getAddressWidth() const
{
    return _symtab->getAddressWidth();
}

Architecture
SymtabCodeRegion::getArch() const
{
#if defined(arch_power)
    if(getAddressWidth() == 8)
        return Arch_ppc64;
    else
        return Arch_ppc32;
#elif defined(arch_x86) || defined(arch_x86_64)
    if(getAddressWidth() == 8)
        return Arch_x86_64;
    else
        return Arch_x86;
#else
    return Arch_none;
#endif
}

bool
SymtabCodeRegion::isCode(const Address addr) const
{
    if(!contains(addr)) return false;

    // XXX this is the predicate from Symtab::isCode(a) +
    //     the condition by which Symtab::codeRegions_ is filled
    return !_region->isBSS() && 
           (_region->getRegionType() == SymtabAPI::Region::RT_TEXT ||
            _region->getRegionType() == SymtabAPI::Region::RT_TEXTDATA ||
            (_symtab->isDefensiveBinary() && _region->isLoadable()) );
}

bool
SymtabCodeRegion::isData(const Address addr) const
{
    if(!contains(addr)) return false;

    // XXX Symtab::isData(a) tests both RT_DATA (Region::isData(a))
    //     and RT_TEXTDATA. Mimicking that behavior
    return _region->isData() || 
           _region->getRegionType()==SymtabAPI::Region::RT_TEXTDATA;
}

Address
SymtabCodeRegion::offset() const
{
    return _region->getMemOffset();
}

Address
SymtabCodeRegion::length() const
{
    return _region->getDiskSize();
}

/** SymtabCodeSource **/

SymtabCodeSource::~SymtabCodeSource()
{
    _have_stats = false;
    delete stats_parse;
    if(owns_symtab && _symtab)
        SymtabAPI::Symtab::closeSymtab(_symtab);
    for(unsigned i=0;i<_regions.size();++i)
        delete _regions[i];
}

SymtabCodeSource::SymtabCodeSource(SymtabAPI::Symtab * st, 
                                   hint_filt * filt,
                                   bool allLoadedRegions) : 
    _symtab(st),
    owns_symtab(false),
    _lookup_cache(NULL),
    stats_parse(new ::StatContainer()),
    _have_stats(false)
{
    init_stats();
    init(filt,allLoadedRegions);
}

SymtabCodeSource::SymtabCodeSource(SymtabAPI::Symtab * st) : 
    _symtab(st),
    owns_symtab(false),
    _lookup_cache(NULL),
    stats_parse(new ::StatContainer()),
    _have_stats(false)
{
    init_stats();
    init(NULL,false);
}

SymtabCodeSource::SymtabCodeSource(char * file) :
    _symtab(NULL),
    owns_symtab(true),
    _lookup_cache(NULL),
    stats_parse(new ::StatContainer()),
    _have_stats(false)
{
    init_stats();
    
    bool valid;

    valid = SymtabAPI::Symtab::openFile(_symtab,file);
    if(!valid) {
        fprintf(stderr,"[%s] FATAL: can't create Symtab object for file %s\n",
            FILE__,file);
        _symtab = NULL;
        return;
    }
    init(NULL,false);
}

bool
SymtabCodeSource::init_stats() {
    if ((getenv("DYNINST_STATS_PARSING"))) {
        parsing_printf("[%s] Enabling ParseAPI parsing statistics\n", FILE__);
        // General counts
        stats_parse->add(PARSE_BLOCK_COUNT, CountStat);
        stats_parse->add(PARSE_FUNCTION_COUNT, CountStat);
        
        // Basic block size information
        stats_parse->add(PARSE_BLOCK_SIZE, CountStat);

        // Function return status counts
        stats_parse->add(PARSE_NORETURN_COUNT, CountStat);
        stats_parse->add(PARSE_RETURN_COUNT, CountStat);
        stats_parse->add(PARSE_UNKNOWN_COUNT, CountStat);
        stats_parse->add(PARSE_NORETURN_HEURISTIC, CountStat);

        // Heuristic information
        stats_parse->add(PARSE_JUMPTABLE_COUNT, CountStat);
        stats_parse->add(PARSE_JUMPTABLE_FAIL, CountStat);
        stats_parse->add(PARSE_TAILCALL_COUNT, CountStat);
        stats_parse->add(PARSE_TAILCALL_FAIL, CountStat);

        _have_stats = true;
    }

    return _have_stats;
}

void
SymtabCodeSource::print_stats() const {
    
    if (_have_stats) {
        fprintf(stderr, "[%s] Printing ParseAPI statistics\n", FILE__);
        fprintf(stderr, "\t Basic Stats:\n");
        fprintf(stderr, "\t\t Block Count: %ld\n", (*stats_parse)[PARSE_BLOCK_COUNT]->value());
        fprintf(stderr, "\t\t Function Count: %ld\n", (*stats_parse)[PARSE_FUNCTION_COUNT]->value());
        
        long int blockSize = (*stats_parse)[PARSE_BLOCK_SIZE]->value();
        if (blockSize) {
            fprintf(stderr, "\t Basic Block Stats:\n");
            fprintf(stderr, "\t\t Sum of block sizes (in bytes): %ld\n", blockSize);
            fprintf(stderr, "\t\t Average block size (in bytes): %lf\n", (double)blockSize/(double)(*stats_parse)[PARSE_BLOCK_COUNT]->value());
            fprintf(stderr, "\t\t Average blocks per function: %lf\n", 
                    (double)(*stats_parse)[PARSE_BLOCK_COUNT]->value()/(double)(*stats_parse)[PARSE_FUNCTION_COUNT]->value());
        } 
        fprintf(stderr, "\t Function Return Status Stats:\n");
        fprintf(stderr, "\t\t NORETURN Count: %ld", (*stats_parse)[PARSE_NORETURN_COUNT]->value());
        long int noretHeuristicCount = (*stats_parse)[PARSE_NORETURN_HEURISTIC]->value();
        if (noretHeuristicCount) {
            fprintf(stderr, " (Labled based on heuristic: %ld)", noretHeuristicCount);
        }
        fprintf(stderr, "\n");
        fprintf(stderr, "\t\t RETURN Count: %ld\n", (*stats_parse)[PARSE_RETURN_COUNT]->value());
        fprintf(stderr, "\t\t UNKNOWN Count: %ld\n", (*stats_parse)[PARSE_UNKNOWN_COUNT]->value());

        fprintf(stderr, "\t Heuristic Stats:\n");
        fprintf(stderr, "\t\t parseJumpTable attempts: %ld\n", (*stats_parse)[PARSE_JUMPTABLE_COUNT]->value());
        fprintf(stderr, "\t\t parseJumpTable failures: %ld\n", (*stats_parse)[PARSE_JUMPTABLE_FAIL]->value());
        fprintf(stderr, "\t\t isTailCall attempts: %ld\n", (*stats_parse)[PARSE_TAILCALL_COUNT]->value());
        fprintf(stderr, "\t\t isTailCall failures: %ld\n", (*stats_parse)[PARSE_TAILCALL_FAIL]->value());

    }
}

void
SymtabCodeSource::incrementCounter(const std::string& name) const
{
    if (_have_stats) {
        stats_parse->incrementCounter(name);
    }
}

void 
SymtabCodeSource::addCounter(const std::string& name, int num) const
{
    if (_have_stats) {
        stats_parse->addCounter(name, num);
    }
}

void
SymtabCodeSource::decrementCounter(const std::string& name) const
{
    if (_have_stats) {
        stats_parse->decrementCounter(name);
    }
}

void
SymtabCodeSource::init(hint_filt * filt , bool allLoadedRegions)
{
    // regions (and hints)
    init_regions(filt, allLoadedRegions);

    // external linkage
    init_linkage();

    // table of contents (only exists for some binary types)
    _table_of_contents = _symtab->getTOCoffset();
}

void 
SymtabCodeSource::init_regions(hint_filt * filt , bool allLoadedRegions)
{
    dyn_hash_map<void*,CodeRegion*> rmap;
    vector<SymtabAPI::Region *> regs;
    vector<SymtabAPI::Region *> dregs;
    vector<SymtabAPI::Region *>::iterator rit;

    if ( ! allLoadedRegions ) {
        _symtab->getCodeRegions(regs);
        _symtab->getDataRegions(dregs);
        regs.insert(regs.end(),dregs.begin(),dregs.end());
    }
    else {
        _symtab->getMappedRegions(regs);
    }

    parsing_printf("[%s:%d] processing %d symtab regions in %s\n",
        FILE__,__LINE__,regs.size(),_symtab->name().c_str());
    for(rit = regs.begin(); rit != regs.end(); ++rit) {
        parsing_printf("   %lx %s",(*rit)->getMemOffset(),
            (*rit)->getRegionName().c_str());
    
        // XXX only TEXT, DATA, TEXTDATA?
        SymtabAPI::Region::RegionType rt = (*rit)->getRegionType();
        if(false == allLoadedRegions &&
           rt != SymtabAPI::Region::RT_TEXT &&
           rt != SymtabAPI::Region::RT_DATA &&
           rt != SymtabAPI::Region::RT_TEXTDATA)
        {
            parsing_printf(" [skipped]\n");
            continue;
        }

	//#if defined(os_vxworks)
        if(0 == (*rit)->getMemSize()) {
            parsing_printf(" [skipped null region]\n");
            continue;
        }
	//#endif
        parsing_printf("\n");

        if(HASHDEF(rmap,*rit)) {
            parsing_printf("[%s:%d] duplicate region at address %lx\n",
                FILE__,__LINE__,(*rit)->getMemOffset());
        }
        CodeRegion * cr = new SymtabCodeRegion(_symtab,*rit);
        rmap[*rit] = cr;
        addRegion(cr);
    }

    // Hints are initialized at the SCS level rather than the SCR level
    // because there is currently no per-Region lookup of functions in
    // SymtabAPI.
    init_hints(rmap,filt);
}

void
SymtabCodeSource::init_hints(dyn_hash_map<void*, CodeRegion*> & rmap,
    hint_filt * filt)
{
    vector<SymtabAPI::Function *> fsyms;
    vector<SymtabAPI::Function *>::iterator fsit;
    dyn_hash_map<Address,bool> seen;
    int dupes = 0;

    if(!_symtab->getAllFunctions(fsyms))
        return;

    parsing_printf("[%s:%d] processing %d symtab hints\n",FILE__,__LINE__,
        fsyms.size());
    for(fsit = fsyms.begin(); fsit != fsyms.end(); ++fsit) {
        if(filt && (*filt)(*fsit))
        {
            parsing_printf("    == filtered hint %s [%lx] ==\n",
                FILE__,__LINE__,(*fsit)->getOffset(),
                (*fsit)->getFirstSymbol()->getPrettyName().c_str());
            continue;
        }

        if(HASHDEF(seen,(*fsit)->getOffset())) {
            // XXX it looks as though symtabapi now does de-duplication
            //     of function symbols automatically, so this code should
            //     never be reached, except in the case of overlapping
            //     regions
           parsing_printf("[%s:%d] duplicate function at address %lx: %s\n",
                FILE__,__LINE__,
                (*fsit)->getOffset(),
                (*fsit)->getFirstSymbol()->getPrettyName().c_str());
            ++dupes;
        }
        seen[(*fsit)->getOffset()] = true;

        SymtabAPI::Region * sr = (*fsit)->getRegion();
        if(!sr) {
            parsing_printf("[%s:%d] missing Region in function at %lx\n",
                FILE__,__LINE__,(*fsit)->getOffset());
            continue;
        }
        if(!HASHDEF(rmap,sr)) {
            parsing_printf("[%s:%d] unrecognized Region %lx in function %lx\n",
                FILE__,__LINE__,sr->getMemOffset(),(*fsit)->getOffset());
            continue;
        }
        CodeRegion * cr = rmap[sr];
        if(!cr->isCode((*fsit)->getOffset()))
        {
            parsing_printf("\t<%lx> skipped non-code, region [%lx,%lx)\n",
                (*fsit)->getOffset(),
                sr->getMemOffset(),
                sr->getMemOffset()+sr->getDiskSize());
        } else {
            _hints.push_back( Hint((*fsit)->getOffset(),
                               cr,
                               (*fsit)->getFirstSymbol()->getPrettyName()) );
            parsing_printf("\t<%lx,%s,[%lx,%lx)>\n",
                (*fsit)->getOffset(),
                (*fsit)->getFirstSymbol()->getPrettyName().c_str(),
                cr->offset(),
                cr->offset()+cr->length());
        }
    }
}

void
SymtabCodeSource::init_linkage()
{
    vector<SymtabAPI::relocationEntry> fbt;
    vector<SymtabAPI::relocationEntry>::iterator fbtit;

    if(!_symtab->getFuncBindingTable(fbt))
        return;

    for(fbtit = fbt.begin(); fbtit != fbt.end(); ++fbtit)
        _linkage[(*fbtit).target_addr()] = (*fbtit).name(); 
}

bool
SymtabCodeSource::nonReturning(Address addr)
{
    SymtabAPI::Function * f = NULL;
   
    _symtab->findFuncByEntryOffset(f,addr); 

    if(f) {
      const std::vector<std::string> &names = f->getAllMangledNames();
      for (unsigned i = 0; i < names.size(); ++i) {
	if (nonReturning(names[i])) {
	  return true;
	}
      }
    }
    return false;
}

dyn_hash_map<std::string, bool>
SymtabCodeSource::non_returning_funcs =
    boost::assign::map_list_of
        ("exit",true)
        ("abort",true)
        ("__f90_stop",true)
        ("fancy_abort",true)
        ("__stack_chk_fail",true)
        ("__assert_fail",true)
        ("ExitProcess",true)
        ("_ZSt17__throw_bad_allocv",true)
        ("_ZSt20__throw_length_errorPKc",true)
        ("_Unwind_Resume",true)
        ("longjmp",true)
        ("siglongjmp",true)
        ("_ZSt16__throw_bad_castv",true)
        ("_ZSt19__throw_logic_errorPKc",true)
        ("_ZSt20__throw_out_of_rangePKc",true)
        ("__cxa_rethrow",true)
        ("__cxa_throw",true)
        ("_ZSt21__throw_runtime_errorPKc",true)
        ("_gfortran_os_error",true)
        ("_gfortran_runtime_error",true)
        ("_gfortran_stop_numeric", true)
   ("for_stop_core", true)
  ("__sys_exit", true);

bool
SymtabCodeSource::nonReturning(string name)
{
#if defined(os_windows)
	// We see MSVCR<N>.exit
	// Of course, it's often reached via indirect call, but hope never fails.
	if ((name.compare(0, strlen("MSVCR"), "MSVCR") == 0) &&
		(name.find("exit") != name.npos)) return true;
#endif
	parsing_printf("Checking non-returning (Symtab) for %s\n", name.c_str());
    return non_returning_funcs.find(name) != non_returning_funcs.end();
}

Address
SymtabCodeSource::baseAddress() const
{
    return _symtab->getBaseOffset();
}

Address
SymtabCodeSource::loadAddress() const
{
    return _symtab->getLoadOffset();
}

Address
SymtabCodeSource::getTOC(Address addr) const
{
    SymtabAPI::Function *func;

    if (_symtab->getContainingFunction(addr, func)) {
        return func->getTOCOffset();
    }
    return _table_of_contents;
}

inline CodeRegion *
SymtabCodeSource::lookup_region(const Address addr) const
{
    CodeRegion * ret = NULL;
    if(_lookup_cache && _lookup_cache->contains(addr))
        ret = _lookup_cache;
    else {
        set<CodeRegion *> stab;
        int rcnt = findRegions(addr,stab);
    
        assert(rcnt <= 1 || regionsOverlap());

        if(rcnt) {
            ret = *stab.begin();
            _lookup_cache = ret;
        } 
    }
    return ret;
}

inline void 
SymtabCodeSource::overlapping_warn(const char * file, unsigned line) const
{
    if(regionsOverlap()) {
        fprintf(stderr,"Invocation of routine at %s:%d is ambiguous for "
                       "binaries with overlapping code regions\n",
            file,line);
    }
}

bool
SymtabCodeSource::isValidAddress(const Address addr) const
{
    overlapping_warn(FILE__,__LINE__);

    CodeRegion * cr = lookup_region(addr);
    if(cr)
        return cr->isValidAddress(addr);
    else
        return false;
}

void *
SymtabCodeSource::getPtrToInstruction(const Address addr) const
{
    overlapping_warn(FILE__,__LINE__);

    CodeRegion * cr = lookup_region(addr);
    if(cr)
        return cr->getPtrToInstruction(addr);
    else
        return NULL;
}

void *
SymtabCodeSource::getPtrToData(const Address addr) const
{
    overlapping_warn(FILE__,__LINE__);

    CodeRegion * cr = lookup_region(addr);
    if(cr)
        return cr->getPtrToData(addr);
    else
        return NULL;
}

unsigned int
SymtabCodeSource::getAddressWidth() const
{
    return _symtab->getAddressWidth();
}

Architecture
SymtabCodeSource::getArch() const
{
#if defined(arch_power)
    if(getAddressWidth() == 8)
        return Arch_ppc64;
    else
        return Arch_ppc32;
#elif defined(arch_x86) || defined(arch_x86_64)
    if(getAddressWidth() == 8)
        return Arch_x86_64;
    else
        return Arch_x86;
#else
    return Arch_none;
#endif
}

bool
SymtabCodeSource::isCode(const Address addr) const
{
    overlapping_warn(FILE__,__LINE__);

    CodeRegion * cr = lookup_region(addr);
    if(cr)
        return cr->isCode(addr);
    else
        return false;
}

bool
SymtabCodeSource::isData(const Address addr) const
{
    overlapping_warn(FILE__,__LINE__);

    CodeRegion * cr = lookup_region(addr);
    if(cr)
        return cr->isData(addr);
    else
        return false;
}

Address
SymtabCodeSource::offset() const
{
    return _symtab->imageOffset();
}

Address
SymtabCodeSource::length() const
{
    return _symtab->imageLength();
}


void 
SymtabCodeSource::removeRegion(CodeRegion &cr)
{
    _region_tree.remove( &cr );

    for (vector<CodeRegion*>::iterator rit = _regions.begin(); 
         rit != _regions.end(); rit++) 
    {
        if ( &cr == *rit ) {
            _regions.erase( rit );
            break;
        }
    }
}

// fails and returns false if it can't find a CodeRegion
// to match the SymtabAPI::region
// has to remove the region before modifying the region's size, 
// otherwise the region can't be found
bool
SymtabCodeSource::resizeRegion(SymtabAPI::Region *sr, Address newDiskSize)
{
    // find region
    std::set<CodeRegion*> regions;
    findRegions(sr->getMemOffset(), regions);
    bool found_it = false;
    set<CodeRegion*>::iterator rit = regions.begin();
    for (; rit != regions.end(); rit++) {
        if (sr == ((SymtabCodeRegion*)(*rit))->symRegion()) {
            found_it = true;
            break;
        }
    }

    if (!found_it) {
        return false;
    }

    // remove, resize, reinsert
    removeRegion( **rit );
    sr->setDiskSize( newDiskSize );
    addRegion( *rit );
    return true;
}

void
SymtabCodeSource::addNonReturning(std::string func_name)
{
    non_returning_funcs[func_name] = true;
}

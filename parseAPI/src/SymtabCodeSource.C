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
#include "symtabAPI/h/Symbol.h"

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
    vector<SymtabAPI::Symbol*> symbols;
    st->getAllSymbols(symbols);
    for (auto sit = symbols.begin(); sit != symbols.end(); ++sit)
        if ( (*sit)->getRegion() == reg && (*sit)->getType() != SymtabAPI::Symbol::ST_FUNCTION && (*sit)->getType() != SymtabAPI::Symbol::ST_INDIRECT) {
	    knownData[(*sit)->getOffset()] = (*sit)->getOffset() + (*sit)->getSize();
	}
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
        names.insert(names.begin(),f->pretty_names_begin(),
		     f->pretty_names_end());
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
    return _symtab->getArchitecture();
}

bool
SymtabCodeRegion::isCode(const Address addr) const
{
    if(!contains(addr)) return false;
    map<Address, Address>::const_iterator dit = knownData.upper_bound(addr);
    if (dit != knownData.begin()) {
        --dit;
	if (dit->first <= addr && dit->second > addr) return false;
    }
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

bool
SymtabCodeRegion::isReadOnly(const Address addr) const
{
    if(!contains(addr)) return false;
    if (_region->getRegionName() == ".data.rel.ro") return true;
//    parsing_printf("Region name %s, permission %d\n", _region->getRegionName().c_str(), _region->getRegionPermissions());
    return _region->getRegionPermissions() == SymtabAPI::Region::RP_R ||
           _region->getRegionPermissions() == SymtabAPI::Region::RP_RX;
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

	stats_parse->add(PARSE_JUMPTABLE_TIME, TimerStat);
	stats_parse->add(PARSE_TOTAL_TIME, TimerStat);


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

	fprintf(stderr, "\t Parsing total time: %.2lf\n", (*stats_parse)[PARSE_TOTAL_TIME]->usecs());
	fprintf(stderr, "\t Parsing jump table time: %.2lf\n", (*stats_parse)[PARSE_JUMPTABLE_TIME]->usecs());

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
SymtabCodeSource::startTimer(const std::string & name) const
{
    if (_have_stats) {
        stats_parse->startTimer(name);
    }
}

void
SymtabCodeSource::stopTimer(const std::string & name) const
{
    if (_have_stats) {
        stats_parse->stopTimer(name);
    }
}

void
SymtabCodeSource::init(hint_filt * filt , bool allLoadedRegions)
{
    // regions (and hints)
    init_regions(filt, allLoadedRegions);

    // external linkage
    init_linkage();

    // Fetch and sort exception blocks
    init_try_blocks();

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

        // Cleaned-up version of a rather ugly series of strcmp's. Is this the
        // right place to do this? Should these symbols not be filtered by the
        // loop above?
        /*Achin added code starts 12/15/2014*/
        static const vector<std::string> skipped_symbols = {
          "_non_rtti_object::`vftable'",
          "bad_cast::`vftable'",
          "exception::`vftable'",
          "bad_typeid::`vftable'" ,
          "sys_errlist",
          "std::_non_rtti_object::`vftable'",
          "std::__non_rtti_object::`vftable'",
          "std::bad_cast::`vftable'",
          "std::exception::`vftable'",
          "std::bad_typeid::`vftable'" };
        if (std::find(skipped_symbols.begin(), skipped_symbols.end(),
          (*fsit)->getFirstSymbol()->getPrettyName()) != skipped_symbols.end()) {
          continue;
        }
        /*Achin added code ends*/

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
	                       (*fsit)->getSize(),
                               cr,
                               (*fsit)->getFirstSymbol()->getPrettyName()) );
            parsing_printf("\t<%lx,%s,[%lx,%lx)>\n",
                (*fsit)->getOffset(),
                (*fsit)->getFirstSymbol()->getPrettyName().c_str(),
                cr->offset(),
                cr->offset()+cr->length());
        }
    }
    sort(_hints.begin(), _hints.end());
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

void
SymtabCodeSource::init_try_blocks()
{
    vector<SymtabAPI::ExceptionBlock *> exBlks;
    _symtab->getAllExceptions(exBlks);
    for (auto bit = exBlks.begin(); bit != exBlks.end(); ++bit) {
        SymtabAPI::ExceptionBlock* b = *bit;
	try_blocks.push_back(try_block(b->tryStart(), b->tryEnd(), b->catchStart()));
    }
    sort(try_blocks.begin(), try_blocks.end());
    for (size_t i = 1; i < try_blocks.size(); ++i) {
        if (try_blocks[i].tryStart < try_blocks[i-1].tryEnd) {
	    assert(!"WARNING: overlapping try blocks\n");
	}
    }
}

bool
SymtabCodeSource::nonReturning(Address addr)
{
    SymtabAPI::Function * f = NULL;
   
    _symtab->findFuncByEntryOffset(f,addr); 

    if(f) {
      for(auto i = f->mangled_names_begin();
	  i != f->mangled_names_end();
	  ++i)
      {
	if(CodeSource::nonReturning(*i)) return true;
      }
    }
    return false;
}

bool
SymtabCodeSource::nonReturningSyscall(int num)
{
  parsing_printf("Checking non-returning (Symtab) for %d\n", num);
  Architecture arch = getArch();
  switch(arch) {
    case(Arch_x86):
      return non_returning_syscalls_x86.find(num) != non_returning_syscalls_x86.end();
    case(Arch_x86_64):
      return non_returning_syscalls_x86_64.find(num) != non_returning_syscalls_x86_64.end();
    default:
      return false;
  }
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
    return _symtab->getArchitecture();
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

bool
SymtabCodeSource::isReadOnly(const Address addr) const
{
    overlapping_warn(FILE__,__LINE__);

    CodeRegion * cr = lookup_region(addr);
    if(cr)
        return cr->isReadOnly(addr);
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

bool 
SymtabCodeSource::findCatchBlockByTryRange(Address addr, std::set<Address> & catchStarts) const {
    try_block target(addr, 0, 0);
    catchStarts.clear();
    std::vector<try_block>::const_iterator bit = upper_bound(try_blocks.begin(), try_blocks.end(), target);
    if (bit == try_blocks.begin()) return false;
    --bit;
    if (bit->tryStart <= addr && addr < bit->tryEnd)
        catchStarts.insert(bit->catchStart); 
    return true;
}


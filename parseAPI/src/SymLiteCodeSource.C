/*
 * See the dyninst/COPYRIGHT file for copyright information.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <vector>
#include <map>

#include <boost/assign/list_of.hpp>

#include "common/src/stats.h"
#include "dyntypes.h"

#include "common/h/SymReader.h"

#include "SymLiteCodeSource.h"
#include "debug_parse.h"
#include "util.h"

#include "symlite/h/SymLite-elf.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

/** SymReaderCodeRegion **/

SymbolReaderFactory* getSymReaderFactory()
{
  static SymbolReaderFactory* se(NULL);
  if(se == NULL) se = new SymElfFactory();
  return se;
}


SymReaderCodeRegion::~SymReaderCodeRegion()
{
  if(rawData && (rawData != MAP_FAILED))
    munmap(rawData, _region->mem_size);

}

SymReaderCodeRegion::SymReaderCodeRegion(
        SymReader * st,
        SymSegment * reg) :
    _symtab(st),
    _region(reg),
    rawData(NULL)
{
  rawData = mmap(NULL, _region->mem_size, PROT_READ,
		 MAP_PRIVATE, _symtab->getFD(), _region->file_offset);
  if(rawData == MAP_FAILED)
  {
    rawData = NULL;
  }
}

void
SymReaderCodeRegion::names(Address entry, vector<string> & names)
{
  Symbol_t func = _symtab->getContainingSymbol(entry);
  std::string mangledName, prettyName;
  mangledName = _symtab->getSymbolName(func);
  prettyName = _symtab->getDemangledName(func);
  if(!mangledName.empty()) names.push_back(mangledName);
  if(!prettyName.empty()) names.push_back(prettyName);
}

bool
SymReaderCodeRegion::findCatchBlock(Address, Address &)
{
  // SymLite doesn't support catch blocks at all.
  return false;
}

bool
SymReaderCodeRegion::isValidAddress(const Address addr) const
{
    if(!contains(addr)) return false;

    return isAligned(addr) && (isCode(addr) || isData(addr));
}

void *
SymReaderCodeRegion::getPtrToInstruction(const Address addr) const
{
    if(!contains(addr)) return NULL;

    if(isCode(addr))
        return (void*)((Address)rawData + 
                       addr - _region->mem_addr);
    else if(isData(addr))
        return getPtrToData(addr);
    else
        return NULL;
}

void *
SymReaderCodeRegion::getPtrToData(const Address addr) const
{
    if(!contains(addr)) return NULL;

    if(isData(addr))
        return (void*)((Address)rawData +
                        addr - _region->mem_addr);
    else
        return NULL;
}

unsigned int
SymReaderCodeRegion::getAddressWidth() const
{
    return _symtab->getAddressWidth();
}

Architecture
SymReaderCodeRegion::getArch() const
{
    return _symtab->getArchitecture();
}

bool
SymReaderCodeRegion::isCode(const Address addr) const
{
    if(!contains(addr)) return false;
    return _region->perms & PF_X;
    
    /*
    // XXX this is the predicate from SymReader::isCode(a) +
    //     the condition by which SymReader::codeRegions_ is filled
    return !_region->isBSS() && 
           (_region->type == SymSegment::RT_TEXT ||
            _region->type == SymSegment::RT_TEXTDATA);
    */
}

bool
SymReaderCodeRegion::isData(const Address addr) const
{
    if(!contains(addr)) return false;
    return !(_region->perms & PF_X);
    

    /*
    // XXX SymReader::isData(a) tests both RT_DATA (Region::isData(a))
    //     and RT_TEXTDATA. Mimicking that behavior
    return _region->isData() || 
           _region->type==SymSegment::RT_TEXTDATA;
    */
}

bool
SymReaderCodeRegion::isReadOnly(const Address addr) const
{
    if (!contains(addr))  {
        return false;
    }

    return !(_region->perms & PF_W);
}

Address
SymReaderCodeRegion::offset() const
{
    return _region->mem_addr;
}

Address
SymReaderCodeRegion::length() const
{
    return _region->file_size;
}

/** SymReaderCodeSource **/

SymReaderCodeSource::~SymReaderCodeSource()
{
    _have_stats = false;
    free(stats_parse);
    if(owns_symtab && _symtab)
      getSymReaderFactory()->closeSymbolReader(_symtab);
}

SymReaderCodeSource::SymReaderCodeSource(SymReader * st) : 
    _symtab(st),
    owns_symtab(false),
    _lookup_cache(NULL),
    stats_parse(new ::StatContainer()),
    _have_stats(false)
{
  init_regions();
  init_stats();
}

bool shouldAddRegion(SymSegment* sr)
{
  // First: text, data, or textdata only
  switch(sr->type)
  {
  case SHT_PROGBITS: return true;
  default: return false;
  }
  // Second: memsize > 0
  return sr->mem_size > 0;
}


void SymReaderCodeSource::init_regions()
{
  for(unsigned i = 0; i < _symtab->numSegments(); i++)
  {
    SymSegment reg;
    
    if(_symtab->getSegment(i, reg))
    {
      SymSegment* tmp = new SymSegment(reg);
      
      CodeRegion* cr = new SymReaderCodeRegion(_symtab, tmp);
      if(shouldAddRegion(tmp)) 
      {
	addRegion(cr);
      }
      else
      {
	delete cr;
      }
    }
    
  }
}


SymReaderCodeSource::SymReaderCodeSource(const char * file) :
    _symtab(NULL),
    owns_symtab(true),
    _lookup_cache(NULL),
    stats_parse(new ::StatContainer()),
    _have_stats(false)
{
  
  _symtab = getSymReaderFactory()->openSymbolReader(file);
  if(!_symtab) {
    fprintf(stderr,"[%s] FATAL: can't create SymReader object for file %s\n",
            FILE__,file);
    return;
  }
  init_stats();
  init_regions();
  
}

bool
SymReaderCodeSource::init_stats() {
    if (getenv("DYNINST_STATS_PARSING")) {
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
SymReaderCodeSource::print_stats() const {
    
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
SymReaderCodeSource::incrementCounter(const std::string& name) const
{
    if (_have_stats) {
        stats_parse->incrementCounter(name);
    }
}

void 
SymReaderCodeSource::addCounter(const std::string& name, int num) const
{
    if (_have_stats) {
        stats_parse->addCounter(name, num);
    }
}

void
SymReaderCodeSource::decrementCounter(const std::string& name) const
{
    if (_have_stats) {
        stats_parse->decrementCounter(name);
    }
}

bool
SymReaderCodeSource::nonReturning(Address addr)
{
  Symbol_t func = _symtab->getContainingSymbol(addr);
  string func_name = _symtab->getSymbolName(func);
  return CodeSource::nonReturning(func_name);
}

bool
SymReaderCodeSource::nonReturningSyscall(int num)
{
  parsing_printf("Checking non-returning (SymLite) for %d\n", num);
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

inline CodeRegion *
SymReaderCodeSource::lookup_region(const Address addr) const
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
SymReaderCodeSource::overlapping_warn(const char * file, unsigned line) const
{
    if(regionsOverlap()) {
        fprintf(stderr,"Invocation of routine at %s:%u is ambiguous for "
                       "binaries with overlapping code regions\n",
            file,line);
    }
}

bool
SymReaderCodeSource::isValidAddress(const Address addr) const
{
    overlapping_warn(FILE__,__LINE__);
    

    CodeRegion * cr = lookup_region(addr);
    if(cr) 
    {
        return cr->isValidAddress(addr);
    }
    else
    {
        return false;
    }
    
}

void *
SymReaderCodeSource::getPtrToInstruction(const Address addr) const
{
    overlapping_warn(FILE__,__LINE__);

    CodeRegion * cr = lookup_region(addr);
    if(cr)
        return cr->getPtrToInstruction(addr);
    else
        return NULL;
}

void *
SymReaderCodeSource::getPtrToData(const Address addr) const
{
    overlapping_warn(FILE__,__LINE__);

    CodeRegion * cr = lookup_region(addr);
    if(cr)
        return cr->getPtrToData(addr);
    else
        return NULL;
}

unsigned int
SymReaderCodeSource::getAddressWidth() const
{
    return _symtab->getAddressWidth();
}

Architecture
SymReaderCodeSource::getArch() const
{
    return _symtab->getArchitecture();
}

bool
SymReaderCodeSource::isCode(const Address addr) const
{
    overlapping_warn(FILE__,__LINE__);

    CodeRegion * cr = lookup_region(addr);
    if(cr)
        return cr->isCode(addr);
    else
        return false;
}

bool
SymReaderCodeSource::isData(const Address addr) const
{
    overlapping_warn(FILE__,__LINE__);

    CodeRegion * cr = lookup_region(addr);
    if(cr)
        return cr->isData(addr);
    else
        return false;
}

bool
SymReaderCodeSource::isReadOnly(const Address addr) const
{
    overlapping_warn(FILE__,__LINE__);

    CodeRegion * cr = lookup_region(addr);
    if(cr)
        return cr->isReadOnly(addr);
    else
        return false;
}

Address
SymReaderCodeSource::offset() const
{
    return _symtab->imageOffset();
}

Address
SymReaderCodeSource::length() const
{
  return 0;//_symtab->imageLength();
}


void 
SymReaderCodeSource::removeRegion(CodeRegion *cr)
{
	CodeSource::removeRegion(cr);
}

// fails and returns false if it can't find a CodeRegion
// to match the region
// has to remove the region before modifying the region's size, 
// otherwise the region can't be found
bool
SymReaderCodeSource::resizeRegion(SymSegment *sr, Address newDiskSize)
{
    // find region
    std::set<CodeRegion*> regions;
    findRegions(sr->mem_addr, regions);
    bool found_it = false;
    set<CodeRegion*>::iterator rit = regions.begin();
    for (; rit != regions.end(); rit++) {
        if (sr == ((SymReaderCodeRegion*)(*rit))->symRegion()) {
            found_it = true;
            break;
        }
    }

    if (!found_it) {
        return false;
    }

    // remove, resize, reinsert
    removeRegion( *rit );
    sr->file_size = newDiskSize;
    addRegion( *rit );
    return true;
}

void
SymReaderCodeSource::addNonReturning(std::string func_name)
{
    non_returning_funcs[func_name] = true;
}


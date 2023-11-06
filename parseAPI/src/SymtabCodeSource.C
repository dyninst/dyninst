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
#include <atomic>

#include <boost/assign/list_of.hpp>

#include "common/src/stats.h"
#include "dyntypes.h"

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Function.h"
#include "symtabAPI/h/Symbol.h"

#include "CodeSource.h"
#include "debug_parse.h"
#include "util.h"
#include "unaligned_memory_access.h"

#include "InstructionDecoder.h"
#include "Instruction.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

typedef std::pair<SymtabAPI::Region *, Offset> RegionOffsetPair;
typedef dyn_c_hash_map<RegionOffsetPair, bool> SeenMap;

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
	    if ((*sit)->getRegion()->isText()) continue;
            knownData[(*sit)->getOffset()] = (*sit)->getOffset() + (*sit)->getSize();
            parsing_printf("Add known data range [%lx, %lx) from symbol %s\n", (*sit)->getOffset(), (*sit)->getOffset() + (*sit)->getSize(), (*sit)->getMangledName().c_str());
        }
}


SymtabCodeRegion::SymtabCodeRegion(
        SymtabAPI::Symtab * st,
        SymtabAPI::Region * reg,
	vector<SymtabAPI::Symbol*> &symbols) :
    _symtab(st),
    _region(reg)
{
    for (auto sit = symbols.begin(); sit != symbols.end(); ++sit)
        if ( (*sit)->getRegion() == reg && (*sit)->getType() != SymtabAPI::Symbol::ST_FUNCTION && (*sit)->getType() != SymtabAPI::Symbol::ST_INDIRECT) {
	    if ((*sit)->getRegion()->isText()) continue;
            knownData[(*sit)->getOffset()] = (*sit)->getOffset() + (*sit)->getSize();
            parsing_printf("Add known data range [%lx, %lx) from symbol %s\n", (*sit)->getOffset(), (*sit)->getOffset() + (*sit)->getSize(), (*sit)->getMangledName().c_str());
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

    return (void*)((Address)_region->getPtrToRawData() +
                   addr - _region->getMemOffset());
}

void *
SymtabCodeRegion::getPtrToData(const Address addr) const
{
    if(!contains(addr)) return NULL;

    return (void*)((Address)_region->getPtrToRawData() +
                    addr - _region->getMemOffset());
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
}

SymtabCodeSource::SymtabCodeSource(SymtabAPI::Symtab * st, 
                                   hint_filt * filt,
                                   bool allLoadedRegions) : 
    _symtab(st),
    owns_symtab(false),
    stats_parse(new ::StatContainer()),
    _have_stats(false)
{
    init_stats();
    init(filt,allLoadedRegions);
}

SymtabCodeSource::SymtabCodeSource(SymtabAPI::Symtab * st) : 
    _symtab(st),
    owns_symtab(false),
    stats_parse(new ::StatContainer()),
    _have_stats(false)
{
    init_stats();
    init(NULL,false);
}

SymtabCodeSource::SymtabCodeSource(const char * file) :
    _symtab(NULL),
    owns_symtab(true),
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
    RegionMap rmap;
    vector<SymtabAPI::Region *> regs;
    vector<SymtabAPI::Region *> dregs;
    vector<SymtabAPI::Symbol*> symbols;
    dyn_mutex reg_lock;

    if ( ! allLoadedRegions ) {
        _symtab->getCodeRegions(regs);
        _symtab->getDataRegions(dregs);
        regs.insert(regs.end(),dregs.begin(),dregs.end());
    }
    else {
        _symtab->getMappedRegions(regs);
    }

    parsing_printf("[%s:%d] processing %lu symtab regions in %s\n",
        FILE__,__LINE__,regs.size(),_symtab->name().c_str());

    _symtab->getAllSymbols(symbols);

#pragma omp parallel for shared(regs,dregs,symbols,reg_lock) schedule(auto)
    for (unsigned int i = 0; i < regs.size(); i++) {
        SymtabAPI::Region *r = regs[i];
        parsing_printf("   %lx %s",r->getMemOffset(),
            r->getRegionName().c_str());
    
        // XXX only TEXT, DATA, TEXTDATA?
        SymtabAPI::Region::RegionType rt = r->getRegionType();
        if(false == allLoadedRegions &&
           rt != SymtabAPI::Region::RT_TEXT &&
           rt != SymtabAPI::Region::RT_DATA &&
           rt != SymtabAPI::Region::RT_TEXTDATA)
        {
            parsing_printf(" [skipped]\n");
            continue;
        }

        if(0 == r->getMemSize()) {
            parsing_printf(" [skipped null region]\n");
            continue;
        }

        parsing_printf("\n");

        CodeRegion * cr = new SymtabCodeRegion(_symtab,r, symbols);
        bool already_present = !rmap.insert(std::make_pair(r, cr));
        if (already_present) {
            parsing_printf("[%s:%d] duplicate region at address %lx\n",
                FILE__,__LINE__,r->getMemOffset());
        }
        {
            dyn_mutex::unique_lock l(reg_lock);
            addRegion(cr);
        }
    }

    // Hints are initialized at the SCS level rather than the SCR level
    // because there is currently no per-Region lookup of functions in
    // SymtabAPI.
    init_hints(rmap,filt);
}

void
SymtabCodeSource::init_hints(RegionMap &rmap, hint_filt * filt)
{
    const vector<SymtabAPI::Function *>& fsyms = _symtab->getAllFunctionsRef();
    SeenMap seen;
    dyn_c_vector<Hint> h;

    parsing_printf("[%s:%d] processing %lu symtab hints\n",FILE__,__LINE__,
        fsyms.size());

    atomic_bool foundEntrySymbol{};
    Address entryOffset = _symtab->getEntryOffset();

#pragma omp parallel for schedule(auto)
    for (unsigned int i = 0; i < fsyms.size(); i++) {
        SymtabAPI::Function *f = fsyms[i];
        vector<SymtabAPI::Symbol*> syms;
        f->getSymbols(syms);
        string fname_s = syms[0]->getPrettyName();
        for (size_t j = 1; j < syms.size(); ++j)
            if (syms[j]->getPrettyName() < fname_s) fname_s = syms[j]->getPrettyName();
        const char *fname = fname_s.c_str();
        if(filt && (*filt)(f)) {
            parsing_printf("[%s:%d}  == filtered hint %s [%lx] ==\n",
                FILE__,__LINE__,fname, f->getOffset());
            continue;
        }

        // Cleaned-up version of a rather ugly series of strcmp's. Is this the
        // right place to do this? Should these symbols not be filtered by the
        // loop above?
        /*Achin added code starts 12/15/2014*/
        if (std::find(skipped_symbols.begin(), skipped_symbols.end(), fname_s) != skipped_symbols.end()) {
          continue;
        }
        /*Achin added code ends*/
        Offset offset = f->getOffset();
        SymtabAPI::Region * sr = f->getRegion();

        bool present = !seen.insert(std::make_pair(RegionOffsetPair(sr, offset), true));

        if (present) {
           parsing_printf("[%s:%d] duplicate function at address %lx: %s\n",
                FILE__,__LINE__, f->getOffset(), fname);
           continue;
        }

        if (!sr) {
            parsing_printf("[%s:%d] missing Region in function at %lx\n",
                FILE__,__LINE__,f->getOffset());
            continue;
        }

        CodeRegion * cr = NULL;

        {
          RegionMap::const_accessor a;
          present = rmap.find(a, sr);
          if (present) cr = a->second;
        }

        if (!present) {
            parsing_printf("[%s:%d] unrecognized Region %lx in function %lx\n",
                FILE__,__LINE__,sr->getMemOffset(),f->getOffset());
            continue;
        }

        if(!cr->isCode(f->getOffset())) {
            parsing_printf("\t<%lx> skipped non-code, region [%lx,%lx)\n",
                           f->getOffset(),
                           sr->getMemOffset(),
                           sr->getMemOffset()+sr->getDiskSize());
        } else {
          if (entryOffset == offset)  {
            foundEntrySymbol = true;
          }

          _hints.push_back(Hint(f->getOffset(), f->getSymbolSize(), cr, fname_s));
          parsing_printf("\t<%lx,%s,[%lx,%lx)>\n",
                         f->getOffset(),
                         fname,
                         cr->offset(),
                         cr->offset()+cr->length());
        }
    }

    if (!foundEntrySymbol && _symtab->isExecutable())  {
      // add entry point as this object is an executable
      // and no symbol referenced the entry point
      parsing_printf("Adding exectable entry point at %lx\n", entryOffset);
      SymtabAPI::Region *sr = _symtab->findEnclosingRegion(entryOffset);
      if (sr)  {
        CodeRegion *cr = NULL;
        {
          RegionMap::const_accessor a;
          bool found = rmap.find(a, sr);
          if (found)  {
            cr = a->second;
          }
        }

        if (cr)  {
	  const char startFuncName[] = "_start";
	  // use 0 for function length as length is unknown and value unused
          _hints.push_back(Hint(entryOffset, 0, cr, startFuncName));
          parsing_printf("\t<%lx,%s,[%lx,%lx)>\n",
                         entryOffset,
                         startFuncName,
                         cr->offset(),
                         cr->offset() + cr->length());
        }  else  {
          parsing_printf("[%s:%d] unrecognized Region %lx entry point function %lx\n",
              FILE__, __LINE__, sr->getMemOffset(), entryOffset);
        }
      }  else  {
        parsing_printf("[%s:%d] Symtab Region for entry point function %lx not found\n",
            FILE__, __LINE__, entryOffset);
      }
    }
}

void
SymtabCodeSource::init_linkage()
{
    vector<SymtabAPI::relocationEntry> fbt;
    vector<SymtabAPI::relocationEntry>::iterator fbtit;

    if(!_symtab->getFuncBindingTable(fbt)){
        parsing_printf("Cannot get function binding table. %s\n", _symtab->file().c_str());
        return;
    }

    for(fbtit = fbt.begin(); fbtit != fbt.end(); ++fbtit){
        //fprintf( stderr, "%lx %s\n", (*fbtit).target_addr(), (*fbtit).name().c_str());
        _linkage[(*fbtit).target_addr()] = (*fbtit).name(); 
    }
    if (getArch() != Arch_x86_64) return;
    SymtabAPI::Region * plt_sec = NULL;
    if (_symtab->findRegion(plt_sec, ".plt.sec")) {
        // Handle 2-PLT style PLT used for Indirect Branch Tracking (IBT) in Intel CET
        SymtabAPI::Region * rela_plt = NULL;
        if (!_symtab->findRegion(rela_plt, ".rela.plt")) return;
        // Get PLT related relocation entries
        map<Address, string> rel_addr_to_name;
        std::vector<SymtabAPI::relocationEntry> &relocs = rela_plt->getRelocations();
        for (auto re_it = relocs.begin(); re_it != relocs.end(); ++re_it) {
            SymtabAPI::relocationEntry &r = *re_it;
            rel_addr_to_name[r.rel_addr()] = r.name();
        }

        // Each PLT stub is 16 byte long
        const int plt_entry_size = 16;
        // Each PLT stub starts with a ENDBR64 instruction, which is 4 byte long,
        // followed by a indirect jump instruction.
        // The indirect jump instruction has a BND prefix and two byte opcode.
        // Therefore, the offset to the PC-relative displacement is 7 bytes.
        const int pc_rela_disp = 7;
        const unsigned char* buffer = (const unsigned char*)plt_sec->getPtrToRawData();

        // Scan each PLT stub
        for (size_t off = 0; off < plt_sec->getMemSize(); off += plt_entry_size) {
            auto disp = Dyninst::read_memory_as<int32_t>(buffer + off + pc_rela_disp);
            Address rel_addr = plt_sec->getMemOffset() + off + pc_rela_disp + 4 /* four byte pc-relative displacment */ + disp;
            if (rel_addr_to_name.find(rel_addr) != rel_addr_to_name.end()) {
                Address tar = plt_sec->getMemOffset() + off;
                if (rel_addr_to_name[rel_addr] == "") {
                    // Sometimes PLT is used to call function in the same library.
                    // One such case is to perform a call to a ifunc to use CPU dispatch feature
                    char ifunc_name[128];
                    snprintf(ifunc_name, 128, "ifunc%lx", tar);
                    _linkage[tar] = std::string(ifunc_name);
                } else {
                    _linkage[tar] = rel_addr_to_name[rel_addr];
                }
            }
        }
        return;
    }
    SymtabAPI::Region * plt_got = NULL;
    SymtabAPI::Region * rela_dyn = NULL;
    if (!_symtab->findRegion(plt_got, ".plt.got")) return;
    if (!_symtab->findRegion(rela_dyn, ".rela.dyn")) return;
    if (plt_got->getMemSize() <= 16) return ;

    map<Address, string> rel_addr_to_name;
    std::vector<SymtabAPI::relocationEntry> &relocs = rela_dyn->getRelocations();
    for (auto re_it = relocs.begin(); re_it != relocs.end(); ++re_it) {
        SymtabAPI::relocationEntry &r = *re_it;
        rel_addr_to_name[r.rel_addr()] = r.name();
    }
    const unsigned char* buffer = (const unsigned char*)plt_got->getPtrToRawData(); 
    InstructionAPI::InstructionDecoder dec(buffer,plt_got->getMemSize(), getArch());
    unsigned long decoded = 0;
    while (decoded < plt_got->getMemSize()) {
        InstructionAPI::Instruction i = dec.decode();
        if (buffer[decoded] == 0xff && buffer[decoded+1U] == 0x25) {
            uint64_t off = 0;
            for (unsigned int j = 5; j >= 2; --j)
                off = (off << 8) + buffer[decoded + j];
            Address rel_addr = plt_got->getMemOffset() + decoded + i.size() + off;
            if (rel_addr_to_name.find(rel_addr) != rel_addr_to_name.end()) {
                _linkage[plt_got->getMemOffset() + decoded] = rel_addr_to_name[rel_addr];
            }
        }
        decoded += i.size();
    }
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
    CodeRegion * cache = _lookup_cache.get();

    if(cache && cache->contains(addr))
        ret = cache;
    else {
        set<CodeRegion *> stab;
        int rcnt = findRegions(addr,stab);
    
        assert(rcnt <= 1 || regionsOverlap());

        if(rcnt) {
          ret = *stab.begin();
          _lookup_cache.set(ret);
        } 
    }
    return ret;
}

inline void 
SymtabCodeSource::overlapping_warn(const char * file, unsigned line) const
{
    
    if(regionsOverlap()) {
        parsing_printf("Invocation of routine at %s:%u is ambiguous for "
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
SymtabCodeSource::removeRegion(CodeRegion *cr)
{
	CodeSource::removeRegion(cr);
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
    removeRegion( *rit );
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


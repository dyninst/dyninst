/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#include "dyntypes.h"

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Function.h"

#include "CodeSource.h"
#include "debug_parse.h"
#include "util.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

/** CodeSource **/
void
CodeSource::addRegion(CodeRegion * cr)
{
    _regions.push_back(cr);

    // check for overlapping regions
    if(!_regions_overlap) {
        set<CodeRegion *> exist;
        _region_tree.find(cr,exist);
        if(!exist.empty())
            _regions_overlap = true;
    }

    _region_tree.insert(cr);
}

int
CodeSource::findRegions(Address addr, set<CodeRegion *> & ret) const
{
    return _region_tree.find(addr,ret);
}

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
                       addr - _region->getRegionAddr());
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
                        addr - _region->getRegionAddr());
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
    return _region->getRegionAddr();
}

Address
SymtabCodeRegion::length() const
{
    return _region->getDiskSize();
}

/** SymtabCodeSource **/

SymtabCodeSource::~SymtabCodeSource()
{
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
    _lookup_cache(NULL)
{
    init(filt,allLoadedRegions);
}

SymtabCodeSource::SymtabCodeSource(SymtabAPI::Symtab * st) : 
    _symtab(st),
    owns_symtab(false),
    _lookup_cache(NULL)
{
    init(NULL,false);
}

SymtabCodeSource::SymtabCodeSource(char * file) :
    _symtab(NULL),
    owns_symtab(true),
    _lookup_cache(NULL)
{
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
        parsing_printf("   %lx %s",(*rit)->getRegionAddr(),
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
                FILE__,__LINE__,(*rit)->getRegionAddr());
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
                FILE__,__LINE__,sr->getRegionAddr(),(*fsit)->getOffset());
            continue;
        }
        CodeRegion * cr = rmap[sr];
        if(!cr->isCode((*fsit)->getOffset()))
        {
            parsing_printf("\t<%lx> skipped non-code, region [%lx,%lx)\n",
                (*fsit)->getOffset(),
                sr->getRegionAddr(),
                sr->getRegionAddr()+sr->getRegionSize());
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
    bool ret = false;
   
    _symtab->findFuncByEntryOffset(f,addr); 

    if(f) {
        SymtabAPI::Symbol * s = f->getFirstSymbol();
        string st_name = s->getMangledName();
        ret = nonReturning(st_name);
    }

    return ret;
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
        ("_gfortran_runtime_error",true);

bool
SymtabCodeSource::nonReturning(string name)
{
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

#if defined(os_aix)    
        // XXX AIX objects frequently have overlapping regions.
        //     Prefer code over data regions, following 
        //     SymtabAPI::findEnclosingRegion()
        if(rcnt) {
            set<CodeRegion*>::iterator sit = stab.begin();
            for( ; sit != stab.end(); ++sit) {
                CodeRegion * tmp = *sit;
                if(tmp->isCode(addr)) {
                    ret = tmp;
                    break;
                } else {
                    // XXX hold the first non-code region as 
                    //     the default return. Don't stop looking,
                    //     however; might find a code region
                    if(!ret)
                        ret = tmp;
                }
            } 
            _lookup_cache = ret; 
        }
#else
        if(rcnt) {
            ret = *stab.begin();
            _lookup_cache = ret;
        } 
#endif
    }
    return ret;
}

inline void 
SymtabCodeSource::overlapping_warn(const char * file, unsigned line) const
{
    // XXX AIX objects frequently have overlapping regions; just deal.
#if !defined(os_aix)
    if(regionsOverlap()) {
        fprintf(stderr,"Invocation of routine at %s:%d is ambiguous for "
                       "binaries with overlapping code regions\n",
            file,line);
    }
#endif
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
    findRegions(sr->getRegionAddr(), regions);
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

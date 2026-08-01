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

// Synthesizes .eh_frame and, for functions with a C++ LSDA, .gcc_except_table
// for relocated code: all the CIE/FDE/LSDA machinery that lets a C++ exception
// unwind through, and be caught in, code dyninst has relocated.

#include "eh_frame_gen.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>   // malloc
#include <cstring>   // memcpy, memset
#include <map>
#include <utility>
#include <vector>

#include "Relocation/CodeTracker.h"          // Relocation::CodeTracker
#include "symtabAPI/h/Symtab.h"              // Symtab, EHFunctionInfo, FrameRegRule
#include "Region.h"                          // Region
#include "VariableLocation.h"
#include "debug.h"                            // eh_printf (DYNINST_DEBUG_EH)
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"   // DT_DYNINST_EH_FRAME

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

// Build the .eh_frame / .gcc_except_table blobs for the relocated code, placed
// at addresses derived from `regionBase` (the low end of where the blobs will
// live -- a link-time vaddr for a rewriter, a live inferiorMalloc address for a
// process). Pure: no delivery, no side effects on symObj beyond reads, so it is
// safe to call twice (once with a placeholder base to learn the sizes, once
// with the real base once the region has been allocated). Returns the number of
// FDEs emitted; the bytes and their chosen placement addresses come back through
// the out-parameters.
unsigned Dyninst::buildRelocatedEHFrame(Symtab *symObj,
                                        std::list<Relocation::CodeTracker *> &relocatedCode,
                                        const EHFrameArch &arch,
                                        Address regionBase,
                                        std::vector<unsigned char> &ehOut,
                                        std::vector<unsigned char> &exOut,
                                        Address &ehVaddrOut,
                                        Address &exVaddrOut,
                                        Address ownLo, Address ownHi) {
  typedef std::list<Relocation::CodeTracker *> CodeTrackers;

  // ---- byte emitters (buffer-parameterized) ----
  typedef std::vector<unsigned char> Bytes;
  auto pU8  = [](Bytes&b, unsigned char v){ b.push_back(v); };
  auto pU32 = [](Bytes&b, uint32_t v){ for(int i=0;i<4;i++) b.push_back((unsigned char)((v>>(8*i))&0xff)); };
  auto pS32 = [&](Bytes&b, int32_t v){ pU32(b,(uint32_t)v); };
  auto pUleb= [](Bytes&b, uint64_t v){ do{ unsigned char c=v&0x7f; v>>=7; if(v)c|=0x80; b.push_back(c);}while(v); };
  auto pSleb= [](Bytes&b, int64_t v){ bool more=true; while(more){ unsigned char c=v&0x7f; v>>=7;
                 if((v==0&&!(c&0x40))||(v==-1&&(c&0x40))) more=false; else c|=0x80; b.push_back(c);} };
  auto dwreg= [&](Dyninst::MachRegister r)->unsigned {
                 return arch.dwarfForFrameOrStack(r); };
  auto rdSleb=[](const Bytes&b, size_t p, int64_t*out)->size_t {
                 int64_t r=0; int sh=0; size_t n=0; unsigned char c;
                 do{ if(p+n>=b.size()){*out=0;return n?n:1;} c=b[p+n]; r|=(int64_t)(c&0x7f)<<sh; sh+=7; n++; }while(c&0x80);
                 if(sh<64 && (c&0x40)) r|=-((int64_t)1<<sh);
                 *out=r; return n; };

  // ---- Pass 1: gather trackers -> per-func original trackers, global
  //      orig->reloc map, and the overall relocated span. ----
  struct OrigTracker { Address relocAddr, origAddr; unsigned size; };
  struct TrackerMapEnt { Address origAddr, relocAddr; unsigned size; bool isOrig; };
  struct FuncRec { void* fp; Address relocStart, relocEnd, origLo, origHi;
                   std::vector<OrigTracker> origTrackers; std::vector<EHFunctionInfo*> eh;
                   // origRuns: maximal runs of this function's ORIGINAL bytes not
                   //   interrupted by another function's code -- the units to
                   //   sample CFA/reg rules over. A hot/cold-split function has
                   //   two; sampling the whole [origLo,origHi] hull instead would
                   //   leak a bracketed foreign function's entry rule.
                   // relocRuns: maximal runs of this function's RELOCATED code --
                   //   one FDE per run, so no FDE over-claims an interleaved
                   //   neighbor.
                   std::vector<std::pair<Address,Address>> origRuns, relocRuns; };
  std::map<void*, FuncRec> byFunc;
  std::vector<TrackerMapEnt> origRelocMap;
  std::vector<std::pair<std::pair<Address,Address>,void*>> allOrig, allReloc;
  Address relocHi = 0;
  for (CodeTrackers::iterator ci = relocatedCode.begin(); ci != relocatedCode.end(); ++ci) {
    for (Relocation::CodeTracker::TrackerList::const_iterator ti = (*ci)->trackers().begin();
         ti != (*ci)->trackers().end(); ++ti) {
      Relocation::TrackerElement *te = *ti;
      Address relocAddr = te->reloc(); unsigned size = te->size();
      if (!relocAddr || !size) continue;
      if (relocAddr + size > relocHi) relocHi = relocAddr + size;
      bool isOrig = (te->type() == Relocation::TrackerElement::original);
      origRelocMap.push_back({ te->orig(), relocAddr, size, isOrig });
      if (te->func())
        allReloc.push_back({ { relocAddr, relocAddr+size }, (void*)te->func() });   // ALL trackers: reloc extent
      if (isOrig && te->func()) {
        void* fp = (void*)te->func();
        FuncRec &rec = byFunc[fp];
        if (rec.origTrackers.empty()) { rec.fp=fp; rec.relocStart=relocAddr; rec.origLo=te->orig(); rec.origHi=te->orig()+size; }
        if (relocAddr < rec.relocStart) rec.relocStart = relocAddr;
        if (te->orig() < rec.origLo) rec.origLo = te->orig();
        if (te->orig()+size > rec.origHi) rec.origHi = te->orig()+size;
        rec.origTrackers.push_back({ relocAddr, te->orig(), size });
        allOrig.push_back({ { te->orig(), te->orig()+size }, fp });    // original-only: orig extent
      }
    }
  }
  std::sort(origRelocMap.begin(), origRelocMap.end(), [](const TrackerMapEnt&a,const TrackerMapEnt&b){
    if (a.origAddr != b.origAddr) return a.origAddr < b.origAddr;
    // A single original instruction can be tracked twice at one origAddr: the
    // verbatim copy (isOrig) and an inserted instrumentation snippet emulated
    // alongside it. origToReloc resolves the LAST entry with origAddr <= a, so
    // the verbatim tracker must sort last -- it yields the precise per-instruction
    // reloc PC (relocAddr + (a-origAddr)). Resolving to the snippet returns its
    // start for every interior PC, collapsing a function's CFA-rule PCs onto one
    // address; the FDE then loses its advance_loc boundaries and flattens to its
    // last def_cfa rule. For a stack-realigning function that drops the rbp-based
    // rule, and the unwinder reads a garbage return address off the realigned
    // rsp -> std::terminate.
    return a.isOrig < b.isOrig;
  });
  // origRelocMap is the ground truth for every address this generator emits;
  // dump it under DYNINST_DEBUG_EH to diagnose a mis-mapping.
  for (auto &e : origRelocMap)
    eh_printf("OMAP os=0x%lx rs=0x%lx sz=%u orig=%d\n",
              (unsigned long)e.origAddr, (unsigned long)e.relocAddr, e.size, (int)e.isOrig);
  // Group globally-ordered trackers into per-function runs, breaking a run
  // wherever another function's code falls between two of this function's
  // trackers. allOrig->origRuns (sampling units); allReloc->relocRuns (FDE units).
  auto buildRuns = [](std::vector<std::pair<std::pair<Address,Address>,void*>> &all,
                      std::map<void*,FuncRec> &bf,
                      std::vector<std::pair<Address,Address>> FuncRec::*dst) {
    std::sort(all.begin(), all.end(),
              [](const std::pair<std::pair<Address,Address>,void*>&a,
                 const std::pair<std::pair<Address,Address>,void*>&b){ return a.first < b.first; });
    for (size_t i=0;i<all.size();) {
      void* fp = all[i].second;
      Address lo = all[i].first.first, hiR = all[i].first.second;
      size_t j=i+1;
      for (; j<all.size() && all[j].second==fp; ++j) hiR = std::max(hiR, all[j].first.second);
      (bf[fp].*dst).push_back({ lo, hiR });
      i=j;
    }
  };
  buildRuns(allOrig, byFunc, &FuncRec::origRuns);
  buildRuns(allReloc, byFunc, &FuncRec::relocRuns);
  // A function's relocated footprint can BEGIN with an emulated tracker -- e.g. a
  // rewritten call at the very start of a hot/cold .cold fragment -- which sits
  // below its first verbatim instruction. relocRuns (built from ALL trackers)
  // carry the true low bound; without lowering relocStart to it, that leading
  // call falls below relocStart, its call-site region is dropped as unmappable,
  // and a throw through it escapes to std::terminate. Only ever lower the bound.
  for (auto &kv : byFunc)
    if (!kv.second.relocRuns.empty() &&
        kv.second.relocRuns.front().first < kv.second.relocStart)
      kv.second.relocStart = kv.second.relocRuns.front().first;
  // orig -> reloc (exact within a verbatim tracker; start-of-tracker otherwise).
  auto origToReloc = [&](Address a)->Address {
    if (origRelocMap.empty()) return 0;
    size_t loIdx=0, hiIdx=origRelocMap.size(); // first ent with origAddr > a
    while (loIdx<hiIdx){ size_t m=(loIdx+hiIdx)/2; if(origRelocMap[m].origAddr<=a) loIdx=m+1; else hiIdx=m; }
    if (loIdx==0) return 0;
    const TrackerMapEnt&e = origRelocMap[loIdx-1];
    if (e.isOrig && a < e.origAddr + e.size) return e.relocAddr + (a - e.origAddr);
    return e.relocAddr;
  };
  // Map the one-past-end of an original range to the one-past-end of its
  // relocated code via the range's LAST byte. (Mapping the end ADDRESS through
  // origToReloc is wrong twice over: that address belongs to the NEXT block, and
  // an emulated tracker's interior bytes all fall back to the tracker start,
  // collapsing the range.)
  auto origToRelocEnd = [&](Address end)->Address {
    if (origRelocMap.empty() || end == 0) return 0;
    Address a = end - 1;
    size_t loIdx=0, hiIdx=origRelocMap.size();
    while (loIdx<hiIdx){ size_t m=(loIdx+hiIdx)/2; if(origRelocMap[m].origAddr<=a) loIdx=m+1; else hiIdx=m; }
    if (loIdx==0) return 0;
    // One original instruction can expand into several relocated trackers at this
    // origAddr (an inserted snippet PLUS the copied instruction), disjoint in
    // reloc space and emitted in either order -- the one that sorts last may have
    // the SMALLER reloc end. Take the MAXIMUM over every tracker at this origAddr;
    // otherwise a call whose snippet sorts last is left just OUTSIDE its call-site
    // region, the throw PC falls into a call-site-table gap, and the personality
    // treats it as a nothrow region and calls std::terminate.
    Address origAddr = origRelocMap[loIdx-1].origAddr, best = 0;
    for (size_t k = loIdx; k-- > 0 && origRelocMap[k].origAddr == origAddr; ) {
      const TrackerMapEnt &e = origRelocMap[k];
      Address r = (e.isOrig && a < e.origAddr + e.size) ? e.relocAddr + (a - e.origAddr) + 1
                                                        : e.relocAddr + e.size;
      if (r > best) best = r;
    }
    return best;
  };

  // sorted function list; relocEnd = next func's start (contiguous, disjoint FDEs).
  std::vector<FuncRec*> funcs;
  for (auto &kv : byFunc) {
    if (kv.second.origTrackers.empty()) continue;   // skip pseudo-entries (emulated-only, from allReloc)
    // Restrict to the instrumented application's own code. Dynamic
    // instrumentation can relocate foreign helpers (e.g. libc fork/vfork pulled
    // in by the iRPC/runtime machinery) into relocatedCode_; we hold no source
    // CFI for them, so any FDE we synthesized would carry only CIE default rules
    // -- a fabricated descriptor that corrupts the whole __register_frame batch
    // and defeats the application's own correct FDEs.
    if (ownHi > ownLo &&
        (kv.second.origLo < ownLo || kv.second.origLo >= ownHi)) {
      eh_printf("EH: skipping foreign relocated func orig[0x%lx,0x%lx] "
                "(outside app range [0x%lx,0x%lx))\n",
                (unsigned long)kv.second.origLo, (unsigned long)kv.second.origHi,
                (unsigned long)ownLo, (unsigned long)ownHi);
      continue;
    }
    funcs.push_back(&kv.second);
  }
  std::sort(funcs.begin(), funcs.end(),
            [](const FuncRec*a,const FuncRec*b){ return a->relocStart < b->relocStart; });
  for (size_t i=0;i<funcs.size();i++)
    funcs[i]->relocEnd = (i+1<funcs.size()) ? funcs[i+1]->relocStart : relocHi;

  // ---- Extract + assign original LSDA info to relocated functions. ----
  std::vector<EHFunctionInfo> ehInfos;
  symObj->getEHFrameInfo(ehInfos);
  // Type-table entry encoding, from the first LSDA that has a type table (PIE:
  // indirect|pcrel|sdata4 = 0x9b). Personality is looked up per-function at
  // CIE-emit time (funcPers), not tracked globally.
  unsigned char ttype_format = 0x9b;
  bool ttypeSet = false;
  for (auto &info : ehInfos) {
    // Attach each original LSDA to the relocated function whose ORIGINAL code it
    // describes, scored by OVERLAP against that function's origRuns rather than
    // by simple "func_low_pc in [origLo,origHi)" containment. A hot/cold-split
    // function has two FDEs (hot body + .cold fragment) that both must land on
    // the one relocated FuncRec that merged them, and containment fails two ways:
    //   1. a .cold FDE's func_low_pc can sit a few bytes BELOW origLo (the first
    //      byte dyninst actually relocated), so containment drops the fragment --
    //      and with it the throw's call-site region -> a terminate trap.
    //   2. a split function's [origLo,origHi) hull SUBSUMES an interleaved
    //      neighbor (cold blocks pool in .text.unlikely), so containment is
    //      ambiguous between functions.
    // origRuns exclude the interleaved foreign code; score with the FDE's full
    // extent (func_low_pc .. end-of-last-call-site), not just its entry pc.
    Address fdeLo = info.func_low_pc, fdeHi = info.func_low_pc + 1;
    for (auto &cs : info.callsites) {
      Address e = cs.region_start + (cs.region_size ? cs.region_size : 1);
      if (cs.region_start < fdeLo) fdeLo = cs.region_start;
      if (e > fdeHi) fdeHi = e;
    }
    FuncRec* best = nullptr; Address bestOv = 0;
    for (auto f : funcs) {
      Address ov = 0;
      for (auto &r : f->origRuns) {
        Address lo = std::max(fdeLo, r.first), hiO = std::min(fdeHi, r.second);
        if (hiO > lo) ov += hiO - lo;
      }
      if (ov > bestOv) { bestOv = ov; best = f; }
    }
    if (best) {
      best->eh.push_back(&info);
      eh_printf("LSDA: assign FDE[0x%lx..0x%lx] -> func reloc 0x%lx "
                "(origRun overlap=%lu, %zu callsites)\n",
                (unsigned long)fdeLo, (unsigned long)fdeHi,
                (unsigned long)best->relocStart, (unsigned long)bestOv,
                info.callsites.size());
      if (!ttypeSet && info.ttype_format != 0xff) {
        ttype_format = info.ttype_format;
        ttypeSet = true;
      }
    } else {
      eh_printf("LSDA: FDE[0x%lx..0x%lx] matched no relocated function "
                "(dropped)\n", (unsigned long)fdeLo, (unsigned long)fdeHi);
    }
  }
  // Byte width of a DW_EH_PE_* value, and whether it is pcrel. PIE uses
  // pcrel/indirect encodings, non-PIE absolute (udata4); honor whichever the
  // original .eh_frame used or the personality reads a bogus pointer.
  auto encSize = [](unsigned char enc)->int {
    switch (enc & 0x0f) {
      case 0x02: case 0x0a: return 2;   // [s]data2
      case 0x03: case 0x0b: return 4;   // [s]data4
      case 0x04: case 0x0c: return 8;   // [s]data8
      case 0x00: return 8;              // absptr (x86-64)
      default: return 4;
    }
  };
  auto isPcrel = [](unsigned char enc)->bool { return (enc & 0x70) == 0x10; };

  // ---- Pass 2: decode each LSDA-bearing function's call sites, action chain,
  //      and type table into a per-function LsdaData. Not emitted here: pass 3
  //      emits a SEPARATE LSDA per relocated run (per FDE), since call-site and
  //      landing-pad offsets are relative to each FDE's own start (per the ELF
  //      LSDA format), so one function-wide table can't be shared across a split
  //      function's multiple FDEs. ----
  Address exVaddr = (regionBase + 0xfffUL) & ~((Address)0xfffUL);
  Bytes ex;
  // Call-site record in ABSOLUTE relocated addresses (made FDE-relative at emit
  // time in pass 3). filters: original action-chain values (>0 catch, 0 cleanup,
  // <0 spec).
  struct CallSiteRec { Address rstart, rend, lp; std::vector<int64_t> filters; };
  struct LsdaData { std::vector<CallSiteRec> callSites; std::vector<unsigned> csAction;
                    Bytes action; std::map<unsigned,Address> filterType;
                    unsigned maxFilter=0; };
  std::map<FuncRec*, LsdaData> lsdaData;

  // The synthesized tables can be delivered far from the mutatee's typeinfo /
  // personality GOT slots -- a live-process inferiorMalloc region may land >2GB
  // away, overflowing a 4-byte pcrel offset (wrong type pointer -> the catch
  // type-match fails -> the exception escapes). Widen the cross-reference
  // encodings (personality, TType) from pcrel|sdata4 (the usual PIE encoding) to
  // pcrel|sdata8: still position-independent, unlimited range, and libgcc reads
  // the widened encoding byte we emit. References to our own adjacent regions
  // (the FDE initial_location, the LSDA pointer) stay sdata4.
  auto widenPcrel = [&](unsigned char enc)->unsigned char {
    return (isPcrel(enc) && (enc & 0x0f) == 0x0b) ? (unsigned char)((enc & 0xf0) | 0x0c) : enc;
  };
  ttype_format = widenPcrel(ttype_format);

  const int ttypeEntrySize = encSize(ttype_format);   // type-table entry width
  for (auto f : funcs) {
    if (f->eh.empty()) continue;
    // Preserve the ORIGINAL filter numbering: relocated landing pads are copied
    // verbatim and still switch on the original selector constants the
    // personality passes them, so a regenerated `filter K` must resolve to the
    // same typeinfo the original `type_targets[K-1]` did. filterType maps a
    // positive filter to its typeinfo (0 == catch-all sentinel); maxFilter sizes
    // the type table.
    std::map<unsigned, Address> filterType;
    unsigned maxFilter = 0;
    std::vector<CallSiteRec> callSites;
    for (auto info : f->eh) {
      for (auto &cs : info->callsites) {
        CallSiteRec site; site.rstart=origToReloc(cs.region_start);
        site.rend = cs.region_size
                 ? origToRelocEnd(cs.region_start + cs.region_size)
                 : site.rstart;
        site.lp = cs.landing_pad ? origToReloc(cs.landing_pad) : 0;
        if (cs.action != 0) {           // walk the original action chain
          uint64_t a=cs.action; int guard=0;
          while (a && guard++<64) {
            size_t p=a-1; if (p>=info->action_table.size()) break;
            int64_t filter; size_t fs=rdSleb(info->action_table,p,&filter);
            int64_t next;   size_t np=p+fs; rdSleb(info->action_table,np,&next);
            // Keep every record (0 cleanup / >0 catch / <0 spec) so the chain --
            // e.g. a cleanup ahead of a catch -- stays intact.
            site.filters.push_back(filter);
            if (filter > 0) {
              Address tgt = ((size_t)filter <= info->type_targets.size())
                            ? info->type_targets[filter-1] : 0;
              filterType[(unsigned)filter] = tgt;   // 0 => catch-all
              if ((unsigned)filter > maxFilter) maxFilter = (unsigned)filter;
            } else if (filter < 0) {
              eh_printf("LSDA: exception-spec filter %ld not reconstructed "
                        "(func reloc 0x%lx)\n", (long)filter,
                        (unsigned long)f->relocStart);
            }
            if (next==0) break;
            long npos=(long)np+next; if (npos<0) break; a=(uint64_t)npos+1;
          }
        }
        // Keep zero-landing-pad rows: a PC MISSING from the call-site table
        // means "call std::terminate" to the personality, while a row with lp=0
        // means "unwind through, nothing to run here". Dropping them turns every
        // may-throw region without a handler -- e.g. the __cxa_throw call in a
        // function whose LSDA has only cleanups -- into a terminate trap.
        //
        // But only rows that MAP into this function's relocated span may be
        // emitted: origToReloc returns 0 / a foreign block for regions whose
        // code was not relocated with this function, and an out-of-range start
        // underflows the FDE-relative uleb, corrupting the table.
        if (!site.rstart || site.rstart < f->relocStart || site.rstart >= f->relocEnd) {
          if (site.lp || !site.filters.empty())
            eh_printf("LSDA: dropping unmappable call-site region 0x%lx+%lu (lp=0x%lx)\n",
                      (unsigned long)cs.region_start, (unsigned long)cs.region_size,
                      (unsigned long)site.lp);
          continue;
        }
        if (site.rend <= site.rstart) site.rend = site.rstart + 1;
        if (site.rend > f->relocEnd) site.rend = f->relocEnd;
        if (site.lp && (site.lp < f->relocStart || site.lp >= f->relocEnd)) {
          eh_printf("LSDA: landing pad 0x%lx maps outside function, clearing\n",
                    (unsigned long)site.lp);
          site.lp = 0; site.filters.clear();
        }
        callSites.push_back(site);
      }
    }
    if (callSites.empty()) { f->eh.clear(); continue; }  // treat as non-LSDA
    // The personality scans the call-site table linearly and stops at the first
    // entry starting past the PC, so entries must be sorted by relocated start
    // (block reordering can permute the originals).
    std::sort(callSites.begin(), callSites.end(),
              [](const CallSiteRec&a, const CallSiteRec&b){ return a.rstart < b.rstart; });
    // action table + per-call-site action index (shared across this function's
    // per-run LSDAs; filter values index the type table).
    LsdaData &lsda = lsdaData[f];
    lsda.filterType = std::move(filterType);
    lsda.maxFilter = maxFilter;
    lsda.csAction.assign(callSites.size(), 0);
    for (size_t i=0;i<callSites.size();i++) {
      if (callSites[i].filters.empty()) { lsda.csAction[i]=0; continue; }  // cleanup: action 0
      lsda.csAction[i] = (unsigned)(lsda.action.size()+1);
      for (size_t k=0;k<callSites[i].filters.size();k++) {
        pSleb(lsda.action,(int64_t)callSites[i].filters[k]);
        pSleb(lsda.action, (k+1<callSites[i].filters.size()) ? 1 : 0);
      }
    }
    lsda.callSites = std::move(callSites);
  }

  // ---- Pass 3a: emit ONE LSDA PER RELOCATED RUN into the .gcc_except_table
  //      blob, and record per-run descriptors. A function split across
  //      non-contiguous relocated runs gets one FDE per run, and each FDE needs
  //      its OWN LSDA whose call-site/landing-pad offsets are relative to that
  //      run's start (= the FDE's initial_location, which is what the omitted
  //      LPStart defaults to). Sharing one function-wide table across the runs
  //      mis-resolves every offset in the runs that don't start at relocStart. ----
  auto emitRunLSDA = [&](Address runLo, Address runHi, LsdaData &lsda) -> size_t {
    Bytes csrec;
    for (size_t i=0;i<lsda.callSites.size();i++) {
      CallSiteRec &site = lsda.callSites[i];
      if (site.rstart < runLo || site.rstart >= runHi) continue;   // not in this run
      Address rend = (site.rend > site.rstart) ? site.rend : site.rstart+1;
      if (rend > runHi) rend = runHi;
      // Landing-pad offset is relative to runLo (the FDE start); a pad in ANOTHER
      // run at a LOWER address can't be encoded as an unsigned uleb, so drop the
      // action there (unwind continues) rather than emit a bogus pad. In-run and
      // forward pads encode fine.
      unsigned act = lsda.csAction[i];
      Address lpoff = 0;
      if (site.lp) { if (site.lp >= runLo) lpoff = site.lp - runLo; else act = 0; }
      pUleb(csrec, site.rstart - runLo);
      pUleb(csrec, rend - site.rstart);
      pUleb(csrec, lpoff);
      pUleb(csrec, act);
    }
    Bytes body; pU8(body,0x01); pUleb(body,csrec.size());   // 0x01 = call-site table enc DW_EH_PE_uleb128
    body.insert(body.end(), csrec.begin(), csrec.end());
    body.insert(body.end(), lsda.action.begin(), lsda.action.end());
    size_t nTypeSlots = lsda.maxFilter;                     // type table has slots 1..nTypeSlots
    size_t bdSize = body.size() + nTypeSlots*ttypeEntrySize;  // == TType base offset
    size_t off = ex.size();
    pU8(ex, 0xff);                            // LPStart: omit -> FDE initial_location (=runLo)
    pU8(ex, nTypeSlots ? ttype_format : 0xff);// TType encoding
    if (nTypeSlots) pUleb(ex, bdSize);        // TType base offset
    ex.insert(ex.end(), body.begin(), body.end());
    for (long fk=(long)nTypeSlots; fk>=1; fk--) {  // type table: slots nTypeSlots..1 (downward)
      auto it = lsda.filterType.find((unsigned)fk);
      Address tgt = (it != lsda.filterType.end()) ? it->second : 0;  // absent/catch-all => 0
      size_t eoff = ex.size();
      // Emit exactly ttypeEntrySize bytes little-endian (the width the TType
      // encoding declares) -- pcrel offset, absolute target, or zero for a
      // catch-all. Must match the encoding width for the same reason the
      // personality pointer must (ppc64le = 8 bytes).
      int64_t v = (tgt == 0) ? 0
                  : (isPcrel(ttype_format) ? ((int64_t)tgt - (int64_t)(exVaddr + eoff))
                                           : (int64_t)tgt);
      for (int b=0;b<ttypeEntrySize;b++) ex.push_back((unsigned char)((v>>(8*b))&0xff));
    }
    return off;
  };
  struct RunDesc { Address lo, hi; size_t lsda; };   // lsda==(size_t)-1: CFA-only run
  std::map<FuncRec*, std::vector<RunDesc>> runDescs;
  for (auto f : funcs) {
    auto ldIt = lsdaData.find(f);
    std::vector<RunDesc> descs;
    for (auto &rrun : f->relocRuns) {
      RunDesc d{ rrun.first, rrun.second, (size_t)-1 };
      if (ldIt != lsdaData.end()) {
        bool has=false;
        for (auto &site : ldIt->second.callSites)
          if (site.rstart >= rrun.first && site.rstart < rrun.second) { has=true; break; }
        if (has) d.lsda = emitRunLSDA(rrun.first, rrun.second, ldIt->second);
      }
      descs.push_back(d);
    }
    runDescs[f] = std::move(descs);
  }

  // ---- Pass 3: build .eh_frame (CIEs + FDEs, one per relocated run). ----
  Address ehVaddr = exVaddr + ((ex.size()+0xfffUL) & ~((Address)0xfffUL));
  if (ex.empty()) ehVaddr = exVaddr;   // no LSDAs -> reuse the page
  Bytes eh;
  // Per-arch CFI parameters (eh_frame_arch.h); the rule sampling (libdw) and the
  // .gcc_except_table are arch-neutral. calleeSavedRegs holds the DWARF register
  // NUMBERS to sample/restore, sampled by number (getFrameRegRulesByDwarf) so a
  // column with no Dyninst MachRegister -- ppc64 LR = 65 -- is reachable. On
  // link-register arches (aarch64, ppc64le) the return-address register is itself
  // in this set and sampled per-PC; x86-64 keeps the RA on the stack at cfa-8 via
  // a constant rule baked into cieInitialCFI and does not sample it.
  const unsigned raReg = arch.returnAddrReg;
  const Bytes& cieInitCFI = arch.cieInitialCFI;
  const std::vector<unsigned>& calleeSavedRegs = arch.calleeSavedDwarfRegs;
  // CIE #0: "zR" (FDE ptr enc pcrel|sdata8), for functions without an LSDA.
  size_t cieZR = eh.size();
  { size_t lp=eh.size(); pU32(eh,0); size_t b=eh.size();
    pU32(eh,0); pU8(eh,1); pU8(eh,0x7a); pU8(eh,0x52); pU8(eh,0);   // "zR"
    pUleb(eh,1); pU8(eh,0x78); pUleb(eh,raReg);                     // caf=1 daf=-8 ra=<arch>
    pUleb(eh,1); pU8(eh,0x1c);                                     // aug data: R=pcrel|sdata8 (wide: reloc code may be far)
    eh.insert(eh.end(), cieInitCFI.begin(), cieInitCFI.end());     // arch initial CFI
    while ((eh.size()-b)&3) eh.push_back(0);
    uint32_t l=(uint32_t)(eh.size()-b); memcpy(&eh[lp],&l,4); }
  // One "zPLR" CIE (personality + LSDA) per DISTINCT PERSONALITY. A mixed C/C++
  // binary references both __gxx_personality_v0 (C++ -- does catch/type matching)
  // and __gcc_personality_v0 (C -- cleanup only, never matches a catch). A single
  // global personality unwinds every C++ frame under whichever comes first; under
  // the C personality a C++ catch silently never fires and the exception escapes
  // to std::terminate. Emit a CIE per (personality target, encoding) actually
  // used, and point each function's FDE at the CIE matching ITS OWN personality.
  auto emitZPLR = [&](Address ptarget, unsigned char pformat) -> size_t {
    pformat = widenPcrel(pformat);   // avoid 4-byte pcrel overflow to a far personality
    size_t cie = eh.size();
    size_t lp=eh.size(); pU32(eh,0); size_t b=eh.size();
    pU32(eh,0); pU8(eh,1);
    pU8(eh,0x7a); pU8(eh,0x50); pU8(eh,0x4c); pU8(eh,0x52); pU8(eh,0); // "zPLR"
    pUleb(eh,1); pU8(eh,0x78); pUleb(eh,raReg);                     // caf=1 daf=-8 (0x78=sleb -8) ra=<arch>
    // aug data = [P_enc][P_val][L_enc][R_enc]; P_val width follows P_enc, and the
    // original personality encoding (pcrel PIE / absolute non-PIE) is preserved.
    int psz = encSize(pformat);
    pUleb(eh, (uint64_t)(1 + psz + 1 + 1));
    pU8(eh, pformat);                                // P enc (as in original)
    { size_t pf=eh.size();
      // Emit exactly encSize(pformat) bytes little-endian: pcrel offset or
      // absolute target. Width MUST match the encoding -- ppc64le udata8 (8),
      // x86-64/aarch64 sdata4 (4) -- or the aug data is short and libgcc's
      // read_encoded_value_with_base crashes on the personality pointer.
      int64_t v = isPcrel(pformat)
                  ? ((int64_t)ptarget - (int64_t)(ehVaddr+pf))
                  : (int64_t)ptarget;
      for (int b2=0;b2<psz;b2++) eh.push_back((unsigned char)((v>>(8*b2))&0xff)); }
    pU8(eh, 0x1b);                                   // L enc: pcrel|sdata4 (LSDA is in our adjacent region)
    pU8(eh, 0x1c);                                   // R enc: pcrel|sdata8 (wide: reloc code may be far)
    eh.insert(eh.end(), cieInitCFI.begin(), cieInitCFI.end());     // arch initial CFI
    while ((eh.size()-b)&3) eh.push_back(0);
    uint32_t l=(uint32_t)(eh.size()-b); memcpy(&eh[lp],&l,4);
    return cie;
  };
  // A function's personality: the first of its original FDEs that carried one
  // (a function's hot and .cold fragments share it).
  auto funcPers = [](FuncRec *f, Address &pt, unsigned char &pf)->bool {
    for (auto info : f->eh)
      if (info->personality_target) {
        pt = info->personality_target; pf = info->personality_format; return true;
      }
    return false;
  };
  std::map<std::pair<Address,unsigned>, size_t> cieForPers;
  if (!ex.empty()) {
    for (auto f : funcs) {
      if (lsdaData.find(f) == lsdaData.end()) continue;
      Address pt=0; unsigned char pf=0;
      if (!funcPers(f, pt, pf)) continue;
      auto key = std::make_pair(pt,(unsigned)pf);
      if (cieForPers.find(key)==cieForPers.end())
        cieForPers[key] = emitZPLR(pt, pf);
    }
  }
  unsigned nfde=0, nlsda=0;
  // A sampled CFA-or-register rule at a relocated PC. regColumn -1 marks a CFA
  // rule; otherwise it is an index into calleeSavedRegs.
  struct RuleEvent { Address rpc; int regColumn; int kind; unsigned reg; int64_t val; };
  for (auto f : funcs) {
    // Sample the CFA and callee-saved rules per ORIGINAL run of THIS function
    // only (never across the foreign functions between a hot/cold split, which
    // would leak a foreign entry rule). +1 catches a transition row that begins
    // exactly at a run boundary. Rules map via origToReloc to this function's own
    // relocated runs; the per-run FDE filter below selects each run's rules.
    std::vector<RuleEvent> ruleEvents;
    for (auto &orun : f->origRuns) {
      std::vector<VariableLocation> cfa;
      if (symObj->getCFALocations(orun.first, orun.second+1, cfa))
        for (auto &L : cfa) {
          Address r = origToReloc(L.lowPC);
          if (!r) continue;
          ruleEvents.push_back({ r, -1, 0, dwreg(L.mr_reg), (int64_t)L.frameOffset });
        }
      for (size_t k = 0; k < calleeSavedRegs.size(); ++k) {
        std::vector<Symtab::FrameRegRule> rr;
        if (!symObj->getFrameRegRulesByDwarf(orun.first, orun.second+1, calleeSavedRegs[k], rr)) continue;
        for (auto &R : rr) {
          Address r = origToReloc(R.lowPC);
          if (!r) continue;
          int64_t v = (R.kind == Symtab::FrameRegRule::InRegister)
                      ? (int64_t)R.regnum : (int64_t)R.offset;
          ruleEvents.push_back({ r, (int)k, (int)R.kind, calleeSavedRegs[k], v });
        }
      }
    }
    std::sort(ruleEvents.begin(), ruleEvents.end(), [](const RuleEvent&a,const RuleEvent&b){
        return a.rpc < b.rpc || (a.rpc == b.rpc && a.regColumn < b.regColumn); });

    // One FDE per relocated run, bounded to the run (never next-func-start, which
    // over-claims an interleaved neighbor). A run carrying an LSDA (lsda != -1)
    // uses a "zPLR" CIE, else the CFA-only "zR" CIE.
    auto emitFDE = [&](Address pStart, Address pEnd, size_t cie, size_t lsda) {
      size_t fdeLenPos=eh.size(); pU32(eh,0);
      size_t fdeBody=eh.size();
      pU32(eh,(uint32_t)(fdeBody - cie));                       // CIE pointer
      { int64_t v=(int64_t)pStart-(int64_t)(ehVaddr+eh.size());           // initial_location (sdata8: reloc code may be far)
        for(int i=0;i<8;i++) eh.push_back((unsigned char)((v>>(8*i))&0xff)); }
      { uint64_t r=(uint64_t)(pEnd - pStart);                            // address_range (8 bytes, matching R enc width)
        for(int i=0;i<8;i++) eh.push_back((unsigned char)((r>>(8*i))&0xff)); }
      if (lsda != (size_t)-1) { pUleb(eh,4);
        size_t lf=eh.size();
        pS32(eh,(int32_t)((int64_t)(exVaddr+lsda) - (int64_t)(ehVaddr+lf))); // LSDA ptr
        nlsda++;
      } else pUleb(eh,0);
      Address cur=pStart;
      unsigned lastCfaReg=0xffffffffu; int64_t lastCfaOff=0x7fffffffLL;
      std::vector<int> lastKind(calleeSavedRegs.size(), (int)Symtab::FrameRegRule::SameValue);
      std::vector<int64_t> lastVal(calleeSavedRegs.size(), 0);
      auto advanceTo = [&](Address at){
        if (at <= cur) return;
        Address d=at-cur;
        // DW_CFA_advance_loc (0x40|delta) / advance_loc1 (0x02) / advance_loc4 (0x04)
        if (d<0x40) eh.push_back((unsigned char)(0x40|d));
        else if (d<0x100){ eh.push_back(0x02); eh.push_back((unsigned char)d); }
        else { eh.push_back(0x04); pU32(eh,(uint32_t)d); }
        cur=at;
      };
      for (auto &e : ruleEvents) {
        if (e.rpc < pStart || e.rpc >= pEnd) continue;   // only this run's rules
        Address at = e.rpc;
        if (e.regColumn < 0) {                    // CFA rule
          if (e.reg==lastCfaReg && e.val==lastCfaOff) continue;
          advanceTo(at);
          pU8(eh,0x0c); pUleb(eh,e.reg); pUleb(eh,(uint64_t)e.val);  // DW_CFA_def_cfa
          lastCfaReg=e.reg; lastCfaOff=e.val;
        } else {                                  // callee-saved register rule
          int k = e.regColumn;
          if (e.kind==lastKind[k] && e.val==lastVal[k]) continue;
          if (e.kind == (int)Symtab::FrameRegRule::AtCFAOffset &&
              (e.val % 8) != 0) continue;                 // not factorable with daf=-8
          advanceTo(at);
          switch (e.kind) {
            case (int)Symtab::FrameRegRule::AtCFAOffset: {
              int64_t foff = e.val / -8;                             // factored (daf=-8)
              if (e.reg < 64 && foff >= 0) {
                pU8(eh,(unsigned char)(0x80 | e.reg));               // DW_CFA_offset (reg<64, >=0)
                pUleb(eh,(uint64_t)foff);
              } else {
                // reg>=64 (ppc64 LR=65) or a positive CFA offset (ppc64 LR at
                // cfa+16) needs the extended, signed-factored form.
                pU8(eh,0x11); pUleb(eh,e.reg); pSleb(eh,foff);       // DW_CFA_offset_extended_sf
              }
              break;
            }
            case (int)Symtab::FrameRegRule::InRegister:
              pU8(eh,0x09); pUleb(eh,e.reg); pUleb(eh,(uint64_t)e.val); // DW_CFA_register
              break;
            case (int)Symtab::FrameRegRule::Undefined:
              pU8(eh,0x07); pUleb(eh,e.reg);                         // DW_CFA_undefined
              break;
            default:
              pU8(eh,0x08); pUleb(eh,e.reg);                         // DW_CFA_same_value
              break;
          }
          lastKind[k]=e.kind; lastVal[k]=e.val;
        }
      }
      while ((eh.size()-fdeBody)&3) eh.push_back(0);
      uint32_t l=(uint32_t)(eh.size()-fdeBody); memcpy(&eh[fdeLenPos],&l,4);
      nfde++;
    };
    Address fpt=0; unsigned char fpf=0;
    size_t fcie = (size_t)-1;
    if (funcPers(f, fpt, fpf)) {
      auto it = cieForPers.find(std::make_pair(fpt,(unsigned)fpf));
      if (it != cieForPers.end()) fcie = it->second;
    }
    for (auto &d : runDescs[f]) {
      bool useLSDA = (d.lsda != (size_t)-1) && fcie != (size_t)-1;
      eh_printf("FDE origFunc[0x%lx,0x%lx] relocStart=0x%lx -> reloc[0x%lx,0x%lx) lsda=%s\n",
                (unsigned long)f->origLo, (unsigned long)f->origHi,
                (unsigned long)f->relocStart, (unsigned long)d.lo, (unsigned long)d.hi,
                useLSDA ? "y" : "-");
      emitFDE(d.lo, d.hi, useLSDA ? fcie : cieZR, useLSDA ? d.lsda : (size_t)-1);
    }
  }
  pU32(eh,0);   // terminator

  eh_printf(".eh_frame: %u FDEs (%u w/LSDA), %zu bytes @0x%lx; "
            ".gcc_except_table %zu bytes @0x%lx\n",
            nfde, nlsda, eh.size(), (unsigned long)ehVaddr, ex.size(), (unsigned long)exVaddr);

  // Hand the blobs and their chosen placement back to the caller, which decides
  // how to deliver them: an output-file region + DT_DYNINST_EH_FRAME tag for a
  // rewriter (synthesizeRelocatedEHFrame, below), or an in-process region +
  // __register_frame iRPC for a live process (PCProcess).
  exVaddrOut = exVaddr;
  ehVaddrOut = ehVaddr;
  exOut.swap(ex);
  ehOut.swap(eh);
  return nfde;
}

// Rewriter delivery: place the synthesized blobs as loadable regions in the
// output object above `regionHighWaterMark`, and point a DT_DYNINST_EH_FRAME
// dynamic tag at the .eh_frame so the runtime registers it at load (RTlinux.c).
// The tag reuses the DT_DYNINST channel to avoid regenerating the object's
// dynamic metadata.
void Dyninst::synthesizeRelocatedEHFrame(Symtab *symObj,
                                         std::list<Relocation::CodeTracker *> &relocatedCode,
                                         const EHFrameArch &arch,
                                         Address regionHighWaterMark,
                                         Address ownLo, Address ownHi) {
  std::vector<unsigned char> eh, ex;
  Address ehVaddr = 0, exVaddr = 0;
  unsigned nfde = buildRelocatedEHFrame(symObj, relocatedCode, arch,
                                        regionHighWaterMark, eh, ex, ehVaddr, exVaddr,
                                        ownLo, ownHi);
  if (!nfde) return;
  if (!ex.empty()) {
    void *exCopy = malloc(ex.size()); memcpy(exCopy, ex.data(), ex.size());
    symObj->addRegion(exVaddr, exCopy, ex.size(), ".dyninst_gcc_except",
                      Region::RT_DATA, true);
  }
  void *ehCopy = malloc(eh.size()); memcpy(ehCopy, eh.data(), eh.size());
  symObj->addRegion(ehVaddr, ehCopy, eh.size(), ".dyninst_ehframe",
                    Region::RT_DATA, true);
  symObj->addSysVDynamic(DT_DYNINST_EH_FRAME, ehVaddr);
}

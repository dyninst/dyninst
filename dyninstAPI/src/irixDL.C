/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

// $Id: irixDL.C,v 1.27 2005/07/29 19:18:50 bernat Exp $

#include <stdio.h>
#include <sys/ucontext.h>             // gregset_t
#include <dlfcn.h>                    // dlopen() modes
#include <string.h>                   // strlen(), strcmp()
#include <sys/procfs.h>
#include <libelf.h>                   // ElfXX_Sym
#include <objlist.h>                  // ElfXX_Obj_Info
#include "common/h/Types.h"             // Address
#include "common/h/Dictionary.h"
#include "dyninstAPI/src/Object.h"            // ELF parsing
#include "dyninstAPI/src/irixDL.h"  // irix-specific definitions
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/arch.h"      // instruction
#include "dyninstAPI/src/ast.h"       // AstNode
#include "dyninstAPI/src/inst-mips.h" // deadList, readAddressInMemory()
#include "dyninstAPI/src/dynamiclinking.h" // dynamiclinking definition


extern void print_proc_flags(int fd);
extern void print_proc_regs(int fd);
extern void cleanUpAndExit(int);

/*
static void print_mmaps(process *p)
{  
  // print memory mappings
  int proc_fd = p->getProcFileDescriptor();
  int nmaps;
  ioctl(proc_fd, PIOCNMAP, &nmaps);
  prmap_t *maps = new prmap_t[nmaps+1];
  ioctl(proc_fd, PIOCMAP, maps);
  fprintf(stderr, "  memory mappings:\n");
  for (int i = 0; i < nmaps; i++) {
    prmap_t *map = &maps[i];
    fprintf(stderr, "    vaddr(0x%p), size(0x%p), offset(0x%p), flags(%#lx)\n",
	    (void *)map->pr_vaddr, (void *)map->pr_size, (void *)map->pr_off, 
	    map->pr_mflags);
  }
  delete [] maps;
}
*/

sharedLibHook::sharedLibHook(process *p, sharedLibHookType t, Address b) 
        : proc_(p), type_(t), breakAddr_(b) {


    // Before putting in the breakpoint, save what is currently at the
    // location that will be overwritten.
    proc_->readDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE,
                         (void *)saved_, true);

    instruction trap_insn;
    genTrap(&trap_insn);
    proc_->writeDataSpace((caddr_t)breakAddr_,
                         instruction::size(),&trap_insn.raw);
    
}

sharedLibHook::~sharedLibHook() {
    proc_->writeDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE, saved_);
}

pdElfObjInfo::pdElfObjInfo(process *p, Address addr, bool is_elf64)
  : pd_self(addr)
{
  assert(p);
  bool ret;
  void *pathname;
  int pathname_size;
  
  if (is_elf64) { // 64-bit ELF
    
    // read rld structure
    Elf64_Obj_Info obj;
    ret = p->readDataSpace((void *)addr, sizeof(obj), &obj, true);
    assert(ret);
    // populate object members
    pd_next = (Address)obj.oi_next;
    pd_prev = (Address)obj.oi_prev;
    pd_ehdr = (Address)obj.oi_ehdr;
    pd_orig_ehdr = (Address)obj.oi_orig_ehdr;
    // pathname info
    pathname = (void *)obj.oi_pathname;
    pathname_size = obj.oi_pathname_len + 1;
    
  } else { // 32-bit ELF
    
    // read rld structure
    Elf32_Obj_Info obj;
    ret = p->readDataSpace((void *)addr, sizeof(obj), &obj, true);
    assert(ret);
    // populate object members
    pd_next = (Address)obj.oi_next;
    pd_prev = (Address)obj.oi_prev;
    pd_ehdr = (Address)obj.oi_ehdr;
    pd_orig_ehdr = (Address)obj.oi_orig_ehdr;
    // pathname info
    pathname = (void *)obj.oi_pathname;
    pathname_size = obj.oi_pathname_len + 1;
    
  }
  
  // read pathname string
  char *name_buf = new char[pathname_size];
  assert(pathname);
  ret = p->readDataSpace(pathname, pathname_size, name_buf, true);
  assert(ret);
  pd_pathname = name_buf;
  delete [] name_buf;
}

bool pdElfObjInfo::operator==(const pdElfObjInfo &t) const
{
  return ((pd_ehdr == t.pd_ehdr) &&
	  (pd_orig_ehdr == t.pd_orig_ehdr) &&
	  (pd_pathname == t.pd_pathname));
}

bool pdElfObjInfo::operator!=(const pdElfObjInfo &t) const
{
  return (!((*this) == t));
}

static bool is_a_out(process *p, const pdstring &dso_name)
{
  if (dso_name == p->getImage()->file()) return true;
  if (dso_name == p->getImage()->name()) return true;
  return false;
}

static bool is_libc(const pdstring &dso_name)
{
  if (strstr(dso_name.c_str(), "libc.")) return true;
  return false;
}

pdvector<pdElfObjInfo *>dynamic_linking::getRldMap()
{
  //fprintf(stderr, ">>> dynamic_linking::getRldMap()\n");
  pdvector<pdElfObjInfo *> ret;
  
  // ELF class: 32/64-bit
  bool is_elf64 = proc->getImage()->getObject().is_elf64();

  // get address of head of rld map
  Address rld_obj_addr = lookup_fn(proc, "__rld_obj_head");
  assert(rld_obj_addr);
  Address rld_obj_head = readAddressInMemory(proc, rld_obj_addr, is_elf64);
  assert(rld_obj_head);

  // iterate through rld map nodes
  pdElfObjInfo *rld_obj = NULL;
  for (Address ptr = rld_obj_head; 
       ptr != 0; 
       ptr = rld_obj->pd_next)
  {
      rld_obj = new pdElfObjInfo(proc, ptr, is_elf64); // wrapper object
      ret.push_back(rld_obj);
  }
  return ret;
}

bool dynamic_linking::initialize() {

    libc_obj = NULL;
    
    // set dynamic linking flags
    proc->setDynamicLinking();
    dynlinked = true;

    return true;
}


bool dynamic_linking::installTracing()
{
  // local copy of runtime linker map
  
  if (!libc_obj) {
      pdvector<pdElfObjInfo *> rlds = getRldMap();
      // populate retval with objects already loaded
      for (unsigned i = 0; i < rlds.size(); i++) {
          pdElfObjInfo *obj = rlds[i];
          pdstring dso_name = obj->pd_pathname;
          // libc info
          if (is_libc(dso_name)) {
              libc_obj = obj;
          }
          else {
              delete obj;
          }
      }
  }
  
    //fprintf(stderr, ">>> dynamic_linking::setMappingHooks()\n");
    Address base = libc_obj->pd_ehdr;
    Address base_orig = libc_obj->pd_orig_ehdr;

    // get file descriptor for libc
    int proc_fd = proc->getRepresentativeLWP()->get_fd();
    
    caddr_t base_proc = (caddr_t)base;
    int libc_fd = ioctl(proc_fd, PIOCOPENM, &base_proc);
    if (libc_fd == -1) {
        perror("handleIfLibc(PIOCOPENM)");
        assert(0);
    }
    
    // ELF class: 32/64-bit
    bool is_elf64 = proc->getImage()->getObject().is_elf64();
    
    // find ".dynsym" and ".dynstr" sections
    Elf *elfp = elf_begin(libc_fd, ELF_C_READ, 0);
    assert(elfp);
    Elf_Scn *strscnp = 0;
    Elf_Scn *symscnp = 0;
    const char *shnames = pdelf_get_shnames(elfp, is_elf64);
    Elf_Scn *scnp = 0;
    while ((scnp = elf_nextscn(elfp, scnp))) {
        pdElfShdr pd_shdr(scnp, is_elf64); // wrapper object
        assert(pd_shdr.err == false);
        
        const char *shname = &shnames[pd_shdr.pd_name];
        if (strcmp(shname, ".dynstr") == 0) strscnp = scnp;
        if (strcmp(shname, ".dynsym") == 0) symscnp = scnp;
        if (symscnp && strscnp) break;
    }
    assert(strscnp);
    assert(symscnp);
    
    // ELF symbol names
    Elf_Data *strdatap = elf_getdata(strscnp, 0);
    assert(strdatap);
    const char *symnames = (const char *)strdatap->d_buf;
    // ELF symbols
    Elf_Data *symdatap = elf_getdata(symscnp, 0);
    assert(symdatap);
    
    // find mapping symbols
    pdElfSymVector pd_syms(symdatap, is_elf64);
    unsigned nsyms = pd_syms.size();
    for (unsigned i = 0; i < nsyms; i++) {
        pdElfSym &pd_sym = pd_syms[i];
        if (pd_sym.pd_shndx == SHN_UNDEF) continue;
        if (pd_sym.pd_type != STT_FUNC) continue;
        const char *name = &symnames[pd_sym.pd_name];
        Address rt_addr = pd_sym.pd_value - base_orig + base;
        
        // used symbols: dlopen, ___tp_dlinsert_post, ___tp_dlremove_post
        // unused: __tp_dlinsert_pre, __tp_dlinsert_version_pre
        // unused: __tp_dlremove_pre
        if (strcmp(name, "dlopen") == 0) {
            dlopen_addr = rt_addr;
        } else if (strcmp(name, "___tp_dlinsert_post") == 0) {
            sharedLibHooks_.push_back(new sharedLibHook(proc, SLH_INSERT_POST, rt_addr));
        } else if (strcmp(name, "___tp_dlremove_post") == 0) {
            sharedLibHooks_.push_back(new sharedLibHook(proc, SLH_REMOVE_POST, rt_addr));
        }
    }
    
    // TODO: should "libc_fd" be close()ed?
    elf_end(elfp); // cleanup

    return true;
}

/* getSharedObjects(): return a list of currently mapped objects and
   establish hooks for future (un)mapping events */
// dlopen() events trigger addASharedObject()
// dlclose() events trigger removeASharedObject()
pdvector<shared_object *> *dynamic_linking::getSharedObjects()
{
    
  //fprintf(stderr, ">>> getSharedObjects()\n");
  pdvector<shared_object *> *ret = new pdvector<shared_object *>;

  // local copy of runtime linker map
  rld_map = getRldMap();

  // populate retval with objects already loaded
  for (unsigned i = 0; i < rld_map.size(); i++) {
      pdElfObjInfo *obj = rld_map[i];
      pdstring dso_name = obj->pd_pathname;

      // skip a.out
      if (is_a_out(proc, dso_name)) {
          assert(i == 0); // a.out should be first
          continue;
      }
      
      // add new shared object
      Address dso_base = obj->pd_ehdr;
      (*ret).push_back(new shared_object(dso_name, dso_base, false, true, true, 0, proc));
  }

  return ret;
}

bool dynamic_linking::handleIfDueToSharedObjectMapping(pdvector<shared_object *> **changed_objs,
				u_int &change_type, 
				bool &error)
{
  //fprintf(stderr, ">>> handleIfDueToSharedObjectMapping()\n");
  bool ret = false;
  error = false;

  // read registers
  int proc_fd = proc->getRepresentativeLWP()->get_fd();
  assert(proc_fd);
  gregset_t regs;
  if (ioctl(proc_fd, PIOCGREG, &regs) == -1) {
      perror("handleIfDueToSharedObjectMapping(PIOCGREG)");
      assert(errno != EBUSY); // procfs thinks the process is active
      error = true;
      return false;
  }
  Address pc = regs[PROC_REG_PC];
  
  // match $pc against DSO event addresses
  sharedLibHook *hook = reachedLibHook(pc);
  if (hook || force_library_load) {
      change_type = SHAREDOBJECT_NOCHANGE; // default retval

      // current snapshot of rld map
      pdvector<pdElfObjInfo *> new_map = getRldMap();
      unsigned old_size = rld_map.size();
      unsigned new_size = new_map.size();

      int event;
      if (hook)
          event = hook->eventType();
      else // forced, assume add
          event = SLH_INSERT_POST;
      
      switch (event) {
          
    case SLH_INSERT_POST:
        // dlopen(): check for inserted objects
        assert(new_size >= old_size);
        if (new_size > old_size) {
            (*changed_objs) = new pdvector<shared_object *>;
            change_type = SHAREDOBJECT_ADDED;
            
            // add inserted objects
            // (assumes new objects inserted at end of list)
            for (unsigned i = old_size; i < new_size; i++) {
                pdElfObjInfo *obj = new_map[i];
                Address base = obj->pd_ehdr;
                pdstring name = obj->pd_pathname;
                (**changed_objs).push_back(new shared_object(name, base, false, true, true, 0, proc));
            }
        }      
        ret = true;
        break;
        
    case SLH_REMOVE_POST:
        // dlclose(): check for deleted objects      
        assert(old_size >= new_size);
        if (new_size < old_size) {
            (*changed_objs) = new pdvector<shared_object *>;
            change_type = SHAREDOBJECT_REMOVED;
            
            // construct list of deleted objects
            // (assumes no new objects inserted)
            pdvector<unsigned> deleted;
            unsigned j = 0; // index for new rld map
            for (unsigned i = 0; i < old_size; i++) {
                if (j == new_size || ((*rld_map[i]) != (*new_map[j]))) 
                {
                    deleted.push_back(i);
                } else {
                    j++;
                }
            }
            assert(j == new_size);
            
            // identify deleted shared_object's
            const pdvector<shared_object *> &sobjs = *proc->sharedObjects();
            for (unsigned i = 0; i < deleted.size(); i++) {
                pdElfObjInfo *eobj = rld_map[deleted[i]];
                // lookup shared_object by base address
                shared_object *sobj = NULL;
                for (unsigned k = 0; k < sobjs.size(); k++) {
                    if (sobjs[k]->getBaseAddress() == eobj->pd_ehdr) {
                        sobj = sobjs[k];
                        break;
                    }
                }
                assert(sobj);
                (**changed_objs).push_back(sobj);
            }
        }
        ret = true;
        break;
        
    default:
        error = true;
        ret = true;
        break;
      }
      
      // update local rld map
      for (unsigned i = 0; i < rld_map.size(); i++) {
          delete rld_map[i];
      }
      rld_map = new_map;
      
      if (!force_library_load) {
          Address ra = regs[PROC_REG_RA];
          if (!(proc->getRepresentativeLWP()->changePC(ra, NULL))) {
              error = true;
              return true;
          }
      }
      
  }
  
  return ret;
}

bool process::loadDYNINSTlibCleanup()
{
    dyninstlib_brk_addr = 0x0;
    
  // restore code and registers
  Address code = lookup_fn(this, "_start");
  assert(code);
  writeDataSpace((void *)code, sizeof(savedCodeBuffer), savedCodeBuffer);
  assert(savedRegs != NULL);
  getRepresentativeLWP()->restoreRegisters(*savedRegs);

  delete savedRegs;
  savedRegs = NULL;
  return true;
  
}


/*
 * Copyright (c) 1998 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: irixDL.C,v 1.10 2000/07/28 17:21:15 pcroth Exp $

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
#include "dyninstAPI/src/irixDL.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/arch.h"      // instruction
#include "dyninstAPI/src/ast.h"       // AstNode
#include "dyninstAPI/src/inst-mips.h" // deadList, readAddressInMemory()


extern Address pcFromProc(int proc_fd);
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

pdDsoEvent::pdDsoEvent(process *p, pdDsoEventType t, Address b)
: type(t), brk(b), proc(p)
{ 
  // save original instructions to buffer
  p->readDataSpace((void *)brk, 2*INSN_SIZE, buf, true);
  // set trap at breakpoint address
  instruction trap;
  genTrap(&trap);
  p->writeDataSpace((void *)brk, INSN_SIZE, &trap);
}

pdDsoEvent::~pdDsoEvent()
{
  //fprintf(stderr, ">>> pdDsoEvent::~pdDsoEvent()\n");
  // extract event trap
  proc->writeDataSpace((void *)brk, INSN_SIZE, buf);
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

dynamic_linking::dynamic_linking() 
  : dynlinked(false), dlopen_addr(0), r_brk_addr(0)
{
  //fprintf(stderr, ">>> dynamic_linking::dynamic_linking()\n");
}

bool dynamic_linking::isDynamic() {
  //if (dynlinked) fprintf(stderr, ">>> dynamic_linking::isDynamic()\n");
  return dynlinked;
}

Address dynamic_linking::get_r_brk_addr() const {
  //fprintf(stderr, ">>> dynamic_linking::get_r_brk_addr()\n");
  return r_brk_addr;
}

Address dynamic_linking::get_dlopen_addr() const {
  //fprintf(stderr, ">>> dynamic_linking::get_dlopen_addr()\n");
  return dlopen_addr;
}


bool process::trapDueToDyninstLib()
{
  bool ret = (pcFromProc(proc_fd) == dyninstlib_brk_addr);
  //if (ret) fprintf(stderr, ">>> process::trapDueToDyninstLib()\n");
  return ret;
}

Address process::get_dlopen_addr() const
{
  //fprintf(stderr, ">>> process::get_dlopen_addr()\n");
  return (dyn) ? (dyn->get_dlopen_addr()) : (0);
}

bool process::trapAtEntryPointOfMain()
{
  bool ret = (pcFromProc(proc_fd) == main_brk_addr);
  //if (ret) fprintf(stderr, ">>> process::trapAtEntryPointOfMain(true)\n");
  return ret;
}

/* insertTrapAtEntryPointOfMain(): For some Fortran apps, main() is
   defined in a shared library.  If main() cannot be found, then we
   check if the executable image contained a call to main().  This is
   usually inside __start(). */
void process::insertTrapAtEntryPointOfMain()
{
  // insert trap near "main"
  main_brk_addr = lookup_fn(this, "main");
  if (main_brk_addr == 0) {
    main_brk_addr = getImage()->get_main_call_addr();
  }
  assert(main_brk_addr);

  // save original instruction
  readDataSpace((void *)main_brk_addr, INSN_SIZE, savedCodeBuffer, true);

  // insert trap instruction
  instruction trapInsn;
  genTrap(&trapInsn);
  writeDataSpace((void *)main_brk_addr, INSN_SIZE, &trapInsn);
}

void process::handleTrapAtEntryPointOfMain()
{
  // restore original instruction to entry point of main()
  writeDataSpace((void *)main_brk_addr, INSN_SIZE, savedCodeBuffer);
}

static bool is_a_out(process *p, const string &dso_name)
{
  if (dso_name == p->getImage()->file()) return true;
  if (dso_name == p->getImage()->name()) return true;
  if (dso_name == p->getArgv0()) return true;
  return false;
}

static bool is_libc(const string &dso_name)
{
  if (strstr(dso_name.string_of(), "libc.")) return true;
  return false;
}

vector<pdElfObjInfo *>dynamic_linking::getRldMap(process *p)
{
  //fprintf(stderr, ">>> dynamic_linking::getRldMap()\n");
  vector<pdElfObjInfo *> ret;
  
  // ELF class: 32/64-bit
  bool is_elf64 = p->getImage()->getObject().is_elf64();

  // get address of head of rld map
  Address rld_obj_addr = lookup_fn(p, "__rld_obj_head");
  assert(rld_obj_addr);
  Address rld_obj_head = readAddressInMemory(p, rld_obj_addr, is_elf64);
  assert(rld_obj_head);

  // iterate through rld map nodes
  pdElfObjInfo *rld_obj = NULL;
  for (Address ptr = rld_obj_head; 
       ptr != 0; 
       ptr = rld_obj->pd_next)
  {
    rld_obj = new pdElfObjInfo(p, ptr, is_elf64); // wrapper object
    ret += rld_obj;
  }

  return ret;
}

bool dynamic_linking::setMappingHooks(process *p, pdElfObjInfo *libc_obj)
{
  //fprintf(stderr, ">>> dynamic_linking::setMappingHooks()\n");
  Address base = libc_obj->pd_ehdr;
  Address base_orig = libc_obj->pd_orig_ehdr;

  // get file descriptor for libc
  int proc_fd = p->getProcFileDescriptor();
  caddr_t base_proc = (caddr_t)base;
  int libc_fd = ioctl(proc_fd, PIOCOPENM, &base_proc);
  if (libc_fd == -1) {
    perror("handleIfLibc(PIOCOPENM)");
    assert(0);
  }

  // ELF class: 32/64-bit
  bool is_elf64 = p->getImage()->getObject().is_elf64();

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
      dso_events += new pdDsoEvent(p, DSO_INSERT_POST, rt_addr);
    } else if (strcmp(name, "___tp_dlremove_post") == 0) {
      dso_events += new pdDsoEvent(p, DSO_REMOVE_POST, rt_addr);
    }
  }

  // TODO: should "libc_fd" be close()ed?
  elf_end(elfp); // cleanup

  // save address of "_rld_text_resolve"
  r_brk_addr = p->getImage()->getObject().get_rbrk_addr();

  return true;
}

void dynamic_linking::unsetMappingHooks(process *)
{
  //fprintf(stderr, ">>> dynamic_linking::unsetMappingHooks()\n");
  for (unsigned i = 0; i < dso_events.size(); i++) {
    delete dso_events[i];
  }
}

/* getSharedObjects(): return a list of currently mapped objects and
   establish hooks for future (un)mapping events */
// dlopen() events trigger addASharedObject()
// dlclose() events trigger removeASharedObject()
vector<shared_object *> *dynamic_linking::getSharedObjects(process *p)
{
  //fprintf(stderr, ">>> getSharedObjects()\n");
  vector<shared_object *> *ret = new vector<shared_object *>;

  // local copy of runtime linker map
  rld_map = getRldMap(p);

  // populate retval with objects already loaded
  pdElfObjInfo *libc_obj = NULL;
  for (unsigned i = 0; i < rld_map.size(); i++) {
    pdElfObjInfo *obj = rld_map[i];
    string dso_name = obj->pd_pathname;
    
    // libc info
    if (is_libc(dso_name)) {
      assert(libc_obj == NULL); // there can be only one
      libc_obj = obj;
    }

    // skip a.out
    if (is_a_out(p, dso_name)) {
      assert(i == 0); // a.out should be first
      continue;
    }
    
    // add new shared object
    Address dso_base = obj->pd_ehdr;
    *ret += new shared_object(dso_name, dso_base, false, true, true, 0);
  }
  assert(libc_obj);

  // set hooks for future shared object events
  setMappingHooks(p, libc_obj);

  // set dynamic linking flags
  p->setDynamicLinking();
  dynlinked = true;

  return ret;
}

bool process::dlopenDYNINSTlib()
{
  //fprintf(stderr, ">>> dlopenDYNINSTlib()\n");

  // find runtime library
  char *rtlib_var;
  char *rtlib_prefix;
#ifdef BPATCH_LIBRARY
  rtlib_var = "DYNINSTAPI_RT_LIB";
  rtlib_prefix = "libdyninstAPI_RT";
#else
  rtlib_var = "PARADYN_LIB";
  rtlib_prefix = "libdyninstRT";
#endif

  if (!dyninstName.length()) {
    dyninstName = getenv(rtlib_var);
    if (!dyninstName.length()) {
      string msg = string("Environment variable ") + string(rtlib_var)
                 + string(" has not been defined for process ") + string(pid);
      showErrorCallback(101, msg);
      return false;
    }
  }

  const char *rtlib_val = dyninstName.string_of();
  assert(strstr(rtlib_val, rtlib_prefix));

  // for 32-bit apps, modify the rtlib environment variable
  char *rtlib_mod = "_n32";
  if (!getImage()->getObject().is_elf64() &&
      !strstr(rtlib_val, rtlib_mod)) 
  {
    char *rtlib_suffix = ".so.1";
    // truncate suffix
    // NOTE: this modifies the actual environment string
    // (which we are about to replace, so it's OK)
    char *ptr_suffix = strstr(rtlib_val, rtlib_suffix);
    assert(ptr_suffix);
    *ptr_suffix = 0;
    // construct environment variable
    char buf[512];
    sprintf(buf, "%s=%s%s%s", rtlib_var, rtlib_val, rtlib_mod, rtlib_suffix);
    dyninstName = string(rtlib_val)+string(rtlib_mod)+string(rtlib_suffix);
    // allocate environment buffer
    char *env = (char *)malloc(strlen(buf)+1);
    assert(env);
    strcpy(env, buf);
    // insert environment name/value pair
    int ret = putenv(env);
    assert(ret == 0);
  }

  
  // use "_start" as scratch buffer to invoke dlopen() on DYNINST
  Address baseAddr = lookup_fn(this, "_start");
  char buf_[BYTES_TO_SAVE], *buf = buf_;
  Address bufSize = 0;
  
  // step 0: illegal instruction (code)
  genIll((instruction *)(buf + bufSize));
  bufSize += INSN_SIZE;
  
  // step 1: DYNINST library string (data)
  //Address libStart = bufSize; // debug
  Address libAddr = baseAddr + bufSize;
  char *libPath = getenv(rtlib_var); // see above
  if (access(libPath, R_OK)) {
       string msg = string("Runtime library ") + string(libPath) + 
                    string(" does not exist or cannot be accessed");
       showErrorCallback(101, msg);
       return false;
  }
  int libSize = strlen(libPath) + 1;
  strcpy(buf + bufSize, libPath);
  bufSize += libSize;
  // pad to aligned instruction boundary
  if (!isAligned(baseAddr + bufSize)) {
    int npad = INSN_SIZE - ((baseAddr + bufSize) % INSN_SIZE);
    for (int i = 0; i < npad; i++)
      buf[bufSize + i] = 0;
    bufSize += npad;
    assert(isAligned(baseAddr + bufSize));
  }

  // step 2: inferior dlopen() call (code)
  Address codeAddr = baseAddr + bufSize;
  registerSpace *regs = new registerSpace(nDead, Dead, 0, (Register *)0);
  vector<AstNode*> args(2);
  int dlopen_mode = RTLD_NOW | RTLD_GLOBAL;
  AstNode *call;
  string callee = "dlopen";
  // inferior dlopen(): build AstNodes
  args[0] = new AstNode(AstNode::Constant, (void *)libAddr);
  args[1] = new AstNode(AstNode::Constant, (void *)dlopen_mode);
  call = new AstNode(callee, args);
  removeAst(args[0]);
  removeAst(args[1]);
  // inferior dlopen(): generate code
  regs->resetSpace();
  //Address codeStart = bufSize; // debug
  call->generateCode(this, regs, buf, bufSize, true, true);
  removeAst(call);
  
  // step 3: trap instruction (code)
  Address trapAddr = baseAddr + bufSize;
  genTrap((instruction *)(buf + bufSize));
  bufSize += INSN_SIZE;
  //int trapEnd = bufSize; // debug

  // debug
  /*
  fprintf(stderr, "inferior dlopen code: (%i bytes)\n", bufSize);
  dis(buf, baseAddr);
  fprintf(stderr, "%0#10x      \"%s\"\n", baseAddr + libStart, buf + libStart);
  for (int i = codeStart; i < bufSize; i += INSN_SIZE) {
    dis(buf + i, baseAddr + i);
  }
  */
  
  // save registers and "_start" code
  readDataSpace((void *)baseAddr, BYTES_TO_SAVE, savedCodeBuffer, true);
  savedRegs = getRegisters();
  assert(savedRegs);

  // write inferior dlopen code and set PC
  assert(bufSize <= BYTES_TO_SAVE);
  //fprintf(stderr, "writing %i bytes to <0x%08x:_start>, $pc = 0x%08x\n",
  //bufSize, baseAddr, codeAddr);
  //fprintf(stderr, ">>> dlopenDYNINSTlib <0x%08x(_start): %i insns>\n",
  //baseAddr, bufSize/INSN_SIZE);
  writeDataSpace((void *)baseAddr, bufSize, (void *)buf);
  bool ret = changePC(codeAddr, savedRegs);
  assert(ret);

  // debug
  /*
  fprintf(stderr, "inferior dlopen code: (%i bytes)\n", bufSize - codeStart);
  disDataSpace(this, (void *)(baseAddr + codeStart), 
	       (bufSize - codeStart) / INSN_SIZE, "  ");
  */

  dyninstlib_brk_addr = trapAddr;
  isLoadingDyninstLib = true;
  return true;
}

char *dso2str(pdDsoEventType t) 
{
  switch (t) {
  case DSO_INSERT_PRE: 
    return "___tp_dlinsert_pre";
  case DSO_INSERT_VERSION_PRE: 
    return "___tp_dlinsert_version_pre";
  case DSO_INSERT_POST: 
    return "___tp_dlinsert_post";
  case DSO_REMOVE_PRE: 
    return "___tp_dlremove_pre";
  case DSO_REMOVE_POST: 
    return "___tp_dlremove_post";
  case DSO_UNKNOWN:
    return "<unknown>";
  }
  return "<unknown>";
}

bool dynamic_linking::handleIfDueToSharedObjectMapping(process *p, 
				vector<shared_object *> **changed_objs,
				u_int &change_type, 
				bool &error)
{
  //fprintf(stderr, ">>> handleIfDueToSharedObjectMapping()\n");
  bool ret = false;
  error = false;

  // read registers
  int proc_fd = p->getProcFileDescriptor();
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
  for (unsigned i = 0; i < dso_events.size() && !ret; i++) {
    pdDsoEvent *event = dso_events[i];
    if (pc != event->brk) continue;
    change_type = SHAREDOBJECT_NOCHANGE; // default retval
        
    // verify stomped insn "jr ra"
    instruction &brk_insn = event->buf[0];
    assert(isInsnType(brk_insn, JRmask, JRmatch));
    assert(brk_insn.rtype.rs == REG_RA);
    instruction &delay_insn = event->buf[1];
    assert(delay_insn.raw == NOP_INSN);
    // emulate stomped insn "jr ra"
    Address ra = regs[PROC_REG_RA];
    if (!(p->changePC(ra))) {
      error = true;
      return true;
    }
    
    // current snapshot of rld map
    vector<pdElfObjInfo *> new_map = getRldMap(p);
    unsigned old_size = rld_map.size();
    unsigned new_size = new_map.size();

    switch (event->type) {

    case DSO_INSERT_POST:
      // dlopen(): check for inserted objects
      assert(new_size >= old_size);
      if (new_size > old_size) {
	(*changed_objs) = new vector<shared_object *>;
	change_type = SHAREDOBJECT_ADDED;
	
	// add inserted objects
	// (assumes new objects inserted at end of list)
	for (unsigned i = old_size; i < new_size; i++) {
	  pdElfObjInfo *obj = new_map[i];
	  Address base = obj->pd_ehdr;
	  string name = obj->pd_pathname;
	  (**changed_objs) += new shared_object(name, base, false, true, true, 0);
	}
      }      
      ret = true;
      break;

    case DSO_REMOVE_POST:
      // dlclose(): check for deleted objects      
      assert(old_size >= new_size);
      if (new_size < old_size) {
	(*changed_objs) = new vector<shared_object *>;
	change_type = SHAREDOBJECT_REMOVED;
	
	// construct list of deleted objects
	// (assumes no new objects inserted)
	vector<unsigned> deleted;
	unsigned j = 0; // index for new rld map
	for (unsigned i = 0; i < old_size; i++) {
	  if (j == new_size || ((*rld_map[i]) != (*new_map[j]))) 
	  {
	    deleted += i;
	  } else {
	    j++;
	  }
	}
	assert(j == new_size);
	
	// identify deleted shared_object's
	const vector<shared_object *> &sobjs = *p->sharedObjects();
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
	  (**changed_objs) += sobj;
	}
      }
      ret = true;
      break;

    default:
      fprintf(stderr, "bogus object event \"%s\"\n", dso2str(event->type));
      error = true;
      ret = true;
      break;
    }
    
    // update local rld map
    for (unsigned i = 0; i < rld_map.size(); i++) {
      delete rld_map[i];
    }
    rld_map = new_map;
  }

  return ret;
}

void process::handleIfDueToDyninstLib()
{
  //fprintf(stderr, ">>> handleIfDueToDyninstLib()\n");

  // restore code and registers
  Address code = lookup_fn(this, "_start");
  assert(code);
  writeDataSpace((void *)code, sizeof(savedCodeBuffer), savedCodeBuffer);
  restoreRegisters(savedRegs);

  delete [] savedRegs;
  savedRegs = NULL;
}


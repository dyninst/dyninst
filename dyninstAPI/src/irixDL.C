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

// $Id: irixDL.C,v 1.4 1999/07/13 04:28:19 csserra Exp $

#include <stdio.h>
#include <sys/ucontext.h> // gregset_t
#include "util/h/Types.h" // Address
#include "util/h/Dictionary.h"
#include "util/h/Object.h" // ELF parsing
#include "dyninstAPI/src/irixDL.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/arch.h" // instruction
#include "dyninstAPI/src/ast.h"  // AstNode
#include "dyninstAPI/src/inst-mips.h" // deadList
#include <objlist.h> // Elf{32,64}_Obj_Info
#include <dlfcn.h> // dlopen() modes
#include <string.h> // strlen(), strcmp()
#include <sys/procfs.h>
#include <libelf.h>


extern Address pcFromProc(int proc_fd); // csserra
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

static bool isExecutable(process *p, const string &dso_name)
{
  if (dso_name == p->getImage()->file()) return true;
  if (dso_name == p->getImage()->name()) return true;
  if (dso_name == p->getArgv0()) return true;
  return false;
}

static bool handleIfLibc(process *p, 
			 const string &dso_name, Address dso_base,
			 int &libc_fd, Address &libc_base)
{
  if (libc_fd == -1 && strstr(dso_name.string_of(), "libc.")) {
    int proc_fd = p->getProcFileDescriptor();
    assert(proc_fd);
    libc_base = dso_base;
    caddr_t libc_base_proc = (caddr_t)dso_base;
    if ((libc_fd = ioctl(proc_fd, PIOCOPENM, &libc_base_proc)) == -1) {
      perror("handleIfLibc(PIOCOPENM)");
      return false;
    }
  }
  return true;
}
			 
vector<shared_object *> *
dynamic_linking::getRldObjects(process *p, int &libc_fd, Address &libc_base)
{
  //fprintf(stderr, ">>> dynamic_linking::getRldObjects()\n");
  vector<shared_object *> *ret = new vector<shared_object *>;
  
  // find rld map
  Address rld_obj_addr = lookup_fn(p, "__rld_obj_head");
  assert(rld_obj_addr);

  // ELF class: 32/64-bit
  bool is_elf64 = p->getImage()->getObject().is_elf64();
  if (is_elf64) {

    // read address of (head of) rld map
    Elf64_Obj_Info *rld_obj_head;
    // address-in-memory
    p->readDataSpace((void *)rld_obj_addr, sizeof(Elf64_Obj_Info *), 
		     &rld_obj_head, true);
    // iterate through Elf64_Obj_Info list
    Elf64_Obj_Info obj, *next;
    for (next = rld_obj_head; 
	 next != NULL; 
	 next = (Elf64_Obj_Info *)obj.oi_next) 
    {
      // read rld object
      p->readDataSpace(next, sizeof(Elf64_Obj_Info), &obj, true);
      assert(obj.oi_magic == 0xffffffff); // TODO
      
      // DSO base address
      Address dso_base = obj.oi_ehdr;
      // DSO name
      int name_len = obj.oi_pathname_len + 1;
      char *name_buf = new char[name_len];
      p->readDataSpace((char *)obj.oi_pathname, name_len, name_buf, true);
      string dso_name = name_buf;
      delete [] name_buf;

      // skip a.out
      if (isExecutable(p, dso_name)) continue;
      
      // check for libc
      if (!handleIfLibc(p, dso_name, dso_base, libc_fd, libc_base)) {
	for (unsigned i = 0; i < ret->size(); i++) 
	  delete (*ret)[i];
	delete ret;
	return NULL;
      }
      
      // add new shared object
      *ret += new shared_object(dso_name, dso_base, false, true, true, 0);
    }

  } else { // 32-bit ELF
    
    // read address of (head of) rld map
    Elf32_Obj_Info *rld_obj_head;
    // address-in-memory
    if (sizeof(void *) == sizeof(uint64_t)) {
      // 32-bit application, 64-bit paradynd
      rld_obj_head = NULL;
      char *local_addr = ((char *)&rld_obj_head) + 4;
      p->readDataSpace((void *)rld_obj_addr, sizeof(uint32_t), 
		       local_addr, true);
    } else { 
      p->readDataSpace((void *)rld_obj_addr, sizeof(Elf32_Obj_Info *), 
		       &rld_obj_head, true);
    }

    // iterate through Elf32_Obj_Info list
    Elf32_Obj_Info obj, *next;
    for (next = rld_obj_head; 
	 next != NULL; 
	 next = (Elf32_Obj_Info *)obj.oi_next) 
    {
      // read rld object
      p->readDataSpace(next, sizeof(Elf32_Obj_Info), &obj, true);
      assert(obj.oi_magic == 0xffffffff);
      
      // DSO base address
      Address dso_base = obj.oi_ehdr;
      // DSO name
      int name_len = obj.oi_pathname_len + 1;
      char *name_buf = new char[name_len];
      p->readDataSpace((char *)obj.oi_pathname, name_len, name_buf, true);
      string dso_name = name_buf;
      delete [] name_buf;
      if (isExecutable(p, dso_name)) continue; // skip a.out
      
      // check for libc
      if (!handleIfLibc(p, dso_name, dso_base, libc_fd, libc_base)) {
	for (unsigned i = 0; i < ret->size(); i++) 
	  delete (*ret)[i];
	delete ret;
	return NULL;
      }
      
      // add new shared object
      *ret += new shared_object(dso_name, dso_base, false, true, true, 0);
    }

  }

  //if (is_elf64) fprintf(stderr, ">>> 64-bit getRldObjects() successful\n");
  return ret;
}


void dynamic_linking::getObjectPath(process *p, Address ret_base, string &ret)
{
  //fprintf(stderr, ">>> dynamic_linking::getObjectPath(0x%08x)\n", base);

  // find head of rld map
  Address rld_obj_addr = lookup_fn(p, "__rld_obj_head");
  assert(rld_obj_addr);

  // ELF class: 32/64-bit
  bool is_elf64 = p->getImage()->getObject().is_elf64();
  if (is_elf64) {

    // read rld map elements
    Elf64_Obj_Info *rld_obj_head;
    // address-in-memory
    p->readDataSpace((void *)rld_obj_addr, sizeof(Elf64_Obj_Info *), 
		     &rld_obj_head, true);
    Elf64_Obj_Info obj, *next;
    for (next = rld_obj_head; 
	 next != NULL; 
	 next = (Elf64_Obj_Info *)obj.oi_next) 
    {
      p->readDataSpace(next, sizeof(Elf64_Obj_Info), &obj, true);
      Address dso_base = obj.oi_ehdr;
      if (dso_base == ret_base) {
	// DSO name
	int name_len = obj.oi_pathname_len + 1;
	char *name_buf = new char[name_len];
	p->readDataSpace((char *)obj.oi_pathname, name_len, name_buf, true);
	//fprintf(stderr, ">>> 64-bit getObjectPath(): \"%s\"\n", name_buf);
	ret = name_buf;
	delete [] name_buf;
      }
    }

  } else { // 32-bit ELF

    // read rld map elements
    Elf32_Obj_Info *rld_obj_head;
    // address-in-memory
    if (sizeof(void *) == sizeof(uint64_t)) {
      // 32-bit application, 64-bit paradynd
      rld_obj_head = NULL;
      char *local_addr = ((char *)&rld_obj_head) + 4;
      p->readDataSpace((void *)rld_obj_addr, sizeof(uint32_t), 
		       local_addr, true);
    } else {
      p->readDataSpace((void *)rld_obj_addr, sizeof(Elf32_Obj_Info *), 
		       &rld_obj_head, true);
    }
    Elf32_Obj_Info obj, *next;
    for (next = rld_obj_head; 
	 next != NULL; 
	 next = (Elf32_Obj_Info *)obj.oi_next) 
    {
      p->readDataSpace(next, sizeof(Elf32_Obj_Info), &obj, true);
      Address dso_base = obj.oi_ehdr;
      if (dso_base == ret_base) {
	// DSO name
	int name_len = obj.oi_pathname_len + 1;
	char *name_buf = new char[name_len];
	p->readDataSpace((char *)obj.oi_pathname, name_len, name_buf, true);
	ret = name_buf;
	delete [] name_buf;
      }
    }

  }
  //if (is_elf64) fprintf(stderr, ">>> 64-bit getObjectPath() successful\n");
}


dsoEvent_t::dsoEvent_t(process *p, dsoEventType_t t, Address f)
: type(t), fn_brk(f), next_brk(0), proc(p)
{ 
  // save original instruction in buffer
  p->readDataSpace((void *)fn_brk, INSN_SIZE, &buf, true);
  // set trap at breakpoint address
  instruction trap;
  genTrap(&trap);
  p->writeDataSpace((void *)fn_brk, INSN_SIZE, &trap);
}

dsoEvent_t::~dsoEvent_t()
{
  //fprintf(stderr, ">>> dsoEvent_t::~dsoEvent_t()\n");
  // extract event trap
  if (next_brk == 0) {
    proc->writeDataSpace((void *)fn_brk, INSN_SIZE, &buf);
  } else {
    proc->writeDataSpace((void *)next_brk, INSN_SIZE, &buf);
  }
}

// TODO: hooks in dlopen() and dlclose()
bool dynamic_linking::setMappingHooks(process *p, int libc_fd, Address /*libc_base*/)
{
  //fprintf(stderr, ">>> dynamic_linking::setMappingHooks()\n");

  // ELF initialization
  assert(libc_fd != -1); 
  Elf *elfp = elf_begin(libc_fd, ELF_C_READ, 0);
  assert(elfp);

  // ELF class: 32/64-bit
  bool is_elf64 = p->getImage()->getObject().is_elf64();

  // find ".dynsym" and ".dynstr" sections
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

    // used symbols: dlopen, ___tp_dlinsert_post
    // deprecated symbols: ___tp_dlremove_post
    // unused: __tp_dlinsert_pre, __tp_dlinsert_version_pre
    // unused: __tp_dlremove_pre
    if (strcmp(name, "dlopen") == 0) {
      dlopen_addr = pd_sym.pd_value;
    } else if (strcmp(name, "___tp_dlinsert_post") == 0) {
      dso_events += new dsoEvent_t(p, DSO_INSERT_POST, pd_sym.pd_value);
    }
  }

  elf_end(elfp); // cleanup

  // "_rld_text_resolve" address stored (but unused)
  // TODO: deprecate
  r_brk_addr = p->getImage()->getObject().get_rbrk_addr();

  //if (is_elf64) fprintf(stderr, ">>> 64-bit setMappingHooks() successful\n");
  return true;
}

void dynamic_linking::unsetMappingHooks(process * /*p*/)
{
  //fprintf(stderr, ">>> dynamic_linking::unsetMappingHooks()\n");
  for (unsigned i = 0; i < dso_events.size(); i++) {
    delete dso_events[i];
  }
}

// set dlopen_addr, r_brk_addr
// dlopen events should result in a call to addASharedObject
// dlclose events should result in a call to removeASharedObject
vector<shared_object *> *dynamic_linking::getSharedObjects(process *p)
{
  //fprintf(stderr, ">>> getSharedObjects()\n");
  int libc_fd = -1;
  Address libc_base;

  // process DSOs that have already been loaded
  vector<shared_object *> *rld_objs = getRldObjects(p, libc_fd, libc_base);

  // set hooks for future DSO mappings (and unmappings)
  setMappingHooks(p, libc_fd, libc_base);

  // set dynamic linking flags
  p->setDynamicLinking();
  dynlinked = true;

  return rld_objs;
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
  char *rtlib_val = getenv(rtlib_var);
  if (!rtlib_val) {
    string msg = "The environment variable ";
    msg += rtlib_var;
    msg += " is not defined properly.";
    showErrorCallback(101, msg);
    return false;
  }
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
    // build environment variable
    char buf[512];
    sprintf(buf, "%s=%s%s%s", rtlib_var, rtlib_val, rtlib_mod, rtlib_suffix);
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

// dlopenToAddr(): find DSO base address from dlopen() return value
Address dynamic_linking::dlopenToAddr(process *p, void *dlopen_ret)
{
  Address ret;

  // ELF class: 32/64-bit
  bool is_elf64 = p->getImage()->getObject().is_elf64();
  if (is_elf64) {

    // (Elf64_Obj_Info *) is offset 2 doublewords from
    // the 64-bit dlopen return value (void *) 
    Address pobj_addr = ((Address)dlopen_ret) + (2 * sizeof(uint64_t));
    Elf64_Obj_Info *obj_addr, obj;
    // address-in-memory
    p->readDataSpace((void *)pobj_addr, sizeof(Elf64_Obj_Info *), &obj_addr, true);
    p->readDataSpace(obj_addr, sizeof(Elf64_Obj_Info), &obj, true);
    // "oi_orig_ehdr": quickstart (precomputed) base address
    // "oi_ehdr"     : runtime (actual) base address
    ret = obj.oi_ehdr;

  } else { // 32-bit ELF

    // (Elf32_Obj_Info *) is offset 3 words from
    // the 32-bit dlopen return value (void *)
    Address pobj_addr = ((Address)dlopen_ret) + (3 * sizeof(uint32_t));
    Elf32_Obj_Info *obj_addr, obj;
    // address-in-memory
    if (sizeof(void *) == sizeof(uint64_t)) {
      // 32-bit application, 64-bit paradynd
      obj_addr = NULL;
      char *local_addr = ((char *)&obj_addr) + 4;
      p->readDataSpace((void *)pobj_addr, sizeof(uint32_t), local_addr, true);
    } else {
      p->readDataSpace((void *)pobj_addr, sizeof(Elf32_Obj_Info *), &obj_addr, true);
    }
    p->readDataSpace(obj_addr, sizeof(Elf32_Obj_Info), &obj, true);
    // "oi_orig_ehdr": quickstart (precomputed) base address
    // "oi_ehdr"     : runtime (actual) base address
    ret = obj.oi_ehdr;
    
  }
  
  //if (is_elf64) fprintf(stderr, ">>> 64-bit dlopenToAddr() successful\n");
  return ret;
}

char *dso2str(dsoEventType_t t) 
{
  switch (t) {
  case DSO_INSERT_PRE: return "___tp_dlinsert_pre";
  case DSO_INSERT_VERSION_PRE: return "___tp_dlinsert_version_pre";
  case DSO_INSERT_POST: return "___tp_dlinsert_post";
  case DSO_REMOVE_PRE: return "___tp_dlremove_pre";
  case DSO_REMOVE_POST: return "___tp_dlremove_post";
  default: return "unknown";
  }
  return "unknown";
}

bool dynamic_linking::handleIfDueToSharedObjectMapping(process *p, 
				vector<shared_object *> **changed_objs,
				u_int &change_type, 
				bool &error)
{
  bool ret = false;
  error = false;

  // read registers (PC)
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

  for (unsigned i = 0; i < dso_events.size(); i++) {
    dsoEvent_t *event = dso_events[i];
    instruction trap;
    genTrap(&trap);

    if (pc == event->fn_brk) {
      // jr ra 
      assert(isInsnType(event->buf, JRmask, JRmatch));
      assert(event->buf.rtype.rs == REG_RA);
      // nop
      instruction delay_insn;
      p->readDataSpace((void *)(pc + INSN_SIZE), INSN_SIZE, &delay_insn, true);
      assert(delay_insn.raw == NOP_INSN);

      // emulate "jr ra"
      Address ret_addr = regs[PROC_REG_RA];
      if (!(p->changePC(ret_addr))) {
	fprintf(stderr, "!!! handleIfDueToSharedObjectMapping(changePC)\n");
	error = true;
	change_type = SHAREDOBJECT_NOCHANGE;
	return true;
      }

      /* symbol addresses are absolute, based on precomputed object
	 base addresses; the "base address" passed to the
	 shared_object ctor is the DSO's actual runtime base address */

      void *dlopen_ret = (void *)regs[PROC_REG_A1];
      if (dlopen_ret) {
	// DSO base address
	Address dso_addr = dlopenToAddr(p, dlopen_ret);
	// DSO name
	string dso_name = "";
	getObjectPath(p, dso_addr, dso_name);

	// return new shared_object
	vector<shared_object *> *newVec = new vector<shared_object *>;
	(*newVec) += new shared_object(dso_name, dso_addr, false, true, true, 0);
	*changed_objs = newVec;
	change_type = SHAREDOBJECT_ADDED;
      } else {
	// failed dlopen() call
	change_type = SHAREDOBJECT_NOCHANGE;
      }
      ret = true;
      break;
    } else if (pc == event->next_brk) {
      assert(0); // TODO: remove this code (not reached)
    }
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


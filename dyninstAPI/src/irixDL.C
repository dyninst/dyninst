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

#include <stdio.h>
#include <sys/ucontext.h> // gregset_t
#include "util/h/Types.h" // Address
#include "util/h/Dictionary.h"
#include "util/h/Object.h" // Object-elf32
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

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

char *Bool[] = { "false", "true" };

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

bool process::trapAtEntryPointOfMain()
{
  bool ret = (pcFromProc(proc_fd) == main_brk_addr);
  //if (ret) fprintf(stderr, ">>> process::trapAtEntryPointOfMain()\n");
  return ret;
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

void process::insertTrapAtEntryPointOfMain()
{
  //fprintf(stderr, ">>> process::insertTrapAtEntryPointOfMain()\n");

  // save trap address: start of main()
  // TODO: use start of "_main" if exists?
  main_brk_addr = lookup_fn(this, "main");

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

vector<shared_object *> *
dynamic_linking::getRldObjects(process *p, int &libc_fd, Address &libc_base)
{
  //fprintf(stderr, ">>> dynamic_linking::getRldObjects()\n");
  vector<shared_object *> *ret = new vector<shared_object *>;

  internalSym rld_obj_sym;
  if (!(p->findInternalSymbol(string("__rld_obj_head"), true, rld_obj_sym))) {
    delete ret;
    return NULL;
  }
  Address rld_obj_addr = rld_obj_sym.getAddr();

  // read address of (head of) rld map
  Elf32_Obj_Info *rld_obj_head;
  p->readDataSpace((void *)rld_obj_addr, sizeof(Elf32_Obj_Info *), 
		   &rld_obj_head, true);

  Elf32_Obj_Info obj, *next;
  for (next = rld_obj_head; next != NULL; next = (Elf32_Obj_Info *)obj.oi_next) {
    // read rld object
    p->readDataSpace(next, sizeof(Elf32_Obj_Info), &obj, true);

    // DSO name
    int name_len = obj.oi_pathname_len + 1;
    char *name_buf = new char[name_len];
    p->readDataSpace((char *)obj.oi_pathname, name_len, name_buf, true);
    string dso_name = name_buf;
    delete [] name_buf;

    // first object in map is executable (skip)
    // TODO: check against PIOCPSINFO strings
    if (dso_name == p->getImage()->file() ||
	dso_name == p->getImage()->name()) 
      {
	continue;
      }

    // DSO base address
    Address dso_base = obj.oi_ehdr;

    // new shared object
    shared_object *so = new shared_object(dso_name, dso_base, false, true, true, 0);
    *ret += so; // add to vector

    // map "libc" to file descriptor
    if (strstr(dso_name.string_of(), "libc.")) {
      int proc_fd = p->getProcFileDescriptor();
      assert(proc_fd);
      libc_base = dso_base;
      caddr_t libc_base_proc = (caddr_t)dso_base;
      if ((libc_fd = ioctl(proc_fd, PIOCOPENM, &libc_base_proc)) == -1) {
	perror("dynamic_linking::getRldObjects(PIOCOPENM)");
	delete ret;
	return NULL;
      }
    }
  }

  return ret;
}


void dynamic_linking::getObjectPath(process *p, Address ret_base, string &ret)
{
  //fprintf(stderr, ">>> dynamic_linking::getObjectPath(0x%08x)\n", base);

  // find head of rld map
  internalSym rld_obj_sym;
  p->findInternalSymbol(string("__rld_obj_head"), true, rld_obj_sym);
  assert(rld_obj_sym.getAddr());
  Address rld_obj_addr = rld_obj_sym.getAddr();
  Elf32_Obj_Info *rld_obj_head;
  p->readDataSpace((void *)rld_obj_addr, sizeof(Elf32_Obj_Info *), 
		   &rld_obj_head, true);

  // read rld map elements
  Elf32_Obj_Info obj, *next;
  for (next = rld_obj_head; next != NULL; next = (Elf32_Obj_Info *)obj.oi_next) {
    p->readDataSpace(next, sizeof(Elf32_Obj_Info), &obj, true);

    // DSO base address
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

  // ELF header
  assert(libc_fd != -1); 
  Elf *elfp = elf_begin(libc_fd, ELF_C_READ, 0);
  assert(elfp);

  // section header names
  Elf32_Ehdr *ehdrp = elf32_getehdr(elfp);
  assert(ehdrp);
  Elf_Scn *shstrscnp = elf_getscn(elfp, ehdrp->e_shstrndx);
  assert(shstrscnp);
  Elf_Data *shstrdatap = elf_getdata(shstrscnp, 0);
  assert(shstrdatap);
  const char *shnames = (const char *)shstrdatap->d_buf;

  // symbol table
  Elf_Scn *symscnp = 0;
  Elf_Scn *strscnp = 0;
  Elf_Scn *scnp = 0;
  while ((scnp = elf_nextscn(elfp, scnp))) {
    Elf32_Shdr *shdrp = elf32_getshdr(scnp);
    assert(shdrp);

    const char *shname = &shnames[shdrp->sh_name];
    if (!strcmp(shname, ".dynsym")) symscnp = scnp;
    else if (!strcmp(shname, ".dynstr")) strscnp = scnp;
    if (symscnp && strscnp) break;
  }
  assert(symscnp);
  assert(strscnp);
  
  // symbol table (names)
  Elf_Data *strdatap = elf_getdata(strscnp, 0);
  assert(strdatap);
  const char *symnames = (const char *)strdatap->d_buf;
  // symbol table (symbols)
  Elf_Data *symdatap = elf_getdata(symscnp, 0);
  assert(symdatap);
  unsigned nsyms = symdatap->d_size / sizeof(Elf32_Sym);
  Elf32_Sym *syms = (Elf32_Sym *)symdatap->d_buf;

  // find dlopen(), dlclose()
  for (unsigned i = 0; i < nsyms; i++) {
    Elf32_Sym *sym = &syms[i];
    if (sym->st_shndx == SHN_UNDEF) continue;
    if (ELF32_ST_TYPE(sym->st_info) == STT_FUNC) {
      const char *name = &symnames[sym->st_name];

      // find symbols: dlopen, ___tp_dlinsert_post, ___tp_dlremove_post
      // unused: __tp_dlinsert_pre, __tp_dlinsert_version_pre
      // unused: __tp_dlremove_pre
      if (strcmp(name, "dlopen") == 0) {
	//dlopen_addr = libc_base + sym->st_value;
	dlopen_addr = sym->st_value;
	//fprintf(stderr, "  %s found (%0#10x)\n", name, sym->st_value);
      } else if (strcmp(name, "___tp_dlinsert_post") == 0) {
	dso_events += new dsoEvent_t(p, DSO_INSERT_POST, sym->st_value);
	//fprintf(stderr, "  %s found (%0#10x)\n", name, sym->st_value);
      } else if (strcmp(name, "___tp_dlremove_post") == 0) {
	//dso_events += new dsoEvent_t(p, DSO_REMOVE_POST, sym->st_value);
	//fprintf(stderr, "  %s found (%0#10x)\n", name, sym->st_value);
      }
    }
  }
  elf_end(elfp);


  // "_rld_text_resolve" address is store, but no trap is set (unused)
  r_brk_addr = p->getImage()->getObject().get_rbrk_addr();

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
  //fprintf(stderr, ">>> dynamic_linking::getSharedObjects()\n");
  int libc_fd;
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
  //fprintf(stderr, ">>> process::dlopenDYNINSTlib()\n");
  
  // use "_start" as scratch buffer to invoke dlopen() on DYNINST
  Address baseAddr = lookup_fn(this, "_start");
  char buf_[BYTES_TO_SAVE], *buf = buf_;
  Address bufSize = 0;
  
  // step 0: illegal instruction (code)
  genIll((instruction *)(buf + bufSize));
  bufSize += INSN_SIZE;
  
  // step 1: DYNINST library string (data)
  //Address libStart = bufSize;
  Address libAddr = baseAddr + bufSize;
#ifdef BPATCH_LIBRARY
  char *libVar = "DYNINSTAPI_RT_LIB";
#else
  char *libVar = "PARADYN_LIB";
#endif
  char *libName = getenv(libVar);
  if (!libName) {
    string msg("DYNINSTAPI_RT_LIB is not defined");
    showErrorCallback(101, msg);
    return false;
  }
  int libSize = strlen(libName) + 1;
  strcpy(buf + bufSize, libName);
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
  AstNode *call;
  string callee = "dlopen";
  // inferior dlopen(): build AstNodes
  args[0] = new AstNode(AstNode::Constant, (void *)libAddr);
  args[1] = new AstNode(AstNode::Constant, (void *)DLOPEN_MODE);
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

  /*
  fprintf(stderr, "inferior dlopen code: (%i bytes)\n", bufSize);
  dis(buf, baseAddr);
  fprintf(stderr, "%0#10x      \"%s\"\n", baseAddr + libStart, buf + libStart);
  for (int i = codeStart; i < trapEnd; i += INSN_SIZE) {
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

  dyninstlib_brk_addr = trapAddr;
  isLoadingDyninstLib = true;
  return true;
}

// dlopenToAddr(): find DSO base address from dlopen() return value
Address dynamic_linking::dlopenToAddr(process *p, void *dlopen_ret)
{
  // TODO: 64-bit version
  // (Elf32_Obj_Info *) is offset 3 words from (void *) dlopen return value
  unsigned *pobj_addr = ((unsigned *)dlopen_ret) + 3;
  Elf32_Obj_Info *obj_addr, obj;
  p->readDataSpace(pobj_addr, sizeof(Elf32_Obj_Info *), &obj_addr, true);
  p->readDataSpace(obj_addr, sizeof(Elf32_Obj_Info), &obj, true);

  // "Elf32_Obj_Info.oi_orig_ehdr": quickstart (precomputed) base address
  // "Elf32_Obj_Info.oi_ehdr"     : runtime (actual) base address
  //Address ret = obj.oi_ehdr - obj.oi_orig_ehdr;
  Address ret = obj.oi_ehdr;
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

// TODO: cleanup (remove?) single-step method
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
      // advance trap one instruction (deprecated)
      //event->next_brk = regs[PROC_REG_RA];
      //p->writeDataSpace((void *)event->fn_brk, INSN_SIZE, &event->buf);
      //p->readDataSpace((void *)event->next_brk, INSN_SIZE, &event->buf, true);
      //p->writeDataSpace((void *)event->next_brk, INSN_SIZE, &trap);

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

      // restore trap to call point
      //p->writeDataSpace((void *)event->next_brk, INSN_SIZE, &event->buf);
      //p->readDataSpace((void *)event->fn_brk, INSN_SIZE, &event->buf, true);
      //p->writeDataSpace((void *)event->fn_brk, INSN_SIZE, &trap);
      //event->next_brk = 0;

      change_type = SHAREDOBJECT_NOCHANGE;
      ret = true;
      break;
    }
  }

  return ret;
}

void process::handleIfDueToDyninstLib()
{
  //fprintf(stderr, ">>> process::handleIfDueToDyninstLib()\n");

  // restore code and registers
  Address code = lookup_fn(this, "_start");
  assert(code);
  writeDataSpace((void *)code, sizeof(savedCodeBuffer), savedCodeBuffer);
  restoreRegisters(savedRegs);

  delete [] savedRegs;
  savedRegs = NULL;
}


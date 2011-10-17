/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include "windows_thread.h"
#include "GeneratorWindows.h"
#include "windows_process.h"
#include <sstream>

int_thread *int_thread::createThreadPlat(int_process *proc, 
										 Dyninst::THR_ID thr_id, 
										 Dyninst::LWP lwp_id,
										 bool initial_thrd)
{
	if (initial_thrd) {
		return NULL;
	}
	windows_thread *wthrd = new windows_thread(proc, thr_id, lwp_id);
	assert(wthrd);
	return static_cast<int_thread *>(wthrd);
}

windows_thread::windows_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
int_thread(p, t, l),
hthread(INVALID_HANDLE_VALUE),
m_StartAddr(0),
m_TLSAddr(0),
stackBase(0),
threadInfoBlockAddr_(0)
{
}

windows_thread::~windows_thread() 
{
}

void windows_thread::setHandle(HANDLE h)
{
	hthread = h;
}

void windows_thread::setStartFuncAddress(Dyninst::Address addr)
{
	m_StartAddr = addr;
}

std::string windows_thread::dumpThreadContext()
{
	plat_suspend();
	std::stringstream s;
	CONTEXT context;
	context.ContextFlags = CONTEXT_FULL;
	int result = GetThreadContext(hthread, &context);
	if(!result) {
		pthrd_printf("Error getting thread context: %d\n", GetLastError());
	}
	s << "TID " << tid << std::hex << ", EIP=" << context.Eip <<
		", Single stepping " << ((context.EFlags & TF_BIT) ? "on" : "off");
	return s.str();
}

void windows_thread::setOptions()
{
	// Should be a no-op on Windows...
}

bool windows_thread::attach()
{
	// All threads on windows are attached automatically.
	assert(getInternalState() == neonatal);
	return true;
}

bool windows_thread::plat_cont()
{
	GeneratorWindows* wGen = static_cast<GeneratorWindows*>(Generator::getDefaultGenerator());
	bool ok = true;
	if(singleStep())
	{
		plat_suspend();
		CONTEXT context;
		int result;
		context.ContextFlags = CONTEXT_FULL;
		result = GetThreadContext(hthread, &context);
		if(!result) {
			pthrd_printf("Couldn't get thread context\n");
			ok = false;
			goto done;
		} else {

			context.ContextFlags = CONTEXT_FULL;
			pthrd_printf("Enabling single-step on %d/%d\n", proc()->getPid(), tid);
			context.EFlags |= TF_BIT;
			result = SetThreadContext(hthread, &context);
			if(!result)
			{
				pthrd_printf("Couldn't set thread context to single-step thread\n");
				ok = false;
				goto done;
			}
		}
done:
		plat_resume();
	}
	plat_resume();
	wGen->wake(llproc()->getPid());
	return ok;
}

bool windows_thread::plat_stop()
{
	pthrd_printf("Stopping thread %d (0x%lx)\n", getLWP(),
		hthread);
	windows_process* wproc = dynamic_cast<windows_process*>(llproc());
	int result = -1;
	if(wproc->pendingDebugBreak())
	{
		return true;
	}
	else
	{
		result = ::DebugBreakProcess(wproc->plat_getHandle());
		if(result == -1) {
			int err = ::GetLastError();
			pthrd_printf("Error from DebugBreakProcess: %d\n", err);
		}
		wproc->setPendingDebugBreak();
	}
	return result != -1;
}

bool windows_thread::plat_suspend()
{
	int result = ::SuspendThread(hthread);
	pthrd_printf("Suspending %d/%d, suspend count is %d\n", llproc()->getPid(), tid, result);
	printf("Suspending %d/%d, suspend count is %d\n", llproc()->getPid(), tid, result);
	return result != -1;
}

bool windows_thread::plat_resume()
{
	int result = ::ResumeThread(hthread);
	pthrd_printf("Resuming %d/%d, suspend count is %d\n", llproc()->getPid(), tid, result);
	printf("Resuming %d/%d, suspend count is %d\n", llproc()->getPid(), tid, result);

	return result != -1;
}

bool windows_thread::plat_getAllRegisters(int_registerPool &regpool)
{
	bool ret = false;
	plat_suspend();
	CONTEXT c;
	c.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	if(::GetThreadContext(hthread, &c))
	{
		regpool.regs[x86::eax] = c.Eax;
		regpool.regs[x86::ebx] = c.Ebx;
		regpool.regs[x86::ecx] = c.Ecx;
		regpool.regs[x86::edx] = c.Edx;
		regpool.regs[x86::esp] = c.Esp;
		regpool.regs[x86::ebp] = c.Ebp;
		regpool.regs[x86::esi] = c.Esi;
		regpool.regs[x86::edi] = c.Edi;
		regpool.regs[x86::flags] = c.ContextFlags;
		regpool.regs[x86::dr0] = c.Dr0;
		regpool.regs[x86::dr1] = c.Dr1;
		regpool.regs[x86::dr2] = c.Dr2;
		regpool.regs[x86::dr3] = c.Dr3;
		regpool.regs[x86::dr6] = c.Dr6;
		regpool.regs[x86::dr7] = c.Dr7;
		regpool.regs[x86::cs] = c.SegCs;
		regpool.regs[x86::ds] = c.SegDs;
		regpool.regs[x86::es] = c.SegEs;
		regpool.regs[x86::fs] = c.SegFs;
		regpool.regs[x86::gs] = c.SegGs;
		regpool.regs[x86::ss] = c.SegSs;
		regpool.regs[x86::eip] = c.Eip;
		ret = true;
	}
	//fprintf(stderr, "Got regs, CS:EIP = 0x%x:0x%x\n", c.SegCs, c.Eip);
	return ret;
}

bool windows_thread::plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_setAllRegisters(int_registerPool &regpool) 
{
	plat_suspend();
	CONTEXT c;
	c.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	c.Eax = regpool.regs[x86::eax];
	c.Ebx = regpool.regs[x86::ebx];
	c.Ecx = regpool.regs[x86::ecx];
	c.Edx = regpool.regs[x86::edx];
	c.Esp = regpool.regs[x86::esp];
	c.Ebp = regpool.regs[x86::ebp];
	c.Esi = regpool.regs[x86::esi];
	c.Edi = regpool.regs[x86::edi];
	c.ContextFlags = regpool.regs[x86::flags];
	c.Dr0 = regpool.regs[x86::dr0];
	c.Dr1 = regpool.regs[x86::dr1];
	c.Dr2 = regpool.regs[x86::dr2];
	c.Dr3 = regpool.regs[x86::dr3];
	c.Dr6 = regpool.regs[x86::dr6];
	c.Dr7 = regpool.regs[x86::dr7];
	c.SegCs = regpool.regs[x86::cs];
	c.SegDs = regpool.regs[x86::ds];
	c.SegEs = regpool.regs[x86::es];
	c.SegFs = regpool.regs[x86::fs];
	c.SegGs = regpool.regs[x86::gs];
	c.SegSs = regpool.regs[x86::ss];
	c.Eip = regpool.regs[x86::eip];
	bool ok = ::SetThreadContext(hthread, &c);
	::FlushInstructionCache(dynamic_cast<windows_process*>(proc_)->plat_getHandle(), 0, 0);
	//fprintf(stderr, "Set regs, CS:EIP = 0x%x:0x%x\n", c.SegCs, c.Eip);
	CONTEXT verification;
	::GetThreadContext(hthread, &verification);
	//assert(verification.Eip == c.Eip);
	plat_resume();
	return ok;
}

bool windows_thread::plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val)
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_convertToSystemRegs(const int_registerPool &regpool, unsigned char *user_area) 
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_getThreadArea(int val, Dyninst::Address &addr)
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_needsPCSaveBeforeSingleStep()
{
	return true;
}

bool windows_thread::plat_needsEmulatedSingleStep(std::vector<Dyninst::Address> &result)
{
	// NOTE: this modifies the vector of addresses with locations that need emulation.
	// true/false is an error return, not a "do we need emulation" return!
	return true;
}
bool windows_thread::haveUserThreadInfo()
{
	return m_StartAddr != 0;
}

bool windows_thread::getTID(Dyninst::THR_ID& tid)
{
	tid = this->tid;
	return true;
}

Address windows_thread::getThreadInfoBlockAddr()
{
	if (threadInfoBlockAddr_) {
		return threadInfoBlockAddr_;
	}
	int_registerPool regs;
	if(!plat_getAllRegisters(regs))
	{
		return 0;
	}
	int fs = regs.regs[x86::fs];
	// use the FS segment selector to look up the segment descriptor in the local descriptor table
	LDT_ENTRY segDesc;
	if (!GetThreadSelectorEntry(hthread, fs, &segDesc)) {
		fprintf(stderr, "%s[%d] Failed to read segment register FS for thread 0x%x with FS index of 0x%x\n", 
			FILE__,__LINE__,hthread, fs);
		return 0;
	}
	// calculate the address of the TIB
	threadInfoBlockAddr_ = (Address) segDesc.BaseLow;
	Address tmp = (Address) segDesc.HighWord.Bytes.BaseMid;
	threadInfoBlockAddr_ = threadInfoBlockAddr_ | (tmp << (sizeof(WORD)*8));
	tmp = segDesc.HighWord.Bytes.BaseHi;
	threadInfoBlockAddr_ = threadInfoBlockAddr_ | (tmp << (sizeof(WORD)*8+8));
	return threadInfoBlockAddr_;
}


bool windows_thread::getStartFuncAddress(Dyninst::Address& start_addr)
{
	if(m_StartAddr) {
		start_addr = m_StartAddr;
		return true;
	}
	return false;
}

bool windows_thread::getStackBase(Dyninst::Address& stack_base)
{
	if(stackBase) {
		stack_base = stackBase;
		return true;
	}
	Address tib_addr = getThreadInfoBlockAddr();
	if(!tib_addr) {
		stack_base = 0;
		return false;
	}
	Address stack_base_addr = tib_addr + 4;
	proc_->plat_readMem(this, &stackBase, stack_base_addr, sizeof(Dyninst::Address));
	stack_base = stackBase;
	return true;
}

bool windows_thread::getStackSize(unsigned long& stack_size)
{
	Address tib_addr = getThreadInfoBlockAddr();
	if(!tib_addr) {
		stack_size = 0;
		return false;
	}
	Address stack_base = 0;
	if(!getStackBase(stack_base))
	{
		stack_size = 0;
		return false;
	}
	Address stack_bottom = 0;
	Address stack_bottom_addr = tib_addr + 8;
	proc_->plat_readMem(this, &stack_bottom, stack_bottom_addr, sizeof(Dyninst::Address));
	stack_size = stack_base - stack_bottom;
	return true;
}

bool windows_thread::getTLSPtr(Dyninst::Address& tls_ptr)
{
	tls_ptr = m_TLSAddr;
	return m_TLSAddr != 0;
}

void windows_thread::setTLSAddress( Dyninst::Address addr )
{
	m_TLSAddr = addr;
}

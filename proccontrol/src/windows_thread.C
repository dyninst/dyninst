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

#include "windows_thread.h"
#include "GeneratorWindows.h"
#include "windows_process.h"
#include <sstream>
#include <iostream>

using namespace std;

int_thread *int_thread::createThreadPlat(int_process *proc, 
										 Dyninst::THR_ID thr_id, 
										 Dyninst::LWP lwp_id,
										 bool initial_thrd)
{
	windows_thread *wthrd = new windows_thread(proc, thr_id, lwp_id);
	assert(wthrd);
	return static_cast<int_thread *>(wthrd);
}

int_thread *int_thread::createRPCThread(int_process *proc) 
{
	pthrd_printf("Creating dummy thread for RPC: initial tid -1, lwp -1\n");
	// We're creating a placeholder thread that will get actual information
	// filled in later. Thus we don't actually have an lwp_id or thr_id...
	// oops? Set to appropriately casted -1 and hope we don't use that info
	// before we actually need it...
	windows_thread *wthrd = new windows_thread(proc, (Dyninst::THR_ID) -1, (Dyninst::LWP) -1);
	assert(wthrd);
	wthrd->markRPCThread();

	// Fake it into a state that will make the iRPC code happy.
	// Update this if postRPCToThread changes
	// We're creating suspended, so we can write to it...
	wthrd->getGeneratorState().setState(int_thread::stopped);
	wthrd->getHandlerState().setState(int_thread::stopped);
	wthrd->getUserState().setState(int_thread::stopped);
	wthrd->handler_exiting_state = false;

	return static_cast<int_thread *>(wthrd);
}

windows_thread::windows_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
int_thread(p, t, l),
hthread(INVALID_HANDLE_VALUE),
m_StartAddr(0),
m_TLSAddr(0),
stackBase(0),
threadInfoBlockAddr_(0),
isUser_(true),
dummyRPC_(notRPCThread),
dummyRpcPC_(0)
{
}

windows_thread::~windows_thread() 
{
	// Do NOT close the handle here. That's handled when ContinueDebugEvent is called on the thread exit event.
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
	if (isRPCpreCreate()) return "<DUMMY RPC THREAD>";
	if(!isSuspended()) return "<RUNNING THREAD>";

	std::stringstream s;
	CONTEXT context;
	context.ContextFlags = CONTEXT_FULL;
	int result = GetThreadContext(hthread, &context);
	if(!result) {
		pthrd_printf("Error getting thread context: %d\n", GetLastError());
	}
	s << "TID " << tid << std::hex <<
#ifdef _WIN64
		", RIP=" << context.Rip <<
#else
		", EIP=" << context.Eip <<
#endif
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
	// Don't assert here; RPC thread might be running.
	return true;
}

bool windows_thread::plat_cont()
{
	if (isRPCpreCreate()) return true;
	pthrd_printf("plat_cont for %d/%d\n", proc()->getPid(), tid);
	bool ok = true;
	if(singleStep())
	{
		pthrd_printf("Singlestepping thread on continue: %d/%d\n", proc()->getPid(), tid);
		plat_suspend();
		CONTEXT context;
		int result;
		context.ContextFlags = CONTEXT_FULL;
		result = GetThreadContext(hthread, &context);
		if(!result) {
			pthrd_printf("Couldn't get thread context: %d\n", ::GetLastError());
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

	//plat_resume();

	return ok;
}

bool windows_thread::plat_stop()
{
	if (isRPCpreCreate()) return true;

	pthrd_printf("Stopping thread %d (0x%lx)\n", getLWP(),
		hthread);
	windows_process* wproc = dynamic_cast<windows_process*>(llproc());
	int result = -1;
	if(wproc->pendingDebugBreak() || (getGeneratorState().getState() == int_thread::stopped))
	{
		// If there's a debug break pending or we're generator stopped, then all we need to do is set state.
		// Only make the debug break call if we don't know that we're going to become stopped at some future point.
		pthrd_printf("Pending debug break (%s) or generator stopped (%s), erasing pending stop and returning true\n",
			(wproc->pendingDebugBreak() ? "<true>" : "<false>"), (getGeneratorState().getState() == int_thread::stopped) ? "<true>" : "<false>");
		setPendingStop(false);
		return true;
	}
	else
	{
		pthrd_printf("... DebugBreak called\n");
		result = ::DebugBreakProcess(wproc->plat_getHandle());
		if(result == -1) {
			int err = ::GetLastError();
			pthrd_printf("Error from DebugBreakProcess: %d\n", err);
		} else {
			// Don't force this thread to run. We want to force the *actual* stop thread to run.
			wproc->setPendingDebugBreak();
			setPendingStop(false);
			return true;
		}
	}
	return result != -1;
}

bool windows_thread::plat_suspend()
{
	if (isRPCpreCreate()) return true;

	int result = ::SuspendThread(hthread);
	if (result == -1 && (::GetLastError() == ERROR_ACCESS_DENIED)) {
		// FIXME
		// This happens if the thread is in a system call and thus cannot be modified. 
		// However, handling such an event is a royal pain in the ass, and so we're ignoring
		// it for now and not failing. 
		pthrd_printf("Thread %d faking suspend, was in a system call\n", tid);
		return true;
	}

	pthrd_printf("Suspending %d/%d, suspend count is %d, error code %d\n", llproc()->getPid(), tid, result, ((result == -1) ? ::GetLastError() : 0));
	//cerr << "Context at suspend: " << dumpThreadContext() << endl;


	return result != -1;
}

bool windows_thread::plat_resume()
{
	if (isRPCpreCreate()) return true;

	int result = ::ResumeThread(hthread);
	pthrd_printf("Resuming %d/%d, suspend count is %d\n", llproc()->getPid(), tid, result);

	if (result == -1 && (::GetLastError() == ERROR_ACCESS_DENIED)) {
		// FIXME
		// This happens if the thread is in a system call and thus cannot be modified. 
		// However, handling such an event is a royal pain in the ass, and so we're ignoring
		// it for now and not failing. 
		return true;
	}

	pthrd_printf("Resuming %d/%d, suspend count is %d, error code %d\n", llproc()->getPid(), tid, result, ((result == -1) ? ::GetLastError() : 0));
	return result != -1;
}

bool windows_thread::plat_getAllRegisters(int_registerPool &regpool)
{
	if (isRPCpreCreate()) return true;

	bool ret = false;
	plat_suspend();
	CONTEXT c;
	c.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	if(::GetThreadContext(hthread, &c))
	{
#ifdef _WIN64
		regpool.regs[x86_64::rax] = c.Rax;
		regpool.regs[x86_64::rbx] = c.Rbx;
		regpool.regs[x86_64::rcx] = c.Rcx;
		regpool.regs[x86_64::rdx] = c.Rdx;
		regpool.regs[x86_64::rsp] = c.Rsp;
		regpool.regs[x86_64::rbp] = c.Rbp;
		regpool.regs[x86_64::rsi] = c.Rsi;
		regpool.regs[x86_64::rdi] = c.Rdi;
		regpool.regs[x86_64::flags] = c.EFlags;
		regpool.regs[x86_64::dr0] = c.Dr0;
		regpool.regs[x86_64::dr1] = c.Dr1;
		regpool.regs[x86_64::dr2] = c.Dr2;
		regpool.regs[x86_64::dr3] = c.Dr3;
		regpool.regs[x86_64::dr6] = c.Dr6;
		regpool.regs[x86_64::dr7] = c.Dr7;
		regpool.regs[x86_64::cs] = c.SegCs;
		regpool.regs[x86_64::ds] = c.SegDs;
		regpool.regs[x86_64::es] = c.SegEs;
		regpool.regs[x86_64::fs] = c.SegFs;
		regpool.regs[x86_64::gs] = c.SegGs;
		regpool.regs[x86_64::ss] = c.SegSs;
		regpool.regs[x86_64::rip] = c.Rip;
#else
		regpool.regs[x86::eax] = c.Eax;
		regpool.regs[x86::ebx] = c.Ebx;
		regpool.regs[x86::ecx] = c.Ecx;
		regpool.regs[x86::edx] = c.Edx;
		regpool.regs[x86::esp] = c.Esp;
		regpool.regs[x86::ebp] = c.Ebp;
		regpool.regs[x86::esi] = c.Esi;
		regpool.regs[x86::edi] = c.Edi;
		regpool.regs[x86::flags] = c.EFlags;
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
#endif
		ret = true;
	}
	plat_resume();
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
	if (isRPCpreCreate()) {
		pthrd_printf("setAllRegisters called on dummy RPC thread, grabbing PC for future use: 0x%lx\n",
			regpool.regs[x86::eip]);
		// Snarf the PC, ignore everything else
		dummyRpcPC_ = regpool.regs[x86::eip];
		return true;
	}

	plat_suspend();

	//std::cerr << "plat_setAllRegisters, EIP = " << std::hex << regpool.regs[x86::eip] << std::dec << std::endl;

	CONTEXT c;
	c.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	if (isRPCThread()) {
		// The context we're given is _wrong_, but we only care about EIP/RIP. I hope.
		::GetThreadContext(hthread, &c);
#ifdef _WIN64
		c.Rip = regpool.regs[x86_64::rip];
#else
		c.Eip = regpool.regs[x86::eip];
#endif
	}
	else {
#ifdef _WIN64
		c.Rax = regpool.regs[x86_64::rax];
		c.Rbx = regpool.regs[x86_64::rbx];
		c.Rcx = regpool.regs[x86_64::rcx];
		c.Rdx = regpool.regs[x86_64::rdx];
		c.Rsp = regpool.regs[x86_64::rsp];
		c.Rbp = regpool.regs[x86_64::rbp];
		c.Rsi = regpool.regs[x86_64::rsi];
		c.Rdi = regpool.regs[x86_64::rdi];
		c.EFlags = regpool.regs[x86_64::flags];
		c.Dr0 = regpool.regs[x86_64::dr0];
		c.Dr1 = regpool.regs[x86_64::dr1];
		c.Dr2 = regpool.regs[x86_64::dr2];
		c.Dr3 = regpool.regs[x86_64::dr3];
		c.Dr6 = regpool.regs[x86_64::dr6];
		c.Dr7 = regpool.regs[x86_64::dr7];
		c.SegCs = regpool.regs[x86_64::cs];
		c.SegDs = regpool.regs[x86_64::ds];
		c.SegEs = regpool.regs[x86_64::es];
		c.SegFs = regpool.regs[x86_64::fs];
		c.SegGs = regpool.regs[x86_64::gs];
		c.SegSs = regpool.regs[x86_64::ss];
		c.Rip = regpool.regs[x86_64::rip];
#else
		c.Eax = regpool.regs[x86::eax];
		c.Ebx = regpool.regs[x86::ebx];
		c.Ecx = regpool.regs[x86::ecx];
		c.Edx = regpool.regs[x86::edx];
		c.Esp = regpool.regs[x86::esp];
		c.Ebp = regpool.regs[x86::ebp];
		c.Esi = regpool.regs[x86::esi];
		c.Edi = regpool.regs[x86::edi];
		c.EFlags = regpool.regs[x86::flags];
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
#endif
	}
	BOOL ok = ::SetThreadContext(hthread, &c);
	if(!ok) {
		int error = GetLastError();
		pthrd_printf("Couldn't set registers: error code %d\n", error);
	}
	::FlushInstructionCache(dynamic_cast<windows_process*>(proc_)->plat_getHandle(), 0, 0);
	//fprintf(stderr, "Set regs, CS:EIP = 0x%x:0x%x\n", c.SegCs, c.Eip);
	//fprintf(stderr, "          TF = %s\n", (c.EFlags & TF_BIT) ? "true" : "false" );
	CONTEXT verification;
	verification.ContextFlags = CONTEXT_FULL;
	::GetThreadContext(hthread, &verification);
#ifdef _WIN64
	assert(verification.Rip == c.Rip);
#else
	assert(verification.Eip == c.Eip);
#endif
	plat_resume();
	return ok ? true : false;
}

bool windows_thread::plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val)
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_convertToSystemRegs(const int_registerPool &regpool, unsigned char *user_area, bool) 
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

bool windows_thread::getTID(Dyninst::THR_ID& t)
{
	t = tid;
	return true;
}

Address windows_thread::getThreadInfoBlockAddr()
{
	if (isRPCpreCreate()) return false;

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
		//fprintf(stderr, "%s[%d] Failed to read segment register FS for thread 0x%x with FS index of 0x%x\n", 
		//	FILE__,__LINE__,hthread, fs);
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
	if (isRPCpreCreate()) return false;

	// If we don't have the member set, look on the stack.
	if(!m_StartAddr) {
		Dyninst::Address stackbase;
		if(getStackBase(stackbase)) {
			windows_process* wp = dynamic_cast<windows_process*>(proc()->llproc());
			wp->plat_readMem(this, &m_StartAddr, stackbase - 8, 4);
		}
	}
	// If the stack query failed, we'll re-query every time. Which is sub-optimal, but should work at least.
	if(m_StartAddr) {
		start_addr = m_StartAddr;
		return true;
	}
	return false;
}

bool windows_thread::getStackBase(Dyninst::Address& stack_base)
{
	if (isRPCpreCreate()) return false;

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
	if (isRPCpreCreate()) return false;

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

bool windows_thread::notAvailableForRPC()
{
	if (isRPCpreCreate()) return false;
	if (!isUser()) return true;

	int_registerPool regs;
	plat_getAllRegisters(regs);
	Address prevInsnIfSysenter = regs.regs[x86::eip] - 2;
	char prevInsn[2];
	proc_->plat_readMem(this, prevInsn, prevInsnIfSysenter, 2);
	if((prevInsn[0] == 0x0F) && (prevInsn[1] == 0x34)) // sysenter
	{
		pthrd_printf("%d/%d: Found sysenter at 0x%lx, thread cannot run RPC\n", llproc()->getPid(), tid, prevInsnIfSysenter);
		return true;
	}
	if((prevInsn[1] & 0xff) == 0xcc) // trap
	{
		pthrd_printf("%d/%d: Found trap at 0x%lx, thread cannot run RPC\n", llproc()->getPid(), tid, prevInsnIfSysenter + 1);
		//return true;
	}
	pthrd_printf("Thread at 0x%lx does not need trap for RPC, prevInsn[1] = 0x%x\n", regs.regs[x86::eip], prevInsn[1]);
	return false;
}

void windows_thread::setUser(bool u) {
	pthrd_printf("Setting userness of thread %d/%d to: %s\n",
		llproc()->getPid(), tid, (u ? "USER" : "SYSTEM"));
	isUser_ = u;
}

bool windows_thread::isUser() const {
	return isUser_;
}

void windows_thread::setDummyRPCStart(Address a) {
	pthrd_printf("Setting dummy PC to 0x%lx\n", a);
	dummyRpcPC_ = a;
}

Address windows_thread::getDummyRPCStart() const {
	return dummyRpcPC_;
}

void windows_thread::updateThreadHandle(Dyninst::THR_ID t, Dyninst::LWP l) {
	tid = t;
	lwp = l;
}

void windows_thread::plat_terminate() {
	::TerminateThread(hthread, 0);
}

bool windows_thread::isRPCEphemeral() const {
//	return (dummyRPC_ == RPCrunning);
	return isRPCThread();
}

bool windows_thread::isRPCThread() const { 
	return (dummyRPC_ != notRPCThread);
}

void windows_thread::markRPCThread() {
	assert(dummyRPC_ == notRPCThread);
	dummyRPC_ = RPCpreCreate;
}

bool windows_thread::isRPCpreCreate() const {
	return (dummyRPC_ == RPCpreCreate);
}

void windows_thread::markRPCRunning() {
	assert(dummyRPC_ == RPCpreCreate);
	dummyRPC_ = RPCrunning;
}

void windows_thread::plat_setSuspendCount(int count) {
	pthrd_printf("%d/%d: setting suspend count to %d\n",
				llproc()->getPid(), tid, count);
	if (isRPCpreCreate()) {
		pthrd_printf("\t pre-created RPC thread, ret with no changed\n");
		return;
	}

	int result = ::SuspendThread(hthread);
	pthrd_printf("\t having suspended once, result is %d\n", result);
	if (result == -1 && (::GetLastError() == ERROR_ACCESS_DENIED)) {
		::ResumeThread(hthread);
		// FIXME
		// This happens if the thread is in a system call and thus cannot be modified. 
		// However, handling such an event is a royal pain in the ass, and so we're ignoring
		// it for now and not failing. 
		pthrd_printf("Thread %d faking suspend, was in a system call\n", tid);
		return;
	}	
	
	int current = result + 1;
	// result is what we _had_, not what we have now.

	while (current != count) {
		if (current < count) {
			pthrd_printf("\t current %d, count %d, suspending\n", current, count);
			::SuspendThread(hthread);
			current++;
		}
		else {
			pthrd_printf("\t current %d, count %d, resuming\n", current, count);
			::ResumeThread(hthread);
			current--;
		}
	}
	pthrd_printf("Setting suspend count of %d/%d to %d, error code %d, last reported value %d\n", llproc()->getPid(), tid, count, ((result == -1) ? ::GetLastError() : 0), current);
}

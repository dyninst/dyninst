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

#if !defined(WINDOWS_PROCESS_H)
#define WINDOWS_PROCESS_H

#include "arch_process.h"

class windows_process : public arch_process
{
public:
	windows_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
		std::vector<std::string> envp, std::map<int,int> f);
	windows_process(Dyninst::PID pid_, int_process *p);
	virtual ~windows_process();

	virtual bool plat_create();
	virtual bool plat_create_int();
	virtual bool plat_attach(bool allStopped);
	virtual bool plat_attach_int();
	virtual bool plat_attachWillTriggerStop();
	virtual bool plat_forked();
	virtual bool plat_execed();
	virtual bool plat_detach();
	virtual bool plat_terminate(bool &needs_sync);

	virtual bool plat_readMem(int_thread *thr, void *local, 
		Dyninst::Address remote, size_t size);
	virtual bool plat_writeMem(int_thread *thr, const void *local, 
		Dyninst::Address remote, size_t size);
	virtual SymbolReaderFactory *plat_defaultSymReader();
	virtual bool needIndividualThreadAttach();
	virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps);
	virtual Dyninst::Architecture getTargetArch();
	virtual bool plat_individualRegAccess();
	virtual bool plat_contProcess();
	virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address min, unsigned size);
	virtual bool plat_supportLWPEvents() const;
	virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates);
	virtual bool plat_convertToBreakpointAddress(psaddr_t &);
	virtual unsigned int getTargetPageSize();
	virtual bool plat_createAllocationSnippet(Dyninst::Address addr, bool use_addr,
		unsigned long size, void*& buffer, unsigned long& buffer_size,
		unsigned long& start_offset);
	virtual bool plat_createDeallocationSnippet(Dyninst::Address addr, unsigned long size,
		void*& buffer, unsigned long& buffer_size,
		unsigned long& start_offset);
	virtual bool plat_collectAllocationResult(int_thread* thr, reg_response::ptr resp);
	virtual bool refresh_libraries(std::set<int_library *> &added_libs,
		std::set<int_library *> &rmd_libs,
		std::set<response::ptr> &async_responses);
	virtual int_library *getExecutableLib();
	virtual bool initLibraryMechanism();
	virtual bool plat_isStaticBinary();
	HANDLE plat_getHandle();
	void plat_setHandle(HANDLE h);
	virtual bool hasPendingDetach() const { return pendingDetach; }
	virtual void clearPendingDebugBreak() { pendingDebugBreak_ = false; }
	virtual void setPendingDebugBreak() { pendingDebugBreak_ = true; }
	virtual bool pendingDebugBreak() const { return pendingDebugBreak_; }
	// Windows lets us do this directly.
	virtual Dyninst::Address infMalloc(unsigned long size, bool use_addr = false, Dyninst::Address addr = 0x0);
	virtual bool infFree(Dyninst::Address addr);

private:
	HANDLE hproc;
	bool pendingDetach;
	bool pendingDebugBreak_;
};

#endif //!defined(WINDOWS_PROCESS_H)

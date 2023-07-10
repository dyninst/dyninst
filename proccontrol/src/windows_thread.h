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
#if !defined(WINDOWS_THREAD_H)
#define WINDOWS_THREAD_H

#include <string>
#include <vector>
#include "int_process.h"

#if !defined(TF_BIT)
#define TF_BIT 0x100
#endif



class windows_thread : public int_thread
{
public:
	windows_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);

	windows_thread();
	virtual ~windows_thread();

	virtual bool plat_cont();
	virtual bool plat_stop();
	virtual bool plat_getAllRegisters(int_registerPool &reg);
	virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);
	virtual bool plat_setAllRegisters(int_registerPool &reg);
	virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val);
	virtual bool attach();

	virtual bool plat_getThreadArea(int val, Dyninst::Address &addr);
	virtual bool plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *regs, bool gprs_only = false);
	virtual bool plat_needsEmulatedSingleStep(std::vector<Dyninst::Address> &result);
	virtual bool plat_needsPCSaveBeforeSingleStep();

    virtual void plat_terminate();

    virtual bool isRPCEphemeral() const;

	void setOptions();
	bool getSegmentBase(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);
	bool plat_suspend();
	bool plat_resume();
	void plat_setSuspendCount(int count);
	bool haveUserThreadInfo();
	bool getTID(Dyninst::THR_ID& tid);
	void setLWP(Dyninst::LWP L) {
		lwp = L;
	}
	bool getStartFuncAddress(Dyninst::Address& start_addr);
	bool getStackBase(Dyninst::Address& stack_base);
	bool getStackSize(unsigned long& stack_size);
	bool getTLSPtr(Dyninst::Address& tls_ptr);
	void setHandle(HANDLE h);
	std::string dumpThreadContext();
	void setStartFuncAddress(Dyninst::Address addr);
	void setTLSAddress(Dyninst::Address addr);
	Address getThreadInfoBlockAddr();
	virtual bool notAvailableForRPC();

	bool isUser() const;
	void setUser(bool);

	bool isRPCThread() const;
	void markRPCThread();
	bool isRPCpreCreate() const;
	void markRPCRunning();

	void setDummyRPCStart(Address);
	Address getDummyRPCStart() const;

	void updateThreadHandle(Dyninst::THR_ID, Dyninst::LWP);
	void* plat_getHandle() const {
		return hthread;
	}

private:
	HANDLE hthread;
	Dyninst::Address m_StartAddr;
	Dyninst::Address m_TLSAddr;
	Dyninst::Address stackBase;
	Address threadInfoBlockAddr_;
	bool isUser_;

	typedef enum {
		notRPCThread,
		RPCpreCreate,
		RPCrunning } dummyRPC_t;

	dummyRPC_t dummyRPC_;
	Address dummyRpcPC_;
};

#endif //!defined(WINDOWS_THREAD_H)

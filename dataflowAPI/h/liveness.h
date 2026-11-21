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
#ifndef LIVENESS_H
#define LIVENESS_H

#include "CFG.h"
#include "CodeObject.h"
#include "CodeSource.h"
#include "Location.h"
#include "Instruction.h"
#include "InstructionDecoder.h"
#include "InstructionCache.h"
#include "bitArray.h"
#include "ABI.h"
#include <map>
#include <set>
#include "Register.h"



struct livenessData{
	bitArray in, out, use, def;
};

class DYNINST_EXPORT LivenessAnalyzer{
	std::map<Dyninst::ParseAPI::Block*, livenessData> blockLiveInfo;
	std::map<Dyninst::ParseAPI::Function*, bool> liveFuncCalculated;
        std::map<Dyninst::ParseAPI::Function*, bitArray> funcRegsDefined;
	InstructionCache cachedLivenessInfo;

	const bitArray& getLivenessIn(Dyninst::ParseAPI::Block *block);
	const bitArray& getLivenessOut(Dyninst::ParseAPI::Block *block, bitArray &allRegsDefined);
	void processEdgeLiveness(Dyninst::ParseAPI::Edge* e, livenessData& data, Dyninst::ParseAPI::Block* block, const bitArray& allRegsDefined);
	
	void summarizeBlockLivenessInfo(Dyninst::ParseAPI::Function* func, Dyninst::ParseAPI::Block *block, bitArray &allRegsDefined);
	bool updateBlockLivenessInfo(Dyninst::ParseAPI::Block *block, bitArray &allRegsDefined);
	
	ReadWriteInfo calcRWSets(Dyninst::InstructionAPI::Instruction curInsn, Dyninst::ParseAPI::Block *blk, Dyninst::Address a);

	void* getPtrToInstruction(Dyninst::ParseAPI::Block *block, Dyninst::Address addr) const;	
	bool isExitBlock(Dyninst::ParseAPI::Block *block);
	bool isMMX(Dyninst::MachRegister machReg);
	Dyninst::MachRegister changeIfMMX(Dyninst::MachRegister machReg);
	int width;
	ABI* abi;

public:
	typedef enum {Before, After} Type;
	typedef enum {Invalid_Location} ErrorType;
	LivenessAnalyzer(int w);
	LivenessAnalyzer(Dyninst::Architecture arch, int w);
	void analyze(Dyninst::ParseAPI::Function *func);

	bool query(Dyninst::ParseAPI::Location loc, Type type, const Dyninst::MachRegister &machReg, bool& live);
	bool query(Dyninst::ParseAPI::Location loc, Type type, bitArray &bitarray);

	ErrorType getLastError(){ return errorno; }

	void clean(Dyninst::ParseAPI::Function *func);
	void clean();

	int getIndex(Dyninst::MachRegister machReg);
	ABI* getABI() { return abi;}

private:
	ErrorType errorno;
};


#endif  // LIVESS_H

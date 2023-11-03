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


using namespace Dyninst;
using namespace Dyninst::InstructionAPI;

struct livenessData{
	bitArray in, out, use, def;
};

class DATAFLOW_EXPORT LivenessAnalyzer{
	std::map<ParseAPI::Block*, livenessData> blockLiveInfo;
	std::map<ParseAPI::Function*, bool> liveFuncCalculated;
        std::map<ParseAPI::Function*, bitArray> funcRegsDefined;
	InstructionCache cachedLivenessInfo;

	const bitArray& getLivenessIn(ParseAPI::Block *block);
	const bitArray& getLivenessOut(ParseAPI::Block *block, bitArray &allRegsDefined);
	void processEdgeLiveness(ParseAPI::Edge* e, livenessData& data, ParseAPI::Block* block, const bitArray& allRegsDefined);
	
	void summarizeBlockLivenessInfo(ParseAPI::Function* func, ParseAPI::Block *block, bitArray &allRegsDefined);
	bool updateBlockLivenessInfo(ParseAPI::Block *block, bitArray &allRegsDefined);
	
	ReadWriteInfo calcRWSets(Instruction curInsn, ParseAPI::Block *blk, Address a);

	void* getPtrToInstruction(ParseAPI::Block *block, Address addr) const;	
	bool isExitBlock(ParseAPI::Block *block);
	bool isMMX(MachRegister machReg);
	MachRegister changeIfMMX(MachRegister machReg);
	int width;
	ABI* abi;

public:
	typedef enum {Before, After} Type;
	typedef enum {Invalid_Location} ErrorType;
	LivenessAnalyzer(int w);
	void analyze(ParseAPI::Function *func);

	template <class OutputIterator>
	bool query(ParseAPI::Location loc, Type type, OutputIterator outIter){
		bitArray liveRegs;
		if (query(loc,type, liveRegs)){
			for (std::map<MachRegister,int>::const_iterator iter = abi->getIndexMap()->begin(); iter != abi->getIndexMap()->end(); ++iter)
				if (liveRegs[iter->second]){
					outIter = iter->first;
					++outIter;
				}
			return true;
		}
		return false;
	}
	bool query(ParseAPI::Location loc, Type type, const MachRegister &machReg, bool& live);
	bool query(ParseAPI::Location loc, Type type, bitArray &bitarray);

	ErrorType getLastError(){ return errorno; }

	void clean(ParseAPI::Function *func);
	void clean();

	int getIndex(MachRegister machReg);
	ABI* getABI() { return abi;}

private:
	ErrorType errorno;
};


#endif  // LIVESS_H

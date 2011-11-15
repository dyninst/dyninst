#if defined(cap_liveness)
#ifndef LIVENESS_H
#define LIVENESS_H

#include "parseAPI/h/CFG.h"
#include "parseAPI/h/CodeObject.h"
#include "parseAPI/h/CodeSource.h"
#include "parseAPI/h/Location.h"
#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/Register.h"
#include "instructionAPI/h/InstructionDecoder.h"
#include "dataflowAPI/src/InstructionCache.h"
#include "dataflowAPI/h/bitArray.h"
#include "dataflowAPI/src/RegisterMap.h"
#include "dataflowAPI/h/ABI.h"
#include <map>
#include <set>


using namespace std;
using namespace Dyninst;
using namespace Dyninst::InstructionAPI;

struct livenessData{
	bitArray in, out, use, def;
};

class LivenessAnalyzer{
	map<Address, livenessData> blockLiveInfo;
	map<Address, bool> liveFuncCalculated;
	InstructionCache cachedLivenessInfo;

	const bitArray& getLivenessIn(ParseAPI::Block *block);
	const bitArray& getLivenessOut(ParseAPI::Block *block);
	void summarizeBlockLivenessInfo(ParseAPI::Function* func, ParseAPI::Block *block);
	bool updateBlockLivenessInfo(ParseAPI::Block *block);
	
	ReadWriteInfo calcRWSets(Instruction::Ptr curInsn, ParseAPI::Block* blk, Address a);

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
			for (map<MachRegister,int>::const_iterator iter = abi->getIndexMap()->begin(); iter != abi->getIndexMap()->end(); ++iter)
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

private:
	ErrorType errorno;
};


#endif  // LIVESS_H
#endif	// cap_liveness

#define BPATCH_FILE

#include <stdio.h>
#include <iostream.h>
#include "BPatch_sourceBlock.h"

//constructor
BPatch_sourceBlock::BPatch_sourceBlock() 
{}

//constructor
BPatch_sourceBlock::BPatch_sourceBlock(BPatch_Set<unsigned short>& sln)
	: sourceLines(sln) {}

//method to return vector of lines in the source block 
void 
BPatch_sourceBlock::getLines(BPatch_Vector<unsigned short>& lines){
	unsigned short* elements = new unsigned short[sourceLines.size()];
	sourceLines.elements(elements);
	for(int i=0;i<sourceLines.size();i++)
		lines.push_back(elements[i]);
	delete[] elements;
}

//print method 
ostream& operator<<(ostream& os,BPatch_sourceBlock& sb){
	os << "{";
	unsigned short* elements = new unsigned short[sb.sourceLines.size()];
	sb.sourceLines.elements(elements);
	for(int i=0;i<sb.sourceLines.size();i++)
		os << " " << elements[i];
	delete[] elements;
	os << " }\n";
	return os;
}

#define BPATCH_FILE

#include <stdio.h>
#include <iostream>
#include "BPatch_sourceBlock.h"

//constructor
BPatch_sourceBlock::BPatch_sourceBlock()
	: sourceFile(NULL),sourceLines(NULL)
{}

//constructor
BPatch_sourceBlock::BPatch_sourceBlock( const char *filePtr,
                                        BPatch_Set<unsigned short>& lines)
{
	sourceFile = filePtr;
	sourceLines = new BPatch_Set<unsigned short>(lines);
}

const char*
BPatch_sourceBlock::getSourceFile(){
	return sourceFile;
}

void
BPatch_sourceBlock::getSourceLines(BPatch_Vector<unsigned short>& lines){

	if(!sourceLines)
		return;

	unsigned short* elements = new unsigned short[sourceLines->size()];
	sourceLines->elements(elements);

	for(int j=0;j<sourceLines->size();j++)
		lines.push_back(elements[j]);
		
	delete[] elements;
}


#ifdef DEBUG 
//print method 
ostream& operator<<(ostream& os,BPatch_sourceBlock& sb){

	os << "{";

	if(sb.sourceFile)
		os << sb.sourceFile << " (";
	else
		os << "<NO_FILE_NAME>" << " (";
		
	if(sb.sourceLines){
		unsigned short* elements = new unsigned short[sb.sourceLines->size()];
		sb.sourceLines->elements(elements);
		for(int j=0;j<sb.sourceLines->size();j++)
			os << " " << elements[j];
		delete[] elements;
	}
	else
		os << "<NO_LINE_NUMBERS>";

	os << ")}" << endl;
	return os;
}
#endif

#ifdef IBM_BPATCH_COMPAT
void BPatch_sourceBlock::getIncPoints(BPatch_Vector<BPatch_point *> &vect)
{
//  nothing here for now...  might need to implement, might not.
}

void BPatch_sourceBlock::getExcPoints(BPatch_Vector<BPatch_point *> &vect)
{
 //  for now, they are the same
 getIncPoints(vect);
}
#endif

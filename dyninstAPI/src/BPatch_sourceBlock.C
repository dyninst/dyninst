/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

#define BPATCH_FILE

#include <stdio.h>
#include <iostream>
#include "BPatch_sourceBlock.h"

//constructor
BPatch_sourceBlock::BPatch_sourceBlock()
	: sourceFile(NULL),sourceLines(NULL)
{
}

//constructor
BPatch_sourceBlock::BPatch_sourceBlock( const char *filePtr,
                                        BPatch_Set<unsigned short>& lines)
{
	sourceFile = filePtr;
	sourceLines = new BPatch_Set<unsigned short>(lines);
}

const char*
BPatch_sourceBlock::getSourceFileInt(){
	return sourceFile;
}

void
BPatch_sourceBlock::getSourceLinesInt(BPatch_Vector<unsigned short>& lines){

	if(!sourceLines)
		return;

	unsigned short* elements = new unsigned short[sourceLines->size()];
	sourceLines->elements(elements);

	for(unsigned j=0;j<sourceLines->size();j++)
		lines.push_back(elements[j]);
		
	delete[] elements;
}

#ifdef IBM_BPATCH_COMPAT
bool BPatch_sourceBlock::getAddressRangeInt(void*& _startAddress, void*& _endAddress)
{
  return false;
}

bool BPatch_sourceBlock::getLineNumbersInt(unsigned int &_startLine,
                                        unsigned int  &_endLine)
{
  if (!sourceLines) return false;
  if (!sourceLines->size()) return false;
  _startLine = (unsigned int) sourceLines->minimum();
  _endLine = (unsigned int) sourceLines->maximum();

  return true;
}
#endif

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
void BPatch_sourceBlock::getIncPointsInt(BPatch_Vector<BPatch_point *> &vect)
{
//  nothing here for now...  might need to implement, might not.
}

void BPatch_sourceBlock::getExcPointsInt(BPatch_Vector<BPatch_point *> &vect)
{
 //  for now, they are the same
 getIncPoints(vect);
}

char *BPatch_sourceBlock::getNameInt(char *buf, int buflen)
{
  if (buflen > strlen("sourceBlock")) {
    strcpy(buf, "sourceBlock")[strlen("sourceBlock")]='\0';
    return buf;
  }
  return NULL;
}

#endif

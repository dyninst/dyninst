/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#define BPATCH_FILE

#include <stdio.h>
#include "common/h/std_namesp.h"
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

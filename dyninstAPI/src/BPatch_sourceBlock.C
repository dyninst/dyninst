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

#define BPATCH_FILE

#include <stdio.h>
#include "common/src/std_namesp.h"
#include "BPatch_sourceBlock.h"
#include <iterator>

//constructor
BPatch_sourceBlock::BPatch_sourceBlock()
	: sourceFile(NULL)
{
}

//constructor
BPatch_sourceBlock::BPatch_sourceBlock( const char *filePtr,
                                        std::set<unsigned short>& lines) :
   sourceFile(filePtr),
   sourceLines(lines)
{
}

const char*
BPatch_sourceBlock::getSourceFile(){
	return sourceFile;
}

void
BPatch_sourceBlock::getSourceLines(BPatch_Vector<unsigned short>& lines){

   std::copy(sourceLines.begin(), sourceLines.end(),
             std::back_inserter(lines));
}

#ifdef IBM_BPATCH_COMPAT
bool BPatch_sourceBlock::getAddressRange(void*& _startAddress, void*& _endAddress)
{
  return false;
}

bool BPatch_sourceBlock::getLineNumbers(unsigned int &_startLine,
                                        unsigned int  &_endLine)
{
   if (sourceLines.empty()) return false;
   _startLine = (unsigned int) *(sourceLines->begin());
   _endLine = (unsigned int) *(sourceLines->rbegin());

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

        if (sb.sourceLines.empty()) {
           os << "<NO_LINE_NUMBERS>";
        }
        else {
           for (std::set<unsigned short>::iterator iter = sb.sourceLines.begin();
                iter != sb.sourceLines.end(); ++iter) {
              os << " " << *iter;
           }
        }
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

char *BPatch_sourceBlock::getName(char *buf, int buflen)
{
  if (buflen > strlen("sourceBlock")) {
    strcpy(buf, "sourceBlock")[strlen("sourceBlock")]='\0';
    return buf;
  }
  return NULL;
}

#endif

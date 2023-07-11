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
#if !defined(DECODER_WINDOWS_H)
#define DECODER_WINDOWS_H

#include <string>
#include <vector>
#include "Decoder.h"
#include "Event.h"
#include "int_process.h"

using namespace Dyninst;
using namespace ProcControlAPI;

class DecoderWindows : public Decoder
{
 public:
   DecoderWindows();
   virtual ~DecoderWindows();
   virtual unsigned getPriority() const;
   virtual bool decode(ArchEvent *ae, std::vector<Event::ptr> &events);


   Dyninst::Address adjustTrapAddr(Dyninst::Address address, Dyninst::Architecture arch);
private:
   Event::ptr decodeBreakpointEvent(DEBUG_EVENT e, int_process* proc, int_thread* thread);
   Event::ptr decodeSingleStepEvent(DEBUG_EVENT e, int_process* proc, int_thread* thread);
   bool checkForFullString( DEBUG_EVENT &details, int chunkSize, wchar_t* libName, bool gotString, char* asciiLibName );
   EventLibrary::ptr decodeLibraryEvent(DEBUG_EVENT details, int_process* proc);
   void dumpSurroundingMemory( unsigned problemArea, int_process* proc );
   bool decodeCreateThread( DEBUG_EVENT &e, Event::ptr &newEvt, int_process* &proc, std::vector<Event::ptr> &events );

   std::string readLibNameFromProc(Address libnameaddr, DEBUG_EVENT details, int_process *p);
   std::string HACKreadFromFile(DEBUG_EVENT details, int_process *p);
};



#endif //!defined(DECODER_WINDOWS_H)

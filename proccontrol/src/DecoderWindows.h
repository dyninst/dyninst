#if !defined(DECODER_WINDOWS_H)
#define DECODER_WINDOWS_H

#include "proccontrol/h/Decoder.h"
#include "proccontrol/h/Event.h"
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
   bool checkForFullString( DEBUG_EVENT &details, int chunkSize, wchar_t* libName, bool gotString, char* asciiLibName );
   EventLibrary::ptr decodeLibraryEvent(DEBUG_EVENT details, int_process* proc);
   void dumpSurroundingMemory( unsigned problemArea, int_process* proc );
   bool decodeCreateThread( DEBUG_EVENT &e, Event::ptr &newEvt, int_process* &proc, std::vector<Event::ptr> &events );

   std::string readLibNameFromProc(Address libnameaddr, DEBUG_EVENT details, int_process *p);
   std::string HACKreadFromFile(DEBUG_EVENT details, int_process *p);
};



#endif //!defined(DECODER_WINDOWS_H)

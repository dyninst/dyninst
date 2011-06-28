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
   EventLibrary::ptr decodeLibraryEvent(DEBUG_EVENT details, int_process* proc);
   Dyninst::Address adjustTrapAddr(Dyninst::Address address, Dyninst::Architecture arch);
   Event::ptr decodeBreakpointEvent(DEBUG_EVENT e, int_process* proc, int_thread* thread);
};



#endif //!defined(DECODER_WINDOWS_H)

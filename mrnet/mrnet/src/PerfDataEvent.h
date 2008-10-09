#if !defined( __performance_data_events_h )
#define __performance_data_events_h 1

#include "mrnet/Packet.h"

namespace MRN {

bool handle_PerfGuiInit( PacketPtr& );
bool handle_PerfDataCPU( PacketPtr&, Rank );

}

#endif /* __performance_data_events_h */

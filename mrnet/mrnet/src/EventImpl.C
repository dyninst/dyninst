#include <list>

#include "mrnet/src/EventImpl.h"

namespace MRN {

std::list<EventImpl> EventImpl::events;
std::list<EventImpl> EventImpl::remote_events;

}

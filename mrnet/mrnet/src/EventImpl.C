/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <list>

#include "mrnet/src/EventImpl.h"

namespace MRN {

std::list<EventImpl> EventImpl::events;
std::list<EventImpl> EventImpl::remote_events;

}

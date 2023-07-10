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
#if !defined(HANDLER_H_)
#define HANDLER_H_

#include "Event.h"
#include "util.h"

#include <string>
#include <set>
#include <vector>

namespace Dyninst {
namespace ProcControlAPI {

class PC_EXPORT Handler 
{
 protected:
   std::string name;
 public:
   Handler(std::string name_ = std::string(""));
   virtual ~Handler();

   typedef enum {
      ret_success,
      ret_async,      
      ret_cbdelay,
      ret_again,
      ret_error
   } handler_ret_t;

   virtual handler_ret_t handleEvent(Event::ptr ev) = 0;
   virtual void getEventTypesHandled(std::vector<EventType> &etypes) = 0;
   virtual int getPriority() const;
   virtual Event::ptr convertEventForCB(Event::ptr orig);

   std::string getName() const;
   static const int PrePlatformPriority = 0x1000;
   static const int DefaultPriority = 0x1008;
   static const int PostPlatformPriority = 0x1010;
   static const int CallbackPriority = 0x1018;
   static const int PostCallbackPriority = 0x1020;
};

}
}

#endif

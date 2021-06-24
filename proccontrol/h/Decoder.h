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
#if !defined(DECODER_H_)
#define DECODER_H_

#include <vector>
#include "Event.h"
#include "util.h"

namespace Dyninst {
namespace ProcControlAPI {

class PC_EXPORT Decoder
{
 public:
   Decoder();
   virtual ~Decoder();

   static const unsigned int default_priority = 0x1000;

   virtual unsigned getPriority() const = 0;
   virtual bool decode(ArchEvent *archE, std::vector<Event::ptr> &events) = 0;
};

struct decoder_cmp
{
   bool operator()(const Decoder* a, const Decoder* b) const
   {
      return a->getPriority() < b->getPriority();
   }
};

}
}

#endif

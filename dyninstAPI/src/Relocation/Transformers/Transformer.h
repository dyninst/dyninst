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

#if !defined(_R_T_BASE_H_)
#define _R_T_BASE_H_

#include <list>
#include <map>
#include <stack>
#include "parseAPI/h/CFG.h"
#include "boost/shared_ptr.hpp"

class block_instance;
class baseTramp;

namespace Dyninst {

namespace Relocation {

class Widget;
class RelocBlock;
class TargetInt;
class CFWidget;
class RelocInsn;
struct RelocEdge;
struct RelocEdges;
class RelocGraph;

// One of the things a Transformer 'returns' (modifies, really) is 
// a list of where we require patches (branches from original code
// to new code). This list is prioritized - Required, 
// Suggested, and Not Required. Required means we have proof
// that a patch is necessary for correct control flow. Suggested means
// that, assuming correct parsing, no patch is necessary. Not Required
// means that even with incorrect parsing no patch is necessary.
// ... not sure how we can have that, but hey, we might as well
// design it in.

 
class Transformer {
 public:
  typedef boost::shared_ptr<Widget> WidgetPtr;
  typedef std::list<WidgetPtr> WidgetList;
  typedef std::map<block_instance *, RelocBlock *> RelocBlockMap;

  virtual bool processGraph(RelocGraph *);
  virtual bool process(RelocBlock *, 
                       RelocGraph *) = 0;

  virtual ~Transformer() {}
};

}
}

#endif

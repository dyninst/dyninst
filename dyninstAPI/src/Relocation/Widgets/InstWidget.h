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

#if !defined (_R_E_INSTRUMENTATION_H_)
#define _R_E_INSTRUMENTATION_H_

#include <string>
#include "Widget.h"

class instPoint;

namespace Dyninst {
namespace Relocation {

class InstWidget : public Widget {
 public:
  typedef boost::shared_ptr<InstWidget> Ptr;

  // I believe I can patch in the current code generation
  // system here...
  static Ptr create(instPoint *i);

     InstWidget(instPoint *i) : point_(i) {}

  // This sucks. It seriously sucks. But hey...
  // this points to all the baseTramps with instrumentation
  // at this point. This can be 0, 1, or 2 - 2 if we have
  // post instruction + pre instruction instrumentation.

  bool empty() const;

  bool generate(const codeGen &, const RelocBlock *, CodeBuffer &);
  
  TrackerElement *tracker() const;

  virtual ~InstWidget();

  virtual std::string format() const;

 private:
  instPoint *point_;
};

struct InstWidgetPatch : public Patch {
  InstWidgetPatch(baseTramp *a) : tramp(a) {}
  
   virtual bool apply(codeGen &gen, CodeBuffer *buf);
  virtual unsigned estimate(codeGen &templ);
  virtual ~InstWidgetPatch();

  baseTramp *tramp;
};

struct RemovedInstWidgetPatch : public Patch {
  RemovedInstWidgetPatch(baseTramp *a) : tramp(a) {}
   virtual bool apply(codeGen &gen, CodeBuffer *);
   virtual unsigned estimate(codeGen &templ);
   virtual ~RemovedInstWidgetPatch() {}
   
   baseTramp *tramp;
};


}
}
#endif

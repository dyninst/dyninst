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

#include "stackwalk/h/swk_errors.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

static err_t last_err;
static const char *last_msg;

int Dyninst::Stackwalker::dyn_debug_stackwalk = 0;
static FILE *debug_out = NULL;

err_t Dyninst::Stackwalker::getLastError() {
  return last_err;
}

const char *Dyninst::Stackwalker::getLastErrorMsg() {
   return last_msg;
}

void Dyninst::Stackwalker::setDebugChannel(FILE *f)
{
  debug_out = f;
}

void Dyninst::Stackwalker::setDebug(bool enable)
{
  dyn_debug_stackwalk = enable;
}

void Dyninst::Stackwalker::setLastError(err_t err, const char *msg) {
  last_err = err;
  last_msg = msg;
}

void Dyninst::Stackwalker::clearLastError()
{
   last_err = 0;
   last_msg = "";
}

FILE *Dyninst::Stackwalker::getDebugChannel() {
  return debug_out;
}

#if !defined(cap_omit_sw_debug)
int Dyninst::Stackwalker::sw_printf(const char *format, ...)
{
  static int initialized = 0;
  if (!initialized)
  {
    if (getenv("DYNINST_DEBUG_STACKWALK"))
      dyn_debug_stackwalk = 1;
    if (!debug_out)
      debug_out = stderr;
    initialized = 1;
  }

  if (!dyn_debug_stackwalk) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(debug_out, format, va);
  va_end(va);

  fflush(debug_out);

  return ret;
}
#endif

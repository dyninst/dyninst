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

#ifndef SWK_ERRORS_
#define SWK_ERRORS_

#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "compiler_annotations.h"

namespace Dyninst {
namespace Stackwalker {

  typedef unsigned err_t;

  const err_t err_badparam       = 0x10000;
  const err_t err_nostepper      = 0x10001;
  const err_t err_nosymlookup    = 0x10002;
  const err_t err_procread       = 0x10003;
  const err_t err_nosymbol       = 0x10004;
  const err_t err_internal       = 0x10005;
  const err_t err_prem           = 0x10006;
  const err_t err_perm           = 0x10006;
  const err_t err_noproc         = 0x10007;
  const err_t err_interrupt      = 0x10008;
  const err_t err_procexit       = 0x10009;
  const err_t err_stackbottom    = 0x10010;
  const err_t err_nofile         = 0x10011;
  const err_t err_unsupported    = 0x10012;
  const err_t err_symtab         = 0x10013;
  const err_t err_nolibtracker   = 0x10014;
  const err_t err_nogroup        = 0x10015;
  const err_t err_nothrd         = 0x10016;
  const err_t err_proccontrol    = 0x10017;

  SW_EXPORT err_t getLastError();
  SW_EXPORT void clearLastError();
  SW_EXPORT const char *getLastErrorMsg();
  SW_EXPORT void setLastError(err_t err, const char *msg = NULL);
  SW_EXPORT void setDebugChannel(FILE *f);
  SW_EXPORT void setDebug(bool enable);

  SW_EXPORT FILE *getDebugChannel();

  extern int sw_printf(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);
  extern int dyn_debug_stackwalk;

}
}

#if defined(cap_omit_sw_debug)
#define sw_printf(...)
#endif


#endif

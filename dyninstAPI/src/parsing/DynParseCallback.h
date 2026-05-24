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

#ifndef DYNINST_DYNINSTAPI_PARSING_DYNPARSECALLBACK_H
#define DYNINST_DYNINSTAPI_PARSING_DYNPARSECALLBACK_H

#include "CFG.h"
#include "ParseCallback.h"
#include "dyntypes.h"

class image;

namespace Dyninst { namespace DyninstAPI {

  class DynParseCallback : public ParseAPI::ParseCallback {
  public:
    DynParseCallback(image *img) : ParseAPI::ParseCallback(), _img(img) {
    }
    ~DynParseCallback() {
    }

  protected:
    void abruptEnd_cf(Dyninst::Address, ParseAPI::Block *, default_details *) override;

    void destroy_cb(ParseAPI::Block *) override;

    void destroy_cb(ParseAPI::Edge *) override;

    void destroy_cb(ParseAPI::Function *) override;

    void foundWeirdInsns(ParseAPI::Function *) override;

    bool hasWeirdInsns(const ParseAPI::Function *) const override;

    void interproc_cf(ParseAPI::Function *, ParseAPI::Block *, Dyninst::Address,
                      interproc_details *) override;

    void newfunction_retstatus(ParseAPI::Function *) override;

    void overlapping_blocks(ParseAPI::Block *, ParseAPI::Block *) override;

    void patch_nop_jump(Dyninst::Address) override;

    void remove_block_cb(ParseAPI::Function *, ParseAPI::Block *) override;

    void remove_edge_cb(ParseAPI::Block *, ParseAPI::Edge *, edge_type_t) override;

    void split_block_cb(ParseAPI::Block *, ParseAPI::Block *) override;

    bool updateCodeBytes(Dyninst::Address target) override;

#if defined(DYNINST_HOST_ARCH_POWER) || defined(DYNINST_HOST_ARCH_AARCH64)
    void instruction_cb(ParseAPI::Function *, ParseAPI::Block *, Dyninst::Address,
                        insn_details *);
#endif
  private:
    image *_img;
  };

}}

#endif

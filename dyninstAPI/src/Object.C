/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: Object.C,v 1.11 2003/07/15 22:43:53 schendel Exp $

#include "common/h/Dictionary.h"
#include "dyninstAPI/src/Object.h"

int symbol_compare(const Symbol &s1, const Symbol &s2) {
    // select the symbol with the lowest address
    Address s1_addr = s1.addr();
    Address s2_addr = s2.addr();
    if (s1_addr > s2_addr) return 1;
    if (s1_addr < s2_addr) return -1;

    // symbols are co-located at the same address
    // select the symbol which is not a function
    if (s1.type() != Symbol::PDST_FUNCTION) return(-1);
    if (s2.type() != Symbol::PDST_FUNCTION) return(1);
    // symbols are both functions
    // select the symbol which has GLOBAL linkage
    if (s1.linkage() == Symbol::SL_GLOBAL) return(-1);
    if (s2.linkage() == Symbol::SL_GLOBAL) return(1);
    // neither function is GLOBAL
    // select the symbol which has LOCAL linkage
    if (s1.linkage() == Symbol::SL_LOCAL) return(-1);
    if (s2.linkage() == Symbol::SL_LOCAL) return(1);
    // both functions are WEAK
    return(0);    
}

ostream & relocationEntry::operator<< (ostream &s) const {
    s << "target_addr_ = " << target_addr_ << endl;
    s << "rel_addr_ = " << rel_addr_ << endl;
    s << "name_ = " << name_ << endl;
    return s; 
}

ostream &operator<<(ostream &os, relocationEntry &q) {
    return q.operator<<(os);
}

bool AObject::needs_function_binding() const {
    return false;
}

bool AObject::get_func_binding_table(pdvector<relocationEntry> &) const {
    return false;
}

bool AObject::get_func_binding_table_ptr(const pdvector<relocationEntry> *&) const {
    return false;
}

/**************************************************
 *
 *  Stream based debuggering output - for regreesion testing.
 *  Dump info on state of object *this....
 *
**************************************************/

const ostream &AObject::dump_state_info(ostream &s) {

    // key and value for distc hash iter.... 
    pdstring str;
    Symbol sym;
    dictionary_hash_iter<pdstring, Symbol> symbols_iter(symbols_);

    s << "Debugging Info for AObject (address) : " << this << endl;

    s << " file_ = " << file_ << endl;
    s << " symbols_ = " << endl;

    // and loop over all the symbols, printing symbol name and data....
    //  or try at least....
    symbols_iter.reset();
    while (symbols_iter.next(str, sym)) {
      s << "  key = " << str << " val " << sym << endl;
    }

    s << " code_ptr_ = " << code_ptr_ << endl;
    s << " code_off_ = " << code_off_ << endl;
    s << " code_len_ = " << code_len_ << endl;
    s << " data_ptr_ = " << data_ptr_ << endl;
    s << " data_off_ = " << data_off_ << endl;
    s << " data_len_ = " << data_len_ << endl;
    return s;
}






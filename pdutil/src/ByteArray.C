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

// ByteArray.C

#include <assert.h>
#include "pdutil/h/ByteArray.h"

// trace data streams
bool_t P_xdr_byteArray(XDR *x, char **c, u_int *sizep, const u_int maxsize) {
  return (xdr_bytes(x, c, sizep, maxsize));}

byteArray::byteArray()
    : bArray_(0), len_(0) {
}

byteArray::byteArray(const void *bArray, unsigned len) {
   len_ = len;
   bArray_ = new char[len];
   (void) P_memcpy(bArray_, bArray, len);
}

byteArray::byteArray(const byteArray& s) {
   len_ = s.len_;
   bArray_ = new char[len_];
   (void) P_memcpy(bArray_, s.bArray_, len_);
}

byteArray::~byteArray() {
    delete [] (char*)bArray_; bArray_ = 0;
}

byteArray&
byteArray::operator=(const byteArray& s) {
    if (this == &s) {
        return *this;
    }

    delete [] (char*)bArray_; bArray_ = 0;

    len_ = s.len_;
    bArray_ = new char[len_];
    (void) P_memcpy(bArray_, s.bArray_, len_);

    return *this;
}


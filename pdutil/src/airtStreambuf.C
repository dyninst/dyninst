/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

#include <stdio.h>
#include <string.h>
#include "pdutil/h/airtStreambuf.h"
#include "pdutil/h/pdDebugOstream.h"

unsigned enable_pd_airt_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define airt_cerr if (enable_pd_airt_debug) cerr
#else
#define airt_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

airtStreambuf::airtStreambuf(sendStrFuncPtr_t ptr) { 
  sendStrFunc = ptr; 
  setbuf(buffer,bufferLen);
}

int airtStreambuf::sync() {
  airt_cerr << "Start - sync: " << (void *) sendStrFunc << "\n";
  streamsize n = pptr() - pbase(); 
  airt_cerr << "n: " << n << " [" << pbase() << "\n";
  if(n>0) (*sendStrFunc)(pbase(), n);
  airt_cerr << "Stop  - sync\n";
  return 0;
}

int airtStreambuf::overflow(int ch) {
  streamsize n = pptr() - pbase();   // <next free char> - <start of put-buf>
  airt_cerr << "Start - overflow: " << ch << ", streamsize = " << n << "\n";
  bool passnl = false;
  if(n>0) {
     if(sgetc() == '\n') {
	airt_cerr << "  == nl\n";
	sputbackc('\0');
	passnl = true;
     }
     sync();
  }

  if(ch != EOF) {
    char tempstr[5];
    tempstr[0] = ch;
    tempstr[1] = 0;
    (*sendStrFunc)(tempstr, 1);
  }
  if(passnl)
    (*sendStrFunc)("\n", strlen("\n"));
  pbump(-n);
  airt_cerr << "Stop  - overflow\n";
  return 0;
}

// NT compiler requires this to be implemented, not used.  underflow is
// only used for istreams
int airtStreambuf::underflow() {
  return 0;
}

/* Careful, currently irix has a bug in that it only calls xsputn when the
   buffer is full.  In other words, we have no way to get control while ios
   is writing in the streambuf, until an buffer overflow (xsputn) or flush
   occurs (sync).  What this means is that writing to a ostream that uses an
   airtStream, you could be surprised in the delays in when it's writing.
   One work around is to write to this stream with explicit flushes.  We'll
   need to wait for sgi to fix the streambuf definition. 
*/
streamsize airtStreambuf::xsputn(const char* text, streamsize n) {
  airt_cerr << "xsputn - : " << n << "  " << text << "\n";
  int ret = sync();
  if(ret == EOF) return 0;
  (*sendStrFunc)(text,n);
  airt_cerr << "xsputn - leaving\n";
  return n;
}

#include <stdio.h>
#include <string.h>
#include "pdutil/h/airtStreambuf.h"
#include "pdutil/h/pdDebugOstream.h"



#ifdef AIRT_DEBUG
pdDebug_ostream airt_cerr(cerr, true);
#else
pdDebug_ostream airt_cerr(cerr, false);
#endif



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

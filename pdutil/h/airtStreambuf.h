#include "common/h/std_namesp.h"

#ifndef streamsize
#define streamsize int
#endif

// airt mean to redirect (and its shorter)
class airtStreambuf : public std::streambuf {
 public:
  enum { bufferLen = 512 };

  typedef void(*sendStrFuncPtr_t)(const char *, int len);
  airtStreambuf(sendStrFuncPtr_t ptr);
  int sync();
  int underflow();
  int overflow(int ch);
  streamsize xsputn(const char *text, streamsize n);  
 private:
  char buffer[bufferLen + 1];
  sendStrFuncPtr_t sendStrFunc;  
};

// xdr_send_recv.C

#include "pdutil/h/xdr_send_recv.h"
#include <assert.h>

bool P_xdr_send(XDR *xdr, const bool &b) {
   assert(xdr->x_op == XDR_ENCODE);
   bool_t actual_b = b; // needed since they're probably of different types
   return xdr_bool(xdr, &actual_b);
}

bool P_xdr_send(XDR *xdr, const short &s) {
   assert(xdr->x_op == XDR_ENCODE);
   short actual_s = s; 
   return xdr_short(xdr, &actual_s);
}

bool P_xdr_send(XDR *xdr, const unsigned short &us) {
   assert(xdr->x_op == XDR_ENCODE);
   unsigned short actual_us = us;
   return xdr_u_short(xdr, &actual_us);
}

#if defined(i386_unknown_nt4_0)

bool P_xdr_send(XDR *xdr, const int &i) {
   int j = i;
   assert(xdr->x_op == XDR_ENCODE);
   return xdr_int(xdr, &j);
}

bool P_xdr_send(XDR *xdr, const unsigned &u) {
   unsigned v = u;
   assert(xdr->x_op == XDR_ENCODE);
   return xdr_u_int(xdr, &v);
}

bool P_xdr_send(XDR *xdr, const long &l) {
   long k = l;
   assert(xdr->x_op == XDR_ENCODE);
   return xdr_long(xdr, &k);
}

bool P_xdr_send(XDR *xdr, const unsigned long &ul) {
   unsigned long vl = ul;
   assert(xdr->x_op == XDR_ENCODE);
   return xdr_u_long(xdr, &vl);
}

#endif //defined(i386_unknown_nt4_0)

bool P_xdr_send(XDR *xdr, const uint32_t &num) {
   assert(xdr->x_op == XDR_ENCODE);
   uint32_t actual_num = num;
   return xdr_u_int(xdr, (unsigned *)&actual_num);
}

bool P_xdr_send(XDR *xdr, const int32_t &num) {
   assert(xdr->x_op == XDR_ENCODE);
   int32_t actual_num = num;
   return xdr_int(xdr, (int *)&actual_num);
}

#if !defined(i386_unknown_nt4_0)

bool P_xdr_send(XDR *xdr, const uint64_t &num) {
   assert(xdr->x_op == XDR_ENCODE);
   uint64_t actual_num = num;
   return xdr_u_hyper(xdr, &actual_num);
}

bool P_xdr_send(XDR *xdr, const int64_t &num) {
   assert(xdr->x_op == XDR_ENCODE);
   int64_t actual_num = num;
   return xdr_hyper(xdr, &actual_num);
}

#endif //!defined(i386_unknown_nt4_0)

bool P_xdr_send(XDR *xdr, const float &f) {
   assert(xdr->x_op == XDR_ENCODE);
   float actual_f = f;
   return xdr_float(xdr, &actual_f);
}

bool P_xdr_send(XDR *xdr, const double &d) {
   assert(xdr->x_op == XDR_ENCODE);
   double actual_d = d;
   return xdr_double(xdr, &actual_d);
}

bool P_xdr_send(XDR *xdr, const byteArray &ba) {
  unsigned length = ba.length();
  if (!P_xdr_send(xdr, length))
    return false;

  if (length > 0) {
    char *buffer = static_cast<char*>(const_cast<void*>(ba.getArray()));
    if (!xdr_bytes(xdr, &buffer, &length, ba.length()))
    return false;
  }

  return true;
}

bool P_xdr_send(XDR *xdr, const string &s) {
   unsigned len = s.length();
   if (!P_xdr_send(xdr, len))
      return false;

   if (len == 0)
      return true;
   
   char *buffer = const_cast<char*>(s.c_str());
      // xdr doesn't want to use "const char *" so we use "char *"

   if (!xdr_bytes(xdr, &buffer, &len, len + 1))
      return false;
   
   return true;
}

// ----------------------------------------------------------------------

bool P_xdr_recv(XDR *xdr, bool &b) {
   assert(xdr->x_op == XDR_DECODE);
   bool_t actual_b;
   const bool result = xdr_bool(xdr, &actual_b); 
   b = actual_b;
   return result;
}

bool P_xdr_recv(XDR *xdr, short &s) {
   assert(xdr->x_op == XDR_DECODE);
   return xdr_short(xdr, &s);
}

bool P_xdr_recv(XDR *xdr, unsigned short &us) {
   assert(xdr->x_op == XDR_DECODE);
   return xdr_u_short(xdr, &us);
}

#if defined(i386_unknown_nt4_0)

bool P_xdr_recv(XDR *xdr, int &num) {
   assert(xdr->x_op == XDR_DECODE);
   return xdr_int(xdr, &num);
}

bool P_xdr_recv(XDR *xdr, unsigned &num) {
   assert(xdr->x_op == XDR_DECODE);
   return xdr_u_int(xdr, &num);
}

bool P_xdr_recv(XDR *xdr, long &l) {
   assert(xdr->x_op == XDR_DECODE);
   return xdr_long(xdr, &l);
}

bool P_xdr_recv(XDR *xdr, unsigned long &ul) {
   assert(xdr->x_op == XDR_DECODE);
   return xdr_u_long(xdr, &ul);
}

#endif //defined(i386_unknown_nt4_0)

bool P_xdr_recv(XDR *xdr, uint32_t &num) {
   assert(xdr->x_op == XDR_DECODE);
   return xdr_u_int(xdr, (unsigned *)&num);
}

bool P_xdr_recv(XDR *xdr, int32_t &num) {
   assert(xdr->x_op == XDR_DECODE);
   return xdr_int(xdr, (int *)&num);
}

#if !defined(i386_unknown_nt4_0)

bool P_xdr_recv(XDR *xdr, uint64_t &num) {
   assert(xdr->x_op == XDR_DECODE);
   return xdr_u_hyper(xdr, &num);
}

bool P_xdr_recv(XDR *xdr, int64_t &num) {
   assert(xdr->x_op == XDR_DECODE);
   return xdr_hyper(xdr, &num);
}

#endif //!defined(i386_unknown_nt4_0)

bool P_xdr_recv(XDR *xdr, float &f) {
   assert(xdr->x_op == XDR_DECODE);
   return xdr_float(xdr, &f);
}

bool P_xdr_recv(XDR *xdr, double &d) {
   assert(xdr->x_op == XDR_DECODE);
   return xdr_double(xdr, &d);
}

bool P_xdr_recv(XDR *xdr, byteArray &ba) {
// reminder: as always for recv routines, the 2d arg is raw memory (no ctor has
// even been called!).  At least it's been allocated, though.
  unsigned length;
  if (!P_xdr_recv(xdr, length))
    return false;
  if (length == 0) {
    (void)new((void*)&ba)byteArray(NULL, 0);
    return true;
  }

  char *temp = new char[length];
  unsigned actual_len;
  if (!xdr_bytes(xdr, &temp, &actual_len, length)) {
    delete [] temp;
    return false;
  }
  else if (actual_len != length) {
    delete [] temp;
    return false;
  }
  else {
    (void)new((void*)&ba)byteArray(temp, actual_len);
    delete [] temp;
    return true;
  }
}

bool P_xdr_recv(XDR *xdr, string &s) {
   // as always "s" is uninitialized, unconstructed raw memory, so we need
   // to manually call the constructor.  As always, constructing a c++ object
   // in-place can best be done via the void* placement operator new.
   
   assert(xdr->x_op == XDR_DECODE);

   unsigned len;
   if (!P_xdr_recv(xdr, len))
      return false;
   
   if (len == 0) {
      (void)new((void*)&s)string();
      return true;
   }

   char *buffer = new char[len + 1];
   buffer[len] = '\0';

   unsigned size = len;
   if (!xdr_bytes(xdr, &buffer, &size, len+1))
      return false;

   //cout << "buffer is " << buffer << ", size is " << size << endl;
   
   (void)new((void*)&s)string(buffer, size);
   
   delete [] buffer;
   
   return true;
}

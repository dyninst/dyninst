#if !defined(_bgl_compute_headers_h)
#define _bgl_compute_headers_h

#include <string.h>
#include <errno.h>
#include <sys/time.h>

inline const char * P_strrchr (const char *P_STRING, int C) {return (strrchr(P_STRING, C));}
inline char * P_strrchr (char *P_STRING, int C) {return (strrchr(P_STRING, C));}

#endif // _bgl_compute_headers_h

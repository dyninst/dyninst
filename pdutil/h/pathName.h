// pathName.h

#ifndef _PATH_NAME_H_
#define _PATH_NAME_H_

#include "util/h/String.h"

string expand_tilde_pathname(const string &dir);
   // e.g. convert "~tamches/hello" to "/u/t/a/tamches/hello",
   // or convert "~/hello" to same.
   // In the spirit of Tcl_TildeSubst

#endif

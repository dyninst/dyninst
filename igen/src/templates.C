
#define KL_PRINT
#define KYL_PRINT

#pragma implementation "klist.h"
#include "util/h/klist.h"

#pragma implementation "keylist.h"
#include "util/h/keylist.h"

#pragma implementation "cstring.h"
#include "util/h/cstring.h"

#include "parse.h"

typedef KList<userDefn*>;
typedef KList<Cstring>;
typedef KList<remoteFunc*>;
typedef KList<char>;
typedef KList<classDefn*>;
typedef KList<argument*>;
typedef KList<pvm_args*>;

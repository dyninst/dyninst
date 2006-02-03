#ifndef __LOG_H__
#define __LOG_H__

#include <cstdarg>
using namespace std;

enum logLevel {
    ERR,
    WARN,
    INFO,
    VERB1,
    VERB2,
    VERB3,
    VERB4,
    DEBUG,
    VERBOSE_MAX,
    LOG_ONLY
};

void dlog(logLevel, char *, ...);

#endif

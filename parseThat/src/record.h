#ifndef __RECORD_H__
#define __RECORD_H__

#include "strlist.h"
#include "config.h"

struct record_t {
    bool enabled;
    strlist prog_line;
    strlist lib_line;
    char filename[ PATH_MAX ];
    FILE *fd;
    FILE *raw_fd;
};

record_t record_init();
bool record_create(record_t *record, const char *prog, int argc, char **argv);
bool record_search(record_t *newRecord);
void record_update(record_t *newRecord);
bool get_libs(strlist *, const char *);

#endif

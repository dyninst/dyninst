#ifndef __REGLIST_H__
#define __REGLIST_H__

#include <vector>
#include <regex.h>
#include <sys/types.h>

using namespace std;

enum rule_t {
    RULE_ONLY,
    RULE_SKIP
};

struct regrule_t {
    bool show_reason;
    const char *pattern;
    regex_t preg;
};

class reglist {
  private:
    vector< regrule_t * > only_rules;
    vector< regrule_t * > skip_rules;

  public:
    ~reglist();

    bool insert(const char *, rule_t, bool = true);
    bool isValid(const char *s);
    const char *getReason(char *s);

    static bool print_regerror(regex_t *, int);
};

#endif

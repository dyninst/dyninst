/************************************************************************
 * String.h: a simple character string class.
************************************************************************/





#if !defined(_String_h_)
#define _String_h_





/************************************************************************
 * header files.
************************************************************************/

#include <iostream.h>
#include "util/h/headers.h"




/************************************************************************
 * class string
************************************************************************/

class string {
public:
     string ();
     string (const char *);
     string (const string &);
    ~string ();

    string&       operator= (const char *);
    string&       operator= (const string &);
    string&      operator+= (const string &);
    string        operator+ (const string &)         const;

    bool         operator== (const string &)         const;
    bool         operator!= (const string &)         const;
    bool          operator< (const string &)         const;
    bool         operator<= (const string &)         const;
    bool          operator> (const string &)         const;
    bool         operator>= (const string &)         const;

    bool          prefix_of (const char *, unsigned) const;
    bool          prefix_of (const char *)           const;
    bool          prefix_of (const string &)         const;

    bool        prefixed_by (const char *, unsigned) const;
    bool        prefixed_by (const char *)           const;
    bool        prefixed_by (const string &)         const;

    const char*   string_of ()                       const;
    unsigned         length ()                       const;

    friend ostream& operator<< (ostream &os, const string &s) {
      return os << s.str_;
    }

    static unsigned       hash (const string &);

private:
    static unsigned      hashs (const char *);

    static unsigned     STRLEN (const char *);
    static char*        STRDUP (const char *);

    static bool          STREQ (const char *, const char *);
    static bool         STREQN (const char *, const char *, unsigned);
    static bool          STRNE (const char *, const char *);
    static bool          STRLT (const char *, const char *);
    static bool          STRLE (const char *, const char *);
    static bool          STRGT (const char *, const char *);
    static bool          STRGE (const char *, const char *);

    char*    str_;
    unsigned len_;
    unsigned key_;
};

inline
string::string()
    : str_(0), len_(0), key_(0) {
}

inline
string::string(const char* str)
    : str_(STRDUP(str)), len_(STRLEN(str)), key_(hashs(str)) {
}

inline
string::string(const string& s)
    : str_(STRDUP(s.str_)), len_(s.len_), key_(s.key_) {
}

inline
string::~string() {
    delete str_; str_ = 0;
}

inline
string&
string::operator=(const char* str) {
    if (str_ == str) {
        return *this;
    }

    delete str_; str_ = 0;

    str_ = STRDUP(str);
    len_ = STRLEN(str);
    key_ = hashs(str);

    return *this;
}

inline
string&
string::operator=(const string& s) {
    if (this == &s) {
        return *this;
    }

    delete str_; str_ = 0;

    str_ = STRDUP(s.str_);
    len_ = s.len_;
    key_ = s.key_;

    return *this;
}

inline
string&
string::operator+=(const string& s) {
    unsigned nlen = len_ + s.len_;
    char*    ptr  = new char[nlen+1];

    memcpy(ptr, str_, len_);
    memcpy(&ptr[len_], s.str_, s.len_);
    ptr[nlen] = '\0';

    delete[] str_; str_ = 0;
    str_ = ptr;
    len_ = nlen;
    key_ = hashs(str_);

    return *this;
}

inline
string
string::operator+(const string& s) const {
    string ret = *this;
    return ret += s;
}

inline
bool
string::operator==(const string& s) const {
    return ((&s == this)
        || ((key_ == s.key_)
        && (len_ == s.len_)
        && STREQ(str_, s.str_)));
}

inline
bool
string::operator!=(const string& s) const {
    return ((!(&s == this)) && (len_ != s.len_)
        || STRNE(str_, s.str_));
}

inline
bool
string::operator<(const string& s) const {
    return STRLT(str_, s.str_);
}

inline
bool
string::operator<=(const string& s) const {
    return ((&s == this) || STRLE(str_, s.str_));
}

inline
bool
string::operator>(const string& s) const {
    return STRGT(str_, s.str_);
}

inline
bool
string::operator>=(const string& s) const {
    return ((&s == this) || STRGE(str_, s.str_));
}

inline
bool
string::prefix_of(const char* s, unsigned sl) const {
    return ((len_ > sl) ? false : STREQN(str_, s, len_));
}

inline
bool
string::prefix_of(const char* s) const {
    return prefix_of(s, STRLEN(s));
}

inline
bool
string::prefix_of(const string& s) const {
    return ((&s == this) || prefix_of(s.str_, s.len_));
}

inline
bool
string::prefixed_by(const char* s, unsigned sl) const {
    return ((sl > len_) ? false : STREQN(str_, s, sl));
}

inline
bool
string::prefixed_by(const char* s) const {
    return prefixed_by(s, STRLEN(s));
}

inline
bool
string::prefixed_by(const string& s) const {
    return ((&s == this) || prefixed_by(s.str_, s.len_));
}

inline
unsigned
string::length() const {
    return len_;
}

inline
const char*
string::string_of() const {
    return str_;
}

inline
unsigned
string::hash(const string& s) {
    return s.key_;
}

inline
unsigned
string::hashs(const char* str) {
    if (!str) {
        return 0;
    }

    unsigned h = 5381;
    while (*str) {
        h = (h << 5) + h + (unsigned) (*str);
        str++;
    }
    return h;
}

inline
unsigned
string::STRLEN(const char* str) {
    return ((str)?(P_strlen(str)):(0));
}

inline
char*
string::STRDUP(const char* str) {
    if (!str) {
        return 0;
    }

    unsigned size = P_strlen(str)+1;
    char*    p    = new char[size];

    (void) P_memcpy(p, str, size);
    return p;
}

inline
bool
string::STREQ(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)==0):(!(s1||s2)));
}

inline
bool
string::STREQN(const char* s1, const char* s2, unsigned len) {
    return ((s1&&s2)?(P_strncmp(s1,s2,len)==0):(!(s1||s2)));
}

inline
bool
string::STRNE(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)!=0):(false));
}

inline
bool
string::STRLT(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)<0):(false));
}

inline
bool
string::STRLE(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)<=0):(!(s1||s2)));
}

inline
bool
string::STRGT(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)>0):(false));
}

inline
bool
string::STRGE(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)>=0):(!(s1||s2)));
}





#endif /* !defined(_String_h_) */

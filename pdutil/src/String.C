/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// String.C

#include <assert.h>
#include "util/h/String.h"

string::string()
    : str_(0), len_(0), key_(0) {
}

string::string(const char* str)
    : str_(STRDUP(str)), len_(STRLEN(str)), key_(hashs(str)) {
}

string::string(const char *str, unsigned len) {
   // same as above constructor, but copies less than the entire string.
   // You specifiy the # of chars to copy.
   bool aflag;
   aflag=(len <= strlen(str));
   assert(aflag);
   len_ = len;
   str_ = new char[len+1];
   (void) P_memcpy(str_, str, len);
   str_[len] = '\0';
   key_ = hashs(str_);
}

string::string(const string& s)
    : str_(STRDUP(s.str_)), len_(s.len_), key_(s.key_) {
}

string::string(int i) {
   char tempBuffer[40];
   sprintf(tempBuffer, "%d", i);

   str_ = STRDUP(tempBuffer);
   len_ = STRLEN(tempBuffer);
   key_ = hashs (tempBuffer);
}

string::string(unsigned u) {
   char tempBuffer[40];
   sprintf(tempBuffer, "%u", u);

   str_ = STRDUP(tempBuffer);
   len_ = STRLEN(tempBuffer);
   key_ = hashs (tempBuffer);
}

string::string(float f) {
   char tempBuffer[40];
   sprintf(tempBuffer, "%f", f);

   str_ = STRDUP(tempBuffer);
   len_ = STRLEN(tempBuffer);
   key_ = hashs (tempBuffer);
}

string::string(double d) {
   char tempBuffer[40];
   sprintf(tempBuffer, "%g", d);

   str_ = STRDUP(tempBuffer);
   len_ = STRLEN(tempBuffer);
   key_ = hashs (tempBuffer);
}

string::~string() {
    delete [] str_; str_ = 0;
}

string&
string::operator=(const char* str) {
    if (str_ == str) {
        return *this;
    }

    delete [] str_; str_ = 0;

    str_ = STRDUP(str);
    len_ = STRLEN(str);
    key_ = hashs(str);

    return *this;
}

string&
string::operator=(const string& s) {
    if (this == &s) {
        return *this;
    }

    delete [] str_; str_ = 0;

    str_ = STRDUP(s.str_);
    len_ = s.len_;
    key_ = s.key_;

    return *this;
}

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

string
string::operator+(const string& s) const {
    string ret = *this;
    return ret += s;
}

bool
string::operator==(const string& s) const {
    return ((&s == this)
        || ((key_ == s.key_)
        && (len_ == s.len_)
        && STREQ(str_, s.str_)));
}

bool
string::operator!=(const string& s) const {
    return ((!(&s == this)) && (len_ != s.len_)
        || STRNE(str_, s.str_));
}

bool
string::operator<=(const string& s) const {
    return ((&s == this) || STRLE(str_, s.str_));
}

bool
string::operator>=(const string& s) const {
    return ((&s == this) || STRGE(str_, s.str_));
}

bool
string::prefix_of(const char* s, unsigned sl) const {
    return ((len_ > sl) ? false : STREQN(str_, s, len_));
}

bool
string::prefix_of(const string& s) const {
    return ((&s == this) || prefix_of(s.str_, s.len_));
}

bool
string::prefixed_by(const char* s, unsigned sl) const {
    return ((sl > len_) ? false : STREQN(str_, s, sl));
}

bool
string::prefixed_by(const string& s) const {
    return ((&s == this) || prefixed_by(s.str_, s.len_));
}

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

unsigned
string::STRLEN(const char* str) {
    return ((str)?(P_strlen(str)):(0));
}

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

bool
string::STREQ(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)==0):(!(s1||s2)));
}

bool
string::STREQN(const char* s1, const char* s2, unsigned len) {
    return ((s1&&s2)?(P_strncmp(s1,s2,len)==0):(!(s1||s2)));
}

bool
string::STRNE(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)!=0):(false));
}

bool
string::STRLT(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)<0):(false));
}

bool
string::STRLE(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)<=0):(!(s1||s2)));
}

bool
string::STRGT(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)>0):(false));
}

bool
string::STRGE(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)>=0):(!(s1||s2)));
}

ostream& operator<< (ostream &os, const string &s) {
   return os << s.str_;
}



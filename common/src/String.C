/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: String.C,v 1.28 2004/03/23 01:11:54 eli Exp $

#include <assert.h>
#include "common/h/headers.h"

#if !(defined i386_unknown_nt4_0) && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
#include <regex.h>
#endif

#include "common/h/String.h"

/* This doesn't actually belong here. */
void dedemangle( const char * demangled, char * result ) {
	/* Lifted from Jeffrey Odom.  Code reformatted so
	   I could figure out how to eliminate compiler warnings.
	   Adjusted to handle spaces inside templates intelligently.
	   We cut off everything after the first l-paren, so we don't
	   need to worry about the space after the parameters but
	   before the 'const'. */
	const char * resultBegins = NULL;
	char * resultEnds = NULL;

	if (	demangled[0] == '(' &&
		strstr( demangled, "::" ) != NULL) {
		/* Local variable.  Strip off the opening ( :: ). */
		resultBegins = strrchr( demangled, ')' ) + 3;

		/* End it at the right-most space, if any. */
		resultEnds = strrchr( resultBegins, ' ' );
		if( resultEnds != NULL ) { * resultEnds = '\0'; }
		}
	else if ( strrchr( demangled, '(' ) != NULL ) {
		/* Strip off return types, if any.  Be careful not to
		   pull off [template?/]class/namespace information.
	
		   The only space that matters is the one that's _not_
		   inside a template, so skip the templates and cut at the
		   first space.  We can ignore 'operator[<[<]|>[>]]' because
		   we'll stop before we reach it.
		   
		   Caveat: conversion operators (e.g., "operator bool") have
		   spaces in the function name.  Right now we deal with this
		   specifically (is the function "operator *"?).  Could be
		   altered to after the last template but before the last
		   left parenthesis.  (Instead of next, for "operator ()".)
		  */

		resultBegins = demangled;
		int stack = 0; bool inTemplate = false;
		unsigned int offset = 0;
		int lastColon = 0;
		for( offset = 0; offset < strlen( resultBegins ); offset++ ) {
			if( resultBegins[offset] == '<' ) {
				stack++;
				inTemplate = true;
				}
			if( resultBegins[offset] == '>' ) {
				stack--;
				if( stack == 0 ) { inTemplate = false; }
				}
			if( !inTemplate && resultBegins[offset] == '(' ) {
				/* We've stumbled on something without a return value. */
				offset = 0;
				resultBegins = demangled;
				break;
			        }
			if( !inTemplate && resultBegins[offset] == ' ' ) {
				/* FIXME: verify that the space isn't in the function name,
				   e.g., 'operator bool'.  If the first space we meet _is_
				   a function name, it doesn't have a(n explicit) return type. */
				if( strstr( &(resultBegins[ lastColon + 1 ]), "operator " ) == resultBegins + lastColon + 1 ) {
					resultBegins = demangled;
					offset = 0;
					}
				else{
					resultBegins = &(resultBegins[offset + 1]);
					offset++;
					}

				break;
				}
			if( !inTemplate && resultBegins[offset] == ':' ) {
				lastColon = offset;
				}
			} /* end template elimination loop */

		/* Scan past the function name; the first left parenthesis
		   not in in a template declaration starts the function arguments. */
		stack = 0; inTemplate = false;
		for( ; offset < strlen( resultBegins ); offset++ ) {
			if( resultBegins[offset] == '<' ) {
				stack++;
				inTemplate = true;
				}
			if( resultBegins[offset] == '>' ) {
				stack--;
				if( stack == 0 ) { inTemplate = false; }
				}
			if( !inTemplate && resultBegins[offset] == '(' ) {
				resultEnds = const_cast<char *>(&(resultBegins[offset]));
				* resultEnds = '\0';
				break;
				} 
			} /* end template elimination loop */
		} /* end if a function prototype */
	else {
		/* Assume demangle OK. */
		resultBegins = demangled;
		}

	strcpy( result, resultBegins );
	} /* end dedemangle */

// Declare static member vrbles:
pdstring *pdstring::nilptr = NULL;
int string_counter::count = 0;

string_ll::string_ll()
    : str_(0), len_(0), key_(0) {
}

string_ll::string_ll(const char* str)
    : str_(STRDUP(str)), len_(STRLEN(str)) {
   key_ = 0; // lazy key define
}

string_ll::string_ll(const char *str, unsigned len) {
   // same as above constructor, but copies less than the entire string.
   // You specifiy the # of chars to copy.
   if (len > strlen(str))
      // just copy the whole string
      len = strlen(str);

   len_ = len;
   str_ = new char[len+1];
   (void) P_memcpy(str_, str, len);
   str_[len] = '\0';

   key_ = 0; // lazy key define
}

string_ll::string_ll(const string_ll& s)
    : str_(STRDUP(s.str_)), len_(s.len_), key_(s.key_) {
   // lazy key define iff "s" lazy key define (as it should be)
}

string_ll::string_ll(const char c) {
  char tempBuffer[2]; //only need space for one character and '\0'
  sprintf(tempBuffer, "%c", c);

  str_ = STRDUP(tempBuffer);
  len_ = STRLEN(tempBuffer);

  key_ = 0; // lazy key define
}

string_ll::string_ll(int i) {
   char tempBuffer[40];
   sprintf(tempBuffer, "%d", i);

   str_ = STRDUP(tempBuffer);
   len_ = STRLEN(tempBuffer);

   key_ = 0; // lazy key define
}

string_ll::string_ll(long l) {
   char tempBuffer[40];
   sprintf(tempBuffer, "%ld", l);

   str_ = STRDUP(tempBuffer);
   len_ = STRLEN(tempBuffer);

   key_ = 0; // lazy key define
}

string_ll::string_ll(unsigned u) {
   char tempBuffer[40];
   sprintf(tempBuffer, "%u", u);

   str_ = STRDUP(tempBuffer);
   len_ = STRLEN(tempBuffer);

   key_ = 0; // lazy key define
}

string_ll::string_ll(unsigned long ul) {
   char tempBuffer[40];
   sprintf(tempBuffer, "%lu", ul);

   str_ = STRDUP(tempBuffer);
   len_ = STRLEN(tempBuffer);

   key_ = 0; // lazy key define
}

string_ll::string_ll(float f) {
   char tempBuffer[40];
   sprintf(tempBuffer, "%f", f);

   str_ = STRDUP(tempBuffer);
   len_ = STRLEN(tempBuffer);

   key_ = 0; // lazy key define
}

string_ll::string_ll(double d) {
   char tempBuffer[40];
   sprintf(tempBuffer, "%g", d);

   str_ = STRDUP(tempBuffer);
   len_ = STRLEN(tempBuffer);

   key_ = 0; // lazy key define
}

string_ll::~string_ll() { 
   delete [] str_;
   str_ = 0;
}

string_ll&
string_ll::operator=(const char* str) {
    if (str_ == str) {
        return *this;
    }

    delete [] str_; str_ = 0;

    str_ = STRDUP(str);
    len_ = STRLEN(str);

    key_ = 0; // lazy key define

    return *this;
}

string_ll&
string_ll::operator=(const string_ll& s) {
    if (this == &s) {
        return *this;
    }

    delete [] str_; str_ = 0;

    str_ = STRDUP(s.str_);
    len_ = s.len_;
    key_ = s.key_; // lazy key define iff "s" lazy key define, which is correct

    return *this;
}

string_ll&
string_ll::operator+=(const string_ll& s) {
    unsigned nlen = len_ + s.len_;
    char*    ptr  = new char[nlen+1];
    assert(ptr);

    memcpy(ptr, str_, len_);
    memcpy(&ptr[len_], s.str_, s.len_);
    ptr[nlen] = '\0';

    delete[] str_; str_ = 0;
    str_ = ptr;
    len_ = nlen;

    key_ = 0;

    return *this;
}

string_ll&
string_ll::operator+=(const char *ptr) {
   // this routine exists as an optimization, sometimes avoiding the need to create
   // a temporary string, which can be expensive.

   const int ptr_len = P_strlen(ptr);
   const unsigned nlen = len_ + ptr_len;
   char *new_ptr = new char[nlen+1];
   assert(new_ptr);

   memcpy(new_ptr, str_, len_);
   memcpy(&new_ptr[len_], ptr, ptr_len);
   new_ptr[nlen] = '\0';
  
   delete [] str_;
   str_ = new_ptr;
   len_ = nlen;

   key_ = 0; // lazy key define

   return *this;
}

string_ll
string_ll::operator+(const string_ll& s) const {
    string_ll ret = *this;
    return (ret += s);
}

string_ll
string_ll::operator+(const char *ptr) const {
   string_ll ret = *this;
   return (ret += ptr);
}

bool
string_ll::operator==(const string_ll& s) const {
   if (&s == this) return true;

   updateKeyIfNeeded(); s.updateKeyIfNeeded();
   if (key_ != s.key_) return false;
   if (len_ != s.len_) return false;
   return STREQ(str_, s.str_);
}

bool
string_ll::operator!=(const string_ll& s) const {
   if (&s == this) return false;
   if (len_ != s.len_) return true;
   return STRNE(str_, s.str_);
}

bool
string_ll::operator<=(const string_ll& s) const {
    return ((&s == this) || STRLE(str_, s.str_));
}

bool
string_ll::operator>=(const string_ll& s) const {
    return ((&s == this) || STRGE(str_, s.str_));
}

bool
string_ll::prefix_of(const char* s, unsigned sl) const {
    return ((len_ > sl) ? false : STREQN(str_, s, len_));
}

bool
string_ll::prefix_of(const string_ll& s) const {
    return ((&s == this) || prefix_of(s.str_, s.len_));
}

bool
string_ll::prefixed_by(const char* s, unsigned sl) const {
    return ((sl > len_) ? false : STREQN(str_, s, sl));
}

bool
string_ll::prefixed_by(const string_ll& s) const {
    return ((&s == this) || prefixed_by(s.str_, s.len_));
}

bool
string_ll::suffix_of(const char* s, unsigned sl) const {
    return ((len_ > sl) ? false : STREQN(str_, s + strlen( s ) - len_, len_));
}

bool
string_ll::suffix_of(const string_ll& s) const {
    return ((&s == this) || suffix_of(s.str_, s.len_));
}

bool
string_ll::suffixed_by(const char* s, unsigned sl) const {
    return ((sl > len_) ? false : STREQN(str_ + len_ - sl, s, sl));
}

bool
string_ll::suffixed_by(const string_ll& s) const {
    return ((&s == this) || suffixed_by(s.str_, s.len_));
}


unsigned
string_ll::find (const char *s, unsigned sl) const {
  for(unsigned int i=0; i<=(len_-sl); i++) {
    if( STREQN(str_ + i, s, sl) ) return i;
  }
  return len_;
}

unsigned 
string_ll::find (const string_ll &s) const{
  return ((&s == this) ? 0 : find(s.str_, s.len_)); 
}

unsigned
string_ll::hashs(const char* str) {
    if (!str)
       return 1; // 0 is reserved for unhashed key

    unsigned h = 5381;
    while (*str) {
        h = (h << 5) + h + (unsigned) (*str);
        str++;
    }
    return h==0 ? 1 : h; // 0 is reserved for unhashed key
}

unsigned
string_ll::STRLEN(const char* str) {
    return ((str)?(P_strlen(str)):(0));
}

char*
string_ll::STRDUP(const char* str) {
    if (!str) {
        return 0;
    }

    unsigned size = P_strlen(str)+1;
    char*    p    = new char[size];

    (void) P_memcpy(p, str, size);
    return p;
}

bool
string_ll::STREQ(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)==0):(!(s1||s2)));
}

bool
string_ll::STREQN(const char* s1, const char* s2, unsigned len) {
    return ((s1&&s2)?(P_strncmp(s1,s2,len)==0):(!(s1||s2)));
}

bool
string_ll::STRNE(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)!=0):(false));
}

bool
string_ll::STRLT(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)<0):(false));
}

bool
string_ll::STRLE(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)<=0):(!(s1||s2)));
}

bool
string_ll::STRGT(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)>0):(false));
}

bool
string_ll::STRGE(const char* s1, const char* s2) {
    return ((s1&&s2)?(P_strcmp(s1,s2)>=0):(!(s1||s2)));
}

const char *
string_ll::STRCHR(const char* s, char c) {
    return (s?(P_strchr(s,c)):(NULL));
}

string_ll
string_ll::substr(unsigned pos, unsigned len) const {
	if( pos >= len_ )
		return string_ll( "" );
	else
		return string_ll( str_ + pos, len );
}

bool
string_ll::wildcardEquiv( const string_ll &them, bool checkCase ) const {
	if( *this == them )
		return true;
	else
		return pattern_match( str_, them.str_, checkCase );
}


// This function will match string s against pattern p.
// Asterisks match 0 or more wild characters, and a question
// mark matches exactly one wild character.  In other words,
// the asterisk is the equivalent of the regex ".*" and the
// question mark is the equivalent of "."

bool
string_ll::pattern_match( const char *p, const char *s, bool checkCase ) {
	//const char *p = ptrn;
	//char *s = str;

	while ( true ) {
		// If at the end of the pattern, it matches if also at the end of the string
		if( *p == '\0' )
			return ( *s == '\0' );

		// Process a '*'
		if( *p == MULTIPLE_WILDCARD_CHAR ) {
			++p;
			
			// If at the end of the pattern, it matches
			if( *p == '\0' )
				return true;

			// Try to match the remaining pattern for each remaining substring of s
			for(; *s != '\0'; ++s )
				if( pattern_match( p, s, checkCase ) )
					return true;
			// Failed
			return false;
		}

		// If at the end of the string (and at this point, not of the pattern), it fails
		if( *s == '\0' )
			return false;

		// Check if this character matches
		bool matchChar = false;
		if( *p == WILDCARD_CHAR || *p == *s )
			matchChar = true;
		else if( !checkCase ) {
			if( *p >= 'A' && *p <= 'Z' && *s == ( *p + ( 'a' - 'A' ) ) )
				matchChar = true;
			else if( *p >= 'a' && *p <= 'z' && *s == ( *p - ( 'a' - 'A' ) ) )
				matchChar = true;
		}

		if( matchChar ) {
			++p;
			++s;
			continue;
		}

		// Did not match
		return false;
	}
}


// Use POSIX regular expression pattern matching to check if string s matches
// the pattern in this string
bool
string_ll::regexEquiv( const char *s, bool checkCase ) const {
// Would this work under NT?  I don't know.
#if !(defined i386_unknown_nt4_0) && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
	regex_t r;
	int err;
	bool match = false;
	int cflags = REG_NOSUB;
	if( !checkCase )
		cflags |= REG_ICASE;

	// Regular expressions must be compiled first, see 'man regexec'
	err = regcomp( &r, str_, cflags );

	if( err == 0 ) {
		// Now we can check for a match
		err = regexec( &r, s, 0, NULL, 0 );
		if( err == 0 )
			match = true;
	}

	// Deal with errors
	if( err != 0 && err != REG_NOMATCH ) {
		char errbuf[80];
		regerror( err, &r, errbuf, 80 );
		cerr << "string_ll::regexEquiv -- " << errbuf << endl;
	}

	// Free the pattern buffer
	regfree( &r );
	return match;
#else
	return false;
#endif
}


ostream& operator<< (ostream &os, const string_ll &s) {
   if(s.str_ != NULL)
      os << s.str_;
   return os;
}

debug_ostream& operator<< (debug_ostream &os, const string_ll &s) {
   if(s.str_ != NULL)
      os << s.str_;
   return os;
}

pdstring operator+(const char *ptr, const pdstring &str) {
   // a syntactical convenience.
   // This fn could probably be optimized quite a bit (pre-allocate exactly
   // the # of bytes that are needed)
   pdstring result(ptr);
   result += str;
   return result;
}

void pdstring::initialize_static_stuff() {
   // should only get called once:
   // VG(06/15/02): this assertion doesn't hold w/VC.NET because nilptr is never initialized!
   //assert(nilptr == NULL);

   nilptr = new pdstring((char*)NULL);
      // the typecast is essential, lest NULL be interpreted
      // as the integer 0 instead of the pointer 0!
}

void pdstring::free_static_stuff() {
   delete nilptr;
   nilptr = NULL;
}




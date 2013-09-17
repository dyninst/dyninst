/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 *  String ADT.
 *  Implements copy semantics. 
 *  This class has been purified.
 *  This class can be used with klist.
 *  See the tests subdirectory for examples of its use.
 */

/*
 * $Log: cstring.h,v $
 * Revision 1.1  1994/08/17 18:23:46  markc
 * Added new classes: Cstring KeyList, KList
 * Added new function: RPCgetArg
 * Changed typedefs in machineType.h to #defines
 *
 */

#ifndef _string_hpp
#define _string_hpp

#include <string.h>
#include <assert.h>
#include <iostream.h>

#define VALID(x) assert(x)
#define MAX_STRING_LEN 2000
#define CS_TRUE 1
#define CS_FALSE 0

#pragma interface

typedef int CS_BOOL;

class Cstring
{
public:
friend ostream &operator << (ostream& os, const Cstring& S) {
  if (!S.len)
    os << "<empty>";
  else 
    os << S.str;
  return os;
}

  // the integer serves to choose the correct constructor
  // otherwise you would have to typecast to const char
  // to get other constructor
  Cstring(int i, char *use_me) :len(0), str((char*) 0) {use(use_me);}
  Cstring() : len(0), str((char*) 0) { ; }
  Cstring(const char *copy_me) : len(0), str((char*) 0) {set(copy_me);}
  Cstring(const Cstring& S) : len(0), str((char*) 0) {set(S);}
  ~Cstring() {destroy();}
  void destroy() {if (str) delete [] str; len=0; str= (char*) 0;}

  CS_BOOL extend (const char *);
  CS_BOOL extend (Cstring &S) {return (extend(S.str));}
  Cstring operator+ ( const Cstring& S) const;
  Cstring operator+ ( const char *s) const;
  Cstring& operator= ( const Cstring& S);
  Cstring& operator= ( const char *c);
  CS_BOOL operator== (const Cstring& S) const { return (*this == S.str);}
  CS_BOOL operator== (const char * s) const;
  CS_BOOL operator< (const char * s) const;
  CS_BOOL operator< (const Cstring& S) const {return ((*this) < S.str);}
  CS_BOOL operator> (const char *s) const;
  CS_BOOL operator> (const Cstring& S) const {return ((*this) > S.str);}

  const char *get() const {return (const char*) str;}
  char *get_copy() const;
  int getLen() const {return len;}
  // copy the string from the input
  void set(const Cstring& S);
  void set(const char *s);

  // use the string in the input
  void use(char *s);

  int invariant()
    {return (((len>=0) && str) || (!len && !str));}

  int empty() {return (len == 0);}

protected:
  int len;
  char *str;
  int prefix_compare (const char *s, int &slen) const;
  int prefix_compare (const Cstring& S, int &slen) const
    {return prefix_compare(S.str, slen);}
};

void 
Cstring::use (char *use_me)
{
  if (str)
    delete [] str;
  str = (char *)  0;

  if (!use_me) {
    len = 0;
    return;
  } else {
    len = strlen(use_me);
    if (len >= 0) {
      str = use_me;
      return;
    } else {
      len = 0;
      return;
    }
  }
}

void Cstring::set(const Cstring& S)
{
  set(S.str);
}

void
Cstring::set (const char *s)
{
  if (str) 
    delete [] str;
  str = (char*) 0;

  if (!s) {
    len = 0;
    return;
  }

  len = strlen(s);
  if (len >= 0) {
    str = new char [len + 1];
    if (str)
      strcpy (str, s);
    else
      len = 0;
  } else
    len = 0;

  VALID(invariant());
}

int Cstring::operator==(const char *s) const
{
  if ((len == strlen(s)) &&
      ((!len) || (!strcmp(str, s))))
    return CS_TRUE;
  else
    return 0;
}

//
// compare up to the length of the smaller string
// used for less than
//
int Cstring::prefix_compare(const char *s, int &slen) const
{
  if (!len || !s)
    return -2;

  slen = strlen(s);
  return (strncmp(str, s, slen > len ? len : slen));
}

Cstring Cstring::operator+ ( const Cstring& S) const return Ret;
{
  Ret = (*this + S.str);
  return Ret;
}

Cstring Cstring::operator + (const char *s) const return Ret;
{
  Ret = (*this);
  Ret.extend(s);
  return Ret;
}

CS_BOOL Cstring::extend (const char *s)
{
  char *new_str;
  int new_len;

  if (s) {
    if (new_len = strlen(s)) {
      if (new_str = new char[len + new_len + 1]) {
	if (strcpy(new_str, str) && strcat(new_str, s)) {
	  if (str)
	    delete (str);
	  len += new_len;
	  str = new_str;
	  return (CS_TRUE);
	}
      }
    }
  } else
    return (CS_TRUE);

  return (CS_FALSE);
}
 
Cstring& Cstring::operator= ( const Cstring& S) 
{
  if (this == &S)
    return (*this);
  else {
    set(S);
    return (*this);
  }
}

Cstring& Cstring::operator= ( const char *c) 
{
  if (str == c)
    return (*this);
  else {
    set(c);
    return (*this);
  }
}

CS_BOOL Cstring::operator< (const char * s) const { 
  int slen, res;

  if ((res = prefix_compare(s, slen)) == -2)
    return CS_FALSE;
  else if (res < 0)
    return CS_TRUE;
  else if (res > 0)
    return CS_FALSE;
  else
    return (len < slen);
}

CS_BOOL Cstring::operator> (const char *s) const {
  int slen, res;

  if ((res = prefix_compare(s, slen)) == -2)
    return CS_TRUE;
  else if (res < 0)
    return (len > slen);
  else if (res > 0)
    return CS_TRUE;
  else
    return (len > slen);
}

char *Cstring::get_copy() const
{
  char *ret= (char*) 0;
  if (str) 
    ret = strdup(str);
  return ret;
}


#endif    


  
      

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

/************************************************************************
 * AIX object files.
 * $Id: Object-xcoff.h,v 1.1 2000/11/21 20:23:59 bernat Exp $
************************************************************************/





#if !defined(_Object_aix_h_)
#define _Object_aix_h_





/************************************************************************
 * header files.
************************************************************************/

#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "common/h/Symbol.h"
#include "common/h/Types.h"
#include "common/h/Vector.h"

extern "C" {
#include <a.out.h>
};

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

class fileDescriptor_AIX : public fileDescriptor {
 public:
  fileDescriptor_AIX():fileDescriptor(), member_(0), data_(0), pid_(0) {}
  fileDescriptor_AIX(string file):fileDescriptor(file), member_(0),
    data_(0), pid_(0) {}
  fileDescriptor_AIX(string file, string member,
		     Address text, Address data,
		     unsigned pid) :
    fileDescriptor(file, text), member_(member), data_(data), pid_(pid) {}
  fileDescriptor_AIX(const fileDescriptor_AIX &fda) :
    fileDescriptor(fda.file_, fda.addr_),
    member_(fda.member_), data_(fda.data_), pid_(fda.pid_) {}
  ~fileDescriptor_AIX() {}

  const string &member() const { return member_; }
  Address data() const { return data_; }
  unsigned pid() const { return pid_; }

 private:
  string member_;
  Address data_;
  unsigned pid_;
};


#define __AR_BIG__
#define __AR_SMALL__
#include <ar.h>
// Object to represent both the 32-bit and 64-bit archive headers
// for ar files (libraries)
class Archive {
 public:
  Archive (int file) : member_name(0),fd(file) {}
  virtual ~Archive () {}
  
  virtual int read_arhdr() = 0;
  virtual int read_mbrhdr() = 0;

  unsigned long long aout_offset;
  unsigned long long next_offset;
  char *member_name;
  int member_len;
 protected:
  unsigned long long first_offset;
  unsigned long long last_offset;
  int fd;
};

class Archive_32 : private Archive {
 public:
  Archive_32 (int file) : Archive(file) {};
  ~Archive_32 () {if (member_name) free(member_name);};
  virtual int read_arhdr();
  virtual int read_mbrhdr();
  
 private:  
  struct fl_hdr filehdr;
  struct ar_hdr memberhdr;
};

class Archive_64 : private Archive {
 public:
  Archive_64 (int file) : Archive(file) {};
  ~Archive_64 () {if (member_name) free(member_name);};
  virtual int read_arhdr();
  virtual int read_mbrhdr();

 private:  
  struct fl_hdr_big filehdr;
  struct ar_hdr_big memberhdr;
};

/************************************************************************
 * class Object
************************************************************************/

class Object : public AObject {
public:
             Object (const string, void (*)(const char *) = log_msg);
             Object (const Object &);
	     Object (const string, const Address baseAddr,
                void (*)(const char *) = log_msg);
	     Object (fileDescriptor *desc,
                void (*)(const char *) = log_msg);
    ~Object ();

    Object&   operator= (const Object &);
    int getTOCoffset() const { return toc_offset_; }
    void get_stab_info(char *&stabstr, int &nstabs, Address &stabs, char *&stringpool) {
	stabstr = (char *) stabstr_;
	nstabs = nstabs_;
	stabs = stabs_;
	stringpool = (char *) stringpool_;
    }
    void get_line_info(int& nlines, char*& lines,unsigned long& fdptr){
	nlines = nlines_;
	lines = (char*) linesptr_; 
	fdptr = linesfdptr_;
    }

private:
    void load_object ();
    void load_archive(int fd);
    void parse_aout(int fd, int offset);

    string member_;
    int  toc_offset_;
    int  nstabs_;
    int  nlines_;
    Address stabstr_;
    Address stabs_;
    Address stringpool_;
    Address linesptr_;
    Address linesfdptr_;
    // Offset to actual start of text segment (for relocation)
    Address text_org_;
    // Offset to actual start of data segment (for relocation)
    Address data_org_;
    // Process PID (used for ptrace_read)
    unsigned pid_;    
};



#endif /* !defined(_Object_aix_h_) */

/*
 * Classes and prototypes for Abstraction specific code
 * $Log: DMabstractions.h,v $
 * Revision 1.4  1995/06/02 20:48:13  newhall
 * * removed all pointers to datamanager class objects from datamanager
 *    interface functions and from client threads, objects are now
 *    refered to by handles or by passing copies of DM internal data
 * * removed applicationContext class from datamanager
 * * replaced List and HTable container classes with STL containers
 * * removed global variables from datamanager
 * * remove redundant lists of class objects from datamanager
 * * some reorginization and clean-up of data manager classes
 * * removed all stringPools and stringHandles
 * * KLUDGE: there are PC friend members of DM classes that should be
 *    removed when the PC is re-written
 *
 * Revision 1.3  1995/02/16  08:09:24  markc
 * Made char* args const char*
 *
 * Revision 1.2  1994/10/25  19:01:37  karavan
 * Protected this header file
 *
 * Revision 1.1  1994/09/30  19:17:42  rbi
 * Abstraction interface change.
 *
 */

#ifndef _abstract_h
#define _abstract_h
#include "util/h/Dictionary.h"
#include "util/h/String.h"

class abstraction {
    friend abstraction *AMfind(const char *aname);
  public:
    const char *getName() { return (name.string_of()); }
    void print();
    abstraction(const char *a);
    ~abstraction();
  private:
    string name; // id for abstraction
    static dictionary_hash<string, abstraction*> allAbstractions;
};

abstraction *AMfind(char *aname);

void AMnewMapping(const char *abstraction, const char *type,
		  const char *key, const char *value);
void AMnewResource(const char *parent, const char *name, const char *abstraction);

#endif

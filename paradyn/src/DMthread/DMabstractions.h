/*
 * Classes and prototypes for Abstraction specific code
 * $Log: DMabstractions.h,v $
 * Revision 1.3  1995/02/16 08:09:24  markc
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

class abstraction {
    friend abstraction *AMfind(const char *aname);
  public:
    stringHandle getName() { return (name); }
    void print();
    abstraction(const char *name);
    ~abstraction();
  private:
    stringHandle name;
    static stringPool names;
    static HTable<abstraction*> allAbstractions;
};

abstraction *AMfind(char *aname);

void AMnewMapping(const char *abstraction, const char *type,
		  const char *key, const char *value);
void AMnewResource(const char *parent, const char *name, const char *abstraction);

#endif

/*
 * Classes and prototypes for Abstraction specific code
 * $Log: DMabstractions.h,v $
 * Revision 1.1  1994/09/30 19:17:42  rbi
 * Abstraction interface change.
 *
 */

class abstraction {
    friend abstraction *AMfind(char *aname);
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

void AMnewMapping(char *abstraction, char *type, char *key, char *value);
void AMnewResource(char *parent, char *name, char *abstraction);

/*
 * parse.h - define the classes that are used in parsing an interface.
 *
 * $Log: parse.h,v $
 * Revision 1.4  1994/02/08 00:17:56  hollings
 * Fixed pointer problems to work on DECstations.
 *
 * Revision 1.3  1994/01/31  20:05:59  hollings
 * Added code to check the protocol name and version tests are run
 * before any upcalls get invoked.
 *
 * Revision 1.2  1994/01/26  06:50:11  hollings
 * made the output of igen pass through g++ -Wall.
 *
 * Revision 1.1  1994/01/25  20:48:44  hollings
 * new utility for interfaces.
 *
*/

#include "util/h/list.h"
#include "util/h/stringPool.h"

class typeDefn;		// forward decl
class remoteFunc;

extern stringPool pool;
extern List<typeDefn*> types;

class interfaceSpec {
  public:
    interfaceSpec(char *n, int v, int lowTag) {
	name = n;
	version = v;
	baseTag = boundTag = lowTag;
    }
    void newMethod(remoteFunc*f) { methods.add(f); }
    void genClass();
    void generateStubs();
    void generateClientCode();
    void generateThreadLoop();
    void generateXDRLoop();
    void generateServerCode();
    void genWaitLoop();
    void genProtoVerify();
    void generateBundlers();
    void genIncludes();
    int getNextTag();
    char *genVariable();
    char *getName() { return(name); }
    int getVersion() { return(version); }
  private:
    char *name;
    char *unionName;
    int version;
    int baseTag;
    int boundTag;
    List <remoteFunc *> methods;
};

extern class interfaceSpec *currentInterface;

class argument {
  public:
    argument(char *t, char *n,  List<char *> *s) {
	type = t;
	name = n;
	stars = s;
    }
    argument(char *t) {
	type = t;
	name = currentInterface->genVariable();
    }
    char *type;
    char *name;
    List<char *> *stars;
};

class field {
  public:
      field(char *t, char *n) {
	  type = t;
	  name = n;
      }
      char *getName() { return(name); }
      char *getType() { return(type); }
      void genBundler() {
	  printf("    xdr_%s(__xdrs__, &(__ptr__->%s));\n", type, name);
      }
      void genHeader() {
	  printf("    %s %s;\n", type, name);
      }
  private: 
      char *type;
      char *name;
};

class typeDefn {
   public:
       typeDefn(char *i, List<field *> f) {
	   name = pool.findAndAdd(i);
	   fields = f;
	   userDefined = TRUE;
	   arrayType = FALSE;
	   types.add(this, name);
       }
       typeDefn(char *i) {
	   name = pool.findAndAdd(i);
	   userDefined = FALSE;
	   arrayType = FALSE;
	   types.add(this, name);
       }
       typeDefn(char *i, char *t) {	// for arrays types.
	   name = pool.findAndAdd(i);
	   userDefined = TRUE;
	   arrayType = TRUE;
	   type = t;
	   types.add(this, name);
       }
       char *name;
       char *type;
       Boolean userDefined;
       Boolean arrayType;
       List<field *> fields;
       void genHeader();
       void genBundler();
};

enum upCallType { notUpcall, syncUpcall, asyncUpcall };

class interfaceSpec;

union parseStack {
    char *cp;
    int i;
    float f;
    argument *arg;
    field *fld;
    enum upCallType uc;
    List<argument*> *args;
    List<field*> *fields;
    List<char*> *cl;
    interfaceSpec *spec;
};


class remoteFunc {
  public:
      remoteFunc(interfaceSpec *sp,
		 List<char *> *st, 
		 char *n, char *r, 
		 List <argument *> a, 
		 enum upCallType uc) {
	  spec = sp;
	  name = n;
	  retType = r;
	  structName = spec->genVariable();
	  args = a;
	  upcall = uc;
      }
      void genSwitch(Boolean);
      void genStub(char *interfaceName, Boolean forUpcalls);
      void genXDRStub(char *);
      void genThreadStub(char *);
      void genHeader();
      void genMethodHeader(char *className);
      char *name;
      char *retType;
      char *structName;
      List <argument *> args;
      enum upCallType upcall;
  private:
      interfaceSpec *spec;
};


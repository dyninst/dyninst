/*
 * parse.h - define the classes that are used in parsing an interface.
 *
 * $Log: parse.h,v $
 * Revision 1.11  1994/08/18 05:56:57  markc
 * Changed char*'s to stringHandles
 *
 * Revision 1.10  1994/08/17  17:52:00  markc
 * Added Makefile for Linux.
 * Support new source files for igen.
 * Separate source files.  ClassDefns supports classes, typeDefns supports
 * structures.
 *
 * Revision 1.8  1994/06/02  23:34:27  markc
 * New igen features: error checking, synchronous upcalls.
 *
 * Revision 1.7  1994/04/01  04:58:10  markc
 * Added checks in genBundler.
 *
 * Revision 1.6  1994/03/07  02:35:18  markc
 * Added code to detect failures for xdr code.  Provides member instance
 * callErr which is set to -1 on failures.
 *
 * Revision 1.5  1994/02/24  05:14:33  markc
 * Man page for igen.
 * Initial version for solaris2.2.
 * Dependencies changed.
 * Added pvm support, virtual function support, inclusion of data members,
 * separate client and header include files.
 *
 * Revision 1.4  1994/02/08  00:17:56  hollings
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

#include "util/h/stringPool.h"
#include "util/h/list.h"

#include <iostream.h>
#include <fstream.h>

//  the files to write the generated code to
extern ofstream dot_h;
extern ofstream dot_c;
extern ofstream srvr_dot_c;
extern ofstream srvr_dot_h;
extern ofstream clnt_dot_c;
extern ofstream clnt_dot_h;

typedef enum { USER_DEFN, CLASS_DEFN, TYPE_DEFN } TYPE_ENUM;
typedef enum { ptrIgnore, ptrDetect, ptrHandle } PTR_TYPE;
extern PTR_TYPE ptrMode;

class userDefn;         // base for user defined types
class typeDefn;		// forward decl
class classDefn;        // forward decl
class remoteFunc;
class argument;
class field;

extern void dump_to_dot_h(char*);

void addCMember (stringHandle type, stringHandle name, char *stars);
void addSMember (stringHandle type, stringHandle name, char *stars);

extern void buildPVMfilters();
extern void buildPVMincludes();
extern void buildPVMargs();
extern void buildFlagHeaders();

extern void genPVMServerCons(stringHandle );
extern void genXDRServerCons(stringHandle );

enum upCallType { syncUpcall,       // from server to client, sync (not allowed)
		  asyncUpcall,      // from server to client, async
		  syncCall,         // from client to server, sync
		  asyncCall         // from client to server, async
		  };

class interfaceSpec;

typedef struct type_data {
  stringHandle cp;
  int mallocs;
  int structs;
} type_data;

typedef struct pvm_args {
  stringHandle type_name;
  stringHandle pvm_name;
  stringHandle arg;
} pvm_args;


typedef struct func_data {
  enum upCallType uc;
  int virtual_f;
} func_data;

class argument {
  public:
    argument(stringHandle t, stringHandle n, char *s, int m);
    stringHandle type;
    stringHandle name;
    int mallocs;
    char *stars;

    IsAPtr() { return 1;}
};

class field {
  public:
      field(stringHandle t, stringHandle n);
      stringHandle getName() { return(name); }
      stringHandle getType() { return(type); }
      void genBundler(ofstream &ofile, char *obj="&(__ptr__->");
      void genHeader(ofstream &ofile);
  private: 
      stringHandle type;
      stringHandle name;
};

class remoteFunc {
 public:
  remoteFunc(interfaceSpec *sp, char *st, stringHandle n, stringHandle r, 
	     List <argument*> &a,
	     enum upCallType uc,
	     int v_f,
	     int rs=0);
  void genSwitch(int, char*, ofstream &);
  void genStub(char *interfaceName, int forUpcalls, ofstream &ofile);
  void genXDRStub(char *, ofstream &ofile);
  void genPVMStub(char *, ofstream &ofile);
  void genThreadStub(char*, ofstream &outfile);
  void genHeader();
  void genMethodHeader(char *className, int in_client, ofstream &output);

  // at least one arg is a pointer
    int ArgsRPtr() {return 1;}
  // return type is a pointer
    int isPointer() {return 1;}
  stringHandle name;
  stringHandle retType;
  char *structName;
  List<argument*> args;
  enum upCallType upcall;
  int retStructs;
  int retVoid;
  int virtual_f;
 private:
  interfaceSpec *spec;
};

class interfaceSpec {
  public:
    interfaceSpec( stringHandle n, int v, int lowTag) {
	name = n;
	version = v;
	baseTag = boundTag = lowTag;
    }
    void newMethod(remoteFunc *f) { methods.add(f); }
    void genClass();
    void generateStubs(ofstream &ofile);
    void genErrHandler(ofstream &ofile, int client);
    void generateClientCode();
    void generateThreadLoop();
    void generateXDRLoop();
    void generatePVMLoop();
    void generateServerCode();
    void genWaitLoop();
    void genProtoVerify();
    void genProtoVerifyPVM();
    void generateBundlers();
    void genIncludes(ofstream &output);
    void genXDRServerVerifyProtocol();
    void genXDRLookForVerify();
    int getNextTag();
    char *genVariable();
    stringHandle getName() { return(name); }
    int getVersion() { return(version); }
  private:
    stringHandle name;
    stringHandle unionName;
    int version;
    int baseTag;
    int boundTag;
    List<remoteFunc*> methods;
};

extern class interfaceSpec *currentInterface;


class userDefn {
public:
  userDefn(stringHandle n, int userD, List<field*> &f);
  userDefn(stringHandle n, int userD);
  stringHandle name;
  int userDefined;
  List<field*> fields;
  int arrayType;
  int do_ptr;

  virtual void genHeader() {printf("called base function, bye\n"); exit(0); }
  virtual void genBundler()  {printf("called base function, bye\n"); exit(0); }
  virtual TYPE_ENUM whichType() { return USER_DEFN;}
};


class classDefn : public userDefn {
public: 
  classDefn(stringHandle declared_name, List<field*> &f,
	    stringHandle parent_name, char *pt);
  int generateClassId() {
    int ret = nextTypeId;
    nextTypeId++;
    if (nextTypeId > 255) {
      printf("OVERFLOW, TOO MANY CLASSES\n");
      exit(0);
    }
    return ret;
  }
  void addChild(classDefn *kid);

  // used to unbundle virtual class derived from a base
  // if I know a class member is a pointer to a class, I
  // I can call the bundler as a virtual function, and the
  // correct bundler is invoked
  // but unbundling is done by looking at a byte stream, so I
  // must switch on the type id read from the byte stream
  int type_id;

  // the classes that are derived from this class
  List<classDefn*> children;

  // the class that this class is derived from
  classDefn *parent;

  stringHandle parentName;
  static int nextTypeId;

  virtual TYPE_ENUM whichType() { return CLASS_DEFN;}

  virtual void genHeader();
  virtual void genBundler();

 private:
  void genPtrBundlerXDR();
  void genPtrBundlerPVM();
  void genBundlerPVM();
  void genBundlerXDR();
  char *passThru;
};

class typeDefn : public userDefn {
public:
  typeDefn(stringHandle i, List<field*> &f);
  typeDefn(stringHandle i);
  typeDefn(stringHandle i, stringHandle t);	// for arrays types.

  virtual TYPE_ENUM whichType() { return TYPE_DEFN;}

  stringHandle type;

  virtual void genHeader();
  virtual void genBundler();

 private:
  void genPtrBundlerXDR();
  void genPtrBundlerPVM();
  void genBundlerPVM();
  void genBundlerXDR();
};

extern List<userDefn*> userPool;
extern stringPool namePool;

union parseStack {
    type_data td;
    char *charp;
    stringHandle cp;
    int i;
    float f;
    argument *arg;
    field *fld;
    func_data fd;
    List<argument*> *args;
    List<field*> *fields;
    interfaceSpec *spec;
};

/*
 * parse.h - define the classes that are used in parsing an interface.
 *
 * $Log: parse.h,v $
 * Revision 1.13  1994/09/22 00:38:02  markc
 * Made all templates external
 * Made array declarations separate
 * Add class support
 * Add support for "const"
 * Bundle pointers for xdr
 *
 * Revision 1.12  1994/08/22  16:07:06  markc
 * Moved inline functions used for bundling from header files to .SRVR. and
 * .CLNT. .C files to decrease compiler warnings.
 *
 * Revision 1.11  1994/08/18  05:56:57  markc
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
extern void buildFlagHeaders(ofstream &);

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
  userDefn *f_type;
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

class userDefn {
public:
  userDefn(stringHandle n, int userD, List<field*> &f, char *st, int at=FALSE);
  userDefn(stringHandle n, int userD, char *st, int at=FALSE);
  virtual void genHeader() = 0;
  virtual void genBundler() = 0; 
  virtual TYPE_ENUM whichType() { return USER_DEFN;}
  int getUserDefined() { return userDefined;}
  int getArrayType() { return arrayType;}
  stringHandle getUsh() { return name;}
  const char *getUname() { return ((const char*) name);}
  int getDoBundler() { return doBundler;}
  int getDoFree() { return doFree;}
  int getIsChar() { return isChar;}
  const char *getMarshallName() { return ((const char*) marshallName);}
  List<field*> getFields() { return (fields);}
  int getDoPtr() { return do_ptr;}
  int getDontGen() { return dontGen;}
private:
  int dontGen;
  int isChar;
  int doBundler;
  int doFree;
  stringHandle name;
  int userDefined;
  List<field*> fields;
  int arrayType;
  int do_ptr;
  char *marshallName;
};


class classDefn : public userDefn {

friend void dump_child_id (classDefn *sibling, ofstream &file_id);
friend void handle_parent_fields_free(classDefn *self);
friend void handle_parent_fields(classDefn *self);

public: 
  classDefn(stringHandle declared_name, List<field*> &f,
	    classDefn *par, char *pt);
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

  virtual TYPE_ENUM whichType() { return CLASS_DEFN;}

  virtual void genHeader();
  virtual void genBundler();

private:
  // the classes that are derived from this class
  List<classDefn*> children;

  // the class that this class is derived from
  classDefn *parent;

  static int nextTypeId;

  void genPtrBundlerXDR();
  void genBundlerXDR();
  char *passThru;
};

class typeDefn : public userDefn {
public:
  typeDefn(stringHandle i, List<field*> &f);
  typeDefn(stringHandle i, char *st=0);
  typeDefn(stringHandle i, userDefn *elem, char *st=0);	// for arrays types.

  const char *getElemUname() { return (arrayElement->getUname());}
  const char *getElemMarshall() { return (arrayElement->getMarshallName());}
  int getElemDoFree() { return (arrayElement->getDoFree());}
  virtual TYPE_ENUM whichType() { return TYPE_DEFN;}

  virtual void genHeader();
  virtual void genBundler();

private:
  userDefn *arrayElement;
  void genPtrBundlerXDR();
  void genBundlerXDR();
};

class varHolder {
public: 
  varHolder (stringHandle n, char *st, int isC, userDefn *my_type);
  const char* getVType() { return (myType->getUname());}
  const char* getConstVType() { return((const char*) constVname); }
  const char* getVName() { return ((const char*) vName); }
  userDefn *getVMyType() { return myType;}
  const char *getMarshallName() { return ((const char*) marshallName);}
  const char *getVNonConstName() { return ((const char*) nonConstName);}
  int getDoFree() {
    return(myType->getDoFree() ||
	   (myType->getIsChar() && stars));
  }
  int isPointer() { return(stars ? 1 : 0);}

private:
  userDefn *myType;
  stringHandle vName;
  char *stars;
  int isConst;
  char *constVname;
  char *marshallName;
  char *nonConstName;
};

class argument : public varHolder {
public: 
  argument(stringHandle n, char *st, int isC, userDefn *my_type)
    : varHolder (n, st, isC, my_type) { }
  
  const char* getAType() { return (getVType());}
  const char* getConstAType() { return (getConstVType());}
  const char* getAName() { return (getVName());}
  userDefn *getAMyType() { return (getVMyType());}
  const char *getANonConstName() { return (getVNonConstName());}
  const char *getAMarshallName() { return (getMarshallName());}
};

class field  : public varHolder {
public:
  field(stringHandle n, char *st, int isC, userDefn *my_type)
    : varHolder (n, st, isC, my_type) { }
  
  const char* getFType() { return (getVType());}
  const char* getConstFType() { return (getConstVType());}
  const char* getFName() { return (getVName());}
  userDefn *getFMyType() { return (getVMyType());}
  const char *getFMarshallName() { return (getMarshallName());}
  const char *getFNonConstName() { return (getVNonConstName());}

  void genBundler(ofstream &ofile, const char *obj="&(__ptr__->");
  void genHeader(ofstream &ofile);
};

class remoteFunc : public varHolder {
public:
  remoteFunc(interfaceSpec *sp, char *st, stringHandle n, List <argument*> &a,
	     enum upCallType uc, int v_f, userDefn *my_type, int df);

  const char* getRFType() { return (getVType());}
  const char* getConstRFType() { return (getConstVType());}
  const char* getRFName() { return (getVName());}
  userDefn *getRFMyType() { return (getVMyType());}
  const char *getRFMarshallName() { return (getMarshallName());}
  const char *getRFNonConstName() { return  (getVNonConstName());}

  void genSwitch(int, const char*, ofstream &);
  void genStub(char *interfaceName, int forUpcalls, ofstream &ofile);
  void genXDRStub(char *, ofstream &ofile);
  void genThreadStub(char*, ofstream &outfile);
  void genHeader();
  void genMethodHeader(char *className, int in_client, ofstream &output);

  int getRetVoid() { return retVoid;}
  // at least one arg is a pointer
  int ArgsRPtr() {return 1;}

  // return type is a pointer
  int isPointer() {return 1;}
  const char *getStructName() { return ((const char*) structName);}
  int getDontFree() { return dontFree;}

private:
  int dontFree;
  char *structName;
  List<argument*> args;
  enum upCallType upcall;
  int retVoid;
  int virtual_f;
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
  void generateServerCode();
  void genWaitLoop();
  void genProtoVerify();
  void generateBundlers();
  void genIncludes(ofstream &output, int inBase=0);
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

extern interfaceSpec *currentInterface;

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

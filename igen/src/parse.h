/*
 * parse.h - define the classes that are used in parsing an interface.
 *
 * $Log: parse.h,v $
 * Revision 1.8  1994/06/02 23:34:27  markc
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

#include "util/h/list.h"
#include "util/h/stringPool.h"

#include <iostream.h>
#include <fstream.h>

// the files to write the generated code to
ofstream dot_h;
ofstream dot_c;
ofstream srvr_dot_c;
ofstream srvr_dot_h;
ofstream clnt_dot_c;
ofstream clnt_dot_h;

class typeDefn;		// forward decl
class remoteFunc;

extern void dump_to_dot_h(char *);
extern stringPool pool;
extern List<typeDefn*> types;
extern typeDefn *foundType;

void addCMember (char *type, char *name, List<char*>*);
void addSMember (char *type, char *name, List<char*>*);
void buildPVMfilters();
void buildPVMincludes();
void buildPVMargs();
void genPVMServerCons(char *);
void genXDRServerCons(char *);
class interfaceSpec {
  public:
    interfaceSpec(char *n, int v, int lowTag) {
	name = n;
	version = v;
	baseTag = boundTag = lowTag;
    }
    void newMethod(remoteFunc*f) { methods.add(f); }
    void genClass();
    void generateStubs(ofstream &ofile);
    void genErrHandler(ofstream &ofile, Boolean client);
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
    argument(char *t, char *n,  List<char *> *s, int m) {
	type = t;
	name = n;
	stars = s;
	mallocs = m;
    }
    argument(char *t) {
	type = t;
	name = currentInterface->genVariable();
	mallocs = 0;
    }
    char *type;
    char *name;
    int mallocs;
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
      void genBundler(ofstream &ofile) {
	  if (generateXDR) {
	    ofile << "    if (!xdr_" << type << "(__xdrs__, &(__ptr__->" << name << "))) return FALSE;\n";
	  } else if (generatePVM) {
	    ofile << "    IGEN_pvm_" << type << "(__dir__, &(__ptr__->" << name << "));\n";
	  }
      }
      void genHeader(ofstream &ofile) {
	  ofile << "    " << type << " " << name << ";\n";
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


enum upCallType { syncUpcall,       // from server to client, sync (not allowed)
		  asyncUpcall,      // from server to client, async
		  syncCall,         // from client to server, sync
		  asyncCall         // from client to server, async
		  };

class interfaceSpec;

struct type_data {
  char *cp;
  int mallocs;
  int structs;
};

typedef struct type_data type_data;

struct func_data {
  enum upCallType uc;
  int virtual_f;
};

typedef struct func_data func_data;

union parseStack {
    type_data td;
    char *cp;
    int i;
    float f;
    argument *arg;
    field *fld;
    func_data fd;
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
		 enum upCallType uc,
		 int v_f,
		 int rs=0) {
	  spec = sp;
	  name = n;
	  retType = r;
	  structName = spec->genVariable();
	  args = a;
	  upcall = uc;
	  retStructs = rs;
	  virtual_f = v_f;
      }
      void genSwitch(Boolean, char*, ofstream &);
      void genStub(char *interfaceName, Boolean forUpcalls, ofstream &ofile);
      void genXDRStub(char *, ofstream &ofile);
      void genPVMStub(char *, ofstream &ofile);
      void genThreadStub(char *, ofstream &outfile);
      void genHeader();
      void genMethodHeader(char *className, int in_client, ofstream &output);
      char *name;
      char *retType;
      char *structName;
      List <argument *> args;
      enum upCallType upcall;
      int retStructs;
      int virtual_f;
  private:
      interfaceSpec *spec;
};


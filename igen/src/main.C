/*
 * main.C - main function of the interface compiler igen.
 *
 * $Log: main.C,v $
 * Revision 1.22  1994/04/06 21:28:45  markc
 * Added constructor for client side of xdr based code.
 *
 * Revision 1.21  1994/04/01  04:57:50  markc
 * Added checks in bundlers.  Fixed xdrrec_endofrecord.
 *
 * Revision 1.20  1994/03/31  22:57:52  hollings
 * Fixed xdr_endofrecord bug and added wellKnownScoket parameter.
 *
 * Revision 1.19  1994/03/31  02:12:18  markc
 * Added code to initialize versionVerifyDone
 *
 * Revision 1.18  1994/03/25  22:35:21  hollings
 * Get the test for callErr right.
 *
 * Revision 1.17  1994/03/25  20:57:02  hollings
 * Changed all asserts to set the error member variable.  This makes it
 * possible to detect/continue when the remote process dies.
 *
 * Revision 1.16  1994/03/07  23:28:46  markc
 * Added support to free arrays of classes.
 *
 * Revision 1.15  1994/03/07  02:50:56  markc
 * Set callErr to 0 in constructors.
 *
 * Revision 1.14  1994/03/07  02:35:17  markc
 * Added code to detect failures for xdr code.  Provides member instance
 * callErr which is set to -1 on failures.
 *
 * Revision 1.13  1994/03/06  20:51:09  markc
 * Added float as a basic type.
 *
 * Revision 1.12  1994/03/01  21:39:37  jcargill
 * Rearranged order of includes for igen-generated code, so that it compiles
 * on MIPS.  Some of the Ultrix headers really suck.
 *
 * Revision 1.11  1994/03/01  01:50:46  markc
 * Fixed memory access errors.
 *
 * Revision 1.10  1994/02/25  11:41:32  markc
 * Fixed bug.  Igen generated versionVerify tests for client code when async.
 *
 * Revision 1.9  1994/02/24  05:14:32  markc
 * Man page for igen.
 * Initial version for solaris2.2.
 * Dependencies changed.
 * Added pvm support, virtual function support, inclusion of data members,
 * separate client and header include files.
 *
 * Revision 1.8  1994/02/08  00:17:55  hollings
 * Fixed pointer problems to work on DECstations.
 *
 * Revision 1.7  1994/02/04  01:25:46  hollings
 * *** empty log message ***
 *
 * Revision 1.6  1994/02/04  00:35:01  hollings
 * constructor inheritance round two.
 *
 * Revision 1.5  1994/01/31  20:05:57  hollings
 * Added code to check the protocol name and version tests are run
 * before any upcalls get invoked.
 *
 * Revision 1.4  1994/01/28  19:42:30  hollings
 * Fixed max variable name length.
 *
 * Revision 1.3  1994/01/27  20:36:29  hollings
 * changes in include syntax round 2 or 3.
 *
 * Revision 1.2  1994/01/26  06:50:10  hollings
 * made the output of igen pass through g++ -Wall.
 *
 * Revision 1.1  1994/01/25  20:48:43  hollings
 * New utility for interfaces.
 * new utility for interfaces.
 *
 *
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/file.h>

#include "util/h/list.h"
#include "util/h/stringPool.h"

int generateXDR;
int generatePVM;

#include "parse.h"

int yyparse();
int emitCode;
int emitHeader;
char *codeFile;
char *serverFile;
char *clientFile;
char *protoFile;
stringPool pool;
char *headerFile;
FILE *kludgeOut = 0;
extern FILE *yyin;
int generateTHREAD;
char *transportBase;
char *serverTypeName;
List <typeDefn *> types;
typeDefn *foundType;
interfaceSpec *currentInterface;
typedef struct pvm_args { char *type_name; char *pvm_name; char *arg;} pvm_args;
List <pvm_args *> pvm_types;
void PVM_map_scalar_includes (const pvm_args *);
void PVM_map_array_includes (const pvm_args *);
void templatePVMscalar (const pvm_args *);
void templatePVMarray (const pvm_args *);
static List<char*> client_pass_thru;
static List<char*> server_pass_thru;

void usage(char *name)
{
    printf("%s -xdr | -thread | -pvm [-header | -code] <fileName>\n", name);
    exit(-1);
}

void interfaceSpec::generateThreadLoop()
{
    List <remoteFunc*> cf;

    unionName = genVariable();
    printf("union %s {\n", unionName);
    for (cf = methods; *cf; cf++) {
	printf("    struct %s %s;\n", (*cf)->structName, (*cf)->name);
    }
    printf("};\n\n");

    printf("int %s::mainLoop(void)\n", name);
    printf("{\n");
    printf("  unsigned int __len__;\n");
    printf("  unsigned int __tag__;\n");
    printf("  union %s __recvBuffer__;\n", unionName);
    printf("\n");
    printf("  __tag__ = MSG_TAG_ANY;\n");
    printf("  __len__ = sizeof(__recvBuffer__);\n");
    printf("  requestingThread = msg_recv(&__tag__, &__recvBuffer__, &__len__);\n");
    printf("  switch (__tag__) {\n");
    for (cf = methods; *cf; cf++) {
	(*cf)->genSwitch(FALSE, "-1");
    }
    printf("    default:\n");
    printf("	    return(__tag__);\n");
    printf("  }\n");
    printf("  return(0);\n");
    printf("}\n");
}

void interfaceSpec::generateXDRLoop()
{
    List <remoteFunc*> cf;

    printf("int %s::mainLoop(void)\n", name);
    printf("{\n");
    printf("    unsigned int __tag__, __status__;\n");
    printf("    callErr = 0;\n");
    printf("    __xdrs__->x_op = XDR_DECODE;\n");
    printf("    if (xdrrec_skiprecord(__xdrs__) == FALSE)\n");
    printf("       {callErr = -1; return -1;}\n");
    printf("    __status__ = xdr_int(__xdrs__, &__tag__);\n");
    printf("	if (!__status__) {callErr = -1; return(-1);}\n");
    printf("    switch (__tag__) {\n");

    // generate the zero RPC that returns interface name & version.
    printf("        case 0:\n");
    printf("            char *__ProtocolName__ = \"%s\";\n", name);
    printf("            int __val__;\n");
    printf("            __xdrs__->x_op = XDR_ENCODE;\n");
    printf("            __val__ = 0;\n");
    printf("            if ((xdr_int(__xdrs__, &__val__) == FALSE) ||\n");
    printf("                (xdr_String(__xdrs__, &__ProtocolName__) == FALSE))\n");
    printf("               {callErr = -1; return -1;}\n");
    printf("            __val__ = %d;\n", version);
    printf("            if (xdr_int(__xdrs__, &__val__) == FALSE)\n");
    printf("               {callErr = -1; return -1;}\n");
    printf("            if (xdrrec_endofrecord(__xdrs__, TRUE) == FALSE)\n");
    printf("               {callErr = -1; return -1;}\n");
    printf("		__versionVerifyDone__ = TRUE;\n");
    printf("            break;\n");

    for (cf = methods; *cf; cf++) {
	(*cf)->genSwitch(FALSE, "-1");
    }
    printf("    default:\n");
    printf("	    return(__tag__);\n");
    printf("  }\n");
    printf("  return(0);\n");
    printf("}\n");
}

void interfaceSpec::generatePVMLoop()
{
    List <remoteFunc*> cf;

    printf("int %s::mainLoop(void)\n", name);
    printf("{\n");
    printf("    int __tag__, __bytes__, __msgtag__, __tid__, __bufid__, __other__, __count__;\n");
    printf("    struct taskinfo __taskp__, *__tp__;\n");
    printf("    __tp__ = &__taskp__;\n");
    printf("    // this code checks for message \n");
    printf("    __other__ = get_other_tid();\n");
    printf("    if (__other__ < -1) return -1;\n");
    printf("    if (get_error() == -1) return -1;\n");
    printf("    if ((__bufid__ = pvm_recv (__other__, -1)) < 0) return -1;\n");
    printf("    if (pvm_bufinfo (__bufid__, &__bytes__, &__msgtag__, &__tid__) < 0) return -1;\n");
    printf("    switch (__msgtag__) {\n");

    // generate the zero PVM that returns interface name & version.
    printf("        case 0:\n");
    printf("            char *__ProtocolName__ = \"%s\";\n", name);
    printf("            int __val__;\n");
    printf("            __val__ = 0;\n");
    printf("            assert(pvm_initsend(0) >= 0);\n");
    printf("            pvm_pkstr(__ProtocolName__);\n");
    printf("            __val__ = %d;\n", version);
    printf("            pvm_pkint(&__val__, 1, 1);\n");
    printf("            pvm_send (__tid__, 0);");
    printf("            break;\n");

    for (cf = methods; *cf; cf++) {
	(*cf)->genSwitch(FALSE, "-1");
    }
    printf("    default:\n");
    printf("	    return(__tag__);\n");
    printf("  }\n");
    printf("  return(0);\n");
    printf("}\n");
}



void interfaceSpec::genIncludes()
{
    printf("#include <stdio.h>\n");
    printf("#include <stdlib.h>\n");
    printf("#include <rpc/types.h>\n");
    printf("#include <assert.h>\n");
    if (generateTHREAD) {
	printf("extern \"C\" {\n");
	printf("#include \"thread/h/thread.h\"\n");
	printf("#include <errno.h>\n");
	printf("}\n");
    }
    if (generateXDR) {
	printf("extern \"C\" {\n");
	printf("#include <rpc/xdr.h>\n");
	printf("#include <errno.h>\n");
	printf("}\n");
    }
    if (generatePVM) {
        printf("extern \"C\" {\n");
	printf("#include <pvm3.h> \n");
	printf("#include <errno.h>\n");
	printf("}\n");
      }

    // printf("#include \"%s\"\n\n", headerFile);
}

void interfaceSpec::generateServerCode()
{
    List<remoteFunc*> cf;

    if (generateTHREAD) {
	generateThreadLoop();
    } else if (generateXDR) {
	FILE *sf;

	// needs to go into a separate file.
	fflush(stdout);

	sf = fopen(serverFile, "w");
	dup2(sf->_file, 1);

	genIncludes();

	// include server header
	printf ("#include \"%sSRVR.h\"\n", protoFile);

	generateXDRLoop();

	//
	// generate XDR-only class constructors to allow server to 
	// start clients
	//
	genXDRServerCons(name);

    } else if (generatePVM) {
        FILE *sf;

	// needs to go into a separate file.
	fflush(stdout);

	sf = fopen(serverFile, "w");
	dup2(sf->_file, 1);

	genIncludes();

	// include server header
	printf ("#include \"%sSRVR.h\"\n", protoFile);
	
	generatePVMLoop();

	//
	// generate PVM-only class constructors to allow server to 
	// start clients
	//
	genPVMServerCons(name);

      }


    // generate stubs for upcalls.
    for (cf = methods; *cf; cf++) {
	(*cf)->genStub(name, TRUE);
    }
}


void interfaceSpec::genWaitLoop() 
{
    List<remoteFunc*> cf;

    // generate a loop to wait for a tag, and call upcalls as the arrive.
    printf("void %sUser::awaitResponce(int __targetTag__) {\n", name);
    printf("    unsigned int __tag__;\n");
    printf("    callErr = 0;\n");
    if (generateTHREAD) {
	printf("  union %s __recvBuffer__;\n", unionName);
	printf("  unsigned __len__ = sizeof(__recvBuffer__);\n");
    } else if (generatePVM) {
	printf("    int __tid__, __bufid__, __bytes__;\n");
    }
    printf("  while (1) {\n");
    if (generateXDR) {
	printf("    __xdrs__->x_op = XDR_DECODE;\n");
	printf("    if (xdrrec_skiprecord(__xdrs__) == FALSE)\n");
	printf("       {callErr = -1; return;}\n");
	printf("    if (xdr_int(__xdrs__, &__tag__) == FALSE)\n");
	printf("       {callErr = -1; return;}\n");
    } else if (generatePVM) {
        printf("    int __other__ = get_other_tid();\n");
	printf("    if (get_error() == -1) abort();\n");
	printf("     if (__other__ < 0) abort();\n");
	printf("    if ((__bufid__ = pvm_recv (__other__, -1)) < 0) abort();\n");
	printf("    if (pvm_bufinfo(__bufid__, &__bytes__, (int*) &__tag__, &__tid__) < 0) abort();\n");
    } else if (generateTHREAD) {
	printf("  __tag__ = MSG_TAG_ANY;\n");
	printf("    requestingThread = msg_recv(&__tag__, (void *) &__recvBuffer__, &__len__); \n");
    }
    // look for success error message
    printf("    if (__tag__ == __targetTag__) return;\n");

    printf("    switch (__tag__) {\n");
    for (cf = methods; *cf; cf++) {
	(*cf)->genSwitch(TRUE, " ");
    }
    printf("	    default: \n        abort();\n");
    printf("    }\n");
    printf("	if (__targetTag__ == -1) return;\n");
    printf("  }\n");
    printf("}\n");

    printf("int %sUser::isValidUpCall(int tag) {\n", name);
    printf("    return((tag >= %d) && (tag <= %d));\n", baseTag, boundTag);
    printf("}\n");
}

void interfaceSpec::genProtoVerify()
{
    // generate stub to verify version.
    printf("void %sUser::verifyProtocolAndVersion() {\n", name);
    printf("    unsigned int __tag__;\n");
    printf("    String proto;\n");
    printf("    int version;\n");
    printf("    __tag__ = 0;\n");
    printf("    __xdrs__->x_op = XDR_ENCODE;\n");
    printf("    callErr = 0;\n");
    printf("    if ((xdr_int(__xdrs__, &__tag__) != TRUE) ||\n");
    printf("        (xdrrec_endofrecord(__xdrs__, TRUE) != TRUE)) {\n");
    printf("        callErr = -1;\n");
    printf("        return;\n");
    printf("    }\n");
    printf("    awaitResponce(0);\n");
    printf("    if (callErr == -1) {\n");
    printf("        printf(\"Protocol verify - no response from server\\n\");\n");
    printf("	    exit(-1);\n");
    printf("    }\n");
    printf("    __xdrs__->x_op = XDR_DECODE;\n");
    printf("    if ((xdr_String(__xdrs__, &(proto)) == FALSE) ||\n");
    printf("        (xdr_int(__xdrs__, &(version)) == FALSE)) {\n");
    printf("        printf(\"Protocol verify - bad response from server\\n\");\n");
    printf("	    exit(-1);\n");
    printf("    }\n");
    printf("    if ((version != %d) || (strcmp(proto, \"%s\"))) {\n",
	version, name);
    printf("        printf(\"protocol %s version %d expected\\n\");\n", 
	name, version);
    printf("        printf(\"protocol %%s version %%d found\\n\", proto, version);\n");
    printf("	    exit(-1);\n");
    printf("    }\n");
    printf("    __xdrs__->x_op = XDR_FREE;\n");
    printf("    xdr_String (__xdrs__, &proto);\n");
    printf("}\n");
    printf("\n\n");
    printf("%sUser::%sUser(int fd, xdrIOFunc r, xdrIOFunc w, int nblock):\n", name, name);
    printf("XDRrpc(fd, r, w, nblock) {\n");
    printf("    if (__xdrs__) verifyProtocolAndVersion();\n");
    printf ("   callErr = 0;\n");
    printf ("}\n");

    printf("%sUser::%sUser(int family, int port, int type, char *machine, xdrIOFunc rf, xdrIOFunc wr, int nblock):\n", name, name);
    printf("XDRrpc(family, port, type, machine, rf, wr, nblock) {\n");
    printf("    if (__xdrs__) verifyProtocolAndVersion();\n");
    printf ("   callErr = 0;\n");
    printf ("}\n");

    printf("%sUser::%sUser(char *m,char *l,char *p,xdrIOFunc r,xdrIOFunc w, char **args, int nblock):\n", name, name);
    printf("    XDRrpc(m, l, p, r, w, args, nblock, __wellKnownPortFd__) {\n");
    printf("    if (__xdrs__) verifyProtocolAndVersion();\n");
    printf ("   callErr = 0;\n");
    printf ("}\n");

    printf("\n");
}


//
// generate code to perform protocol verification 
//
void interfaceSpec::genProtoVerifyPVM()
{
    // generate stub to verify version.
    printf("void %sUser::verifyProtocolAndVersion() {\n", name);
    printf("    unsigned int __tag__;\n");
    printf("    String proto;\n");
    printf("    int version = -1;\n");
    printf("    __tag__ = 0;\n");
    printf("    // msgtag = 0 --> verify protocol\n");
    printf("    assert (pvm_initsend(0) >= 0);\n");
    printf("    pvm_send (get_other_tid(), 0);\n");
    printf("    awaitResponce(0);\n");
    printf("    IGEN_pvm_String (IGEN_PVM_DECODE, &proto);\n");
    printf("    pvm_upkint(&(version), 1, 1);\n");
    printf("    if ((version != %d) || (strcmp(proto, \"%s\"))) {\n",
	version, name);
    printf("        printf(\"protocol %s version %d expected\\n\");\n", 
	name, version);
    printf("        printf(\"protocol %%s version %%d found\\n\", proto, version);\n");
    printf("	    pvm_exit(); exit(-1);\n");
    printf("    }\n");
    printf("    IGEN_pvm_String (IGEN_PVM_FREE, &proto);\n");
    printf("}\n");
    printf("\n\n");
    printf("%sUser::%sUser(char *w, char *p, char **a, int f):\n", name, name);
    printf("PVMrpc(w, p, a, f) {\n");
    printf("if (get_error() != -1) verifyProtocolAndVersion();\n");
    printf(" callErr = 0;\n");
    printf("}\n");

    printf("%sUser::%sUser(int o):\n", name, name);
    printf("PVMrpc(o) {\n");
    printf("if (get_error() != -1) verifyProtocolAndVersion();\n");
    printf(" callErr = 0;\n");
    printf("}\n");

    printf("%sUser::%sUser():\n", name, name);
    printf("PVMrpc() {\n");
    printf("if (get_error() != -1) verifyProtocolAndVersion();\n");
    printf(" callErr = 0;\n");
    printf("}\n");
    printf("\n");
}


void interfaceSpec::generateStubs()
{
    List <remoteFunc*> cf;
    char className[80];

    sprintf(className, "%sUser", name);
    for (cf = methods; *cf; cf++) {
	(*cf)->genStub(className, FALSE);
    }
}

void interfaceSpec::generateClientCode()
{
    if (generateXDR) {
	FILE *cf;

	// needs to go into a separate file.
	fflush(stdout);

	cf = fopen(clientFile, "w");
	dup2(cf->_file, 1);

	genIncludes();

	// include client header
	printf ("#include \"%sCLNT.h\"\n", protoFile);

	printf("int %sUser::__wellKnownPortFd__;\n", name);

    } else if (generatePVM) {
        FILE *cf;

	// needs to go into a separate file.
	fflush(stdout);

	cf = fopen(clientFile, "w");
	dup2(cf->_file, 1);

	genIncludes();

	// include client header
	printf ("#include \"%sCLNT.h\"\n", protoFile);
      }

    generateStubs();

    if (generateXDR) {
	genProtoVerify();
    } else if (generatePVM) {
        genProtoVerifyPVM();
      }

    genWaitLoop();
}

void interfaceSpec::generateBundlers()
{
    List <typeDefn*> cs;

    for (cs = types; *cs; cs++) {
	(*cs)->genBundler();
    }
}

int main(int argc, char *argv[])
{
    int i;
    FILE *of;
    char *temp;

    /* define pre-defined types */
    (void) new typeDefn("int");
    (void) new typeDefn("double");
    (void) new typeDefn("String");
    (void) new typeDefn("void");
    (void) new typeDefn("Boolean");
    (void) new typeDefn("u_int");
    (void) new typeDefn("float");

    emitCode = 1;
    emitHeader = 1;
    for (i=0; i < argc-1; i++) {
	if (!strcmp("-pvm", argv[i])) {
	    generatePVM = 1;
	    generateXDR = 0;
	    generateTHREAD = 0;
	    serverTypeName = pool.findAndAdd("int");
	    transportBase = "PVMrpc";
	} if (!strcmp("-xdr", argv[i])) {
	    generatePVM = 0;
	    generateXDR = 1;
	    generateTHREAD = 0;
	    (void) new typeDefn("XDRptr");
	    serverTypeName = pool.findAndAdd("XDRptr");
	    transportBase = "XDRrpc";
	} if (!strcmp("-thread", argv[i])) {
	    generatePVM = 0;
	    generateXDR = 0;
	    generateTHREAD = 1;
	    serverTypeName = pool.findAndAdd("int");
	    transportBase = "THREADrpc";
	} else if (!strcmp("-header", argv[i])) {
	    emitCode = 0;
	    emitHeader = 1;
	} else if (!strcmp("-code", argv[i])) {
	    emitCode = 1;
	    emitHeader = 0;
	}
    }
    if (!emitHeader && !emitCode) {
	usage(argv[0]);
    }
    if (!generatePVM && !generateXDR && !generateTHREAD) {
	usage(argv[0]);
    }

    // build the list that contains PVM type information
    if (generatePVM)
      buildPVMargs();

    protoFile = argv[argc-1];

    yyin = fopen(protoFile, "r");
    if (!yyin) {
	printf("unable to open %s\n", protoFile);
	exit(-1);
    }

    // skip to trailing component of file name.
    temp = strrchr(protoFile, '/');
    if (temp) protoFile = temp+1;

    temp = strstr(protoFile, ".I");
    if (!temp) {
	printf("file names must end in .I\n");
	exit(-1);
    }
    *(temp+1) = '\0';

    headerFile = (char *) malloc(strlen(protoFile)+2);
    sprintf(headerFile, "%sh", protoFile);

    codeFile = (char *) malloc(strlen(protoFile)+2);
    sprintf(codeFile, "%sC", protoFile);

    serverFile = (char *) malloc(strlen(protoFile)+7);
    sprintf(serverFile, "%sSRVR.C", protoFile);

    clientFile = (char *) malloc(strlen(protoFile)+7);
    sprintf(clientFile, "%sCLNT.C", protoFile);

    if (emitHeader) {
        char *prefix;
	int len;
	len = strlen(protoFile);
	prefix = new char[len];
	strncpy (prefix, protoFile, len - 1);
	prefix[len-1] = '\0';
	of = fopen(headerFile, "w");
	dup2(of->_file, 1);
	printf("#ifndef %sBASE_H\n", prefix);
	printf("#define %sBASE_H\n", prefix);
	printf("#include \"util/h/rpcUtil.h\"\n");

	if (generatePVM)
	  printf("#include \"util/h/rpcUtilPVM.h\"\n");
	printf("#include <sys/types.h>\n");
	delete [] prefix;

	kludgeOut = of;
	if (generatePVM) 
	  {
	    buildPVMincludes();
	  }
      }

    yyparse();

    // this is opened in genClass
    if (emitHeader) {
      printf("#endif\n");
      fflush(stdout);
      fclose(kludgeOut);
    }

    if (!currentInterface) {
	printf("no interface defined\n");
	exit(-1);
    }

    if (emitCode) {
	FILE *cf;

	cf = fopen(codeFile, "w");
	dup2(cf->_file, 1);

	currentInterface->genIncludes();
	
	if (generateTHREAD) {
	  printf("#include \"%sSRVR.h\"\n", protoFile);
	  printf("#include \"%sCLNT.h\"\n", protoFile);
	} else {
	  printf("#include \"%sh\"\n", protoFile);	  
	}


	if (generatePVM) {
	  buildPVMfilters();
	}
	  
	  
	

	if (generateXDR || generatePVM) currentInterface->generateBundlers();
	
	currentInterface->generateServerCode();

	currentInterface->generateClientCode();
	fflush(stdout);
    }

    exit(0);
}

char *interfaceSpec::genVariable()
{
    static int count;
    char *ret;

    ret = (char *) malloc(strlen(name)+10);
    sprintf(ret, "%s__%d", name, count++);
    return(ret);
}

int interfaceSpec::getNextTag()
{
    return(++boundTag);
}

// in_client == 1 when client headers are being generated
void remoteFunc::genMethodHeader(char *className, int in_client)
{
    int spitOne;
    List <char *> cl;
    List<argument*> lp;

    if (className) {
	printf("%s %s::%s(", retType, className, name);
    } else {
      // figure out if "virtual" should be printed
      // here is the logic
      // for the client --> if it is an upcall then print virtual
      // for the server --> if it is not an upcall the print virtual
      // for both an additional constraint is that virtual_f must be true
      if ((((upcall == syncUpcall) || (upcall == asyncUpcall)) && virtual_f && in_client) ||
	  (!in_client && virtual_f && ((upcall == notUpcallAsync) || (upcall == notUpcall))))
	{
	  // print virtual
	  printf("virtual %s %s(", retType, name);
	}
      else
	{
	  // don't print virtual
	  printf("%s %s(", retType, name);
	}
    }
    for (lp = args, spitOne = 0; *lp; lp++) {
	if (spitOne) printf(",");
	spitOne =1;
	printf("%s ", (*lp)->type);
	for (cl = *(*lp)->stars; *cl; cl++) printf("*");
	printf("%s", (*lp)->name);
    }
    if (!spitOne) printf("void");
    printf(")");
}

void remoteFunc::genSwitch(Boolean forUpcalls, char *ret_str)
{
    int first;
    List <argument *> ca;


    if (forUpcalls && ( (upcall == notUpcall) || (upcall == notUpcallAsync))) return;
    if (!forUpcalls && (upcall != notUpcall) && (upcall != notUpcallAsync))return;

    printf("        case %s_%s_REQ: {\n", spec->getName(), name);
    printf("            ");
    if ((generateTHREAD) && (upcall != asyncUpcall) && (upcall != notUpcallAsync))
	printf("	        int __val__;\n");
    if (strcmp(retType, "void")) {
	printf("	        %s __ret__;\n", retType);
    }
    if (generateXDR) {
	printf("	   // extern xdr_%s(XDR*, %s*);\n", retType, retType);
	if (args.count()) {
	    printf("            %s __recvBuffer__;\n", structName);
		for (ca = args; *ca; ca++) {
		printf("            if (xdr_%s(__xdrs__, &__recvBuffer__.%s) == FALSE)\n", (*ca)->type, (*ca)->name);
		printf("              {callErr = -1; return %s;}\n", ret_str);
	    }
	}
    } else if (generatePVM) {
        printf("            extern IGEN_pvm_%s(IGEN_PVM_FILTER, %s*);\n", retType, retType);
	if ((upcall != asyncUpcall) && (upcall != notUpcallAsync))
	  printf("            assert(pvm_initsend(0) >= 0);\n");
	if (args.count()) {
	  printf("            %s __recvBuffer__;\n", structName);
	  for (ca = args; *ca; ca++) {
	    printf("            IGEN_pvm_%s(IGEN_PVM_DECODE, &__recvBuffer__.%s);\n", (*ca)->type,
		   (*ca)->name);
	  }
	}
      }
    if (strcmp(retType, "void")) {
	printf("            __ret__ = %s(", name);
    } else {
	printf("            %s(", name);
    }

    for (ca = args, first = 1; *ca; ca++) {
	if (!first) printf(",");
	first = 0;
	if (generateTHREAD) {
	    printf("__recvBuffer__.%s.%s", name, (*ca)->name);
	} else if (generateXDR) {
	    printf("__recvBuffer__.%s", (*ca)->name);
	} else if (generatePVM) {
	  printf("__recvBuffer__.%s", (*ca)->name);
	}
    }
    printf(");\n");

    if (generateTHREAD && (upcall != asyncUpcall) && (upcall != notUpcallAsync)) {
	if (strcmp(retType, "void")) {
	    printf("        __val__ = msg_send(requestingThread, %s_%s_RESP, (void *) &__ret__, sizeof(%s));\n", spec->getName(), name, retType);
	} else {
	    printf("        __val__ = msg_send(requestingThread, %s_%s_RESP, NULL, 0);\n", 
		spec->getName(), name);
	}
	printf("	    assert(__val__ == THR_OKAY);\n");
    } else if (generateXDR && (upcall != asyncUpcall) && (upcall != notUpcallAsync)) {
	printf("	    __xdrs__->x_op = XDR_ENCODE;\n");
	printf("            __tag__ = %s_%s_RESP;\n", spec->getName(), name);
	printf("            if (xdr_int(__xdrs__, &__tag__) == FALSE)\n");
	printf("               {callErr = -1; return %s;}\n", ret_str);
	if (strcmp(retType, "void")) {
	    printf("            if (xdr_%s(__xdrs__,&__ret__) == FALSE)\n", retType);
	    printf("               {callErr = -1; return %s;}\n", ret_str);
	}
	printf("	    if (xdrrec_endofrecord(__xdrs__, TRUE) == FALSE)\n");
	printf("               {callErr = -1; return %s;}\n", ret_str);
    } else if (generatePVM && (upcall != asyncUpcall) && (upcall != notUpcallAsync)) {
	printf("            __tag__ = %s_%s_RESP;\n", spec->getName(), name);
	if (strcmp(retType, "void")) {
	    printf("            IGEN_pvm_%s(IGEN_PVM_ENCODE,&__ret__);\n", retType);
	  }
	printf("            assert (pvm_send (__tid__, __tag__) >= 0);\n");
      }
    // free allocated memory
    if (generateTHREAD)
      {
      }
    else if (generateXDR)
      {
	printf("            __xdrs__->x_op = XDR_FREE;\n");
	if (args.count())
	  {
	    for (ca = args; *ca; ca++)
	      {
		// only if Array or string or user defined
		if ((*ca)->mallocs)
		  printf("            xdr_%s(__xdrs__, &__recvBuffer__.%s);\n", (*ca)->type, (*ca)->name);
	      }
	  }
	if (retStructs)
	  printf("            xdr_%s(__xdrs__, &__ret__);\n", retType);
      }
    else if (generatePVM)
      {
	if (args.count())
	  {
	    for (ca = args; *ca; ca++)
	      {
		// only if Array or string 
		if ((*ca)->mallocs)
		  printf("            IGEN_pvm_%s(IGEN_PVM_FREE, &__recvBuffer__.%s);\n", (*ca)->type, (*ca)->name);
	      }
	  }
	if (retStructs)
	  printf("            IGEN_pvm_%s(IGEN_PVM_FREE, &__ret__);\n", retType);
      }
    printf("            break;\n         }\n\n");
}
 

void remoteFunc::genThreadStub(char *className)
{
    List<argument*> lp;
    char *retVar = spec->genVariable();
    char *structUseName = spec->genVariable();

    genMethodHeader(className, 0);
    printf(" {\n");

    if (*args) printf("    struct %s %s;\n", structName, structUseName);
    if (upcall != asyncUpcall) printf("    unsigned int __tag__;\n");
    if (strcmp(retType, "void")) printf("    unsigned int __len__;\n");
    printf("    int __val__;\n");
    if (strcmp(retType, "void")) {
	printf("    %s %s;\n", retType, retVar);
    }

    // set callErr for non-upcalls
    if ((upcall == notUpcall) || (upcall == notUpcallAsync))
      printf("     callErr = 0;\n");

    for (lp = args; *lp; lp++) {
	printf("    %s.%s = %s;  \n",structUseName,
	    (*lp)->name, (*lp)->name);
    }
    if (*args) {
	printf("    __val__ = msg_send(tid, %s_%s_REQ, (void *) &%s, sizeof(%s));\n",
	    spec->getName(), name, structUseName, structUseName);
    } else {
	printf("    __val__ = msg_send(tid, %s_%s_REQ, NULL, 0); \n", 
	    spec->getName(), name);
    }
    printf("    assert(__val__ == THR_OKAY);\n");
    if (upcall != asyncUpcall) {
	printf("    __tag__ = %s_%s_RESP;\n", spec->getName(), name);
	if (strcmp(retType, "void")) {
	    printf("    __len__ = sizeof(%s);\n", retVar);
	    printf("    msg_recv(&__tag__, (void *) &%s, &__len__); \n",retVar);
	    printf("    assert(__len__ == sizeof(%s));\n", retVar);
	    printf("    return(%s);\n", retVar);
	} else {
	    printf("    msg_recv(&__tag__, NULL, 0); \n",spec->getName(), name);
	}
	printf("    assert(__tag__ == %s_%s_RESP);\n",spec->getName(),name);
    }
    printf("}\n\n");
}

void remoteFunc::genXDRStub(char *className)
{
    List<argument*> lp;
    char *retVar = spec->genVariable();
    int retS = 0;

    genMethodHeader(className, 0);
    printf(" {\n");

    printf("    unsigned int __tag__;\n");
    if (strcmp(retType, "void")) {
	printf("    %s %s;\n", retType, retVar);
	retS = 1;
    }

    // set callErr for non-upcalls
    if ((upcall == notUpcall) || (upcall == notUpcallAsync))
      printf("     callErr = 0;\n");

    // check to see protocol verify has been done.
    if ((upcall != notUpcall) && (upcall != notUpcallAsync)) {
	printf("    if (!__versionVerifyDone__) {\n");
	printf("        char *__ProtocolName__ = \"%s\";\n", spec->getName());
	printf("	int __status__;\n");
	printf("	int __version__;\n");
	printf("        __xdrs__->x_op = XDR_DECODE;\n");
	printf("        if (xdrrec_skiprecord(__xdrs__) == FALSE)\n");
	printf("          {callErr = -1; return ");
	if (retS) printf("(%s);}\n", retVar);
	else printf(";}\n");
	printf("        __status__ = xdr_int(__xdrs__, &__tag__);\n");
	printf("	if ((__status__ != TRUE) || (__tag__ != 0)) {\n");
	printf("            callErr = -1; return");
	if (retS) printf("(%s)", retVar);
	printf(";\n        }\n");
	printf("        __xdrs__->x_op = XDR_ENCODE;\n");
	printf("        __version__ = %d;\n", spec->getVersion());
	printf("        if ((xdr_int(__xdrs__, &__tag__) != TRUE) ||\n");
	printf("            (xdr_String(__xdrs__,&__ProtocolName__) != TRUE) || \n");
	printf("            (xdr_int(__xdrs__, &__version__) != TRUE) ||\n");
	printf("            (xdrrec_endofrecord(__xdrs__, TRUE) != TRUE)) {\n");
	printf("            callErr = -1; return");
	if (retS) printf("(%s)", retVar);
	printf(";\n        }\n");
	printf("	__versionVerifyDone__ = TRUE;\n");
	printf("    }\n");
    }
    printf("    __tag__ = %s_%s_REQ;\n", spec->getName(), name);
    printf("    __xdrs__->x_op = XDR_ENCODE;\n");

    // the error check depends on short circuit boolean expression evaluation!
    printf("    if ((xdr_int(__xdrs__, &__tag__) != TRUE) ||\n");
    for (lp = args; *lp; lp++) {
	  printf("        (xdr_%s(__xdrs__, &%s) != TRUE) ||\n",
		 (*lp)->type, (*lp)->name);
    }
    printf("        !xdrrec_endofrecord(__xdrs__, TRUE)) {\n");
    printf("            callErr = -1;\n");
    printf("            return");
    if (retS) printf("(%s)", retVar);
    printf(";\n    }\n");

    if (upcall != asyncUpcall) {
	if (upcall == notUpcall) {
	    printf("    awaitResponce(%s_%s_RESP);\n",spec->getName(), name);
	    if (retS)
	      printf("    if (callErr == -1) return(%s);\n", retVar);
	} else if (upcall == syncUpcall) {
	    printf("    __xdrs__->x_op = XDR_DECODE;\n");
	    printf("    if (xdrrec_skiprecord(__xdrs__) == FALSE)\n");
	    printf("        {callErr = -1; return ");
	    if (retS)
	      printf("(%s);}\n", retVar);
	    else
	      printf(";}\n");
	    printf("    if (xdr_int(__xdrs__, &__tag__) == FALSE)\n");
	    printf("        {callErr = -1; return ");
	    if (retS)
	      printf("(%s);}\n", retVar);
	    else
	      printf(";}\n");
	    printf("    assert(__tag__ == %s_%s_RESP);\n", spec->getName(), name);
	}
	if (strcmp(retType, "void")) {
	  printf("    __xdrs__->x_op = XDR_DECODE;\n");
	  printf("    if (xdr_%s(__xdrs__, &(%s)) == FALSE)\n", retType, retVar);
	  printf("       callErr = -1;\n");
	  printf("    return(%s);\n", retVar);
	}
    }
    printf("}\n\n");
}


void remoteFunc::genPVMStub(char *className)
{
    List<argument*> lp;
    char *retVar = spec->genVariable();

    genMethodHeader(className, 0);
    printf(" {\n");

    printf("    unsigned int __tag__;\n");
    if (strcmp(retType, "void")) {
	printf("    %s %s;\n", retType, retVar);
    }

    // set callErr for non-upcalls
    if ((upcall == notUpcall) || (upcall == notUpcallAsync))
      printf("     callErr = 0;\n");

    printf("    __tag__ = %s_%s_REQ;\n", spec->getName(), name);
    printf("    assert(pvm_initsend(0) >= 0);\n");
    for (lp = args; *lp; lp++) {
	printf("    IGEN_pvm_%s(IGEN_PVM_ENCODE, &%s);\n", (*lp)->type, (*lp)->name);
    }
    printf("    pvm_send ( get_other_tid(), __tag__);\n");
    if (upcall != asyncUpcall)
      {
	if (upcall == notUpcall)
	  {
	    printf("    awaitResponce(%s_%s_RESP);\n",spec->getName(), name);
	  }
	else if (upcall == syncUpcall)
	  {
	    printf("    if (pvm_recv(-1, %s_%s_RESP) < 0) abort();\n",
		   spec->getName(), name);
	  }
	if (strcmp(retType, "void"))
	  {
	    printf("    IGEN_pvm_%s(IGEN_PVM_DECODE, &(%s)); \n", retType, retVar);
	    printf("    return(%s);\n", retVar);
	  }
      }
    printf("}\n\n");
}

void remoteFunc::genStub(char *className, Boolean forUpcalls)
{
    if (forUpcalls && ((upcall == notUpcall) || (upcall == notUpcallAsync))) return;
    if (!forUpcalls && (upcall != notUpcall) && (upcall != notUpcallAsync)) return;

    printf("\n");

    if (generateXDR) {
	genXDRStub(className);
    } else if (generateTHREAD) {
	genThreadStub(className);
    } else if (generatePVM) {
        genPVMStub(className);
    }
}

void remoteFunc::genHeader()
{
    List<char *> cl;
    List<argument*> lp;

    printf("struct %s {\n", structName);
    for (lp = args; *lp; lp++) {
	printf("    %s ", (*lp)->type);
	for (cl = *(*lp)->stars; *cl; cl++) printf("*");
	printf("%s", (*lp)->name);
	printf(";\n");
    }
    printf("};\n\n");

    printf("#define %s_%s_REQ %d\n", spec->getName(),name, spec->getNextTag());
    printf("#define %s_%s_RESP %d\n", spec->getName(), name,spec->getNextTag());
}

void typeDefn::genHeader()
{
    List<field*> fp;

    printf("#ifndef %s_TYPE\n", name);
    printf("#define %s_TYPE\n", name);
    printf("class %s {  \npublic:\n", name);
    if (arrayType) {
	printf("    int count;\n");
	printf("    %s* data;\n", type);
    } else {
	for (fp = fields; *fp; fp++) {
	    (*fp)->genHeader();
	}
    }
    printf("};\n\n");
    printf("#endif\n");

    if (generateXDR) {
	if (userDefined) printf("extern xdr_%s(XDR*, %s*);\n", name, name);
    } else if (generatePVM) {
        if (userDefined) printf("extern IGEN_pvm_%s(IGEN_PVM_FILTER, %s*);\n", name, name);
    }
}

//
// this is only called if generatePVM or generateXDR is TRUE
//
void typeDefn::genBundler()
{
    List<field*> fp;

    if (!userDefined) return;

    if (generateXDR) {
      printf("bool_t xdr_%s(XDR *__xdrs__, %s *__ptr__) {\n", name, name);
      printf("    if (__xdrs__->x_op == XDR_FREE) {\n");
      if (arrayType)
	{
	  // 
	  assert (foundType = types.find(type));
	  if (foundType->userDefined == TRUE)
	    {
	      printf("        int i;\n");
	      printf("        for (i=0; i<__ptr__->count; ++i)\n");
	      printf("            if (!xdr_%s(__xdrs__, &(__ptr__->data[i])))\n", type);
	      printf("              return FALSE;\n");
	    }
	  printf("        free ((char*) __ptr__->data);\n");
	  printf("        __ptr__->data = 0;\n");
	  printf("        __ptr__->count = 0;\n");
	}
      else for (fp = fields; *fp; fp++) {
	foundType = types.find((*fp)->getType());
	assert (foundType);
	if (foundType->userDefined ||
	    foundType->arrayType ||
	    !(strcmp("String", foundType->name)))
	  (*fp)->genBundler();
      }
    } else if (generatePVM) {
      printf("bool_t IGEN_pvm_%s(IGEN_PVM_FILTER __dir__, %s *__ptr__) {\n", name, name);
      printf("    if (__dir__ == IGEN_PVM_FREE) {\n");
      if (arrayType)
	printf("      free((char*) __ptr__->data);\n");
      for (fp = fields; *fp; fp++) {
	foundType = types.find((*fp)->getType());
	assert (foundType);
	if (foundType->userDefined ||
	    foundType->arrayType ||
	    !(strcmp("String", foundType->name)))
	  (*fp)->genBundler();
      }
    }
    printf ("    } else {\n");

    if (arrayType) {
      if (generateXDR) {
	printf("if (__xdrs__->x_op == XDR_DECODE) __ptr__->data = NULL;\n");
	printf("    if (xdr_array(__xdrs__, &(__ptr__->data), &__ptr__->count, ~0, sizeof(%s), xdr_%s) == FALSE)\n", type, type);
	printf("      return FALSE;\n");
      } else if (generatePVM) {
	printf("    IGEN_pvm_Array_of_%s(__dir__, &(__ptr__->data),&(__ptr__->count));\n", type);
      }
    } else {
	for (fp = fields; *fp; fp++) {
	    (*fp)->genBundler();
	}
      }
    printf("    }\n");
    printf("    return(TRUE);\n");
    printf("}\n\n");
}


char *findAndAddArrayType(char *name) 

{
    typeDefn *s;
    char temp[80];
    char *arrayTypeName;

    // now find the associated array type.
    sprintf(temp, "%s_Array", name);
    arrayTypeName = pool.findAndAdd(temp);
    if (!types.find(arrayTypeName)) {
	s = new typeDefn(arrayTypeName, name);
	s->genHeader();
    }
    return arrayTypeName;
}

void
print_pass_thru (const char *the_string)
{
  printf ("       %s;\n", the_string);
}

// changed to print to separate files, one include for the server, and
// one for the client 
// g++ was attempting to link the virtual functions for the client in the
// server code
// 
// kludgeOut is used in an inelegant manner, hence the name
// Two .h files are produced for the client and server classes, and these
// files used to go into the normal .h file, but the g++ linker was trying
// to link the virtual functions when it should not have been.  Thus, 
// the normal .h file is closed here and two new files are created.
// kludgeOut points to the original .h file that had been opened.
void interfaceSpec::genClass()
{
    List <remoteFunc *> curr; 
    FILE *tempHeader;
    char *filename;

    fflush (stdout);
    fclose(kludgeOut);

    filename = new char[strlen(protoFile) + 7];
    sprintf (filename, "%sCLNT.h", protoFile);
    tempHeader = fopen (filename, "w");
    dup2(tempHeader->_file, 1);

    printf( "#ifndef _%sCLNT_H\n", name);
    printf( "#define _%sCLNT_H\n", name);
    printf( "#include \"%sh\"\n\n", protoFile);
    printf( "class %sUser: public RPCUser, public %s {\n", name, transportBase);
    printf( "  public:\n");
    client_pass_thru.map(&print_pass_thru);
    printf( "    static int __wellKnownPortFd__;\n");

    if (generateXDR) {
      printf( "    virtual void verifyProtocolAndVersion();\n");
      printf( "    %sUser(int fd, xdrIOFunc r, xdrIOFunc w, int nblock=0);\n", name);
      printf( "    %sUser(int family, int port, int type, char *host, xdrIOFunc rf, xdrIOFunc wf, int nblock=0);\n", name);
      printf( "    %sUser(char *machine, char *login, char *program, xdrIOFunc r, xdrIOFunc w, char **args=0, int nblock=0);\n", name);
    } else if (generatePVM) {
      printf( "    virtual void verifyProtocolAndVersion();\n");
      printf( "    %sUser(char *w, char *p, char **a, int f);\n", name);
      printf( "    %sUser(int other);\n", name);
      printf( "    %sUser();\n", name);
    } else if (generateTHREAD) {
	printf("    %sUser(int tid): THREADrpc(tid) {}\n", name);
    }
      
    printf( "    void awaitResponce(int);\n");
    printf( "    int isValidUpCall(int);\n");
    
    for (curr = methods; *curr; curr++) {
      printf("    ");
	(*curr)->genMethodHeader(NULL, 1);
	printf(";\n");
    }
    printf("};\n");
    printf("#endif\n");

    fflush (stdout);
    fclose(tempHeader);

    sprintf (filename, "%sSRVR.h", protoFile);
    tempHeader = fopen (filename, "w");
    dup2(tempHeader->_file, 1);

    printf( "#ifndef _%sSRVR_H\n", name);
    printf( "#define _%sSRVR_H\n", name);
    printf( "#include \"%sh\"\n\n", protoFile);
    printf("class %s: private RPCServer, public %s {\n", name, transportBase);
    printf("  public:\n");

    server_pass_thru.map(&print_pass_thru);

    if (generatePVM) {
      printf("   %s();\n", name);
      printf("   %s(int o);\n", name);
      printf("   %s(char *w, char *p, char **a, int f);\n", name);
    }
    else if (generateXDR) {
      printf("   %s(int family, int port, int type, char *host, xdrIOFunc rf, xdrIOFunc wf, int nblock=0);\n", name);
      printf("   %s(int fd, xdrIOFunc r, xdrIOFunc w, int nblock=0);\n", name);
      printf("   %s(char *m, char *l, char *p, xdrIOFunc r, xdrIOFunc w, char **args=0, int nblock=0);\n", name);
    } else if (generateTHREAD) {
	printf("    %s(int tid): THREADrpc(tid) {}\n", name);
      }
    printf("    mainLoop(void);\n");
    printf("    int callErr;\n");
    for (curr = methods; *curr; curr++) {
	printf("    ");
	(*curr)->genMethodHeader(NULL, 0);
	printf(";\n");
    }
    if (generatePVM || generateXDR) {
      printf("    void set_versionVerifyDone(bool_t val)\n");
      printf("           { __versionVerifyDone__ = val;}\n");
      printf("  private:\n");
      printf("    bool_t __versionVerifyDone__;\n");
    }
    printf("};\n\n");
    printf("#endif\n");

    fflush (stdout);
    fclose(tempHeader);
    delete [] filename;

    // must reopen this file since some code may be passed through to it
    kludgeOut = fopen (headerFile, "a");
    assert (kludgeOut);
    dup2(kludgeOut->_file, 1);
}

//
// builds filters for PVM to go into *.C
// these filters are similar to the xdr_TYPE {TYPE = int, char...}
// these filters are meant to be bidirectional
// these filters will malloc for arrays and strings
//
void
buildPVMfilters()
{
  printf ("bool_t IGEN_pvm_String (IGEN_PVM_FILTER direction, String *data) {\n");
  printf ("   int buf_id, bytes, msgtag, tid;\n");
  printf ("   assert(data);\n");
  printf ("   switch (direction) \n");
  printf ("       {\n");
  printf ("         case IGEN_PVM_DECODE:\n");
  printf ("             buf_id = pvm_getrbuf();\n");
  printf ("             if (buf_id == 0) return (FALSE);\n");
  printf ("             if (pvm_bufinfo(buf_id, &bytes, &msgtag, &tid) < 0) return(FALSE);\n");
  printf ("             *data = (String) new char[bytes+1];\n");
  printf ("             if (!(*data)) return (FALSE);\n");
  printf ("             data[bytes] = '\\0';\n");
  printf ("             if (pvm_upkstr(*data) < 0) return (FALSE);\n");
  printf ("             break;\n");
  printf ("         case IGEN_PVM_ENCODE:\n");
  printf ("             if (pvm_pkstr (*data) < 0) return (FALSE);\n");
  printf ("             break;\n");
  printf ("         case IGEN_PVM_FREE:\n");
  printf ("             delete [] (*data);\n");
  printf ("             *data = 0;\n");
  printf ("             break;\n");
  printf ("         default:\n");
  printf ("             assert(0);\n");
  printf ("        }\n");
  printf ("   return(TRUE);\n}\n");

  pvm_types.map(&templatePVMscalar);
  pvm_types.map(&templatePVMarray);
}


void
templatePVMscalar (const pvm_args *the_arg)
{
  if (!the_arg || !(the_arg->type_name) || !(the_arg->pvm_name)) return;
  printf ("\n");
  printf ("bool_t IGEN_pvm_%s (IGEN_PVM_FILTER direction, %s *data) {\n", the_arg->type_name, the_arg->type_name);
  printf ("   assert(data);\n");
  printf ("   switch (direction)\n");
  printf ("     {\n");
  printf ("        case IGEN_PVM_DECODE:\n");
  printf ("            if (pvm_upk%s (data, 1, 1) < 0)\n", the_arg->pvm_name);
  printf ("                return (FALSE);\n");
  printf ("            break;\n");
  printf ("        case IGEN_PVM_ENCODE:\n");
  printf ("            if (pvm_pk%s (data, 1, 1) < 0)\n", the_arg->pvm_name);
  printf ("                return (FALSE);\n");
  printf ("            break;\n");
  printf ("        case IGEN_PVM_FREE:\n");
  printf ("            break;\n");
  printf ("        default:\n");
  printf ("            assert(0);\n");
  printf ("     }\n");
  printf ("   return(TRUE);\n");
  printf ("}\n\n");
}

void
templatePVMarray (const pvm_args * the_arg)
{
  if (!the_arg ||
      !(the_arg->type_name) ||
      !(the_arg->pvm_name) ||
      !(the_arg->arg)) 
    return;
  printf ("bool_t IGEN_pvm_Array_of_%s (IGEN_PVM_FILTER dir, %s **data, int *count) {\n", the_arg->type_name, the_arg->type_name);
  printf ("   int bytes, msgtag, tid;\n");
  printf ("   switch (dir)\n");
  printf ("     {\n");
  printf ("        case IGEN_PVM_DECODE:\n");
  printf ("           int buf_id = pvm_getrbuf();\n");
  printf ("           if (buf_id == 0) return (FALSE);\n");
  printf ("           if (pvm_bufinfo(buf_id, &bytes, &msgtag, &tid) < 0) return(FALSE);\n");
  printf ("           *count = bytes %s;\n", the_arg->arg);
  printf ("           *data = (%s *) new char[*count];\n", the_arg->type_name);
  printf ("           if (!(*data)) return (FALSE);\n");
  printf ("           if (pvm_upk%s(*data, *count, 1) < 0) return (FALSE);\n", the_arg->pvm_name);
  printf ("           break;\n");
  printf ("        case IGEN_PVM_ENCODE:\n");
  printf ("           if (pvm_pk%s (*data, *count, 1) < 0) return (FALSE);\n", the_arg->pvm_name);
  printf ("           break;\n");
  printf ("        case IGEN_PVM_FREE:\n");
  printf ("           delete [] (*data);\n");
  printf ("           break;\n");
  printf ("        default:\n");
  printf ("           assert(0);\n");
  printf ("     }\n");
  printf ("   return(TRUE);\n");
  printf ("}\n\n");
}


//
// generate the pvm specific includes for _.h
//
void
buildPVMincludes()
{
  printf ("enum IGEN_PVM_FILTER {IGEN_PVM_ENCODE, IGEN_PVM_DECODE, IGEN_PVM_FREE};\n\n");
  printf ("bool_t IGEN_pvm_String (IGEN_PVM_FILTER direction, String *data);\n");
  pvm_types.map(&PVM_map_scalar_includes);
  pvm_types.map(&PVM_map_array_includes);
}

void genPVMServerCons(char *name)
{
  printf("%s::%s(char *w, char *p, char **a, int f):\n", name, name);
  printf("PVMrpc(w, p, a, f) { __versionVerifyDone__ = FALSE; }\n");
  printf("%s::%s(int o):\n", name, name);
  printf("PVMrpc(o) { __versionVerifyDone__ = FALSE; }\n");
  printf("%s::%s():\n", name, name);
  printf("PVMrpc() { __versionVerifyDone__ = FALSE; }\n");
}

void genXDRServerCons(char *name)
{
  printf("%s::%s(int family, int port, int type, char *host, xdrIOFunc rf, xdrIOFunc wf, int nblock):\n", name, name);
  printf("XDRrpc(family, port, type, host, rf, wf, nblock)\n");
  printf("  { __versionVerifyDone__ = FALSE;}\n");

  printf("%s::%s(int fd, xdrIOFunc r, xdrIOFunc w, int nblock):\n", name, name);
  printf("XDRrpc(fd, r, w, nblock)\n");
  printf("  { __versionVerifyDone__ = FALSE;}\n");

  printf("%s::%s(char *m,char *l,char *p,xdrIOFunc r,xdrIOFunc w, char **args, int nblock):\n",
	name, name);
  printf("    XDRrpc(m, l, p, r, w, args, nblock)\n");
  printf("  { __versionVerifyDone__ = FALSE;}\n");
}

pvm_args *
buildPVMargs_ptr (char *type_name, char *pvm_name, char *arg)
{
  pvm_args *arg_ptr = NULL;
  
  if (!pvm_name || !type_name)
    return NULL;

  arg_ptr = (pvm_args *) malloc (sizeof(pvm_args));
  if (!arg_ptr)
    return NULL;

  arg_ptr->type_name = (char *) malloc (strlen(type_name) + 1);
  arg_ptr->pvm_name = (char *) malloc (strlen(type_name) + 1);
  if (arg)
    {
      arg_ptr->arg = (char *) malloc (strlen(arg) + 1);
      if (!(arg_ptr->arg)) return NULL;
      if (!strcpy (arg_ptr->arg, arg)) return NULL;
    }
  if (!(arg_ptr->type_name) || !(arg_ptr->pvm_name))
    return NULL;

  if (!strcpy (arg_ptr->pvm_name, pvm_name)) return NULL;
  if (!strcpy (arg_ptr->type_name, type_name)) return NULL;

  return arg_ptr;
}

//
// builds List pvm_types with PVM type information
// this list is used by 2 procedures
//
void
buildPVMargs()
{
  pvm_args *arg_ptr = 0;

  pvm_args *buildPVMargs_ptr (char *type_name, char *pvm_name, char *arg);

  assert (arg_ptr = buildPVMargs_ptr ("int", "int", " >> 2 "));
  pvm_types.add(arg_ptr);

  assert (arg_ptr = buildPVMargs_ptr ("char", "byte", "  "));
  pvm_types.add(arg_ptr);

  assert (arg_ptr = buildPVMargs_ptr ("float", "float", " >> 2 "));
  pvm_types.add(arg_ptr);

  assert (arg_ptr = buildPVMargs_ptr ("double", "double", " >> 3 "));
  pvm_types.add(arg_ptr);

  assert (arg_ptr = buildPVMargs_ptr ("u_int", "uint", " >> 2 "));
  pvm_types.add(arg_ptr);
}

void
PVM_map_scalar_includes (const pvm_args * the_arg)
{
  if (!the_arg || !(the_arg->type_name)) return;
  printf ("bool_t IGEN_pvm_%s (IGEN_PVM_FILTER direction, %s *data);\n", the_arg->type_name, the_arg->type_name);
}

void
PVM_map_array_includes (const pvm_args * the_arg)
{
  if (!the_arg || !(the_arg->type_name)) return;
  printf ("bool_t IGEN_pvm_Array_of_%s(IGEN_PVM_FILTER direction, %s **data, int *count);\n", the_arg->type_name, the_arg->type_name, the_arg->type_name);
}

void
add_to_list (char *type, char *name, List<char*> *ptrs, List<char*> *holder) 
{
  int new_length;
  char *new_string;
  char *ptr_string;
  int i, count;

  count = ptrs->count();

  ptr_string = new char[count + 1];
  ptr_string[0] = '\0';
  for (i=0; i < count; ++i)
    ptr_string[i] = '*';
  ptr_string[count] = '\0';

  new_length = strlen(type) + strlen(name) + 3 + count;
  new_string = new char[new_length];
  
  sprintf (new_string, "%s %s %s", type, ptr_string, name);
  holder->add (new_string);
}

void 
addCMember (char *type, char *name, List<char*> *ptrs)
{
  add_to_list (type, name, ptrs, &client_pass_thru);
}

void 
addSMember (char *type, char *name, List<char*> *ptrs)
{
  add_to_list (type, name, ptrs, &server_pass_thru);
}


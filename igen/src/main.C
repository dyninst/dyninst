/*
 * main.C - main function of the interface compiler igen.
 *
 * $Log: main.C,v $
 * Revision 1.25  1994/07/28 22:28:05  krisna
 * changed output file definitions to declarations,
 * xdr_array changes to conform to prototype
 * changed "," to "<<"
 *
 * Revision 1.24  1994/06/02  23:34:26  markc
 * New igen features: error checking, synchronous upcalls.
 *
 * Revision 1.23  1994/04/07  19:03:33  markc
 * Fixed bug for async client calls with threads.  Removed msg_recv.
 *
 * Revision 1.22  1994/04/06  21:28:45  markc
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
char *protoFile;
stringPool pool;
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

// generate code to handle igen errors
// the code to set the error state will be generated prior to this function being
// called
void interfaceSpec::genErrHandler(ofstream &ofstr, Boolean client)
{
  if (client)
    ofstr << " void " << name << "User::handle_error()\n" << "  {\n";
  else
    ofstr << " void " << name << "::handle_error()\n" << " {\n";

  ofstr << "       fprintf(stderr, \"Error not handled: err_state = %d\\n\", err_state);\n";
  ofstr << "       IGEN_ERR_ASSERT;\n";
  ofstr << "       exit(-1);\n";
  ofstr << " }\n";
}

// this goes into the dot_c file since all client and server code go into one file
// for threads. (an igen artifact).
// this generates code for ::mainLoop, including the code for the switch statement
// that handles all of the message types.
//
void interfaceSpec::generateThreadLoop()
{
    List <remoteFunc*> cf;

    unionName = genVariable();
    dot_c << "union " << unionName << " {\n";
    for (cf = methods; *cf; cf++)
      dot_c << "    struct " << (*cf)->structName << " " << (*cf)->name << ";\n";

    dot_c << "};\n\n";

    dot_c << "int " << name << "::mainLoop(void)\n";
    dot_c << "{\n";
    dot_c << "  unsigned int __len__;\n";
    dot_c << "  unsigned int __tag__;\n";
    dot_c << "  int  __val__ = THR_OKAY;\n";
    dot_c << "  union " << unionName << " __recvBuffer__;\n";
    dot_c << "\n";
    dot_c << "  if (err_state != igen_no_err) return (-1);\n";
    dot_c << "  __tag__ = MSG_TAG_ANY;\n";
    dot_c << "  __len__ = sizeof(__recvBuffer__);\n";
    dot_c << "  if ((requestingThread = msg_recv(&__tag__, &__recvBuffer__, &__len__)) ==\n";
    dot_c << "            THR_ERR)\n";
    dot_c << "      {\n";
    dot_c << "         err_state = igen_read_err;\n";
    dot_c << "         handle_error();\n";
    dot_c << "         return (-1);\n";
    dot_c << "      }\n";

    dot_c << "  switch (__tag__)\n" << "     {\n";

    for (cf = methods; *cf; cf++)
     (*cf)->genSwitch(FALSE, "-1", dot_c);

    dot_c << "    default:\n";
    dot_c << "	    return(__tag__);\n";
    dot_c << "  }\n";
    
    dot_c << "  if (__val__ != THR_OKAY)\n";
    dot_c << "    {\n";
    dot_c << "        err_state = igen_send_err;\n";
    dot_c << "        handle_error();\n";
    dot_c << "        return (-1);\n";
    dot_c << "    }\n";

    dot_c << "  return(0);\n";
    dot_c << "}\n";
}

// generate the main loop for xdr
void interfaceSpec::generateXDRLoop()
{
    List <remoteFunc*> cf;

    genXDRServerVerifyProtocol();
    genXDRLookForVerify();

    srvr_dot_c << "int " << name  << "::mainLoop(void)\n";
    srvr_dot_c << "{\n";
    srvr_dot_c << "    unsigned int __tag__, __status__;\n";
    srvr_dot_c << "    if (err_state != igen_no_err) return (-1);\n";
    srvr_dot_c << "    __xdrs__->x_op = XDR_DECODE;\n";
    srvr_dot_c << "    if (xdrrec_skiprecord(__xdrs__) == FALSE)\n";
    srvr_dot_c << "        {\n";
    srvr_dot_c << "             err_state = igen_read_err;\n";
    srvr_dot_c << "             handle_error();\n";
    srvr_dot_c << "             return (-1);\n";
    srvr_dot_c << "        }\n";
    srvr_dot_c << "    __status__ = xdr_int(__xdrs__, &__tag__);\n";
    srvr_dot_c << "	if (!__status__)\n";
    srvr_dot_c << "        {\n";
    srvr_dot_c << "             err_state = igen_read_err;\n";
    srvr_dot_c << "             handle_error();\n";
    srvr_dot_c << "             return(-1);\n";
    srvr_dot_c << "        }\n";
    srvr_dot_c << "    switch (__tag__)\n " << "     {\n";
    
    // generate the zero RPC that returns interface name & version.
    srvr_dot_c << "        case 0:\n";
    srvr_dot_c << "            if (verify_protocol())\n";
    srvr_dot_c << "                return (-1);\n";
    srvr_dot_c << "            break;\n";

    for (cf = methods; *cf; cf++)
      (*cf)->genSwitch(FALSE, "-1", srvr_dot_c);

    srvr_dot_c << "    default:\n";
    srvr_dot_c << "     return(__tag__);\n";
    srvr_dot_c << "  }\n";
    srvr_dot_c << "  return(0);\n";
    srvr_dot_c << "}\n";
}

// generate the main loop for pvm
void interfaceSpec::generatePVMLoop()
{
    List <remoteFunc*> cf;

    srvr_dot_c << "int " << name << "::mainLoop(void)\n";
    srvr_dot_c << "{\n";
    srvr_dot_c << "    int __tag__, __bytes__, __msgtag__, __tid__, __bufid__, __other__, __count__;\n";
    srvr_dot_c << "    struct taskinfo __taskp__, *__tp__;\n";
    srvr_dot_c << "    __tp__ = &__taskp__;\n";
    srvr_dot_c << "    if (err_state != igen_no_err) return (-1);\n";
    srvr_dot_c << "    // this code checks for message \n";
    srvr_dot_c << "    __other__ = get_other_tid();\n";
    srvr_dot_c << "    if (__other__ < -1) return -1;\n";
    srvr_dot_c << "    if (get_error() == -1) return -1;\n";
    srvr_dot_c << "    if ((__bufid__ = pvm_recv (__other__, -1)) < 0) return -1;\n";
    srvr_dot_c << "    if (pvm_bufinfo (__bufid__, &__bytes__, &__msgtag__, &__tid__) < 0) return -1;\n";
    srvr_dot_c << "    switch (__msgtag__)\n" << "     {\n";

    // generate the zero PVM that returns interface name & version.
    srvr_dot_c << "        case 0:\n";
    srvr_dot_c << "            char *__ProtocolName__ = \"" << name << "\";\n";
    srvr_dot_c << "            int __val__;\n";
    srvr_dot_c << "            __val__ = 0;\n";
    srvr_dot_c << "            assert(pvm_initsend(0) >= 0);\n";
    srvr_dot_c << "            pvm_pkstr(__ProtocolName__);\n";
    srvr_dot_c << "            __val__ = %d;\n" << version;
    srvr_dot_c << "            pvm_pkint(&__val__, 1, 1);\n";
    srvr_dot_c << "            pvm_send (__tid__, 0);";
    srvr_dot_c << "            break;\n";

    for (cf = methods; *cf; cf++)
      (*cf)->genSwitch(FALSE, "-1", srvr_dot_c);

    srvr_dot_c << "    default:\n";
    srvr_dot_c << "	    return(__tag__);\n";
    srvr_dot_c << "  }\n";
    srvr_dot_c << "  return(0);\n";
    srvr_dot_c << "}\n";
}


// the include files for *SRVR.C and *.C
void interfaceSpec::genIncludes(ofstream &output)
{
    output << "#include <stdio.h>\n";
    output << "#include <stdlib.h>\n";
    output << "#include <rpc/types.h>\n";
    output << "#include <assert.h>\n";

    if (generateTHREAD) {
	output << "extern \"C\" {\n";
	output << "#include \"thread/h/thread.h\"\n";
	output << "#include <errno.h>\n";
	output << "}\n";
    }
    if (generateXDR) {
	output << "extern \"C\" {\n";
	output << "#include <rpc/xdr.h>\n";
	output << "#include <errno.h>\n";
	output << "}\n";
    }
    if (generatePVM) {
        output << "extern \"C\" {\n";
	output << "#include <pvm3.h> \n";
	output << "#include <errno.h>\n";
	output << "}\n";
      }

    // printf("#include \"%s\"\n\n", headerFile);
}

// generates code for the server
// this code is written into  *.SRVR.C, except for thread code
void interfaceSpec::generateServerCode()
{
    List<remoteFunc*> cf;

    if (generateTHREAD) {
	generateThreadLoop();

	// generate error handler code
	genErrHandler(dot_c, FALSE);
    } else if (generateXDR) {
	genIncludes(srvr_dot_c);

	// include server header
	srvr_dot_c << "#include \"" << protoFile << "SRVR.h\"\n";

	generateXDRLoop();

	//
	// generate XDR-only class constructors to allow server to 
	// start clients
	//
	genXDRServerCons(name);

	// generate error handler code
	genErrHandler(srvr_dot_c, FALSE);
    } else if (generatePVM) {

	genIncludes(srvr_dot_c);

	// include server header
	srvr_dot_c << "#include \"" << protoFile << "SRVR.h\"\n";
	
	generatePVMLoop();

	//
	// generate PVM-only class constructors to allow server to 
	// start clients
	//
	genPVMServerCons(name);

	// generate error handler code
	genErrHandler(srvr_dot_c, FALSE);
      }

    // generate stubs for upcalls.
    for (cf = methods; *cf; cf++)
      (*cf)->genStub(name, TRUE, srvr_dot_c);
}


void interfaceSpec::genWaitLoop() 
{
    List<remoteFunc*> cf;

    // generate a loop to wait for a tag, and call upcalls as they arrive.
    clnt_dot_c << "void " << name << "User::awaitResponce(int __targetTag__) {\n";
    clnt_dot_c << "    unsigned int __tag__;\n";
    if (generateTHREAD)
      {
	clnt_dot_c << "  union " << unionName << " __recvBuffer__;\n";
	clnt_dot_c << "  unsigned __len__ = sizeof(__recvBuffer__);\n";
      }
    else if (generatePVM)
      clnt_dot_c << "    int __tid__, __bufid__, __bytes__;\n";

    // make sure it is safe to proceed
    clnt_dot_c << "  if (err_state != igen_no_err) return;\n";

    clnt_dot_c << "  while (1)\n " << "      {\n";
    if (generateXDR) {
	clnt_dot_c << "    __xdrs__->x_op = XDR_DECODE;\n";
	clnt_dot_c << "    if (xdrrec_skiprecord(__xdrs__) == FALSE)\n";
	clnt_dot_c << "       {\n";
	clnt_dot_c << "          err_state = igen_read_err;\n";
	clnt_dot_c << "          handle_error();\n";
	clnt_dot_c << "          return;\n";
	clnt_dot_c << "       }\n";
	clnt_dot_c << "    if (xdr_int(__xdrs__, &__tag__) == FALSE)\n"
;	clnt_dot_c << "       {\n";
	clnt_dot_c << "          err_state = igen_decode_err;\n";
	clnt_dot_c << "          handle_error();\n";
	clnt_dot_c << "          return;\n";
	clnt_dot_c << "       }\n";
    } else if (generatePVM) {
        clnt_dot_c << "    int __other__ = get_other_tid();\n";
	clnt_dot_c << "    if (get_error() == -1) abort();\n";
	clnt_dot_c << "     if (__other__ < 0) abort();\n";
	clnt_dot_c << "    if ((__bufid__ = pvm_recv (__other__, -1)) < 0)\n";
	clnt_dot_c << "        abort();\n";
	clnt_dot_c << "    if (pvm_bufinfo(__bufid__, &__bytes__, (int*) &__tag__, &__tid__) < 0)\n";
	clnt_dot_c << "       abort();\n";
    } else if (generateTHREAD) {
	clnt_dot_c << "  __tag__ = MSG_TAG_ANY;\n";
	clnt_dot_c << "    if ((requestingThread = msg_recv(&__tag__, (void *) &__recvBuffer__, &__len__)) == THR_ERR)\n";
      	clnt_dot_c << "       {\n";
	clnt_dot_c << "          err_state = igen_read_err;\n";
	clnt_dot_c << "          handle_error();\n";
	clnt_dot_c << "          return;\n";
	clnt_dot_c << "       }\n";
	
    }
    // look for success error message
    clnt_dot_c << "    if (__tag__ == __targetTag__) return;\n";

    clnt_dot_c << "    switch (__tag__)\n" << "         {\n";
    for (cf = methods; *cf; cf++) {
	(*cf)->genSwitch(TRUE, " ", clnt_dot_c);
    }
    clnt_dot_c << "	    default: \n";
    clnt_dot_c << "             err_state = igen_request_err;\n";
    clnt_dot_c << "             handle_error();\n";
    clnt_dot_c << "             return;\n";
    clnt_dot_c << "    }\n";
    clnt_dot_c << "	if (__targetTag__ == -1) return;\n";
    clnt_dot_c << "  }\n";
    clnt_dot_c << "}\n";

    clnt_dot_c << "int " << name << "User::isValidUpCall(int tag) {\n";
    clnt_dot_c << "    return((tag >= " << baseTag << ") && (tag <= " << boundTag << "));\n",
    clnt_dot_c << "}\n";
}

void interfaceSpec::genProtoVerify()
{
    // generate stub to verify version.
    clnt_dot_c << "void " << name << "User::verifyProtocolAndVersion() {\n";
    clnt_dot_c << "    unsigned int __tag__;\n";
    clnt_dot_c << "    String proto;\n";
    clnt_dot_c << "    int version;\n";
    clnt_dot_c << "    __tag__ = 0;\n";
    clnt_dot_c << "    __xdrs__->x_op = XDR_ENCODE;\n";
    clnt_dot_c << "    if (xdr_int(__xdrs__, &__tag__) != TRUE)\n";
    clnt_dot_c << "       {\n";
    clnt_dot_c << "          err_state = igen_encode_err;\n";
    clnt_dot_c << "          handle_error();\n";
    clnt_dot_c << "          return;\n";
    clnt_dot_c << "       }\n";
    clnt_dot_c << "     if (xdrrec_endofrecord(__xdrs__, TRUE) != TRUE)";
    clnt_dot_c << "       {\n";
    clnt_dot_c << "          err_state = igen_read_err;\n";
    clnt_dot_c << "          handle_error();\n";
    clnt_dot_c << "          return;\n";
    clnt_dot_c << "       }\n";
    clnt_dot_c << "    awaitResponce(0);\n";
    clnt_dot_c << "    if (err_state != igen_no_err)\n";
    clnt_dot_c << "       {\n";
    clnt_dot_c << "           printf(\"Protocol verify - no response from server\\n\");\n";
    clnt_dot_c << "           handle_error();";
    clnt_dot_c << "	      exit(-1);\n";
    clnt_dot_c << "       }\n";
    clnt_dot_c << "    __xdrs__->x_op = XDR_DECODE;\n";
    clnt_dot_c << "    if ((xdr_String(__xdrs__, &(proto)) == FALSE) ||\n";
    clnt_dot_c << "        (xdr_int(__xdrs__, &(version)) == FALSE))\n";
    clnt_dot_c << "       {\n";
    clnt_dot_c << "          err_state = igen_decode_err;\n";
    clnt_dot_c << "          printf(\"Protocol verify - bad response from server\\n\");\n";
    clnt_dot_c << "          handle_error();";
    clnt_dot_c << "	     exit(-1);\n";
    clnt_dot_c << "       }\n";
    clnt_dot_c << "    if ((version != " << version << ") || (strcmp(proto, \"" << name << "\"))) {\n";
    clnt_dot_c << "         printf(\"protocol " << name << " version " << version << " expected\\n\");\n", 
    clnt_dot_c << "         printf(\"protocol %s version %d found\\n\", proto, version);\n";
    clnt_dot_c << "	    exit(-1);\n";
    clnt_dot_c << "    }\n";
    clnt_dot_c << "    __xdrs__->x_op = XDR_FREE;\n";
    clnt_dot_c << "    xdr_String (__xdrs__, &proto);\n";
    clnt_dot_c << "}\n";
    clnt_dot_c << "\n\n";
    clnt_dot_c << name << "User::" << name << "User(int fd, xdrIOFunc r, xdrIOFunc w, int nblock):\n";
    clnt_dot_c << "RPCUser(igen_no_err),\n";
    clnt_dot_c << "XDRrpc(fd, r, w, nblock) {\n";
    clnt_dot_c << "    if (__xdrs__) verifyProtocolAndVersion();\n";
    clnt_dot_c << "    IGEN_in_call_handler = 0;\n";
    clnt_dot_c << "}\n";
    clnt_dot_c << name << "User::" << name << "User(int family, int port, int type, char *machine, xdrIOFunc rf, xdrIOFunc wr, int nblock):\n";
    clnt_dot_c << "RPCUser(igen_no_err),\n";
    clnt_dot_c << "XDRrpc(family, port, type, machine, rf, wr, nblock) {\n";
    clnt_dot_c << "    if (__xdrs__) verifyProtocolAndVersion();\n";
    clnt_dot_c << "    IGEN_in_call_handler = 0;\n";
    clnt_dot_c << "}\n";
    clnt_dot_c << name << "User::" << name << "User(char *m,char *l,char *p,xdrIOFunc r,xdrIOFunc w, char **args, int nblock):\n";
    clnt_dot_c << "RPCUser(igen_no_err),\n";
    clnt_dot_c << "    XDRrpc(m, l, p, r, w, args, nblock, __wellKnownPortFd__) {\n";
    clnt_dot_c << "    if (__xdrs__) verifyProtocolAndVersion();\n";
    clnt_dot_c << "    IGEN_in_call_handler = 0;\n";
    clnt_dot_c << "}\n";
    clnt_dot_c << "\n";
}


//
// generate code to perform protocol verification 
//
void interfaceSpec::genProtoVerifyPVM()
{
    // generate stub to verify version.
    clnt_dot_c << "void " << name << "User::verifyProtocolAndVersion() {\n";
    clnt_dot_c << "    unsigned int __tag__;\n";
    clnt_dot_c << "    String proto;\n";
    clnt_dot_c << "    int version = -1;\n";
    clnt_dot_c << "    __tag__ = 0;\n";
    clnt_dot_c << "    // msgtag = 0 --> verify protocol\n";
    clnt_dot_c << "    assert (pvm_initsend(0) >= 0);\n";
    clnt_dot_c << "    pvm_send (get_other_tid(), 0);\n";
    clnt_dot_c << "    awaitResponce(0);\n";
    clnt_dot_c << "    IGEN_pvm_String (IGEN_PVM_DECODE, &proto);\n";
    clnt_dot_c << "    pvm_upkint(&(version), 1, 1);\n";
    clnt_dot_c << "    if ((version != " << version << " ) || (strcmp(proto, \"";
    clnt_dot_c << name << "\"))) {\n";
    clnt_dot_c << "        printf(\"protocol " << name << " version " << version;
    clnt_dot_c << " expected\\n\");\n";
    clnt_dot_c << "        printf(\"protocol %%s version %%d found\\n\", proto, version);\n";
    clnt_dot_c << "	    pvm_exit(); exit(-1);\n";
    clnt_dot_c << "    }\n";
    clnt_dot_c << "    IGEN_pvm_String (IGEN_PVM_FREE, &proto);\n";
    clnt_dot_c << "}\n";
    clnt_dot_c << "\n\n";
    clnt_dot_c << name << "User::" << name << "User(char *w, char *p, char **a, int f):\n";
    clnt_dot_c << "PVMrpc(w, p, a, f) {\n";
    clnt_dot_c << "if (get_error() != -1) verifyProtocolAndVersion();\n";
    clnt_dot_c << "    IGEN_in_call_handler = 0;\n";
    clnt_dot_c << "}\n";
    clnt_dot_c << name << "User::" << name << "User(int o):\n";
    clnt_dot_c << "PVMrpc(o) {\n";
    clnt_dot_c << "if (get_error() != -1) verifyProtocolAndVersion();\n";
    clnt_dot_c << "    IGEN_in_call_handler = 0;\n";
    clnt_dot_c << "}\n";

    clnt_dot_c << name << "User::" << "User():\n";
    clnt_dot_c << "PVMrpc() {\n";
    clnt_dot_c << "if (get_error() != -1) verifyProtocolAndVersion();\n";
    clnt_dot_c << "    IGEN_in_call_handler = 0;\n";
    clnt_dot_c << "}\n";
    clnt_dot_c << "\n";
}


void interfaceSpec::generateStubs(ofstream &output)
{
    List <remoteFunc*> cf;
    char className[80];

    sprintf(className, "%sUser", name);
    for (cf = methods; *cf; cf++) {
	(*cf)->genStub(className, FALSE, output);
    }
}

void interfaceSpec::generateClientCode()
{
    if (generateXDR) {
	genIncludes(clnt_dot_c);

	// include client header
	clnt_dot_c << "#include \"" << protoFile << "CLNT.h\"\n";

	clnt_dot_c << "int " << name << "User::__wellKnownPortFd__;\n";

	// generate the error handler for the client
	genErrHandler(clnt_dot_c, TRUE);
    } else if (generatePVM) {
	genIncludes(clnt_dot_c);

	// include client header
	clnt_dot_c << "#include \"" << protoFile << "CLNT.h\"\n";

	// generate the error handler for the client
	genErrHandler(clnt_dot_c, TRUE);
      }
    else {
      // generate the error handler for the client
      genErrHandler(dot_c, TRUE);
    }

    generateStubs(clnt_dot_c);

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


    // open the files for output
    char *dump_to = new char[strlen(protoFile) + 10];
    assert(dump_to);

    sprintf(dump_to, "%sh", protoFile);
    dot_h.open(dump_to, ios::out);
    assert(dot_h.good());

    sprintf(dump_to, "%sC", protoFile);
    dot_c.open(dump_to, ios::out);
    assert(dot_c.good());

    sprintf(dump_to, "%sSRVR.h", protoFile);
    srvr_dot_h.open(dump_to, ios::out);
    assert(srvr_dot_h.good());

    sprintf(dump_to, "%sCLNT.h", protoFile);
    clnt_dot_h.open(dump_to, ios::out);
    assert(clnt_dot_h.good());

    if (generateXDR || generatePVM)
      {
	sprintf(dump_to, "%sSRVR.C", protoFile);
	srvr_dot_c.open(dump_to, ios::out);
	assert(srvr_dot_c.good());

	sprintf(dump_to, "%sCLNT.C", protoFile);
	clnt_dot_c.open(dump_to, ios::out);
	assert(clnt_dot_c.good());
      }
    else
      {
	srvr_dot_c = dot_c;
	clnt_dot_c = dot_c;
	assert(clnt_dot_c.good());
	assert(srvr_dot_c.good());
      }

    if (emitHeader) {
        char *prefix;
	int len;
	len = strlen(protoFile);
	prefix = new char[len];
	strncpy (prefix, protoFile, len - 1);
	prefix[len-1] = '\0';

	dot_h << "#ifndef " << prefix << "BASE_H\n";
	dot_h << "#define " << prefix << "BASE_H\n";
	dot_h << "#include \"util/h/rpcUtil.h\"\n";

	if (generatePVM)
	  dot_h << "#include \"util/h/rpcUtilPVM.h\"\n";

	dot_h << "#include <sys/types.h>\n";
	delete [] prefix;

	dot_h << "#include <assert.h>\n";

	dot_h << "#ifdef IGEN_ERR_ASSERT_ON\n";
	dot_h << "#ifndef IGEN_ERR_ASSERT\n";
	dot_h << "#define IGEN_ERR_ASSERT assert(0)\n";
	dot_h << "#endif\n";
	dot_h << "#else\n";
	dot_h << "#define IGEN_ERR_ASSERT\n";
	dot_h << "#endif\n\n";

	dot_h << "\n // Errors that can occur internal to igen\n";
	dot_h << "#ifndef _IGEN_ERR_DEF\n";
	dot_h << "#define _IGEN_ERR_DEF\n";
	dot_h << "typedef enum e_IGEN_ERR {\n";
	dot_h << "                       igen_no_err=0,\n";
	dot_h << "                       igen_encode_err,\n";
	dot_h << "                       igen_decode_err,\n";
	dot_h << "                       igen_send_err,\n";
	dot_h << "                       igen_read_err,\n";
	dot_h << "                       igen_request_err,\n";
	dot_h << "                       igen_call_err\n";
	dot_h << "                       }  IGEN_ERR;\n\n";
	dot_h << "#endif\n";

	if (generatePVM) 
	  buildPVMincludes();
      }

    yyparse();

    // this is opened in genClass
    if (emitHeader) {
      dot_h << "\n#endif\n";
      dot_h << endl;
    }

    if (!currentInterface) {
	printf("no interface defined\n");
	exit(-1);
    }

    if (emitCode) {
        currentInterface->genIncludes(dot_c);
	
	if (generateTHREAD) {
	  dot_c << "#include \"" << protoFile << "SRVR.h\"\n";
	  dot_c << "#include \"" << protoFile << "CLNT.h\"\n";
	} else {
	  dot_c << "#include \"" << protoFile << "h\"\n";
	}

	if (generatePVM)
	  buildPVMfilters();

	if (generateXDR || generatePVM) currentInterface->generateBundlers();
	
	currentInterface->generateServerCode();

	currentInterface->generateClientCode();
	fflush(stdout);
    }

    
    if (generateTHREAD)
      {
      }
    else
      {
	srvr_dot_c.close();
	clnt_dot_c.close();
      }
    exit(0);

    srvr_dot_h.close();
    clnt_dot_h.close();

    dot_c.close();
    dot_h.close();
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
void remoteFunc::genMethodHeader(char *className, int in_client,
				 ofstream &current)
{
  int spitOne;
  List <char *> cl;
  List<argument*> lp;
  
  if (className) {
    current << retType << " " << className << "::" << name << "(";
  } else {
    // figure out if "virtual" should be printed
    // here is the logic
    // for the client --> if it is an upcall then print virtual
    // for the server --> if it is not an upcall the print virtual
    // for both an additional constraint is that virtual_f must be true
    if ((((upcall == syncUpcall) || (upcall == asyncUpcall)) && virtual_f && in_client) ||
	(!in_client && virtual_f && ((upcall == asyncCall) || (upcall == syncCall))))
      {
	// print virtual
	current << "virtual " << retType << " " << name << "(";
      }
    else
      {
	// don't print virtual
	current << retType << " " << name << "(";
      }
  }
  for (lp = args, spitOne = 0; *lp; lp++)
    {
      if (spitOne) current << ",";
      spitOne =1;
      current << (*lp)->type << " ";
      
      for (cl = *(*lp)->stars; *cl; cl++)
	current << "*";
      current << (*lp)->name;
    }
  if (!spitOne) current << "void";
  current << ")";
}

// forUpcalls == TRUE -> when client code is being generated
// client code is being generated to handle upcalls in awaitResponce
//
// this procedure prints code under 2 circumstances
// 1-  (client code) forUpcalls == TRUE, call must be an upcall
// 2-  (server code) forUpcalls == FALSE, call must not be an upcall
//
void remoteFunc::genSwitch(Boolean forUpcalls, char *ret_str, ofstream &output)
{
    int first;
    List <argument *> ca;

    // return if generating client code, and call is not an upcall
    if (forUpcalls &&
	((upcall == syncCall) || (upcall == asyncCall)))
      return;

    // return if generating server code, and call is an upcall
    if (!forUpcalls && (upcall != syncCall) && (upcall != asyncCall))
      return;

    output << "        case " << spec->getName() << "_" << name << "_REQ:\n";

    // print out the return type
    if ((generateTHREAD) && (upcall != asyncUpcall) && (upcall != asyncCall))
      // output << "	        int __val__;\n";
      ;

    output << "           {\n";
    if (strcmp(retType, "void")) {
	output << "	        " << retType << " __ret__;\n";
    }

    // print out the local types
    if (generateXDR) {
	output << "	   // extern xdr_" << retType << "(XDR*, " << retType << "*);\n";
	if (args.count()) {
	    output << "            " << structName << " __recvBuffer__;\n";
		for (ca = args; *ca; ca++) {
		output << "            if (xdr_" << (*ca)->type << "(__xdrs__, &__recvBuffer__.";
		output << (*ca)->name << ") == FALSE)\n";
		output << "              {\n";
		output << "                  err_state = igen_decode_err;\n";
		output << "                  handle_error();\n";
		output << "                  return " << ret_str << ";\n";
		output << "              }\n";

	    }
	}
    } else if (generatePVM) {
        output << "            extern IGEN_pvm_" << retType << "(IGEN_PVM_FILTER, " << retType << "*);\n";
	if ((upcall != asyncUpcall) && (upcall != asyncCall))
	  output << "            assert(pvm_initsend(0) >= 0);\n";
	if (args.count()) {
	  output << "            " << structName << " __recvBuffer__;\n";
	  for (ca = args; *ca; ca++) {
	    output << "            IGEN_pvm_" << (*ca)->type << "(IGEN_PVM_DECODE, &__recvBuffer__." << (*ca)->name << ");\n";

	  }
	}
      }

    // to determine when sync upcalls cannot be made
    if (generateXDR &&
        (upcall == asyncUpcall || upcall == asyncCall))
      output << "                 IGEN_in_call_handler = 1;\n";

    if (strcmp(retType, "void")) {
	output << "                __ret__ = " << name << "(";
    } else {
	output << "                 " << name <<  "(";
    }

    for (ca = args, first = 1; *ca; ca++) {
	if (!first) output << ",";
	first = 0;
	if (generateTHREAD) {
	    output << "__recvBuffer__." << name << "." << (*ca)->name;
	} else if (generateXDR) {
	    output << "__recvBuffer__." << (*ca)->name;
	} else if (generatePVM) {
	  output << "__recvBuffer__." << (*ca)->name;
	}
    }
    output << ");\n";

    // to determine when sync upcalls cannot be made
    if (generateXDR &&
        (upcall == asyncUpcall || upcall == asyncCall))
      output << "                 IGEN_in_call_handler = 0;\n";

    if (generateTHREAD && (upcall != asyncUpcall) && (upcall != asyncCall)) {
	if (strcmp(retType, "void")) {
	    output << "               __val__ = msg_send(requestingThread, ";
	    output << spec->getName();
	    output << "_" << name << "_RESP, (void *) &__ret__, sizeof(";
	    output << retType << "));\n";
	} else {
	  output << "                 __val__ = msg_send(requestingThread, ";
	  output << spec->getName();
	  output << "_" << name << "_RESP, NULL, 0);\n";
	}
	// output << "         if (__val__ != THR_OKAY)\n";
	// output << "            {\n";
	// output << "                 err_state = igen_send_error;\n";
	// output << "                 handle_error();\n";
	// output << "                 return " << ret_str << ";\n";
	// output << "            }\n";
    } else if (generateXDR && (upcall != asyncUpcall) && (upcall != asyncCall)) {
	output << "       	    __xdrs__->x_op = XDR_ENCODE;\n";
	output << "                 __tag__ = " << spec->getName() << "_" << name;
	output << "_RESP;\n";
	output << "                if (xdr_int(__xdrs__, &__tag__) == FALSE)\n";
	output << "                    {\n";
	output << "                      err_state = igen_encode_err;\n";
	output << "                      handle_error();\n";
	output << "                      return " << ret_str << ";\n";
	output << "                    }\n";
	if (strcmp(retType, "void")) {
	    output << "                   if (xdr_" << retType << "(__xdrs__,&__ret__) == FALSE)\n";
	    output << "                   {\n";
	    output << "                      err_state = igen_encode_err;\n";
	    output << "                      handle_error();\n";
	    output << "                      return " << ret_str << ";\n";
	    output << "                   }\n";
	}
	output << "	    if (xdrrec_endofrecord(__xdrs__, TRUE) == FALSE)\n";
	output << "            {\n";
	output << "               err_state = igen_send_err;\n";
	output << "               handle_error();\n";
	output << "               return " << ret_str << ";\n";
	output << "            }\n";
    } else if (generatePVM && (upcall != asyncUpcall) && (upcall != asyncCall)) {
	output << "            __tag__ = " << spec->getName() << "_" << name << "_RESP;\n";
	if (strcmp(retType, "void")) {
	    output << "            IGEN_pvm_" << retType << " (IGEN_PVM_ENCODE,&__ret__);\n";
	  }
	output << "            assert (pvm_send (__tid__, __tag__) >= 0);\n";
      }

    // free allocated memory
    if (generateTHREAD)
      {
      }
    else if (generateXDR)
      {
	output << "            __xdrs__->x_op = XDR_FREE;\n";
	if (args.count())
	  {
	    for (ca = args; *ca; ca++)
	      {
		// only if Array or string or user defined
		if ((*ca)->mallocs)
		  output << "            xdr_" << (*ca)->type << "(__xdrs__, &__recvBuffer__." << (*ca)->name<< ");\n";
	      }
	  }
	if (retStructs)
	  output << "            xdr_" << retType << "(__xdrs__, &__ret__);\n";
      }
    else if (generatePVM)
      {
	if (args.count())
	  {
	    for (ca = args; *ca; ca++)
	      {
		// only if Array or string 
		if ((*ca)->mallocs)
		  output << "            IGEN_pvm_" << (*ca)->type;
		  output << "(IGEN_PVM_FREE, &__recvBuffer__." << (*ca)->name << ");\n";
	      }
	  }
	if (retStructs)
	  output << "            IGEN_pvm_" << retType << "(IGEN_PVM_FREE, &__ret__);\n";
      }

    output << "           }\n              break;\n\n";
}
 

// generates the stub code for the client or the server
//
void remoteFunc::genThreadStub(char *className, ofstream &output)
{
    List<argument*> lp;
    char *retVar = 0;
    char *structUseName = spec->genVariable();

    genMethodHeader(className, 0, output);
    output << " {\n";

    if (*args)
      output << "    struct " << structName << " " << structUseName << ";\n";
    if ((upcall != asyncUpcall) && (upcall != asyncCall))
      output << "    unsigned int __tag__;\n";
    if (strcmp(retType, "void"))
      output << "    unsigned int __len__;\n";
    output << "    int __val__;\n";
    if (strcmp(retType, "void")) {
      retVar = spec->genVariable();
      output << "    " << retType << " " << retVar << ";\n";
    }
    else
      retVar = strdup(" ");

    output << "    if (err_state != igen_no_err)\n";
    output << "      {\n";
    output << "        IGEN_ERR_ASSERT;\n";
    output << "        return " << retVar << ";\n";
    output << "      }\n";

    for (lp = args; *lp; lp++) {
	output << "    " << structUseName << "." << (*lp)->name << " = ";
	output << (*lp)->name << ";  \n";
    }
    if (*args) {
	output << "    __val__ = msg_send(tid, " << spec->getName() << "_";
	output << name << "_REQ, (void *) &";
	output << structUseName << ", sizeof(" << structUseName << "));\n";
    } else {
	output << "    __val__ = msg_send(tid, " << spec->getName() << "_";
	output << name <<  "_REQ, NULL, 0); \n";
    }
    output << "    if (__val__ != THR_OKAY)\n";
    output << "      {\n";
    output << "          err_state = igen_send_err;\n";
    output << "          handle_error();\n";
    output << "          return " << retVar << " ;\n";
    output << "      }\n";
    if ((upcall != asyncUpcall) && (upcall != asyncCall)) {
	output << "    __tag__ = " << spec->getName() << "_" << name << "_RESP;\n";
	if (strcmp(retType, "void")) {
	    output << "    __len__ = sizeof(" << retVar << ");\n";
	    output << "    if (msg_recv(&__tag__, (void *) &" << retVar;
	    output << ", &__len__)\n        == THR_ERR) \n";
	    output << "      {\n";
	    output << "          err_state = igen_read_err;\n";
	    output << "          handle_error();\n";
	    output << "          return " << retVar << " ;\n";
	    output << "      }\n";
	    output << "    if (__len__ != sizeof(" << retVar << "))\n";
	    output << "      {\n";
	    output << "          err_state = igen_read_err;\n";
	    output << "          handle_error();\n";
	    output << "          return " << retVar << " ;\n";
	    output << "      }\n";
	} else {
	    output << "    if (msg_recv(&__tag__, NULL, 0) == THR_ERR)\n";
	    output << "      {\n";
	    output << "          err_state = igen_read_err;\n";
	    output << "          handle_error();\n";
	    output << "          return " << retVar << " ;\n";
	    output << "      }\n";

	}
	output << "    if (__tag__ != " << spec->getName() << "_" << name;
	output << "_RESP)\n";
	output << "      {\n";
	output << "          err_state = igen_read_err;\n";
	output << "          handle_error();\n";
	output << "          return " << retVar << " ;\n";
	output << "      }\n";

	output << "    return " << retVar << " ;\n";
    }
    output << "}\n\n";
}

void remoteFunc::genXDRStub(char *className, ofstream &output)
{
  List<argument*> lp;
  char *retVar = 0;
  int retS = 0;

  genMethodHeader(className, 0, output);
  output << "\n {\n";

  output << "    unsigned int __tag__;\n";
  if (strcmp(retType, "void"))
    {
      retVar = spec->genVariable();
      output << "    " << retType << " " << retVar << ";\n";
      retS = 1;
    }
  else
    retVar = strdup(" ");

  output << "    if (err_state != igen_no_err)\n";
  output << "      {\n";
  output << "        IGEN_ERR_ASSERT;\n";
  output << "        return " << retVar << ";\n";
  output << "      }\n";

  if ((upcall == syncUpcall) || (upcall == syncCall))
    {
      output << "       if (IGEN_in_call_handler)\n";
      output << "         {\n";
      output << "            fprintf(stderr, \"cannot do sync call when in async call handler\\n\");\n";
      output << "            IGEN_ERR_ASSERT;\n";
      output << "            err_state = igen_call_err;\n";
      output << "            return " << retVar << ";\n";
      output << "         }\n";
    }

  // check to see protocol verify has been done.
  if ((upcall != syncCall) && (upcall != asyncCall)) {
    output << "    if (!__versionVerifyDone__)\n";
    output << "       {\n";
    output << "    // handle error is called for errors\n";
    output << "          if (!look_for_verify() && !verify_protocol())\n";
    output << "              __versionVerifyDone__ = TRUE;\n";
    output << "          else\n";
    output << "              return " << retVar << " ;\n";
    output << "       }\n";
  }
  output << "    __tag__ = " << spec->getName() << "_" << name << "_REQ;\n";
  output << "    __xdrs__->x_op = XDR_ENCODE;\n";

  // the error check depends on short circuit boolean expression evaluation!
  output << "    if ((xdr_int(__xdrs__, &__tag__) != TRUE)\n";
  for (lp = args; *lp; lp++)
    {
      output << "      ||  (xdr_" << (*lp)->type << "(__xdrs__, &";
      output << (*lp)->name << ") != TRUE)\n";
    }
  output << " )\n";
  output << "         {\n";
  output << "             err_state = igen_encode_err;\n";
  output << "             handle_error();\n";
  output << "             return " << retVar << " ;\n";
  output << "         }\n";
  output << "       if(!xdrrec_endofrecord(__xdrs__, TRUE))\n";
  output << "         {\n";
  output << "             err_state = igen_send_err;\n";
  output << "             handle_error();\n";
  output << "             return " << retVar << " ;\n";
  output << "         }\n";

  if ((upcall != asyncUpcall) && (upcall != asyncCall))
    {
      if (upcall == syncCall)
	{
	  output << "    awaitResponce(" << spec->getName() << "_";
	  output << name << "_RESP);\n";
	}
      else if (upcall == syncUpcall)
	{
          output << " int res;\n";
          output << " while (!(res = mainLoop())) ;\n";
          output << " if (res != " << spec->getName() << "_" << name << "_RESP)\n";
          output << "        { err_state = igen_request_err; handle_error(); return ";
          if (retS)
            output << "(" << retVar << ");}\n";
          else
            output << ";}\n";
	}
      output << "    if (err_state != igen_no_err)\n";
      output << "    return " << retVar << ";\n";

      if (strcmp(retType, "void"))
	{
	  output << "    __xdrs__->x_op = XDR_DECODE;\n";
	  output << "    if (xdr_" << retType << "(__xdrs__, &(";
	  output << retVar << ")) == FALSE)\n";
	  output << "         {\n";
	  output << "             err_state = igen_decode_err;\n";
	  output << "             handle_error();\n";
	  output << "             return " << retVar << " ;\n";
	  output << "         }\n";
	  
	  output << "    return(" << retVar << ");\n";
	}
    }
  output << "}\n\n";
}

void remoteFunc::genPVMStub(char *className, ofstream &output)
{
    List<argument*> lp;
    char *retVar = spec->genVariable();

    genMethodHeader(className, 0, output);
    output << " {\n";

    output << "    unsigned int __tag__;\n";
    if (strcmp(retType, "void")) {
	output << "    " << retType << " " << retVar << ";\n";
    }

    output << "    if (err_state != igen_no_err)\n";
    output << "      {\n";
    output << "        IGEN_ERR_ASSERT;\n";
    output << "        return " << retVar << ";\n";
    output << "      }\n";

    output << "    __tag__ = " << spec->getName() << "_" << name << "_REQ;\n";
    output << "    assert(pvm_initsend(0) >= 0);\n";
    for (lp = args; *lp; lp++) {
	output << "    IGEN_pvm_" << (*lp)->type << " (IGEN_PVM_ENCODE, &" << (*lp)->name << ");\n";
    }
    output << "    pvm_send ( get_other_tid(), __tag__);\n";
    if (upcall != asyncUpcall)
      {
	if (upcall == syncCall)
	  {
	    output << "    awaitResponce(" << spec->getName() << "_" << name << "_RESP);\n";
	  }
	else if (upcall == syncUpcall)
	  {
	    output << "    if (pvm_recv(-1, " << spec->getName() << "_" << name << "_RESP) < 0) abort();\n";
	  }
	if (strcmp(retType, "void"))
	  {
	    output << "    IGEN_pvm_" << retType << "(IGEN_PVM_DECODE, &(" << retVar << ")); \n";
	    output << "    return(" << retVar << ");\n";
	  }
      }
    output << "}\n\n";
}

void remoteFunc::genStub(char *className, Boolean forUpcalls, ofstream &output)
{
    if (forUpcalls && ((upcall == syncCall) || (upcall == asyncCall))) return;
    if (!forUpcalls && (upcall != syncCall) && (upcall != asyncCall)) return;

    output << "\n";

    if (generateXDR) {
	genXDRStub(className, output);
    } else if (generateTHREAD) {
	genThreadStub(className, output);
    } else if (generatePVM) {
        genPVMStub(className, output);
    }
}

void remoteFunc::genHeader()
{
    List<char *> cl;
    List<argument*> lp;

    dot_h << "struct " << structName << " {\n";
    for (lp = args; *lp; lp++) {
	dot_h << "    " << (*lp)->type << " ";
	for (cl = *(*lp)->stars; *cl; cl++)
	  dot_h << "*";
	dot_h << (*lp)->name;
	dot_h << ";\n";
    }
    dot_h << "};\n\n";

    dot_h << "#define " << spec->getName() << "_" << name << "_REQ " << spec->getNextTag() << "\n";
    dot_h << "#define " << spec->getName() << "_" << name << "_RESP " << spec->getNextTag() << "\n";
}

void typeDefn::genHeader()
{
    List<field*> fp;

    dot_h << "#ifndef " << name << "_TYPE\n";
    dot_h << "#define " << name << "_TYPE\n";
    dot_h << "class " << name << " {  \npublic:\n";
    if (arrayType) {
	dot_h << "    int count;\n";
	dot_h << "    " << type << "* data;\n";
    } else {
	for (fp = fields; *fp; fp++) {
	    (*fp)->genHeader(dot_h);
	}
    }
    dot_h << "};\n\n";
    dot_h << "#endif\n";

    if (generateXDR) {
	if (userDefined) dot_h << "extern xdr_" << name << "(XDR*, " << name << "*);\n";
    } else if (generatePVM) {
        if (userDefined) dot_h << "extern IGEN_pvm_" << name << "(IGEN_PVM_FILTER, " << name << "*);\n";
    }
}

//
// this is only called if generatePVM or generateXDR is TRUE
// this is not called for generateTHREAD since no bundling is done since everything
// exists in the same address space
// builds data bundlers - code that bundles user defined structures for the transport
// note - the basic filters {string, int, ...} are taken care of elsewhere
//
//
// how are bundlers built
//     each bundler has an if branch and an else branch.  The if branch is taken if the
//     request is to free memory, otherwise the else branch is taken.  If the type of the
//     bundler is an array of <x>, each element of the array must have its bundler called
//     in the if branch.  If the bundler is not an array of some type, but is a structure,
//     with fields, the bundlers for each field is called.
//
void typeDefn::genBundler()
{
    List<field*> fp;


    if (!userDefined) return;

    // the code for the if branch - taken to free memory

    if (generateXDR) {
      // build the handlers for xdr
      dot_c << "bool_t xdr_" << name << "(XDR *__xdrs__, " << name << " *__ptr__) {\n";
      dot_c << "    if (__xdrs__->x_op == XDR_FREE) {\n";

      // note - a user defined type contains an array of other types, each element in the
      // array must be freed
      if (arrayType)
	{
	  assert (foundType = types.find(type));
	  if (foundType->userDefined == TRUE)
	    {
	      dot_c << "        int i;\n";
	      dot_c << "        for (i=0; i<__ptr__->count; ++i)\n";
	      dot_c << "            if (!xdr_" << type << "(__xdrs__, &(__ptr__->data[i])))\n";
	      dot_c << "              return FALSE;\n";
	    }
	  // free the memory used for this structure
	  dot_c << "        free ((char*) __ptr__->data);\n";
	  dot_c << "        __ptr__->data = 0;\n";
	  dot_c << "        __ptr__->count = 0;\n";
	}

      else for (fp = fields; *fp; fp++) {
	// xdr - non-array types
	// call the bundler for each element of this user defined type
	// don't call for non-user defined types such as {int, char, bool}
	// since those will not have had memory allocated for them
	foundType = types.find((*fp)->getType());
	assert (foundType);
	if (foundType->userDefined ||
	    foundType->arrayType ||
	    !(strcmp("String", foundType->name)))
	  (*fp)->genBundler(dot_c);
      }
    } else if (generatePVM) {
      // build the handlers for PVM
      dot_c << "bool_t IGEN_pvm_" << name << "(IGEN_PVM_FILTER __dir__, " << name << " *__ptr__) {\n";
      dot_c << "    if (__dir__ == IGEN_PVM_FREE) {\n";

      // free the array of user defined types
      // this calls the bundlers
      if (arrayType)
	{
	  assert (foundType = types.find(type));
	  if (foundType->userDefined == TRUE)
	    {
	      dot_c << "        int i;\n";
	      dot_c << "        for (i=0; i<__ptr__->count; ++i)\n";
	      dot_c << "            if (!IGEN_pvm_" << type << "(__dir__, &(__ptr__->data[i])))\n";
	      dot_c << "              return FALSE;\n";
	    }
	  dot_c << "        free ((char*) __ptr__->data);\n";
	  dot_c << "        __ptr__->data = 0;\n";
	  dot_c << "        __ptr__->count = 0;\n";
	}
      else for (fp = fields; *fp; fp++) {
	foundType = types.find((*fp)->getType());
	assert (foundType);
	if (foundType->userDefined ||
	    foundType->arrayType ||
	    !(strcmp("String", foundType->name)))
	  (*fp)->genBundler(dot_c);
      }
    }
    dot_c << "    } else {\n";

    // the code for the else branch - taken to encode or decode 

    if (arrayType) {
      if (generateXDR) {
	dot_c << "if (__xdrs__->x_op == XDR_DECODE) __ptr__->data = NULL;\n";
	dot_c << "    if (xdr_array(__xdrs__, (char **) &(__ptr__->data), &__ptr__->count, ~0, sizeof(";
	dot_c << type << "), xdr_" << type << ") == FALSE)\n";
	dot_c << "      return FALSE;\n";
      } else if (generatePVM) {
	dot_c << "    IGEN_pvm_Array_of_" << type << "(__dir__, &(__ptr__->data),&(__ptr__->count));\n";
      }
    } else {
	for (fp = fields; *fp; fp++)
	  (*fp)->genBundler(dot_c);
      }
    dot_c << "    }\n";
    dot_c << "    return(TRUE);\n";
    dot_c << "}\n\n";
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
print_pass_thru_clnt (const char *the_string)
{
  clnt_dot_h << "       " << the_string << ";\n";
}

void
print_pass_thru_srvr (const char *the_string)
{
  srvr_dot_h << "       " << the_string << ";\n";
}

// changed to print to separate files, one include for the server, and
// one for the client 
// g++ was attempting to link the virtual functions for the client in the
// server code
// 
void interfaceSpec::genClass()
{
    List <remoteFunc *> curr; 



    clnt_dot_h <<  "#ifndef _" << name << "CLNT_H\n";
    clnt_dot_h <<  "#define _" << name << "CLNT_H\n";
    clnt_dot_h <<  "#include \"" << protoFile << "h\"\n\n";
    clnt_dot_h <<  "class " << name << "User: public RPCUser, public " << transportBase << " {\n";
    clnt_dot_h <<  "  public:\n";
    client_pass_thru.map(&print_pass_thru_clnt);
    clnt_dot_h <<  "    static int __wellKnownPortFd__;\n";

    if (generateXDR) {
      clnt_dot_h <<  "    virtual void verifyProtocolAndVersion();\n";
      clnt_dot_h <<  "    " << name << "User(int fd, xdrIOFunc r, xdrIOFunc w, int nblock=0);\n";
      clnt_dot_h <<  "    " << name << "User(int family, int port, int type, char *host, xdrIOFunc rf, xdrIOFunc wf, int nblock=0);\n";
      clnt_dot_h <<  "    " << name << "User(char *machine, char *login, char *program, xdrIOFunc r, xdrIOFunc w, char **args=0, int nblock=0);\n";
    } else if (generatePVM) {
      clnt_dot_h <<  "    virtual void verifyProtocolAndVersion();\n";
      clnt_dot_h <<  "    " << name << "User(char *w, char *p, char **a, int f);\n";
      clnt_dot_h <<  "    " << name << "User(int other);\n";
      clnt_dot_h <<  "    " << name << "User();\n";
    } else if (generateTHREAD) {
	clnt_dot_h << "    " << name << "User(int tid): THREADrpc(tid), RPCUser(igen_no_err) {}\n";
    }
      
    clnt_dot_h << "    void awaitResponce(int);\n";
    clnt_dot_h << "    int isValidUpCall(int);\n";
    
    for (curr = methods; *curr; curr++) {
      clnt_dot_h << "    ";
	(*curr)->genMethodHeader(NULL, 1, clnt_dot_h);
	clnt_dot_h << ";\n";
    }
    clnt_dot_h << " protected:\n";
    clnt_dot_h << "   virtual void handle_error();\n";
    clnt_dot_h << "   int IGEN_in_call_handler;\n";
    clnt_dot_h << "};\n";
    clnt_dot_h << "#endif\n";

    srvr_dot_h <<  "#ifndef _" << name << "SRVR_H\n";
    srvr_dot_h <<  "#define _" << name << "SRVR_H\n";
    srvr_dot_h <<  "#include \"" << protoFile << "h\"\n\n";
    srvr_dot_h << "class " << name << ": protected RPCServer, public " << transportBase << " {\n";
    srvr_dot_h << "  public:\n";

    server_pass_thru.map(&print_pass_thru_srvr);

    if (generatePVM) {
      srvr_dot_h << "   " << name << "();\n";
      srvr_dot_h << "   " << name << "(int o);\n";
      srvr_dot_h << "   " << name << "(char *w, char *p, char **a, int f);\n";
    }
    else if (generateXDR) {
      srvr_dot_h << "   " << name << "(int family, int port, int type, char *host, xdrIOFunc rf, xdrIOFunc wf, int nblock=0);\n";
      srvr_dot_h << "   " << name << "(int fd, xdrIOFunc r, xdrIOFunc w, int nblock=0);\n";
      srvr_dot_h << "   " << name << "(char *m, char *l, char *p, xdrIOFunc r, xdrIOFunc w, char **args=0, int nblock=0);\n";
      srvr_dot_h << "    verify_protocol();\n";
      srvr_dot_h << "    look_for_verify();\n";
    } else if (generateTHREAD) {
	srvr_dot_h << "    " << name << "(int tid): THREADrpc(tid), RPCServer(igen_no_err) {}\n";
      }
    srvr_dot_h << "    mainLoop(void);\n";

    for (curr = methods; *curr; curr++) {
	srvr_dot_h << "    ";
	(*curr)->genMethodHeader(NULL, 0, srvr_dot_h);
	srvr_dot_h << ";\n";
      }
    if (generatePVM || generateXDR) {
      srvr_dot_h << "    void set_versionVerifyDone(bool_t val)\n";
      srvr_dot_h << "           { __versionVerifyDone__ = val;}\n";
    }
    srvr_dot_h << " protected:\n";
    srvr_dot_h << "   virtual void handle_error();\n";
    srvr_dot_h << "   bool_t __versionVerifyDone__;\n";
    srvr_dot_h << "   int IGEN_in_call_handler;\n";
    srvr_dot_h << "};\n\n";
    srvr_dot_h << "#endif\n";
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
  dot_c << "bool_t IGEN_pvm_String (IGEN_PVM_FILTER direction, String *data) {\n";
  dot_c << "   int buf_id, bytes, msgtag, tid;\n";
  dot_c << "   assert(data);\n";
  dot_c << "   switch (direction) \n";
  dot_c << "       {\n";
  dot_c << "         case IGEN_PVM_DECODE:\n";
  dot_c << "             buf_id = pvm_getrbuf();\n";
  dot_c << "             if (buf_id == 0) return (FALSE);\n";
  dot_c << "             if (pvm_bufinfo(buf_id, &bytes, &msgtag, &tid) < 0) return(FALSE);\n";
  dot_c << "             *data = (String) new char[bytes+1];\n";
  dot_c << "             if (!(*data)) return (FALSE);\n";
  dot_c << "             data[bytes] = '\\0';\n";
  dot_c << "             if (pvm_upkstr(*data) < 0) return (FALSE);\n";
  dot_c << "             break;\n";
  dot_c << "         case IGEN_PVM_ENCODE:\n";
  dot_c << "             if (pvm_pkstr (*data) < 0) return (FALSE);\n";
  dot_c << "             break;\n";
  dot_c << "         case IGEN_PVM_FREE:\n";
  dot_c << "             delete [] (*data);\n";
  dot_c << "             *data = 0;\n";
  dot_c << "             break;\n";
  dot_c << "         default:\n";
  dot_c << "             assert(0);\n";
  dot_c << "        }\n";
  dot_c << "   return(TRUE);\n}\n";

  pvm_types.map(&templatePVMscalar);
  pvm_types.map(&templatePVMarray);
}


void
templatePVMscalar (const pvm_args *the_arg)
{
  if (!the_arg || !(the_arg->type_name) || !(the_arg->pvm_name)) return;
  dot_c << "\n";
  dot_c << "bool_t IGEN_pvm_" << the_arg->type_name << " (IGEN_PVM_FILTER direction,";
  dot_c << the_arg->type_name << " *data) {\n";
  dot_c << "   assert(data);\n";
  dot_c << "   switch (direction)\n";
  dot_c << "     {\n";
  dot_c << "        case IGEN_PVM_DECODE:\n";
  dot_c << "            if (pvm_upk" << the_arg->pvm_name << " (data, 1, 1) < 0)\n";
  dot_c << "                return (FALSE);\n";
  dot_c << "            break;\n";
  dot_c << "        case IGEN_PVM_ENCODE:\n";
  dot_c << "            if (pvm_pk" << the_arg->pvm_name << " (data, 1, 1) < 0)\n";
  dot_c << "                return (FALSE);\n";
  dot_c << "            break;\n";
  dot_c << "        case IGEN_PVM_FREE:\n";
  dot_c << "            break;\n";
  dot_c << "        default:\n";
  dot_c << "            assert(0);\n";
  dot_c << "     }\n";
  dot_c << "   return(TRUE);\n";
  dot_c << "}\n\n";
}

void
templatePVMarray (const pvm_args * the_arg)
{
  if (!the_arg ||
      !(the_arg->type_name) ||
      !(the_arg->pvm_name) ||
      !(the_arg->arg)) 
    return;
  dot_c << "bool_t IGEN_pvm_Array_of_" << the_arg->type_name << " (IGEN_PVM_FILTER dir, ";
  dot_c << the_arg->type_name << " **data, int *count) {\n";
  dot_c << "   int bytes, msgtag, tid;\n";
  dot_c << "   switch (dir)\n";
  dot_c << "     {\n";
  dot_c << "        case IGEN_PVM_DECODE:\n";
  dot_c << "           int buf_id = pvm_getrbuf();\n";
  dot_c << "           if (buf_id == 0) return (FALSE);\n";
  dot_c << "           if (pvm_bufinfo(buf_id, &bytes, &msgtag, &tid) < 0) return(FALSE);\n";
  dot_c << "           *count = bytes " << the_arg->arg << ";\n";
  dot_c << "           *data = (" << the_arg->type_name << " *) new char[*count];\n";
  dot_c << "           if (!(*data)) return (FALSE);\n";
  dot_c << "           if (pvm_upk" << the_arg->pvm_name << " (*data, *count, 1) < 0) return (FALSE);\n";
  dot_c << "           break;\n";
  dot_c << "        case IGEN_PVM_ENCODE:\n";
  dot_c << "           if (pvm_pk" << the_arg->pvm_name << " (*data, *count, 1) < 0) return (FALSE);\n";
  dot_c << "           break;\n";
  dot_c << "        case IGEN_PVM_FREE:\n";
  dot_c << "           delete [] (*data);\n";
  dot_c << "           break;\n";
  dot_c << "        default:\n";
  dot_c << "           assert(0);\n";
  dot_c << "     }\n";
  dot_c << "   return(TRUE);\n";
  dot_c << "}\n\n";
}


//
// generate the pvm specific includes for _.h
//
void
buildPVMincludes()
{
  dot_h << "enum IGEN_PVM_FILTER {IGEN_PVM_ENCODE, IGEN_PVM_DECODE, IGEN_PVM_FREE};\n\n";
  dot_h << "bool_t IGEN_pvm_String (IGEN_PVM_FILTER direction, String *data);\n";
  pvm_types.map(&PVM_map_scalar_includes);
  pvm_types.map(&PVM_map_array_includes);
}

// the constructor for the pvm constructor class
void genPVMServerCons(char *name)
{
  srvr_dot_c << name << "::" << name << " (char *w, char *p, char **a, int f):\n";
  srvr_dot_c << "PVMrpc(w, p, a, f) {\n";
  srvr_dot_c << "    __versionVerifyDone__ = FALSE; IGEN_in_call_handler = 0;}\n";
  srvr_dot_c << name << "::" << name << " (int o):\n";
  srvr_dot_c << "PVMrpc(o) {\n";
  srvr_dot_c << "    __versionVerifyDone__ = FALSE; IGEN_in_call_handler = 0;}\n";
  srvr_dot_c << name << "::" << name << " ():\n";
  srvr_dot_c << "PVMrpc() {\n";
  srvr_dot_c << "    __versionVerifyDone__ = FALSE; IGEN_in_call_handler = 0;}\n";
}

// the constructor for the xdr server class
void genXDRServerCons(char *name)
{
  srvr_dot_c << name << "::" << name << "(int family, int port, int type, char *host, xdrIOFunc rf, xdrIOFunc wf, int nblock):\n";
  srvr_dot_c << "XDRrpc(family, port, type, host, rf, wf, nblock),\n";
  srvr_dot_c << "RPCServer(igen_no_err)\n";
  srvr_dot_c << "  { __versionVerifyDone__ = FALSE; IGEN_in_call_handler = 0;}\n";
  srvr_dot_c << name << "::" << name << "(int fd, xdrIOFunc r, xdrIOFunc w, int nblock):\n";
  srvr_dot_c << "XDRrpc(fd, r, w, nblock),\n";
  srvr_dot_c << "RPCServer(igen_no_err)\n";
  srvr_dot_c << "  { __versionVerifyDone__ = FALSE; IGEN_in_call_handler = 0;}\n";

  srvr_dot_c << name << "::" << name << "(char *m,char *l,char *p,xdrIOFunc r,xdrIOFunc w, char **args, int nblock):\n";
  srvr_dot_c << "    XDRrpc(m, l, p, r, w, args, nblock),\n";
  srvr_dot_c << "    RPCServer(igen_no_err)\n";
  srvr_dot_c << "  { __versionVerifyDone__ = FALSE; IGEN_in_call_handler = 0;}\n";
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
  if (!the_arg || !(the_arg->type_name))
    return;
  dot_h << "bool_t IGEN_pvm_" << the_arg->type_name << " (IGEN_PVM_FILTER direction, ";
  dot_h << the_arg->type_name << " *data);\n";
}

void
PVM_map_array_includes (const pvm_args * the_arg)
{
  if (!the_arg || !(the_arg->type_name))
    return;
  dot_h << "bool_t IGEN_pvm_Array_of_" << the_arg->type_name << " (IGEN_PVM_FILTER ";
  dot_h << "direction, " << the_arg->type_name << " **data, int *count);\n";
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

void
dump_to_dot_h(char *msg)
{
  dot_h << msg;
}

void interfaceSpec::genXDRServerVerifyProtocol()
{
  srvr_dot_c << "   int " << name << "::verify_protocol()\n";
  srvr_dot_c << "   {\n";
  srvr_dot_c << "   char *__ProtocolName__ = \"" << name << "\";\n";
  srvr_dot_c << "   int __val__;\n";
  srvr_dot_c << "   int __sig__ = 0;\n";
  srvr_dot_c << "   __xdrs__->x_op = XDR_ENCODE;\n";
  srvr_dot_c << "    __val__ = " << version << ";\n";
  srvr_dot_c << "   if ((xdr_int(__xdrs__, &__sig__) == FALSE) ||\n";
  srvr_dot_c << "       (xdr_String(__xdrs__, &__ProtocolName__) == FALSE) ||\n";
  srvr_dot_c << "       (xdr_int(__xdrs__, &__val__) == FALSE))\n";
  srvr_dot_c << "      {\n";
  srvr_dot_c << "        err_state = igen_encode_err;\n";
  srvr_dot_c << "        handle_error();\n";
  srvr_dot_c << "        return(-1);\n";
  srvr_dot_c << "      }\n";
  srvr_dot_c << "    if (xdrrec_endofrecord(__xdrs__, TRUE) == FALSE)\n";
  srvr_dot_c << "      {\n";
  srvr_dot_c << "        err_state = igen_send_err;\n";
  srvr_dot_c << "        handle_error();\n";
  srvr_dot_c << "        return(-1);\n";
  srvr_dot_c << "      }\n";
  srvr_dot_c << "    __versionVerifyDone__ = TRUE;\n";
  srvr_dot_c << "    return 0;\n";
  srvr_dot_c << "    }\n";
}

void interfaceSpec::genXDRLookForVerify()
{
  srvr_dot_c << "   int " << name << "::look_for_verify()\n";
  srvr_dot_c << "   {\n";
  srvr_dot_c << "     int __status__, __tag__;\n\n";
  srvr_dot_c << "     if (xdrrec_skiprecord(__xdrs__) == FALSE)\n";
  srvr_dot_c << "        {\n";
  srvr_dot_c << "            err_state = igen_read_err;\n";
  srvr_dot_c << "            handle_error();\n";
  srvr_dot_c << "            return (-1);\n";
  srvr_dot_c << "        }\n\n";
  srvr_dot_c << "     __xdrs__->x_op = XDR_DECODE;\n\n";

  srvr_dot_c << "        __status__ = xdr_int(__xdrs__, &__tag__);\n";
  srvr_dot_c << "	if (__status__ != TRUE)\n";
  srvr_dot_c << "        {\n";
  srvr_dot_c << "            err_state = igen_decode_err;\n";
  srvr_dot_c << "            handle_error();\n";
  srvr_dot_c << "            return (-1);\n";
  srvr_dot_c << "        }\n";
  srvr_dot_c << "       if (__tag__ != 0)\n";
  srvr_dot_c << "        {\n";
  srvr_dot_c << "            err_state = igen_request_err;\n";
  srvr_dot_c << "            handle_error();\n";
  srvr_dot_c << "            return (-1);\n";
  srvr_dot_c << "        }\n";
  srvr_dot_c << "     return 0;\n";
  srvr_dot_c << "   }\n";
}

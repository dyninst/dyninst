
/*
 *
 * $Log: interfaceSpec.C,v $
 * Revision 1.1  1994/08/17 17:51:54  markc
 * Added Makefile for Linux.
 * Support new source files for igen.
 * Separate source files.  ClassDefns supports classes, typeDefns supports
 * structures.
 *
 *
 */

#include "parse.h"

extern int generatePVM;
extern int generateXDR;
extern int generateTHREAD;

extern char *protoFile;
extern char *transportBase;

extern List<char*> client_pass_thru;
extern List<char*> server_pass_thru;

interfaceSpec *currentInterface;

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


// generate code to handle igen errors
// the code to set the error state will be generated prior to this function being
// called
void interfaceSpec::genErrHandler(ofstream &ofstr, int client)
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

// this goes into the dot_c file since all client and server code go
// into one file for threads. (an igen artifact).
// this generates code for ::mainLoop, including the code for the
// switch statement that handles all of the message types.
//
void interfaceSpec::generateThreadLoop()
{
    List <remoteFunc*> cf;

    unionName = genVariable();
    dot_c << "union " << unionName << " {\n";
    for (cf = methods; *cf; cf++)
      dot_c << "    struct " << (*cf)->structName << " " <<
	(*cf)->name << ";\n";

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
    
    dot_c << "  if (__val__ != THR_OKAY) {\n";
    dot_c << "     err_state = igen_send_err;\n";
    dot_c << "     handle_error();\n";
    dot_c << "     return (-1);\n";
    dot_c << "  }\n";

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
    srvr_dot_c << "    if (!xdrrec_skiprecord(__xdrs__)) {\n";
    srvr_dot_c << "          err_state = igen_read_err;\n";
    srvr_dot_c << "          handle_error();\n";
    srvr_dot_c << "          return (-1);\n";
    srvr_dot_c << "    }\n";
    srvr_dot_c << "    if (!(__status__ = xdr_int(__xdrs__, &__tag__))) {\n";
    srvr_dot_c << "          err_state = igen_read_err;\n";
    srvr_dot_c << "          handle_error();\n";
    srvr_dot_c << "          return(-1);\n";
    srvr_dot_c << "    }\n";
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
    srvr_dot_c << "            __val__ = " << version << ";\n";
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

    output << "// Hash pointers to detect previously seen pointers\n";
    if (ptrMode != ptrIgnore) {
      output << "#pragma implementation \"keylist.h\"\n";
      output << "#include \"util/h/keylist.h\"\n";
      output << "\nKHTable<void*> __ptrTable__;\n";
    }

    
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
	clnt_dot_c << "    if (!xdrrec_skiprecord(__xdrs__)) {\n";
	clnt_dot_c << "        err_state = igen_read_err;\n";
	clnt_dot_c << "        handle_error();\n";
	clnt_dot_c << "        return;\n";
	clnt_dot_c << "    }\n";
	clnt_dot_c << "    if (!xdr_int(__xdrs__, &__tag__)) {\n";
	clnt_dot_c << "        err_state = igen_decode_err;\n";
	clnt_dot_c << "        handle_error();\n";
	clnt_dot_c << "        return;\n";
	clnt_dot_c << "    }\n";
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
    clnt_dot_c << "    if (!xdr_String(__xdrs__, &(proto)) ||\n";
    clnt_dot_c << "        !xdr_int(__xdrs__, &(version))) {\n";
    clnt_dot_c << "          err_state = igen_proto_err;\n";
    clnt_dot_c << "          printf(\"Protocol verify - bad response from server\\n\");\n";
    clnt_dot_c << "          handle_error();";
    clnt_dot_c << "	     exit(-1);\n";
    clnt_dot_c << "    }\n";
    clnt_dot_c << "    if ((version != " << version << ") || (strcmp(proto, \"" << name << "\"))) {\n";
    clnt_dot_c << "         printf(\"protocol " << name << " version " << version << " expected\\n\");\n", 
    clnt_dot_c << "         printf(\"protocol %s version %d found\\n\", proto, version);\n";
    clnt_dot_c << "         err_state = igen_proto_err;\n";
    clnt_dot_c << "         handle_error();\n";
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
    List <userDefn*> ud;

    for (ud = userPool; *ud; ud++)
      (*ud)->genBundler();
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
void interfaceSpec::genXDRServerVerifyProtocol()
{
  srvr_dot_c << "   int " << name << "::verify_protocol()\n";
  srvr_dot_c << "   {\n";
  srvr_dot_c << "   char *__ProtocolName__ = \"" << name << "\";\n";
  srvr_dot_c << "   int __val__;\n";
  srvr_dot_c << "   int __sig__ = 0;\n";
  srvr_dot_c << "   __xdrs__->x_op = XDR_ENCODE;\n";
  srvr_dot_c << "    __val__ = " << version << ";\n";
  srvr_dot_c << "   if (!xdr_int(__xdrs__, &__sig__) ||\n";
  srvr_dot_c << "       !xdr_String(__xdrs__, &__ProtocolName__) ||\n";
  srvr_dot_c << "       !xdr_int(__xdrs__, &__val__)) {\n";
  srvr_dot_c << "      err_state = igen_encode_err;\n";
  srvr_dot_c << "      handle_error();\n";
  srvr_dot_c << "      return(-1);\n";
  srvr_dot_c << "   }\n";
  srvr_dot_c << "   if (!xdrrec_endofrecord(__xdrs__, TRUE)) {\n";
  srvr_dot_c << "      err_state = igen_send_err;\n";
  srvr_dot_c << "      handle_error();\n";
  srvr_dot_c << "      return(-1);\n";
  srvr_dot_c << "   }\n";
  srvr_dot_c << "   __versionVerifyDone__ = TRUE;\n";
  srvr_dot_c << "   return 0;\n";
  srvr_dot_c << "   }\n";
}

void interfaceSpec::genXDRLookForVerify()
{
  srvr_dot_c << "   int " << name << "::look_for_verify()\n";
  srvr_dot_c << "   {\n";
  srvr_dot_c << "     int __status__, __tag__;\n\n";
  srvr_dot_c << "     if (!xdrrec_skiprecord(__xdrs__)) {\n";
  srvr_dot_c << "         err_state = igen_read_err;\n";
  srvr_dot_c << "         handle_error();\n";
  srvr_dot_c << "         return (-1);\n";
  srvr_dot_c << "     }\n\n";
  srvr_dot_c << "     __xdrs__->x_op = XDR_DECODE;\n\n";
  srvr_dot_c << "     if (!(__status__  = xdr_int(__xdrs__, &__tag__))) {\n";
  srvr_dot_c << "         err_state = igen_decode_err;\n";
  srvr_dot_c << "         handle_error();\n";
  srvr_dot_c << "         return (-1);\n";
  srvr_dot_c << "     }\n";
  srvr_dot_c << "     if (__tag__) {\n";
  srvr_dot_c << "         err_state = igen_request_err;\n";
  srvr_dot_c << "         handle_error();\n";
  srvr_dot_c << "         return (-1);\n";
  srvr_dot_c << "     }\n";
  srvr_dot_c << "     return 0;\n";
  srvr_dot_c << "   }\n";
}

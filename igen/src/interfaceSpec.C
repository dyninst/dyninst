
/*
 *
 * $Log: interfaceSpec.C,v $
 * Revision 1.4  1994/09/22 00:37:56  markc
 * Made all templates external
 * Made array declarations separate
 * Add class support
 * Add support for "const"
 * Bundle pointers for xdr
 *
 * Revision 1.3  1994/08/18  19:53:25  markc
 * Added support for new files.
 * Removed compiler warnings for solaris2.3
 *
 * Revision 1.2  1994/08/18  05:56:51  markc
 * Changed char*'s to stringHandles
 *
 * Revision 1.1  1994/08/17  17:51:54  markc
 * Added Makefile for Linux.
 * Support new source files for igen.
 * Separate source files.  ClassDefns supports classes, typeDefns supports
 * structures.
 *
 *
 */

#include "parse.h"

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
    ofstr << " void " << (char*)name << "User::handle_error()\n" << "  {\n";
  else
    ofstr << " void " << (char*)name << "::handle_error()\n" << " {\n";

  ofstr << "       fprintf(stderr, \"Error not handled: err_state = %d\\n\", get_err_state());\n";
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
    dot_c << "union " << (char*)unionName << " {\n";
    for (cf = methods; *cf; ++cf)
      dot_c << "    struct " << (*cf)->getStructName() << " " <<
	(*cf)->getRFName() << ";\n";

    dot_c << "};\n\n";

    dot_c << "int " << (char*)name << "::mainLoop(void)\n";
    dot_c << "{\n";
    dot_c << "  unsigned int __len__;\n";
    dot_c << "  unsigned int __tag__;\n";
    dot_c << "  int  __val__ = THR_OKAY;\n";
    dot_c << "  union " << (char*)unionName << " __recvBuffer__;\n";
    dot_c << "\n";
    dot_c << "  if (get_err_state() != igen_no_err) return (-1);\n";
    dot_c << "  __tag__ = MSG_TAG_ANY;\n";
    dot_c << "  __len__ = sizeof(__recvBuffer__);\n";
    dot_c << "  setRequestingThread(msg_recv(&__tag__, &__recvBuffer__, &__len__));\n";
    dot_c << "  if (getRequestingThread() == THR_ERR)\n";
    dot_c << "      {\n";
    dot_c << "         set_err_state(igen_read_err);\n";
    dot_c << "         handle_error();\n";
    dot_c << "         return (-1);\n";
    dot_c << "      }\n";

    dot_c << "  switch (__tag__)\n" << "     {\n";

    for (cf = methods; *cf; ++cf)
     (*cf)->genSwitch(FALSE, "-1", dot_c);

    dot_c << "    default:\n";
    dot_c << "	    return(__tag__);\n";
    dot_c << "  }\n";
    
    dot_c << "  if (__val__ != THR_OKAY) {\n";
    dot_c << "     set_err_state(igen_send_err);\n";
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

    srvr_dot_c << "int " << (char*)name  << "::mainLoop(void)\n";
    srvr_dot_c << "{\n";
    srvr_dot_c << "    unsigned int __tag__, __status__;\n";
    srvr_dot_c << "    if (get_err_state() != igen_no_err) return (-1);\n";
    srvr_dot_c << "    setDir(XDR_DECODE);\n";
    srvr_dot_c << "    if (!xdrrec_skiprecord(getXdrs())) {\n";
    srvr_dot_c << "          set_err_state(igen_read_err);\n";
    srvr_dot_c << "          handle_error();\n";
    srvr_dot_c << "          return (-1);\n";
    srvr_dot_c << "    }\n";
    srvr_dot_c << "    if (!(__status__ = xdr_u_int(getXdrs(), &__tag__))) {\n";
    srvr_dot_c << "          set_err_state(igen_read_err);\n";
    srvr_dot_c << "          handle_error();\n";
    srvr_dot_c << "          return(-1);\n";
    srvr_dot_c << "    }\n";
    srvr_dot_c << "    switch (__tag__)\n " << "     {\n";
    
    // generate the zero RPC that returns interface name & version.
    srvr_dot_c << "        case 0:\n";
    srvr_dot_c << "            if (verify_protocol())\n";
    srvr_dot_c << "                return (-1);\n";
    srvr_dot_c << "            break;\n";

    for (cf = methods; *cf; ++cf)
      (*cf)->genSwitch(FALSE, "-1", srvr_dot_c);

    srvr_dot_c << "    default:\n";
    srvr_dot_c << "     return(__tag__);\n";
    srvr_dot_c << "  }\n";
    srvr_dot_c << "  return(0);\n";
    srvr_dot_c << "}\n";
}

// the include files for *SRVR.C and *.C
void interfaceSpec::genIncludes(ofstream &output, int inBase)
{
    output << "extern \"C\" {\n";
    output << "#include <stdio.h>\n";
    output << "#include <stdlib.h>\n";
    output << "#include <rpc/types.h>\n";
    output << "#include <assert.h>\n";
    output << "}\n";

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
	if (inBase) {
	  output << "bool_t xdr_array (XDR*, char**, u_int *, u_int, u_int, xdrproc_t);\n";
	  output << "bool_t xdr_int (XDR*, int*);\n";
	  output << "bool_t xdr_u_int(XDR*, u_int*);\n";
	  output << "bool_t xdr_u_long(XDR*, u_long*);\n";
	  output << "bool_t xdr_long(XDR*, long*);\n";
	  output << "bool_t xdr_float(XDR*, float*);\n";
	  output << "bool_t xdr_double(XDR*, double*);\n";
	  output << "bool_t xdr_char(XDR*, char*);\n";
	  output << "bool_t xdr_u_char(XDR*, u_char*);\n";
	}
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
    }

    // generate stubs for upcalls.
    for (cf = methods; *cf; ++cf)
      (*cf)->genStub((char*)name, TRUE, srvr_dot_c);
}


void interfaceSpec::genWaitLoop() 
{
  List<remoteFunc*> cf;

  // generate a loop to wait for a tag, and call upcalls as they arrive.
  clnt_dot_c << "void " << (char*)name << "User::awaitResponce(int __targetTag__) {\n";
  clnt_dot_c << "    unsigned int __tag__;\n";
  if (generateTHREAD) {
    clnt_dot_c << "  union " << (char*)unionName << " __recvBuffer__;\n";
    clnt_dot_c << "  unsigned __len__ = sizeof(__recvBuffer__);\n";
  }

  // make sure it is safe to proceed
  clnt_dot_c << "  if (get_err_state() != igen_no_err) return;\n";

  clnt_dot_c << "  while (1)\n " << "      {\n";
  if (generateXDR) {
    clnt_dot_c << "    setDir(XDR_DECODE);\n";
    clnt_dot_c << "    if (!xdrrec_skiprecord(getXdrs())) {\n";
    clnt_dot_c << "        set_err_state(igen_read_err);\n";
    clnt_dot_c << "        handle_error();\n";
    clnt_dot_c << "        return;\n";
    clnt_dot_c << "    }\n";
    clnt_dot_c << "    if (!xdr_u_int(getXdrs(), &__tag__)) {\n";
    clnt_dot_c << "        set_err_state(igen_decode_err);\n";
    clnt_dot_c << "        handle_error();\n";
    clnt_dot_c << "        return;\n";
    clnt_dot_c << "    }\n";
  } else if (generateTHREAD) {
    clnt_dot_c << "  __tag__ = MSG_TAG_ANY;\n";
    clnt_dot_c << "    setRequestingThread(msg_recv(&__tag__, (void*)&__recvBuffer__, &__len__));\n";
    clnt_dot_c << "    if (getRequestingThread() == THR_ERR)\n";
    clnt_dot_c << "       {\n";
    clnt_dot_c << "          set_err_state(igen_read_err);\n";
    clnt_dot_c << "          handle_error();\n";
    clnt_dot_c << "          return;\n";
    clnt_dot_c << "       }\n";
    
  }
  // look for success error message
  clnt_dot_c << "    if (__tag__ == __targetTag__) return;\n";

  clnt_dot_c << "    switch (__tag__)\n" << "         {\n";
  for (cf = methods; *cf; ++cf) {
    (*cf)->genSwitch(TRUE, " ", clnt_dot_c);
  }
  clnt_dot_c << "	    default: \n";
  clnt_dot_c << "             set_err_state(igen_request_err);\n";
  clnt_dot_c << "             handle_error();\n";
  clnt_dot_c << "             return;\n";
  clnt_dot_c << "    }\n";
  clnt_dot_c << "	if (__targetTag__ == -1) return;\n";
  clnt_dot_c << "  }\n";
  clnt_dot_c << "}\n";

  clnt_dot_c << "int " << (char*)name << "User::isValidUpCall(int tag) {\n";
  clnt_dot_c << "    return((tag >= " << baseTag << ") && (tag <= " << boundTag << "));\n",
  clnt_dot_c << "}\n";
}

void interfaceSpec::genProtoVerify()
{
    // generate stub to verify version.
    clnt_dot_c << "void " << (char*)name << "User::verifyProtocolAndVersion() {\n";
    clnt_dot_c << "    unsigned int __tag__;\n";
    clnt_dot_c << "    char *proto;\n";
    clnt_dot_c << "    int version;\n";
    clnt_dot_c << "    __tag__ = 0;\n";
    clnt_dot_c << "    setDir(XDR_ENCODE);\n";
    clnt_dot_c << "    if (xdr_u_int(getXdrs(), &__tag__) != TRUE)\n";
    clnt_dot_c << "       {\n";
    clnt_dot_c << "          set_err_state(igen_encode_err);\n";
    clnt_dot_c << "          handle_error();\n";
    clnt_dot_c << "          return;\n";
    clnt_dot_c << "       }\n";
    clnt_dot_c << "     if (xdrrec_endofrecord(getXdrs(), TRUE) != TRUE)";
    clnt_dot_c << "       {\n";
    clnt_dot_c << "          set_err_state(igen_read_err);\n";
    clnt_dot_c << "          handle_error();\n";
    clnt_dot_c << "          return;\n";
    clnt_dot_c << "       }\n";
    clnt_dot_c << "    awaitResponce(0);\n";
    clnt_dot_c << "    if (get_err_state() != igen_no_err)\n";
    clnt_dot_c << "       {\n";
    clnt_dot_c << "           printf(\"Protocol verify - no response from server\\n\");\n";
    clnt_dot_c << "           handle_error();";
    clnt_dot_c << "	      exit(-1);\n";
    clnt_dot_c << "       }\n";
    clnt_dot_c << "    setDir(XDR_DECODE);\n";
    clnt_dot_c << "    if (!xdr_char_PTR(getXdrs(), &(proto)) ||\n";
    clnt_dot_c << "        !xdr_int(getXdrs(), &(version))) {\n";
    clnt_dot_c << "          set_err_state(igen_proto_err);\n";
    clnt_dot_c << "          printf(\"Protocol verify - bad response from server\\n\");\n";
    clnt_dot_c << "          handle_error();";
    clnt_dot_c << "	     exit(-1);\n";
    clnt_dot_c << "    }\n";
    clnt_dot_c << "    if ((version != " << version << ") || (strcmp(proto, \"" << (char*)name << "\"))) {\n";
    clnt_dot_c << "         printf(\"protocol " << (char*)name << " version " << version << " expected\\n\");\n", 
    clnt_dot_c << "         printf(\"protocol %s version %d found\\n\", proto, version);\n";
    clnt_dot_c << "         set_err_state(igen_proto_err);\n";
    clnt_dot_c << "         handle_error();\n";
    clnt_dot_c << "	    exit(-1);\n";
    clnt_dot_c << "    }\n";
    clnt_dot_c << "    setDir(XDR_FREE);\n";
    clnt_dot_c << "    xdr_char_PTR (getXdrs(), &proto);\n";
    clnt_dot_c << "}\n";
    clnt_dot_c << "\n\n";
    clnt_dot_c << (char*)name << "User::" << (char*)name << "User(int fd, xdrIOFunc r, xdrIOFunc w, int nblock):\n";
    clnt_dot_c << "RPCBase(igen_no_err, 0),\n";
    clnt_dot_c << "XDRrpc(fd, r, w, nblock) {\n";
    clnt_dot_c << "    if (getXdrs()) verifyProtocolAndVersion();\n";
    clnt_dot_c << "    IGEN_in_call_handler = 0;\n";
    clnt_dot_c << "}\n";
    clnt_dot_c << (char*)name << "User::" << (char*)name << "User(int family, int port, int type, char *machine, xdrIOFunc rf, xdrIOFunc wr, int nblock):\n";
    clnt_dot_c << "RPCBase(igen_no_err, 0),\n";
    clnt_dot_c << "XDRrpc(family, port, type, machine, rf, wr, nblock) {\n";
    clnt_dot_c << "    if (getXdrs()) verifyProtocolAndVersion();\n";
    clnt_dot_c << "    IGEN_in_call_handler = 0;\n";
    clnt_dot_c << "}\n";
    clnt_dot_c << (char*)name << "User::" << (char*)name << "User(char *m,char *l,char *p,xdrIOFunc r,xdrIOFunc w, char **args, int nblock):\n";
    clnt_dot_c << "RPCBase(igen_no_err, 0),\n";
    clnt_dot_c << "    XDRrpc(m, l, p, r, w, args, nblock, __wellKnownPortFd__) {\n";
    clnt_dot_c << "    if (getXdrs()) verifyProtocolAndVersion();\n";
    clnt_dot_c << "    IGEN_in_call_handler = 0;\n";
    clnt_dot_c << "}\n";
    clnt_dot_c << "\n";
}

void interfaceSpec::generateStubs(ofstream &output)
{
    List <remoteFunc*> cf;
    char className[80];


    sprintf(className, "%sUser", (char*)name);
    for (cf = methods; *cf; ++cf) {
	(*cf)->genStub(className, FALSE, output);
    }
}

void interfaceSpec::generateClientCode()
{
  if (generateXDR) {
    genIncludes(clnt_dot_c);

    // include client header
    clnt_dot_c << "#include \"" << protoFile << "CLNT.h\"\n";

    clnt_dot_c << "int " << (char*)name << "User::__wellKnownPortFd__;\n";

    // generate the error handler for the client
    genErrHandler(clnt_dot_c, TRUE);
  } else {
    // generate the error handler for the client
    genErrHandler(dot_c, TRUE);
  }

  generateStubs(clnt_dot_c);

  if (generateXDR) 
    genProtoVerify();

  genWaitLoop();
}

void interfaceSpec::generateBundlers()
{
    List <userDefn*> ud;

    for (ud = userPool; *ud; ++ud)
      (*ud)->genBundler();
}

char *interfaceSpec::genVariable()
{
    static int count=0;
    char *ret;

    ret = (char *) malloc(strlen((char*)name)+10);
    sprintf(ret, "%s__%d", (char*)name, count++);
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

  clnt_dot_h <<  "#ifndef _" << (char*)name << "CLNT_H\n";
  clnt_dot_h <<  "#define _" << (char*)name << "CLNT_H\n";
  clnt_dot_h <<  "#include \"" << protoFile << "h\"\n\n";
  clnt_dot_h <<  "class " << (char*)name << "User: public RPCBase, public " << transportBase << " {\n";
  clnt_dot_h <<  "  public:\n";
  client_pass_thru.map(&print_pass_thru_clnt);
  clnt_dot_h <<  "    static int __wellKnownPortFd__;\n";

  if (generateXDR) {
    clnt_dot_h <<  "    virtual void verifyProtocolAndVersion();\n";
    clnt_dot_h <<  "    " << (char*)name << "User(int fd, xdrIOFunc r, xdrIOFunc w, int nblock=0);\n";
    clnt_dot_h <<  "    " << (char*)name << "User(int family, int port, int type, char *host, xdrIOFunc rf, xdrIOFunc wf, int nblock=0);\n";
    clnt_dot_h <<  "    " << (char*)name << "User(char *machine, char *login, char *program, xdrIOFunc r, xdrIOFunc w, char **args=0, int nblock=0);\n";
  } else if (generateTHREAD) {
    clnt_dot_h << "    " << (char*)name << "User(int id): THREADrpc(id), RPCBase(igen_no_err, 0) {}\n";
  }
  
  clnt_dot_h << "    void awaitResponce(int);\n";
  clnt_dot_h << "    int isValidUpCall(int);\n";
  
  for (curr = methods; *curr; ++curr) {
    clnt_dot_h << "    ";
    (*curr)->genMethodHeader(NULL, 1, clnt_dot_h);
    clnt_dot_h << ";\n";
  }
  clnt_dot_h << " protected:\n";
  clnt_dot_h << "   virtual void handle_error();\n";
  clnt_dot_h << "   int IGEN_in_call_handler;\n";
  clnt_dot_h << "};\n";
  clnt_dot_h << "#endif\n";

  srvr_dot_h <<  "#ifndef _" << (char*)name << "SRVR_H\n";
  srvr_dot_h <<  "#define _" << (char*)name << "SRVR_H\n";
  srvr_dot_h <<  "#include \"" << protoFile << "h\"\n\n";
  srvr_dot_h << "class " << (char*)name << ": public RPCBase, public " << transportBase << " {\n";
  srvr_dot_h << "  public:\n";

  server_pass_thru.map(&print_pass_thru_srvr);

  if (generateXDR) {
    srvr_dot_h << "   " << (char*)name << "(int family, int port, int type, char *host, xdrIOFunc rf, xdrIOFunc wf, int nblock=0);\n";
    srvr_dot_h << "   " << (char*)name << "(int fd, xdrIOFunc r, xdrIOFunc w, int nblock=0);\n";
    srvr_dot_h << "   " << (char*)name << "(char *m, char *l, char *p, xdrIOFunc r, xdrIOFunc w, char **args=0, int nblock=0);\n";
    srvr_dot_h << "    int verify_protocol();\n";
    srvr_dot_h << "    int look_for_verify();\n";
  } else if (generateTHREAD) {
    srvr_dot_h << "    " << (char*)name << "(int id): THREADrpc(id), RPCBase(igen_no_err, 0) {}\n";
  }
  srvr_dot_h << "   int mainLoop(void);\n";

  for (curr = methods; *curr; ++curr) {
    srvr_dot_h << "    ";
    (*curr)->genMethodHeader(NULL, 0, srvr_dot_h);
    srvr_dot_h << ";\n";
  }
  if (generateXDR) {
    srvr_dot_h << "    void set_versionVerifyDone(bool_t val)\n";
    srvr_dot_h << "           { setVersionVerifyDone();}\n";
  }
  srvr_dot_h << " protected:\n";
  srvr_dot_h << "   virtual void handle_error();\n";
  srvr_dot_h << "   int IGEN_in_call_handler;\n";
  srvr_dot_h << "};\n\n";
  srvr_dot_h << "#endif\n";
}

void interfaceSpec::genXDRServerVerifyProtocol()
{
  srvr_dot_c << "   int " << (char*)name << "::verify_protocol()\n";
  srvr_dot_c << "   {\n";
  srvr_dot_c << "   const char *__ProtocolName__ = \"" << (char*)name << "\";\n";
  srvr_dot_c << "   int __val__;\n";
  srvr_dot_c << "   int __sig__ = 0;\n";
  srvr_dot_c << "   setDir(XDR_ENCODE);\n";
  srvr_dot_c << "    __val__ = " << version << ";\n";
  srvr_dot_c << "   if (!xdr_int(getXdrs(), &__sig__) ||\n";
  srvr_dot_c << "       !xdr_char_PTR(getXdrs(), &__ProtocolName__) ||\n";
  srvr_dot_c << "       !xdr_int(getXdrs(), &__val__)) {\n";
  srvr_dot_c << "      set_err_state(igen_encode_err);\n";
  srvr_dot_c << "      handle_error();\n";
  srvr_dot_c << "      return(-1);\n";
  srvr_dot_c << "   }\n";
  srvr_dot_c << "   if (!xdrrec_endofrecord(getXdrs(), TRUE)) {\n";
  srvr_dot_c << "      set_err_state(igen_send_err);\n";
  srvr_dot_c << "      handle_error();\n";
  srvr_dot_c << "      return(-1);\n";
  srvr_dot_c << "   }\n";
  srvr_dot_c << "   setVersionVerifyDone();\n";
  srvr_dot_c << "   return 0;\n";
  srvr_dot_c << "   }\n";
}

void interfaceSpec::genXDRLookForVerify()
{
  srvr_dot_c << "   int " << (char*)name << "::look_for_verify()\n";
  srvr_dot_c << "   {\n";
  srvr_dot_c << "     unsigned int __tag__;\n";
  srvr_dot_c << "     int __status__;\n\n";
  srvr_dot_c << "     if (!xdrrec_skiprecord(getXdrs())) {\n";
  srvr_dot_c << "         set_err_state(igen_read_err);\n";
  srvr_dot_c << "         handle_error();\n";
  srvr_dot_c << "         return (-1);\n";
  srvr_dot_c << "     }\n\n";
  srvr_dot_c << "     setDir(XDR_DECODE);\n\n";
  srvr_dot_c << "     if (!(__status__  = xdr_u_int(getXdrs(), &__tag__))) {\n";
  srvr_dot_c << "         set_err_state(igen_decode_err);\n";
  srvr_dot_c << "         handle_error();\n";
  srvr_dot_c << "         return (-1);\n";
  srvr_dot_c << "     }\n";
  srvr_dot_c << "     if (__tag__) {\n";
  srvr_dot_c << "         set_err_state(igen_request_err);\n";
  srvr_dot_c << "         handle_error();\n";
  srvr_dot_c << "         return (-1);\n";
  srvr_dot_c << "     }\n";
  srvr_dot_c << "     return 0;\n";
  srvr_dot_c << "   }\n";
}

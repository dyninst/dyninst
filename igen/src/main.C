/*
 * main.C - main function of the interface compiler igen.
 *
 * $Log: main.C,v $
 * Revision 1.28  1994/08/18 19:53:28  markc
 * Added support for new files.
 * Removed compiler warnings for solaris2.3
 *
 * Revision 1.27  1994/08/18  05:56:54  markc
 * Changed char*'s to stringHandles
 *
 * Revision 1.26  1994/08/17  17:51:57  markc
 * Added Makefile for Linux.
 * Support new source files for igen.
 * Separate source files.  ClassDefns supports classes, typeDefns supports
 * structures.
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

int generateXDR;
int generatePVM;
int generateTHREAD;

#include "parse.h"

stringPool namePool;

// how are pointer to the same memory to be handled
PTR_TYPE ptrMode = ptrIgnore;

int yyparse();
int emitCode;
int emitHeader;
char *protoFile;

List<userDefn*> userPool;

userDefn *foundType;

extern FILE *yyin;
char *transportBase;
char *serverTypeName;

extern void generate_ptr_type(stringHandle orig, stringHandle with_ptr);

extern interfaceSpec *currentInterface;

List <pvm_args*> pvm_types;
void PVM_map_scalar_includes (pvm_args *);
void PVM_map_array_includes (pvm_args *);
void templatePVMscalar (pvm_args *);
void templatePVMarray (pvm_args *);
List<char*> client_pass_thru;
List<char*> server_pass_thru;

void usage(char *name)
{
    printf("%s -xdr | -thread | -pvm [-header | -code]\n", name);
    printf("      [-ignore | -detect | -handle] <fileName>\n");
    printf("  CODE OPTIONS\n");
    printf("  -xdr     -->  produce code for sockets/xdr\n");
    printf("  -pvm     -->  produce code for PVM\n");
    printf("  -thread  -->  produce code for threads\n");
    printf("  MARSHALLING OPTIONS\n");
    printf("  -ignore  -->  don't detect pointers to the same memory\n");
    printf("  -detect  -->  detect (and assert) duplicate pointers\n");
    printf("  -handle  -->  handle duplicate\n");
    exit(-1);
}


int main(int argc, char *argv[])
{
    int i;
    char *temp;
    typeDefn *ign;

    /* define pre-defined types */
    /* assign value to remove compiler warnings */
    ign = new typeDefn(namePool.findAndAdd("int"));
    ign = new typeDefn(namePool.findAndAdd("double"));
    ign = new typeDefn(namePool.findAndAdd("String"));
    ign = new typeDefn(namePool.findAndAdd("void"));
    ign = new typeDefn(namePool.findAndAdd("Boolean"));
    ign = new typeDefn(namePool.findAndAdd("u_int"));
    ign = new typeDefn(namePool.findAndAdd("float"));
    ign = new typeDefn(namePool.findAndAdd("u_char"));
    ign = new typeDefn(namePool.findAndAdd("char"));

    emitCode = 1;
    emitHeader = 1;
    for (i=0; i < argc-1; i++) {
	if (!strcmp("-pvm", argv[i])) {
	    generatePVM = 1;
	    generateXDR = 0;
	    generateTHREAD = 0;
	    transportBase = "PVMrpc";
	} if (!strcmp("-xdr", argv[i])) {
	    generatePVM = 0;
	    generateXDR = 1;
	    generateTHREAD = 0;
	    (void) new typeDefn("XDRptr");
	    transportBase = "XDRrpc";
	} if (!strcmp("-thread", argv[i])) {
	    generatePVM = 0;
	    generateXDR = 0;
	    generateTHREAD = 1;
	    transportBase = "THREADrpc";
	} else if (!strcmp("-header", argv[i])) {
	    emitCode = 0;
	    emitHeader = 1;
	} else if (!strcmp("-code", argv[i])) {
	    emitCode = 1;
	    emitHeader = 0;
	} else if (!strcmp("-ignore", argv[i])) {
	  ptrMode = ptrIgnore;
	} else if (!strcmp("-detect", argv[i])) {
	  ptrMode = ptrDetect;
	} else if (!strcmp("-handle", argv[i])) {
	  ptrMode = ptrHandle;
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
	dot_h << "                       igen_call_err,\n";
	dot_h << "                       igen_proto_err\n";
	dot_h << "                       }  IGEN_ERR;\n\n";
	dot_h << "#endif\n";

	if (generatePVM || generateXDR) 
	  buildFlagHeaders();

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

	if (generateXDR || generatePVM) {
	  currentInterface->generateBundlers();
	}
	
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


// in_client == 1 when client headers are being generated
void remoteFunc::genMethodHeader(char *className, int in_client,
				 ofstream &current)
{
  int spitOne=0;
  List<argument*> lp;
  
  if (className) {
    current << (char*)retType << " " << className << "::" << (char*)name << "(";
  } else {
    // figure out if "virtual" should be printed
    // here is the logic
    // for the client --> if it is an upcall then print virtual
    // for the server --> if it is not an upcall the print virtual
    // for both an additional constraint is that virtual_f must be true
    if ((((upcall == syncUpcall) || (upcall == asyncUpcall)) &&
	 virtual_f && in_client) ||
	(!in_client && virtual_f && ((upcall == asyncCall) || (upcall == syncCall))))
      {
	// print virtual
	current << "virtual " << (char*) retType << " " << (char*)name << "(";
      }
    else
      {
	// don't print virtual
	current << (char*)retType << " " << (char*)name << "(";
      }
  }

  for (lp=args; *lp; lp++) {
    if (spitOne) {
      current << ",";
    }      
    current << (char*)(*lp)->type << " ";
    if ((*lp)->stars)
      current << (*lp)->stars;
    current << (char*)(*lp)->name;
    spitOne = 1;
  }
  if (!spitOne) current << "void";
  current << ") ";
}

// forUpcalls == TRUE -> when client code is being generated
// client code is being generated to handle upcalls in awaitResponce
//
// this procedure prints code under 2 circumstances
// 1-  (client code) forUpcalls == TRUE, call must be an upcall
// 2-  (server code) forUpcalls == FALSE, call must not be an upcall
//
void remoteFunc::genSwitch(int forUpcalls, char *ret_str, ofstream &output)
{
    int first;
    List<argument*> ca;

    // return if generating client code, and call is not an upcall
    if (forUpcalls &&
	((upcall == syncCall) || (upcall == asyncCall)))
      return;

    // return if generating server code, and call is an upcall
    if (!forUpcalls && (upcall != syncCall) && (upcall != asyncCall))
      return;

    output << "        case " << (char*)spec->getName() << "_" << (char*)name << "_REQ:\n";

    // print out the return type
    if ((generateTHREAD) && (upcall != asyncUpcall) && (upcall != asyncCall))
      // output << "	        int __val__;\n";
      ;

    output << "           {\n";
    if (strcmp((char*)retType, "void")) {
      output << "	        " << (char*)retType << " __ret__;\n";
    }

    // print out the local types
    if (generateXDR) {
      if (args.count()) {
	output << "            " << structName << " __recvBuffer__;\n";
	for (ca = args; *ca; ca++) {
	  output << "            if (!xdr_" << (char*)(*ca)->type <<
	    "(__xdrs__, &__recvBuffer__.";
	  output << (char*)(*ca)->name << ")) {\n";
	  output << "                err_state = igen_decode_err;\n";
	  output << "                handle_error();\n";
	  output << "                return " << ret_str << ";\n";
	  output << "            }\n";
	}
      }
    } else if (generatePVM) {
      output << "            extern IGEN_pvm_" << (char*)retType
	<< "(IGEN_PVM_FILTER, " << (char*)retType << "*);\n";
      if ((upcall != asyncUpcall) && (upcall != asyncCall))
	output << "            assert(pvm_initsend(0) >= 0);\n";
      if (!args.count()) {
	output << "            " << structName << " __recvBuffer__;\n";
	for (ca = args; *ca; ca++) {
	  output << "            IGEN_pvm_" << (char*)(*ca)->type <<
	    "(IGEN_PVM_DECODE, &__recvBuffer__." << (char*)(*ca)->name << ");\n";
	}
      }
    }

    // to determine when sync upcalls cannot be made
    if (generateXDR &&
        (upcall == asyncUpcall || upcall == asyncCall))
      output << "                 IGEN_in_call_handler = 1;\n";

    if (strcmp((char*)retType, "void")) {
	output << "                __ret__ = " << (char*)name << "(";
    } else {
	output << "                 " << (char*)name <<  "(";
    }

    for (ca = args, first = 1; *ca; ca++) {
	if (!first) output << ",";
	first = 0;
	if (generateTHREAD) {
	    output << "__recvBuffer__." << (char*)name << "." << (char*)(*ca)->name;
	} else if (generateXDR) {
	    output << "__recvBuffer__." << (char*)(*ca)->name;
	} else if (generatePVM) {
	  output << "__recvBuffer__." << (char*)(*ca)->name;
	}
    }
    output << ");\n";

    // to determine when sync upcalls cannot be made
    if (generateXDR &&
        (upcall == asyncUpcall || upcall == asyncCall))
      output << "                 IGEN_in_call_handler = 0;\n";

    // generate return value
    if (generateTHREAD && (upcall != asyncUpcall) && (upcall != asyncCall)) {
	if (strcmp((char*)retType, "void")) {
	    output << "               __val__ = msg_send(requestingThread, ";
	    output << (char*)spec->getName();
	    output << "_" << (char*)name << "_RESP, (void *) &__ret__, sizeof(";
	    output << (char*)retType << "));\n";
	} else {
	  output << "                 __val__ = msg_send(requestingThread, ";
	  output << (char*)spec->getName();
	  output << "_" << (char*)name << "_RESP, NULL, 0);\n";
	}
	// output << "         if (__val__ != THR_OKAY)\n";
	// output << "            {\n";
	// output << "                 err_state = igen_send_error;\n";
	// output << "                 handle_error();\n";
	// output << "                 return " << ret_str << ";\n";
	// output << "            }\n";
    } else if (generateXDR &&
	       (upcall != asyncUpcall) &&
	       (upcall != asyncCall)) {
	output << "       	    __xdrs__->x_op = XDR_ENCODE;\n";
	output << "                 __tag__ = " <<
	  (char*)spec->getName() << "_" << (char*)name;
	output << "_RESP;\n";
	output << "                if (!xdr_u_int(__xdrs__, &__tag__)) {\n";
	output << "                   err_state = igen_encode_err;\n";
	output << "                   handle_error();\n";
	output << "                   return " << ret_str << ";\n";
	output << "                }\n";
	if (strcmp((char*)retType, "void")) {
	  if (ptrMode == ptrHandle &&
	      isPointer())
	    output << "              __ptrTable__.destroy();\n";
	  output << "                   if (!xdr_" << (char*)retType
	    << "(__xdrs__,&__ret__)) {\n";
	  output << "                      err_state = igen_encode_err;\n";
	  output << "                      handle_error();\n";
	  output << "                      return " << ret_str << ";\n";
	  output << "                   }\n";
	}
	output << "	    if (!xdrrec_endofrecord(__xdrs__, TRUE)) {\n";
	output << "            err_state = igen_send_err;\n";
	output << "            handle_error();\n";
	output << "            return " << ret_str << ";\n";
	output << "         }\n";
    } else if (generatePVM &&
	       (upcall != asyncUpcall) &&
	       (upcall != asyncCall)) {
	output << "            __tag__ = " << (char*)spec->getName() <<
	  "_" << (char*)name << "_RESP;\n";
	if (strcmp((char*)retType, "void")) {
	  if (ptrMode == ptrHandle &&
	      isPointer())
	    output << "              __ptrTable__.destroy();\n";
	  output << "            IGEN_pvm_" << (char*)retType <<
	    " (IGEN_PVM_ENCODE,&__ret__);\n";
	}
	output << "            assert (pvm_send (__tid__, __tag__) >= 0);\n";
      }

    // free allocated memory
    if (generateTHREAD) {
    } else if (generateXDR) {
      output << "            __xdrs__->x_op = XDR_FREE;\n";
      if (args.count()) {
	for (ca = args; *ca; ca++) {
	  // only if Array or string or user defined
	  if ((*ca)->mallocs)
	    output << "            xdr_" << (char*)(*ca)->type <<
	      "(__xdrs__, &__recvBuffer__." << (char*)(*ca)->name<< ");\n";
	}
      }
      if (retStructs)
	output << "            xdr_" << (char*)retType << "(__xdrs__, &__ret__);\n";
    } else if (generatePVM) {
      if (args.count()) {
	for (ca = args; *ca; ca++) {
	  // only if Array or string 
	  if ((*ca)->mallocs)
	    output << "            IGEN_pvm_" << (char*)(*ca)->type;
	  output << "(IGEN_PVM_FREE, &__recvBuffer__." <<
	    (char*)(*ca)->name << ");\n";
	}
      }
      if (retStructs)
	output << "            IGEN_pvm_" << (char*)retType <<
	  "(IGEN_PVM_FREE, &__ret__);\n";
    }
    
    output << "           }\n              break;\n\n";
}
 

// generates the stub code for the client or the server
//
void remoteFunc::genThreadStub(char *className, ofstream &output)
{
    List<argument*> lp;
    char *retVar;
    char *structUseName = spec->genVariable();

    genMethodHeader(className, 0, output);
    output << " {\n";

    if (*args)
      output << "    struct " << structName << " " << structUseName << ";\n";
    if ((upcall != asyncUpcall) && (upcall != asyncCall))
      output << "    unsigned int __tag__;\n";
    if (strcmp((char*)retType, "void"))
      output << "    unsigned int __len__;\n";
    output << "    int __val__;\n";
    if (strcmp((char*)retType, "void")) {
      retVar = spec->genVariable();
      output << "    " << (char*)retType << " " << retVar << ";\n";
    }
    else
      retVar = " ";

    output << "    if (err_state != igen_no_err)\n";
    output << "      {\n";
    output << "        IGEN_ERR_ASSERT;\n";
    output << "        return " << retVar << ";\n";
    output << "      }\n";

    for (lp = args; *lp; lp++) {
	output << "    " << structUseName << "." << (char*)(*lp)->name << " = ";
	output << (char*)(*lp)->name << ";  \n";
    }
    if (*args) {
	output << "    __val__ = msg_send(tid, " << (char*)spec->getName() << "_";
	output << (char*)name << "_REQ, (void *) &";
	output << structUseName << ", sizeof(" << structUseName << "));\n";
    } else {
	output << "    __val__ = msg_send(tid, " << (char*)spec->getName() << "_";
	output << (char*)name <<  "_REQ, NULL, 0); \n";
    }
    output << "    if (__val__ != THR_OKAY)\n";
    output << "      {\n";
    output << "          err_state = igen_send_err;\n";
    output << "          handle_error();\n";
    output << "          return " << retVar << " ;\n";
    output << "      }\n";
    if ((upcall != asyncUpcall) && (upcall != asyncCall)) {
	output << "    __tag__ = " << (char*)spec->getName() << "_" << (char*)name << "_RESP;\n";
	if (strcmp((char*)retType, "void")) {
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
	output << "    if (__tag__ != " << (char*)spec->getName() << "_" << (char*)name;
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
  char *retVar;
  int retS = 0;

  genMethodHeader(className, 0, output);
  output << "\n {\n";

  output << "    unsigned int __tag__;\n";
  if (strcmp((char*)retType, "void")) {
    retVar = spec->genVariable();
    output << "    " << (char*)retType << " " << retVar << ";\n";
    retS = 1;
  } else
    retVar = strdup(" ");

  output << "    if (err_state != igen_no_err) {\n";
  output << "      IGEN_ERR_ASSERT;\n";
  output << "      return " << retVar << ";\n";
  output << "    }\n";

  if ((upcall == syncUpcall) || (upcall == syncCall)) {
    output << "       if (IGEN_in_call_handler) {\n";
    output <<
      "          fprintf(stderr, \"cannot do sync call when in async call handler\\n\");\n";
    output << "          IGEN_ERR_ASSERT;\n";
    output << "          err_state = igen_call_err;\n";
    output << "          return " << retVar << ";\n";
    output << "       }\n";
    }

  // check to see protocol verify has been done.
  if ((upcall != syncCall) && (upcall != asyncCall)) {
    output << "    if (!__versionVerifyDone__) {\n";
    output << "    // handle error is called for errors\n";
    output << "        if (!look_for_verify() && !verify_protocol())\n";
    output << "            __versionVerifyDone__ = TRUE;\n";
    output << "        else\n";
    output << "            return " << retVar << " ;\n";
    output << "    }\n";
  }
  output << "    __tag__ = " << (char*)spec->getName() << "_" << (char*)name << "_REQ;\n";
  output << "    __xdrs__->x_op = XDR_ENCODE;\n";

  if (ptrMode == ptrHandle && ArgsRPtr())
    output << "   __ptrTable__.destroy();\n";

  // the error check depends on short circuit boolean expression evaluation
  output << "    if (!xdr_u_int(__xdrs__, &__tag__)\n";
  for (lp = args; *lp; lp++) {
    output << "      ||  !xdr_" << (char*)(*lp)->type << "(__xdrs__, &";
    output << (char*)(*lp)->name << ")\n";
    if (ptrMode == ptrDetect && (*lp)->IsAPtr())
      output << "      || !__ptrTable__.destroy()\n";
  }
  output << " ) {\n";
  output << "      err_state = igen_encode_err;\n";
  output << "      handle_error();\n";
  output << "      return " << retVar << " ;\n";
  output << "   }\n";
  output << "       if(!xdrrec_endofrecord(__xdrs__, TRUE)) {\n";
  output << "          err_state = igen_send_err;\n";
  output << "          handle_error();\n";
  output << "          return " << retVar << " ;\n";
  output << "       }\n";

  if ((upcall != asyncUpcall) && (upcall != asyncCall)) {
    if (upcall == syncCall) {
      output << "    awaitResponce(" << (char*)spec->getName() << "_";
      output << (char*)name << "_RESP);\n";
    } else if (upcall == syncUpcall) {
      output << " int res;\n";
      output << " while (!(res = mainLoop())) ;\n";
      output << " if (res != " << (char*)spec->getName() << "_" <<
	(char*)name << "_RESP)\n";
      output << "        { err_state = igen_request_err; handle_error(); return ";
      if (retS)
	output << "(" << retVar << ");}\n";
      else
	output << ";}\n";
    }
    output << "    if (err_state != igen_no_err)\n";
    output << "    return " << retVar << ";\n";
    
    if (strcmp((char*)retType, "void")) {
      output << "    __xdrs__->x_op = XDR_DECODE;\n";
      output << "    if (!xdr_" << (char*)retType << "(__xdrs__, &(";
      output << retVar << "))) {\n";
      output << "        err_state = igen_decode_err;\n";
      output << "        handle_error();\n";
      output << "        return " << retVar << " ;\n";
      output << "    }\n";
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
  if (strcmp((char*)retType, "void")) {
    output << "    " << (char*)retType << " " << retVar << ";\n";
  }

  output << "    if (err_state != igen_no_err) {\n";
  output << "      IGEN_ERR_ASSERT;\n";
  output << "      return " << retVar << ";\n";
  output << "    }\n";

  output << "    __tag__ = " << (char*)spec->getName() << "_" << (char*)name << "_REQ;\n";
  output << "    assert(pvm_initsend(0) >= 0);\n";
  for (lp = args; *lp; lp++) {
    output << "    IGEN_pvm_" << (char*)(*lp)->type << " (IGEN_PVM_ENCODE, &" << (char*)(*lp)->name << ");\n";
  }
  output << "    pvm_send ( get_other_tid(), __tag__);\n";
  if (upcall != asyncUpcall) {
    if (upcall == syncCall) {
      output << "    awaitResponce(" << (char*)spec->getName() <<
	"_" << (char*)name << "_RESP);\n";
    } else if (upcall == syncUpcall) {
      output << "    if (pvm_recv(-1, " << (char*)spec->getName() <<
	"_" << (char*)name << "_RESP) < 0) abort();\n";
    }
    if (strcmp((char*)retType, "void")) {
      output << "    IGEN_pvm_" << (char*)retType <<
	"(IGEN_PVM_DECODE, &(" << retVar << ")); \n";
      output << "    return(" << retVar << ");\n";
    }
  }
  output << "}\n\n";
}

void remoteFunc::genStub(char *className, int forUpcalls, ofstream &output)
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
    List<argument*> lp;

    // if (!retVoid) {
    if (1) {
      dot_h << "struct " << structName << " {\n";
      for (lp = args; *lp; lp++) {
	dot_h << "    " << (char*)(*lp)->type << " ";
	if ((*lp)->stars)
	  dot_h << (*lp)->stars;
	dot_h << (char*)(*lp)->name;
	dot_h << ";\n";
      }
      dot_h << "};\n\n";
    }

    dot_h << "#define " << (char*)spec->getName() << "_" << (char*)name <<
      "_REQ " << spec->getNextTag() << "\n";
    dot_h << "#define " << (char*)spec->getName() << "_" << (char*)name <<
      "_RESP " << spec->getNextTag() << "\n";
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
templatePVMscalar (pvm_args *the_arg)
{
  if ((!the_arg->type_name) || (!the_arg->pvm_name)) return;
  dot_c << "\n";
  dot_c << "bool_t IGEN_pvm_" << (char*)the_arg->type_name << " (IGEN_PVM_FILTER direction,";
  dot_c << (char*)the_arg->type_name << " *data) {\n";
  dot_c << "   assert(data);\n";
  dot_c << "   switch (direction)\n";
  dot_c << "     {\n";
  dot_c << "        case IGEN_PVM_DECODE:\n";
  dot_c << "            if (pvm_upk" << (char*)the_arg->pvm_name <<
    " (data, 1, 1) < 0)\n";
  dot_c << "                return (FALSE);\n";
  dot_c << "            break;\n";
  dot_c << "        case IGEN_PVM_ENCODE:\n";
  dot_c << "            if (pvm_pk" << (char*)the_arg->pvm_name <<
    " (data, 1, 1) < 0)\n";
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
templatePVMarray (pvm_args *the_arg)
{
  if ((!the_arg->type_name) ||
      (!the_arg->pvm_name) ||
      (!the_arg->arg)) 
    return;
  dot_c << "bool_t IGEN_pvm_Array_of_" << (char*)the_arg->type_name <<
    " (IGEN_PVM_FILTER dir, ";
  dot_c << (char*)the_arg->type_name << " **data, unsigned int *count) {\n";
  dot_c << "   int bytes, msgtag, tid;\n";
  dot_c << "   switch (dir)\n";
  dot_c << "     {\n";
  dot_c << "        case IGEN_PVM_DECODE:\n";
  dot_c << "           int buf_id = pvm_getrbuf();\n";
  dot_c << "           if (buf_id == 0) return (FALSE);\n";
  dot_c << "           if (pvm_bufinfo(buf_id, &bytes, &msgtag, &tid) < 0) return(FALSE);\n";
  dot_c << "           *count = bytes " << the_arg->arg << ";\n";
  dot_c << "           *data = (" << (char*)the_arg->type_name << " *) new char[*count];\n";
  dot_c << "           if (!(*data)) return (FALSE);\n";
  dot_c << "           if (pvm_upk" << (char*)the_arg->pvm_name <<
    " (*data, *count, 1) < 0) return (FALSE);\n";
  dot_c << "           break;\n";
  dot_c << "        case IGEN_PVM_ENCODE:\n";
  dot_c << "           if (pvm_pk" << (char*)the_arg->pvm_name <<
    " (*data, *count, 1) < 0) return (FALSE);\n";
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
void genPVMServerCons(stringHandle name)
{
  srvr_dot_c << (char*)name << "::" << (char*)name <<
    " (char *w, char *p, char **a, int f):\n";
  srvr_dot_c << "PVMrpc(w, p, a, f) {\n";
  srvr_dot_c <<
    "    __versionVerifyDone__ = FALSE; IGEN_in_call_handler = 0;}\n";
  srvr_dot_c << (char*)name << "::" << (char*)name << " (int o):\n";
  srvr_dot_c << "PVMrpc(o) {\n";
 srvr_dot_c <<
   "    __versionVerifyDone__ = FALSE; IGEN_in_call_handler = 0;}\n";
  srvr_dot_c << (char*)name << "::" << (char*)name << " ():\n";
  srvr_dot_c << "PVMrpc() {\n";
  srvr_dot_c <<
    "    __versionVerifyDone__ = FALSE; IGEN_in_call_handler = 0;}\n";
}

// the constructor for the xdr server class
void genXDRServerCons(stringHandle name)
{
  srvr_dot_c << (char*)name << "::" << (char*)name <<
    "(int family, int port, int type, char *host, xdrIOFunc rf, xdrIOFunc wf, int nblock):\n";
  srvr_dot_c << "XDRrpc(family, port, type, host, rf, wf, nblock),\n";
  srvr_dot_c << "RPCServer(igen_no_err)\n";
  srvr_dot_c <<
    "  { __versionVerifyDone__ = FALSE; IGEN_in_call_handler = 0;}\n";
  srvr_dot_c << (char*)name << "::" << (char*)name <<
    "(int fd, xdrIOFunc r, xdrIOFunc w, int nblock):\n";
  srvr_dot_c << "XDRrpc(fd, r, w, nblock),\n";
  srvr_dot_c << "RPCServer(igen_no_err)\n";
  srvr_dot_c <<
    "  { __versionVerifyDone__ = FALSE; IGEN_in_call_handler = 0;}\n";

  srvr_dot_c << (char*)name << "::" << (char*)name <<
    "(char *m,char *l,char *p,xdrIOFunc r,xdrIOFunc w, char **args, int nblock):\n";
  srvr_dot_c << "    XDRrpc(m, l, p, r, w, args, nblock),\n";
  srvr_dot_c << "    RPCServer(igen_no_err)\n";
  srvr_dot_c <<
    "  { __versionVerifyDone__ = FALSE; IGEN_in_call_handler = 0;}\n";
}

pvm_args 
buildPVMargs_ptr (stringHandle type_name, stringHandle pvm_name, stringHandle arg)
{
  pvm_args arg_ptr;
  
  if (!pvm_name || !type_name)
    return arg_ptr;

  arg_ptr.type_name = type_name;
  arg_ptr.pvm_name = pvm_name;
  arg_ptr.arg = arg;

  return arg_ptr;
}

//
// builds List pvm_types with PVM type information
// this list is used by 2 procedures
//
void
buildPVMargs()
{
  pvm_args *arg_ptr = new pvm_args;

  *arg_ptr = buildPVMargs_ptr ("int", "int", " >> 2 ");
  pvm_types.add(arg_ptr);

  *arg_ptr = buildPVMargs_ptr ("char", "byte", "  ");
  pvm_types.add(arg_ptr);

  *arg_ptr = buildPVMargs_ptr ("float", "float", " >> 2 ");
  pvm_types.add(arg_ptr);

  *arg_ptr = buildPVMargs_ptr ("double", "double", " >> 3 ");
  pvm_types.add(arg_ptr);

  *arg_ptr = buildPVMargs_ptr ("u_int", "uint", " >> 2 ");
  pvm_types.add(arg_ptr);
}

void
PVM_map_scalar_includes (pvm_args *the_arg)
{
  if (!the_arg->type_name)
    return;
  dot_h << "bool_t IGEN_pvm_" << (char*)the_arg->type_name <<
    " (IGEN_PVM_FILTER direction, ";
  dot_h << (char*)the_arg->type_name << " *data);\n";
}

void
PVM_map_array_includes (pvm_args *the_arg)
{
  if (!the_arg->type_name)
    return;
  dot_h << "bool_t IGEN_pvm_Array_of_" << (char*)the_arg->type_name <<
    " (IGEN_PVM_FILTER ";
  dot_h << "direction, " << (char*)the_arg->type_name << " **data, unsigned int *count);\n";
}

void
add_to_list (stringHandle type, stringHandle name, char *stars,
	     List<char*> &holder) 
{
  int new_length;
  char *new_string;

  if (!type || !name) return;
  new_length = strlen((char*)type) + strlen((char*)name);
  if (stars)
    new_length += strlen(stars);
  new_length += 3;

  new_string = new char[new_length];

  if (stars)
    sprintf(new_string, "%s %s %s", (char*)type, stars, (char*)name);
  else
    sprintf(new_string, "%s %s", (char*)type, (char*)name);

  holder.add (new_string);
}

void 
addCMember (stringHandle type, stringHandle name, char *stars)
{
  add_to_list (type, name, stars, client_pass_thru);
}

void 
addSMember (stringHandle type, stringHandle name, char *stars)
{
  add_to_list (type, name, stars, server_pass_thru);
}

void
dump_to_dot_h(char *msg)
{
  dot_h << msg;
}



argument::argument(stringHandle t, stringHandle n,  char *s, int m) {
  int slen=0, tlen;
  char *typeStr;

  if (s)
    slen = strlen(s);
  if (!generateTHREAD) {
    if (!slen) {
      type = t;
      stars = s;
      mallocs = m;
    } else if (slen == 1){
      stars = s;
      stars = 0;
      tlen = strlen((char*)t);
      tlen += 5;
      typeStr = new char[tlen];
      assert(typeStr);
      sprintf(typeStr, "%s%s", (char*)t, "_PTR");
      type = namePool.findAndAdd(typeStr);
      delete (typeStr);
      generate_ptr_type(t, type);
      mallocs = m;
    } else {
      cout << "Too many levels of indirection for type " << (char*)t << endl;
      exit(0);
    }
  } else {
    type = t;
    stars = s;
    mallocs = m;
  }
  name = n;
}

remoteFunc::remoteFunc(interfaceSpec *sp,
		       char *s,
		       stringHandle n,
		       stringHandle r, 
		       List <argument*> &a, 
		       enum upCallType uc,
		       int v_f,
		       int rs) {
  spec = sp;
  name = n;
  structName = spec->genVariable();
  args = a;
  upcall = uc;
  retStructs = rs;
  virtual_f = v_f;
  int ct=0, slen;
  retVoid = 0;

  if (s)
    ct = strlen(s);

  if (!generateTHREAD) {
    if (ct > 1) {
      cout << "Too many stars for " << (char*)r << (char*)n << " exiting\n";
      exit(0);
    } else if (!ct) {
      if (!strcmp("void", (char*) r))
	retVoid = 1;
      retType = r;
    } else {
      // 1 star
      char *retTypeStr;
      slen = strlen((char*) r) + 5;
      retTypeStr = new char[slen];
      assert(retTypeStr);
      sprintf(retTypeStr, "%s%s", (char*)r, "_PTR");
      retType = namePool.findAndAdd(retTypeStr);
      delete(retTypeStr);
      generate_ptr_type(r, retType);
    }
  } else {
    // generateTHREAD

    if (!ct && !strcmp("void", (char*)r))
      retVoid = 1;
    slen = strlen((char*)r) + 1 + ct;
    char *retTypeStr;
    retTypeStr = new char[slen];
    assert(retTypeStr);
    if (s)
      sprintf(retTypeStr, "%s%s", (char*)r, s);
    else 
      sprintf(retTypeStr, "%s", (char*)r);
    retType = namePool.findAndAdd(retTypeStr);
    delete retTypeStr;
  }
}

field::field(stringHandle t, stringHandle n)
{
  if (!userPool.find(t)) {
    cout << "In field name: "<< (char*)n << ", (char*)type " << (char*)t << " is not defined\n";
    cout << "Error, exiting\n";
    exit(0);
  }
  type = t;
  name = n;
}

void field::genBundler(ofstream &ofile, char *obj) {
  if (generateXDR) {
    ofile << "    if (!xdr_" << (char*)type <<
      "(__xdrs__, " << obj <<(char*) name << "))) return FALSE;\n";
  } else if (generatePVM) {
    ofile << "    IGEN_pvm_" << (char*)type <<
      "(__dir__, " << obj << (char*)name << "));\n";
  }
}

void field::genHeader(ofstream &ofile) {
  ofile << "    " << (char*)type << " " << (char*)name << ";\n";
}

int isClass(stringHandle cName)
{
  userDefn *res;
  res = userPool.find((char*)cName);
  return(res->whichType() == CLASS_DEFN);
}

int isStruct(stringHandle sName)
{
  userDefn *res;
  res = userPool.find((char*)sName);
  return(res->whichType() == TYPE_DEFN);
}

void verify_pointer_use(char *stars, stringHandle retType, stringHandle name)
{
  int star_count;
  if (!stars)
    star_count = 0;
  else
    star_count = strlen(stars);
  switch (star_count) {
  case 0: 
    break;
  case 1:
    break;
  default:
    if (!generateTHREAD) {
      printf("Cannot use 1 level of indirection with %s %s\n",
	     (char*)retType, (char*)name);
      printf("Error, exiting\n");
      exit(0);
    }
    break;
  }
}

void buildFlagHeaders()
{
  dot_h << "#ifndef _IGEN_FLAG_DEF\n";
  dot_h << "#define _IGEN_FLAG_DEF\n";
  dot_h << "inline void igen_flag_set_null(unsigned long &f) {\n";
  dot_h << "     f &= 0xfffffffe;\n";
  dot_h << "}\n";
  dot_h << "inline void igen_flag_set_nonull(unsigned long &f) {\n";
  dot_h << "     f |= 1;\n";
  dot_h << "}\n";
  dot_h << "inline void igen_flag_set_shared(unsigned long &f) {\n";
  dot_h << "     f |= 2;\n";
  dot_h << "}\n";
  dot_h << "inline void igen_flag_set_noshared(unsigned long &f) {\n";
  dot_h << "     f &= 0xfffffffd;\n";
  dot_h << "}\n";
  dot_h << "inline unsigned char igen_flag_get_id(unsigned long f) {\n";
  dot_h << "     f >>= 24;\n";
  dot_h << "     return((unsigned char) f);\n";
  dot_h << "}\n";
  dot_h << "inline void igen_flag_set_id(const unsigned char &v, unsigned long &f) {\n";
  dot_h << "     unsigned long rot;\n";
  dot_h << "     rot = v;\n";
  dot_h << "     rot <<= 24;\n";
  dot_h << "     f |= rot;\n";
  dot_h << "}\n";
  dot_h << "inline int igen_flag_is_null(unsigned long f) {\n";
  dot_h << "     return(f & 1);\n";
  dot_h << "}\n";
  dot_h << "inline int  igen_flag_is_shared(unsigned long f) {\n";
  dot_h << "     return(f & 2);\n";
  dot_h << "}\n";
  dot_h << "#endif /* _IGEN_FLAG_DEF */\n";
}



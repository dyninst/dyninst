/*
 * main.C - main function of the interface compiler igen.
 *
 * $Log: main.C,v $
 * Revision 1.1  1994/01/25 20:48:43  hollings
 * New utility for interfaces.
 * new utility for interfaces.
 *
 *
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>

#include <util/list.h>
#include <util/stringPool.h>

#include "parse.h"

int yyparse();
int emitCode;
int emitHeader;
char *codeFile;
char *serverFile;
char *clientFile;
int generateXDR;
int generatePVM;
stringPool pool;
char *headerFile;
extern FILE *yyin;
int generateTHREAD;
char *transportBase;
char *serverTypeName;
List <typeDefn *> types;
interfaceSpec *currentInterface;

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
    printf("  unsigned int __msgTag__;\n");
    printf("  union %s __recvBuffer__;\n", unionName);
    printf("\n");
    printf("  __msgTag__ = MSG_TAG_ANY;\n");
    printf("  __len__ = sizeof(__recvBuffer__);\n");
    printf("  requestingThread = msg_recv(&__msgTag__, &__recvBuffer__, &__len__);\n");
    printf("  switch (__msgTag__) {\n");
    for (cf = methods; *cf; cf++) {
	(*cf)->genSwitch(FALSE);
    }
    printf("    default:\n");
    printf("	    return(__msgTag__);\n");
    printf("  }\n");
    printf("  return(0);\n");
    printf("}\n");
}

void interfaceSpec::generateXDRLoop()
{
    List <remoteFunc*> cf;

    printf("int %s::mainLoop(void)\n", name);
    printf("{\n");
    printf("    unsigned int __msgTag__, __status__;\n");
    printf("    __xdrs__->x_op = XDR_DECODE;\n");
    printf("    xdrrec_skiprecord(__xdrs__);\n");
    printf("    __status__ = xdr_int(__xdrs__, &__msgTag__);\n");
    printf("	if (!__status__) return(-1);\n");
    printf("    switch (__msgTag__) {\n");

    // generate the zero RPC that returns interface name & version.
    printf("        case 0:\n");
    printf("            char *__ProtocolName__ = \"%s\";\n", name);
    printf("            int __val__;\n");
    printf("            __xdrs__->x_op = XDR_ENCODE;\n");
    printf("            __val__ = 0;\n");
    printf("            xdr_int(__xdrs__, &__val__);\n");
    printf("            xdr_String(__xdrs__, &__ProtocolName__);\n");
    printf("            __val__ = %d;\n", version);
    printf("            xdr_int(__xdrs__, &__val__);\n");
    printf("            xdrrec_endofrecord(__xdrs__, TRUE);\n");
    printf("            break;\n");

    for (cf = methods; *cf; cf++) {
	(*cf)->genSwitch(FALSE);
    }
    printf("    default:\n");
    printf("	    return(__msgTag__);\n");
    printf("  }\n");
    printf("  return(0);\n");
    printf("}\n");
}



void interfaceSpec::genIncludes()
{
    printf("#include <stdio.h>\n");
    printf("#include <stdlib.h>\n");
    printf("#include <assert.h>\n");
    if (generateTHREAD) {
	printf("extern \"C\" {\n");
	printf("#include \"thread/thread.h\"\n");
	printf("}\n");
    }
    if (generateXDR) {
	printf("extern \"C\" {\n");
	printf("#include <rpc/types.h>\n");
	printf("#include <rpc/xdr.h>\n");
	printf("}\n");
    }

    printf("#include \"%s\"\n\n", headerFile);
}

void interfaceSpec::generateServerCode()
{
    List<remoteFunc*> cf;

    if (generateTHREAD) {
	generateThreadLoop();
    } else if (generateXDR) {
	FILE *sf;

	// needs to go into a seperate file.
	fflush(stdout);

	sf = fopen(serverFile, "w");
	dup2(sf->_file, 1);

	genIncludes();

	generateXDRLoop();
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
    if (generateTHREAD) {
	printf("  union %s __recvBuffer__;\n", unionName);
	printf("  unsigned __len__ = sizeof(__recvBuffer__);\n");
    }
    printf("  while (1) {\n");
    if (generateXDR) {
	printf("    __xdrs__->x_op = XDR_DECODE;\n");
	printf("    xdrrec_skiprecord(__xdrs__);\n");
	printf("    xdr_int(__xdrs__, &__tag__);\n");
    } else if (generateTHREAD) {
	printf("  __tag__ = MSG_TAG_ANY;\n");
	printf("    requestingThread = msg_recv(&__tag__, (void *) &__recvBuffer__, &__len__); \n");
    }
    printf("    if (__tag__ == __targetTag__) return;\n");
    printf("    switch (__tag__) {\n");
    for (cf = methods; *cf; cf++) {
	(*cf)->genSwitch(TRUE);
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
    printf("    xdr_int(__xdrs__, &__tag__);\n");
    printf("    xdrrec_endofrecord(__xdrs__, TRUE);\n");
    printf("    awaitResponce(0);\n");
    printf("    xdr_String(__xdrs__, &(proto));\n");
    printf("    xdr_int(__xdrs__, &(version));\n");
    printf("    if ((version != %d) || (strcmp(proto, \"%s\"))) {\n",
	version, name);
    printf("        printf(\"protocol %s version %d expected\\n\");\n", 
	name, version);
    printf("        printf(\"protocol %%s version %%d found\\n\", proto, version);\n");
    printf("	    exit(-1);\n");
    printf("    }\n");
    printf("}\n");
    printf("\n\n");
    printf("%sUser::%sUser(int fd, xdrIOFunc r, xdrIOFunc w):\n", name, name);
    printf("XDRrpc(fd, r, w) { if (__xdrs__) verifyProtocolAndVersion(); }\n");
    printf("%sUser::%sUser(char *m,char *l,char *p,xdrIOFunc r,xdrIOFunc w):\n",
	name, name);
    printf("    XDRrpc(m, l, p, r, w) { if (__xdrs__) \n");
    printf("       verifyProtocolAndVersion(); }\n");
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

	// needs to go into a seperate file.
	fflush(stdout);

	cf = fopen(clientFile, "w");
	dup2(cf->_file, 1);

	genIncludes();
    }

    generateStubs();

    if (generateXDR) {
	genProtoVerify();
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
    char *file;

    /* define pre-defined types */
    (void) new typeDefn("int");
    (void) new typeDefn("double");
    (void) new typeDefn("String");
    (void) new typeDefn("void");
    (void) new typeDefn("Boolean");

    emitCode = 1;
    emitHeader = 1;
    for (i=0; i < argc-1; i++) {
	if (!strcmp("-pvm", argv[i])) {
	    generatePVM = 1;
	    generateXDR = 0;
	    generateTHREAD = 0;
	    serverTypeName = pool.findAndAdd("int");
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

    file = argv[argc-1];

    yyin = fopen(file, "r");
    if (!yyin) {
	printf("unable to open %s\n", file);
	exit(-1);
    }

    // skip to trailing component of file name.
    temp = strrchr(file, '/');
    if (temp) file = temp+1;

    temp = strstr(file, ".I");
    if (!temp) {
	printf("file names must end in .I\n");
	exit(-1);
    }
    *(temp+1) = '\0';

    headerFile = (char *) malloc(strlen(file)+1);
    sprintf(headerFile, "%sh", file);

    codeFile = (char *) malloc(strlen(file)+1);
    sprintf(codeFile, "%sC", file);

    serverFile = (char *) malloc(strlen(file)+6);
    sprintf(serverFile, "%sSRVR.C", file);

    clientFile = (char *) malloc(strlen(file)+6);
    sprintf(clientFile, "%sCLNT.C", file);

    if (emitHeader) {
	of = fopen(headerFile, "w");
	dup2(of->_file, 1);
	printf("#include \"util/rpcUtil.h\"\n");
    }

    yyparse();

    if (emitHeader) {
	fflush(stdout);
	fclose(of);
    }

    if (emitCode) {
	FILE *cf;

	cf = fopen(codeFile, "w");
	dup2(cf->_file, 1);

	currentInterface->genIncludes();
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

    ret = (char *) malloc(20);
    sprintf(ret, "%s__%d", name, count++);
    return(ret);
}

int interfaceSpec::getNextTag()
{
    return(++boundTag);
}

void remoteFunc::genMethodHeader(char *className)
{
    int spitOne;
    List <char *> cl;
    List<argument*> lp;

    if (className) {
	printf("%s %s::%s(", retType, className, name);
    } else {
	printf("%s %s(", retType, name);
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

void remoteFunc::genSwitch(Boolean forUpcalls)
{
    int first;
    List <argument *> ca;


    if (forUpcalls && (upcall == notUpcall)) return;
    if (!forUpcalls && (upcall != notUpcall)) return;

    printf("        case %s_%s_REQ: {\n", spec->getName(), name);
    printf("            ");
    printf("	    int __val__;\n");
    if (strcmp(retType, "void")) {
	printf("	    %s __ret__;\n", retType);
    }
    if (generateXDR) {
	printf("	    extern xdr_%s(XDR*, %s*);\n", retType, retType);
	printf("            %s __recvBuffer__;\n", structName);
	for (ca = args; *ca; ca++) {
	    printf("            xdr_%s(__xdrs__, &__recvBuffer__.%s);\n", 
		(*ca)->type, (*ca)->name);
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
	}
    }
    printf(");\n");
    if (generateTHREAD && (upcall != asyncUpcall)) {
	if (strcmp(retType, "void")) {
	    printf("        __val__ = msg_send(requestingThread, %s_%s_RESP, (void *) &__ret__, sizeof(%s));\n", spec->getName(), name, retType);
	} else {
	    printf("        __val__ = msg_send(requestingThread, %s_%s_RESP, NULL, 0);\n", 
		spec->getName(), name);
	}
	printf("	    assert(__val__ == THR_OKAY);\n");
    } else if (generateXDR && (upcall != asyncUpcall)) {
	printf("	    __xdrs__->x_op = XDR_ENCODE;\n");
	printf("            __val__ = %s_%s_RESP;\n", spec->getName(), name);
	printf("            xdr_int(__xdrs__, &__val__);\n");
	if (strcmp(retType, "void")) {
	    printf("            xdr_%s(__xdrs__,&__ret__);\n", retType);
	}
	printf("	    xdrrec_endofrecord(__xdrs__, TRUE);\n");
    }
    printf("            break;\n         }\n\n");
}


void remoteFunc::genThreadStub(char *className)
{
    List<argument*> lp;
    char *retVar = spec->genVariable();
    char *structUseName = spec->genVariable();

    genMethodHeader(className);
    printf(" {\n");

    printf("    struct %s %s;\n", structName, structUseName);
    printf("    unsigned int __tag__;\n");
    printf("    unsigned int __len__;\n");
    printf("    int __val__;\n");
    if (strcmp(retType, "void")) {
	printf("    %s %s;\n", retType, retVar);
    }
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

    genMethodHeader(className);
    printf(" {\n");

    printf("    unsigned int __tag__;\n");
    printf("    int __val__;\n");
    if (strcmp(retType, "void")) {
	printf("    unsigned int __len__;\n");
	printf("    %s %s;\n", retType, retVar);
    }
    printf("    __tag__ = %s_%s_REQ;\n", spec->getName(), name);
    printf("    __xdrs__->x_op = XDR_ENCODE;\n");
    printf("    xdr_int(__xdrs__, &__tag__);\n");
    for (lp = args; *lp; lp++) {
	printf("    xdr_%s(__xdrs__, &%s);\n", (*lp)->type, (*lp)->name);
    }
    printf("    xdrrec_endofrecord(__xdrs__, TRUE);\n");
    if (upcall != asyncUpcall) {
	if (upcall == notUpcall) {
	    printf("    awaitResponce(%s_%s_RESP);\n",spec->getName(), name);
	} else if (upcall == syncUpcall) {
	    printf("    __xdrs__->x_op = XDR_DECODE;\n");
	    printf("    xdrrec_skiprecord(__xdrs__);\n");
	    printf("    xdr_int(__xdrs__, &__tag__);\n");
	    printf("    assert(__tag__ == %s_%s_RESP);\n", spec->getName(), name);
	}
	if (strcmp(retType, "void")) {
	    printf("    xdr_%s(__xdrs__, &(%s)); \n", retType, retVar);
	    printf("    return(%s);\n", retVar);
	}
    }
    printf("}\n\n");
}

void remoteFunc::genStub(char *className, Boolean forUpcalls)
{
    if (forUpcalls && (upcall == notUpcall)) return;
    if (!forUpcalls && (upcall != notUpcall)) return;

    printf("\n");
    if (generateXDR) {
	genXDRStub(className);
    } else if (generateTHREAD) {
	genThreadStub(className);
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
    }
}


void typeDefn::genBundler()
{
    List<field*> fp;

    if (!userDefined) return;

    printf("bool_t xdr_%s(XDR *__xdrs__, %s *__ptr__) {\n", name, name);
    if (arrayType) {
	printf("if (__xdrs__->x_op == XDR_DECODE) __ptr__->data = NULL;\n");
	printf("    xdr_array(__xdrs__, &(__ptr__->data), &__ptr__->count, ~0, sizeof(%s), xdr_%s);\n",
	    type, type);
    } else {
	for (fp = fields; *fp; fp++) {
	    (*fp)->genBundler();
	}
    }
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

void interfaceSpec::genClass()
{
    List <remoteFunc *> curr; 

    printf("class %sUser: public RPCUser, public %s {\n", name, transportBase);
    printf("  public:\n");
    if (generateXDR) {
	printf("    virtual void verifyProtocolAndVersion();\n");
	printf("    %sUser(int fd, xdrIOFunc r, xdrIOFunc w);\n", name);
	printf("    %sUser(char *,char *,char*, xdrIOFunc r, xdrIOFunc w);\n", 
		name);
    }
    printf("    void awaitResponce(int);\n");
    printf("    int isValidUpCall(int);\n");

    for (curr = methods; *curr; curr++) {
	printf("    ");
	(*curr)->genMethodHeader(NULL);
	printf(";\n");
    }
    printf("};\n");


    printf("class %s: private RPCServer, public %s {\n", name, transportBase);
    printf("  public:\n");
    printf("    mainLoop(void);\n");
    for (curr = methods; *curr; curr++) {
	printf("    ");
	(*curr)->genMethodHeader(NULL);
	printf(";\n");
    }
    printf("};\n\n");
}

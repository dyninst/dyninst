
/*
 * defns.C - type and structure definition support
 *
 * $Log: typeDefns.C,v $
 * Revision 1.4  1994/09/22 00:38:09  markc
 * Made all templates external
 * Made array declarations separate
 * Add class support
 * Add support for "const"
 * Bundle pointers for xdr
 *
 * Revision 1.3  1994/08/18  19:53:34  markc
 * Added support for new files.
 * Removed compiler warnings for solaris2.3
 *
 * Revision 1.2  1994/08/18  05:57:00  markc
 * Changed char*'s to stringHandles
 *
 * Revision 1.1  1994/08/17  17:52:05  markc
 * Added Makefile for Linux.
 * Support new source files for igen.
 * Separate source files.  ClassDefns supports classes, typeDefns supports
 * structures.
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
#include "parse.h"

extern int generateXDR;
extern int generateTHREAD;

// found is set to the type defn for the array type, which contains
// a pointer to the element type

char *check_user_star(stringHandle n, char *st, char *&newN,
		      int &userD, int &dontGen)
{
  int slen;
  char *mName=0, strBuf[100];

  slen = strlen(st);
  switch (slen) {
  case 1:
    if (generateXDR &&
	(strcmp("char", (char*) n) && !userD)) {
      cout << "Too many levels of indirection for variable name " << n << endl;
      exit(0);
    }
    // if this is a pointer to a character, dont generate a class for it
    if (generateXDR &&
	!strcmp("char", (char*) n))
      dontGen = 1;
    userD = 1; // for char_PTR
    sprintf(strBuf, "%s_PTR", (char*)n);
    mName = strdup(strBuf);
    sprintf(strBuf, "%s%s", (char*)n, st);
    newN = strdup(strBuf);
    break;
  default:
    if (generateTHREAD) {
      ;
    } else {
      cout << "Too many levels of indirection in defining a pointer to a type\n";
      cout << "Name " << (char*) n << " Levels " << slen << endl;
      exit(-1);
    }
    sprintf(strBuf, "%s%s", (char*)n, st);
    newN = strdup(strBuf);
    break;
  }
  return mName;
}

// if st is non-null, then this type is a pointer to another type, which
// may be defined here
userDefn::userDefn(stringHandle n, int userD, List<field*> &f, char *st, int at)
{
  userDefn *fd;
  char *newName;

  dontGen=0;
  if (st) {
    marshallName = check_user_star(n, st, newName, userD, dontGen);
    name = namePool.findAndAdd(newName); 
    delete newName;
  } else {
    marshallName = strdup((const char*) n);
    name = n;
  }

  if(fd = userPool.find(name)) {
    printf("Name %s for class definition is already used\n", (char*)n);
    printf("Error, exiting.\n");
    exit(0);
  }
  do_ptr = userD;
  fields = f;
  userDefined = userD;
  arrayType = at;
  doFree = userD;
  isChar = 0;
}
						 
userDefn::userDefn(stringHandle n, int userD, char *st, int at)
{
  userDefn *fd;
  char *newName;

  dontGen=0;
  if (st) {
    marshallName = check_user_star(n, st, newName, userD, dontGen);
    name = namePool.findAndAdd(newName);
    delete newName;
  } else {
    marshallName = strdup((const char*) n);
    name = n;
  }

  if(fd = userPool.find(name)) {
    printf("Name %s for class definition is already used\n", (char*)n);
    printf("Error, exiting.\n");
    exit(0);
  }
  do_ptr = userD;
  userDefined = userD;
  arrayType = at;
  doFree = userDefined;
  isChar = !strcmp("char", (char *) n);
}

void typeDefn::genHeader()
{
    List<field*> fp;

    if (getDontGen())
      return;
    dot_h << "\n#ifndef " << getMarshallName() << "_TYPE\n";
    dot_h << "#define " << getMarshallName() << "_TYPE\n";
    // dot_h << "#define " << getMarshallName() << "_PTR " << getMarshallName() << "* \n";
    dot_h << "class " << getMarshallName() << " {  \npublic:\n";
    if (getArrayType()) {
	dot_h << "    unsigned int count;\n";
	dot_h << "    " << getElemUname() << "* data;\n";
    } else {
	for (fp = getFields(); *fp; ++fp) {
	    (*fp)->genHeader(dot_h);
	}
    }
    dot_h << "};\n\n";
    dot_h << "#endif\n";
}

//
// this is only called if generatePVM or generateXDR is TRUE
// this is not called for generateTHREAD since no bundling is
//  done since everything
// exists in the same address space
// builds data bundlers - code that bundles user defined structures
// for the transport
// note - the basic filters {string, int, ...} are taken care of
//  elsewhere
//
// how are bundlers built
//  each bundler has an if branch and an else branch.  The if branch
//  is taken if the request is to free memory, otherwise the else
//  branch is taken.  If the type of the bundler is an array of <x>,
//  each element of the array must have its bundler called in the
//  if branch.  If the bundler is not an array of some type, but is
//  a structure, with fields, the bundlers for each field is called.
//
//
void typeDefn::genBundler()
{
  if (!getUserDefined()) return;

  if (generateXDR) {
    genBundlerXDR();
    if (getDoPtr())
      genPtrBundlerXDR();
  } 
}

void typeDefn::genBundlerXDR()
{
  List<field*> fp;
  userDefn *foundType;

  // dont generate for char*
  if (getDontGen())
    return;

  // the declaration for the bundler
  dot_h << "extern bool_t xdr_" << getMarshallName() << "(XDR*, " <<
    getUname() << "*);\n";

  // the code for the if branch - taken to free memory
  // build the handlers for xdr
  dot_c << "bool_t xdr_" << getMarshallName() << "(XDR *__xdrs__, " <<
    getUname() << " *__ptr__) {\n";
  // should not be passing in null pointers to free
  dot_c << "    if (!__ptr__) return FALSE;\n";

  // ***************************************************
  // handle the free'ing
  dot_c << "    if (__xdrs__->x_op == XDR_FREE) {\n";

  // note - a user defined type contains an array of other
  // types, each element in the array must be freed
  if (getArrayType()) {
    if (getElemDoFree()) {
      dot_c << "        int i;\n";
      dot_c << "        for (i=0; i<__ptr__->count; ++i)\n";
      dot_c << "            if (!xdr_" << getElemMarshall() <<
	"(__xdrs__, &(__ptr__->data[i])))\n";
      dot_c << "              return FALSE;\n";
    }
    // free the memory used for this structure
    dot_c << "        free ((char*) __ptr__->data);\n";
    dot_c << "        __ptr__->data = 0;\n";
    dot_c << "        __ptr__->count = 0;\n";
  } else for (fp = getFields(); *fp; ++fp) {
    // xdr - non-array types
    // call the bundler for each element of this user defined type
    // don't call for non-user defined types such as {int, char, bool}
    // since those will not have had memory allocated for them
    foundType = (*fp)->getFMyType();
    assert (foundType);
    if (foundType->getDoFree())
      (*fp)->genBundler(dot_c);
  }

  // ****************************************************
  // handle encoding/decoding
  dot_c << "    } else {\n";
  // the code for the else branch - taken to encode or decode 
  if (getArrayType()) {
    dot_c << "if (__xdrs__->x_op == XDR_DECODE) __ptr__->data = NULL;\n";
    dot_c << "    if (!xdr_array(__xdrs__, (char**)" <<
      " &(__ptr__->data), &__ptr__->count, 0xffffff, sizeof(";
    dot_c << getElemUname() << "), (xdrproc_t) xdr_" << getElemMarshall() << "))\n";
    dot_c << "      return FALSE;\n";
  } else {
    for (fp = getFields(); *fp; ++fp)
      (*fp)->genBundler(dot_c);
  }
  dot_c << "    }\n";
  dot_c << "    return(TRUE);\n";
  dot_c << "}\n\n";
}

void typeDefn::genPtrBundlerXDR()
{
  
  dot_h << "\nbool_t xdr_" << getMarshallName() << "_PTR" <<
    "(XDR *__xdrs__, " << getUname() << " **__ptr__);\n";

  dot_c << "\nbool_t xdr_" << getMarshallName() << "_PTR" <<
    "(XDR *__xdrs__, " << getUname() << " **__ptr__) {\n";
  dot_c << "    unsigned long __flag__ = 0;\n";
  if (ptrMode == ptrHandle)
    dot_c << "    void *__val__; int __fd__;\n";

  dot_c << "    switch (__xdrs__->x_op) {\n";
  dot_c << "       case XDR_DECODE:\n";
  dot_c << "          if (!__ptr__)\n";
  dot_c << "              return FALSE;\n";
  dot_c << "          else if (!xdr_u_long(__xdrs__, &__flag__))\n";
  dot_c << "              return FALSE;\n";
  dot_c << "          else if (igen_flag_is_null(__flag__)) {\n";
  dot_c << "              *__ptr__ = (" << getUname() << "*) 0;\n";
  dot_c << "              return TRUE;\n";
  dot_c << "          }\n";


  switch (ptrMode) {
  case ptrIgnore:
  case ptrDetect:
    dot_c << "        *__ptr__ = new " << getUname() <<";\n";
    break;
  case ptrHandle:
    dot_c << "        if (!xdr_u_long(__xdrs__, (unsigned long*)&__val__))\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        if (igen_flag_is_shared(__flag__)) {\n";
    dot_c << "            *__ptr__ = (" << getUname() <<
      "_PTR) __ptrTable__.find(__val__, __fd__);\n";
    dot_c << "            if (!*__ptr__ || !__fd__)\n";
    dot_c << "              return FALSE;\n";
    dot_c << "        } else {\n";
    dot_c << "            if (!(*__ptr__ = new " << getUname() <<"))\n";
    dot_c << "                return FALSE;\n";
    dot_c << "        if (!__ptrTable__.add((void*) *__ptr__, __val__))\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        };\n";
  }

  dot_c << "          return(xdr_" << getMarshallName() << "(__xdrs__, *__ptr__));\n";
  dot_c << "          break;\n";

  dot_c << "       case XDR_ENCODE:\n";
  dot_c << "          if (!__ptr__)\n";
  dot_c << "              return FALSE;\n";
  dot_c << "          else if (!*__ptr__) {\n";
  dot_c << "              // signifies NULL\n";
  dot_c << "              igen_flag_set_null(__flag__);\n";
  dot_c << "              if (!xdr_u_long(__xdrs__, &__flag__))\n";
  dot_c << "                  return FALSE;\n";
  dot_c << "              else\n";
  dot_c << "                  return TRUE;\n";
  dot_c << "          } else {\n";


  // detect duplicate pointer here
  switch (ptrMode) {
  case ptrIgnore:
    dot_c << "             // ignore duplicate pointers\n";
    dot_c << "              if (!xdr_u_long(__xdrs__, &__flag__))\n";
    dot_c << "                  return FALSE;\n";
    break;
  case ptrDetect:
    dot_c << "        // detect duplicate pointers\n";
    dot_c << "        if (__ptrTable__.inTable((void*) *__ptr__)) {\n";
    dot_c << "            printf(\"DUPLICATE POINTER\\n\");\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        };\n";
    dot_c << "        if (!xdr_u_long(__xdrs__, &__flag__))\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        if (!__ptrTable__.add((void*) *__ptr__, (void*) *__ptr__))\n";
    dot_c << "            return FALSE;\n";
    break;
  case ptrHandle:
    dot_c << "        // handle duplicate pointers\n";
    dot_c << "        if (__ptrTable__.inTable((void*) *__ptr__)) {\n";
    dot_c << "            igen_flag_set_shared(__flag__);\n";
    dot_c << "        };\n";
    dot_c << "        if (!xdr_u_long(__xdrs__, &__flag__))\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        __flag__ = (unsigned long)__ptr__;\n";
    dot_c << "        if (!xdr_u_long(__xdrs__, &__flag__))\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        if (!__ptrTable__.add((void*) *__ptr__, (void*) *__ptr__))\n";
    dot_c << "            return FALSE;\n";
    break;
  }
  dot_c << "          return(xdr_" << getMarshallName() << "(__xdrs__, *__ptr__));\n";
  dot_c << "          }\n";
  dot_c << "          break;\n";
  dot_c << "       case XDR_FREE:\n";
  dot_c << "          if (!__ptr__)\n";
  dot_c << "              return FALSE;\n";
  dot_c << "          else if (!*__ptr__)\n";
  dot_c << "              return TRUE;\n";
  dot_c << "          else if (!xdr_" << getMarshallName() <<
    "(__xdrs__, *__ptr__))\n";
  dot_c << "                return FALSE;\n";
  dot_c << "          else {\n";
  dot_c << "            delete *__ptr__;\n";
  dot_c << "            *__ptr__ = 0;\n";
  dot_c << "            return TRUE;\n";
  dot_c << "          };\n";
  dot_c << "          break;\n";
  dot_c << "     }\n";
  dot_c << "   return FALSE;\n";
  dot_c << "}\n";
}

typeDefn::typeDefn(stringHandle i, List<field *> &f) : userDefn(i, TRUE, f, 0) {
  userPool.add(this, getUsh());
  arrayElement = 0;
}

typeDefn::typeDefn(stringHandle i, char *st) : userDefn(i, FALSE, st) {
  userPool.add(this, getUsh());
  arrayElement = 0;
}

// for arrays types.
typeDefn::typeDefn(stringHandle i, userDefn *ae, char *st)
: userDefn(i, TRUE, st, TRUE) {
  
  userPool.add(this, getUsh());
  arrayElement = ae;
}





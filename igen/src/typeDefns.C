
/*
 * defns.C - type and structure definition support
 *
 * $Log: typeDefns.C,v $
 * Revision 1.2  1994/08/18 05:57:00  markc
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
extern int generatePVM;
extern int generateTHREAD;

void generate_ptr_type (stringHandle type, stringHandle ptr_type);
void generate_ptr_type(stringHandle baset, stringHandle type)
{
  userDefn *old_base;

  old_base = userPool.find((char*)baset);
  if (!old_base) {
    printf("Base type %s for %s not found, exiting\n", (char*) baset,(char*)type);
    exit(0);
  }
}


stringHandle findAndAddArrayType(stringHandle name) 
{
    typeDefn *s;
    char temp[80];
    stringHandle tempF;
    userDefn *found;

    // now find the associated array type.
    sprintf(temp, "%s_Array", (char*)name);
    tempF = namePool.find(temp);

    if (tempF && (found = userPool.find(tempF))) {
      if (found->arrayType)
	return (found->name);
      else {
	cout << "Type " << temp << " is already in use\n, exiting";
	exit(0);
      }
    } else if ((found = userPool.find(name)) || (generateTHREAD)) {
      tempF = namePool.findAndAdd(temp);
      s = new typeDefn(tempF, name);
      s->genHeader();      
      return (s->name);
    } else {
      cout << "For array " << temp << " element type " << (char*)name <<
	" is illegal, exiting\n";
      exit(0);
    }
}

userDefn::userDefn(stringHandle n, int userD, List<field*> &f)
{
  userDefn *fd;
  if(fd = userPool.find(n)) {
    printf("Name %s for class definition is already used\n", (char*)n);
    printf("Error, exiting.\n");
    exit(0);
  }
  do_ptr = userD;
  name = n;
  fields = f;
  userDefined = userD;
  arrayType = FALSE;
}

userDefn::userDefn(stringHandle n, int userD)
{
  userDefn *fd;
  if(fd = userPool.find(n)) {
    printf("Name %s for class definition is already used\n", (char*)n);
    printf("Error, exiting.\n");
    exit(0);
  }
  do_ptr = userD;
  name = n;
  userDefined = userD;
  arrayType = FALSE;
}

void typeDefn::genHeader()
{
    List<field*> fp;


    dot_h << "\n#ifndef " << (char*)name << "_TYPE\n";
    dot_h << "#define " << (char*)name << "_TYPE\n";
    dot_h << "#define " << (char*)name << "_PTR " << (char*)name << "* \n";
    dot_h << "class " << (char*)name << " {  \npublic:\n";
    if (arrayType) {
	dot_h << "    int count;\n";
	dot_h << "    " << (char*)type << "* data;\n";
    } else {
	for (fp = fields; *fp; fp++) {
	    (*fp)->genHeader(dot_h);
	}
    }
    dot_h << "};\n\n";
    dot_h << "#endif\n";

    if (generateXDR) {
	if (userDefined) {
	  dot_h << "extern xdr_" << (char*)name << "(XDR*, " << (char*)name << "*);\n";
	  if (do_ptr)
	    dot_h << "extern xdr_" << (char*)name << "_PTR" <<
	      "(XDR*, " << (char*)name << "_PTR*);\n";
	}
    } else if (generatePVM) {
        if (userDefined) {
	  dot_h << "extern IGEN_pvm_" << (char*)name <<
	    "(IGEN_PVM_FILTER, " << (char*)name << "*);\n";
	  if (do_ptr)
	    dot_h << "extern IGEN_pvm_" << (char*)name << "_PTR" <<
	      "(IGEN_PVM_FILTER, " << (char*)name << "_PTR*);\n";
	}
    }
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
  if (!userDefined) return;

  if (generateXDR) {
    genBundlerXDR();
    if (do_ptr)
      genPtrBundlerXDR();
  } else if (generatePVM) {
    genBundlerPVM();
    if (do_ptr)
      genPtrBundlerPVM();
  }
}


void typeDefn::genBundlerPVM()
{
  List<field*> fp;
  userDefn *foundType;

  // build the handlers for PVM
  dot_c << "bool_t IGEN_pvm_" << (char*)name <<
    "(IGEN_PVM_FILTER __dir__, " << (char*)name << " *__ptr__) {\n";
  dot_c << "    if (__dir__ == IGEN_PVM_FREE) {\n";

  // free the array of user defined types
  // this calls the bundlers
  if (arrayType) {
    assert (foundType = userPool.find(type));
    if (foundType->userDefined) {
      dot_c << "        int i;\n";
      dot_c << "        for (i=0; i<__ptr__->count; ++i)\n";
      dot_c << "            if (!IGEN_pvm_" << (char*)type <<
	"(__dir__, &(__ptr__->data[i])))\n";
      dot_c << "              return FALSE;\n";
    }
    dot_c << "        free ((char*) __ptr__->data);\n";
    dot_c << "        __ptr__->data = 0;\n";
    dot_c << "        __ptr__->count = 0;\n";
  } else for (fp = fields; *fp; fp++) {
    foundType = userPool.find((*fp)->getType());
    assert (foundType);
    if (foundType->userDefined ||
	foundType->arrayType ||
	!(strcmp("String", (char*) foundType->name)))
      (*fp)->genBundler(dot_c);
  }

  dot_c << "    } else {\n";
  // the code for the else branch - taken to encode or decode 
  if (arrayType) {
    dot_c << "    IGEN_pvm_Array_of_" << (char*)type <<
      "(__dir__, &(__ptr__->data),&(__ptr__->count));\n";
  } else {
    for (fp = fields; *fp; fp++)
      (*fp)->genBundler(dot_c);
  }
  dot_c << "    }\n";
  dot_c << "    return(TRUE);\n";
  dot_c << "}\n\n";
}

void typeDefn::genPtrBundlerPVM()
{

}

void typeDefn::genBundlerXDR()
{
  List<field*> fp;
  userDefn *foundType;

  // the code for the if branch - taken to free memory
  // build the handlers for xdr
  dot_c << "bool_t xdr_" << (char*)name << "(XDR *__xdrs__, " <<
    (char*)name << " *__ptr__) {\n";
  // should not be passing in null pointers to free
  dot_c << "    if (!__ptr__) return FALSE;\n";

  // ***************************************************
  // handle the free'ing
  dot_c << "    if (__xdrs__->x_op == XDR_FREE) {\n";

  // note - a user defined type contains an array of other
  // types, each element in the array must be freed
  if (arrayType) {
    assert (foundType = userPool.find(type));
    if (foundType->userDefined) {
      dot_c << "        int i;\n";
      dot_c << "        for (i=0; i<__ptr__->count; ++i)\n";
      dot_c << "            if (!xdr_" << (char*)type <<
	"(__xdrs__, &(__ptr__->data[i])))\n";
      dot_c << "              return FALSE;\n";
    }
    // free the memory used for this structure
    dot_c << "        free ((char*) __ptr__->data);\n";
    dot_c << "        __ptr__->data = 0;\n";
    dot_c << "        __ptr__->count = 0;\n";
  } else for (fp = fields; *fp; fp++) {
    // xdr - non-array types
    // call the bundler for each element of this user defined type
    // don't call for non-user defined types such as {int, char, bool}
    // since those will not have had memory allocated for them
    foundType = userPool.find((*fp)->getType());
    assert (foundType);
    if (foundType->userDefined ||
	foundType->arrayType ||
	!(strcmp("String", (char*) foundType->name)))
      (*fp)->genBundler(dot_c);
  }

  // ****************************************************
  // handle encoding/decoding
  dot_c << "    } else {\n";
  // the code for the else branch - taken to encode or decode 
  if (arrayType) {
    dot_c << "if (__xdrs__->x_op == XDR_DECODE) __ptr__->data = NULL;\n";
    dot_c << "    if (!xdr_array(__xdrs__, (char**)" <<
      " &(__ptr__->data), &__ptr__->count, ~0, sizeof(";
    dot_c << (char*)type << "), (xdrproc_t) xdr_" << (char*)type << "))\n";
    dot_c << "      return FALSE;\n";
  } else {
    for (fp = fields; *fp; fp++)
      (*fp)->genBundler(dot_c);
  }
  dot_c << "    }\n";
  dot_c << "    return(TRUE);\n";
  dot_c << "}\n\n";
}

void typeDefn::genPtrBundlerXDR()
{
  dot_c << "\nbool_t xdr_" << (char*)name << "_PTR" <<
    "(XDR *__xdrs__, " << (char*)name << " **__ptr__) {\n";
  dot_c << "    int __flag__ = 0;\n";
  if (ptrMode == ptrHandle)
    dot_c << "    void *__val__; int __fd__;\n";

  dot_c << "    switch (__xdrs__->x_op) {\n";
  dot_c << "       case XDR_DECODE:\n";
  dot_c << "          if (!__ptr__)\n";
  dot_c << "              return FALSE;\n";
  dot_c << "          else if (!xdr_u_int(__xdrs__, &__flag__))\n";
  dot_c << "              return FALSE;\n";
  dot_c << "          else if (igen_flag_is_null(__flag__)) {\n";
  dot_c << "              *__ptr__ = (" << (char*)name << "_PTR) 0;\n";
  dot_c << "              return TRUE;\n";
  dot_c << "          }\n";


  switch (ptrMode) {
  case ptrIgnore:
  case ptrDetect:
    dot_c << "        *__ptr__ = new " << (char*)name <<";\n";
    break;
  case ptrHandle:
    dot_c << "        if (!xdr_u_int(__xdrs__, (unsigned int*)&__val__))\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        if (igen_flag_is_shared(__flag__)) {\n";
    dot_c << "            *__ptr__ = (" << (char*)name <<
      "_PTR) __ptrTable__.find(__val__, __fd__);\n";
    dot_c << "            if (!*__ptr__ || !__fd__)\n";
    dot_c << "              return FALSE;\n";
    dot_c << "        } else {\n";
    dot_c << "            if (!(*__ptr__ = new " << (char*)name <<"))\n";
    dot_c << "                return FALSE;\n";
    dot_c << "        if (!__ptrTable__.add((void*) *__ptr__, __val__))\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        };\n";
  }

  dot_c << "          return(xdr_" << (char*)name << "(__xdrs__, *__ptr__));\n";
  dot_c << "          break;\n";

  dot_c << "       case XDR_ENCODE:\n";
  dot_c << "          if (!__ptr__)\n";
  dot_c << "              return FALSE;\n";
  dot_c << "          else if (!*__ptr__) {\n";
  dot_c << "              // signifies NULL\n";
  dot_c << "              igen_flag_set_null(__flag__);\n";
  dot_c << "              if (!xdr_u_int(__xdrs__, &__flag__))\n";
  dot_c << "                  return FALSE;\n";
  dot_c << "              else\n";
  dot_c << "                  return TRUE;\n";
  dot_c << "          } else {\n";


  // detect duplicate pointer here
  switch (ptrMode) {
  case ptrIgnore:
    dot_c << "             // ignore duplicate pointers\n";
    dot_c << "              if (!xdr_u_int(__xdrs__, &__flag__))\n";
    dot_c << "                  return FALSE;\n";
    break;
  case ptrDetect:
    dot_c << "        // detect duplicate pointers\n";
    dot_c << "        if (__ptrTable__.inTable((void*) *__ptr__)) {\n";
    dot_c << "            printf(\"DUPLICATE POINTER\\n\");\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        };\n";
    dot_c << "        if (!xdr_u_int(__xdrs__, &__flag__))\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        if (!__ptrTable__.add((void*) *__ptr__, (void*) *__ptr__))\n";
    dot_c << "            return FALSE;\n";
    break;
  case ptrHandle:
    dot_c << "        // handle duplicate pointers\n";
    dot_c << "        if (__ptrTable__.inTable((void*) *__ptr__)) {\n";
    dot_c << "            igen_flag_set_shared(__flag__);\n";
    dot_c << "        };\n";
    dot_c << "        if (!xdr_u_int(__xdrs__, &__flag__))\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        __flag__ = (unsigned int)__ptr__;\n";
    dot_c << "        if (!xdr_u_int(__xdrs__, &__flag__))\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        if (!__ptrTable__.add((void*) *__ptr__, (void*) *__ptr__))\n";
    dot_c << "            return FALSE;\n";
    break;
  }
  dot_c << "          return(xdr_" << (char*)name << "(__xdrs__, *__ptr__));\n";
  dot_c << "          }\n";
  dot_c << "          break;\n";
  dot_c << "       case XDR_FREE:\n";
  dot_c << "          if (!__ptr__)\n";
  dot_c << "              return FALSE;\n";
  dot_c << "          else if (!*__ptr__)\n";
  dot_c << "              return TRUE;\n";
  dot_c << "          else\n";
  dot_c << "              return(xdr_" << (char*)name << "(__xdrs__, *__ptr__));\n";
  dot_c << "          break;\n";
  dot_c << "     }\n";
  dot_c << "   return FALSE;\n";
  dot_c << "}\n";
}

typeDefn::typeDefn(stringHandle i, List<field *> &f) : userDefn(i, TRUE, f) {
  arrayType = FALSE;
  userPool.add(this, name);
  type = 0;
}

typeDefn::typeDefn(stringHandle i) : userDefn(i, FALSE) {
  arrayType = FALSE;
  userPool.add(this, name);
  type = 0;
}

// for arrays types.
typeDefn::typeDefn(stringHandle i, stringHandle t) : userDefn(i, TRUE) {
  arrayType = TRUE;

  type = t;
  userPool.add(this, name);
}





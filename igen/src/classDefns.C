
/*
 * classDefns.C - class definition support
 *
 * $Log: classDefns.C,v $
 * Revision 1.3  1994/08/18 19:53:24  markc
 * Added support for new files.
 * Removed compiler warnings for solaris2.3
 *
 * Revision 1.2  1994/08/18  05:56:43  markc
 * Changed char*'s to stringHandles
 *
 * Revision 1.1  1994/08/17  17:51:52  markc
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

void handle_parent_fields(classDefn*);
void handle_parent_fields_free(classDefn*);
void dump_child_id(classDefn *kid, ofstream &ofstr);

// static members from class classDefn
int classDefn::nextTypeId = 0;


void classDefn::genPtrBundlerXDR()
{
  dot_c << "\nbool_t xdr_" << (char*)name << "_PTR" <<
    "(XDR *__xdrs__, " << (char*)name << "_PTR *__ptr__) {\n";
  dot_c << "    unsigned char __class_type__;\n";
  dot_c << "    unsigned long __flag__ = 0;\n";
  if (ptrMode == ptrHandle) {
    dot_c << "      void *__val__; int __fd__;\n";
    dot_c << "      unsigned long __share__ = 0;\n";
  }

  dot_c << "    switch (__xdrs__->x_op) {\n";
  dot_c << "       case XDR_DECODE:\n";
  dot_c << "          *__ptr__ = (" << (char*)name << "_PTR) 0;\n";
  dot_c << "          if (!__ptr__)\n";
  dot_c << "              return FALSE;\n";
  dot_c << "          else if (!xdr_u_long(__xdrs__, &__flag__))\n";
  dot_c << "              return FALSE;\n";
  dot_c << "          else if (igen_flag_is_null(__flag__)) {\n";
  dot_c << "              return TRUE;\n";
  dot_c << "          }\n";

  switch (ptrMode) {
  case ptrIgnore:
  case ptrDetect:
    break;
  case ptrHandle:
    dot_c << "          if (!xdr_u_long(__xdrs__, (unsigned long*)&__val__))\n";
    dot_c << "              return FALSE;\n";
    dot_c << "          if (igen_flag_is_shared(__flag__)) {\n";
    dot_c << "              *__ptr__ =(" << (char*)name <<
      "_PTR) __ptrTable__.find(__val__, __fd__);\n";
    dot_c << "              if (!*__ptr__ || !__fd__)\n";
    dot_c << "                  return FALSE;\n";
    dot_c << "          };\n";
  }

  dot_c << "          if (!*__ptr__) {\n";
  dot_c << "             __class_type__ = igen_flag_get_id(__flag__);\n";
  dot_c << "             switch(__class_type__) {\n";
  dot_c << "                 case " << (char*)name << "_CLASS_ID:\n";
  dot_c << "                     if (!(*__ptr__ = new " << (char*)name << "))\n";
  dot_c << "                         return FALSE;\n";
  dot_c << "                     break;\n";

  // check for all classes derived from this class
  // dump_child_id is recursive
  List<classDefn*> iter;
  classDefn *kid;
  for (iter = children; kid = *iter; iter++)
    dump_child_id(kid, dot_c);

  dot_c << "                 default:\n";
  dot_c << "                     return FALSE;\n";
  dot_c << "             }\n";
  dot_c << "          }\n";
  if (ptrMode == ptrHandle) {
    dot_c << "        if (!__ptrTable__.add((void*) *__ptr__,  __val__))\n";
    dot_c << "            return FALSE;\n";
  }

  dot_c << "          return ((*__ptr__)->bundler(__xdrs__));\n";
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
  dot_c << "              igen_flag_set_id((*__ptr__)->get_id(), __flag__);\n";

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
    dot_c << "        if (!__ptrTable__.add((void*) *__ptr__, (void*) __ptr__))\n";
    dot_c << "            return FALSE;\n";
    break;
  case ptrHandle:
    dot_c << "        // handle duplicate pointers\n";
    dot_c << "        if (__ptrTable__.inTable((void*) *__ptr__)) {\n";
    dot_c << "            igen_flag_set_shared(__flag__);\n";
    dot_c << "            __share__ = 1;\n";
    dot_c << "        };\n";
    dot_c << "        if (!xdr_u_long(__xdrs__, &__flag__))\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        __flag__ = (unsigned long)__ptr__;\n";
    dot_c << "        if (!xdr_u_long(__xdrs__, &__flag__))\n";
    dot_c << "            return FALSE;\n";
    dot_c << "        if (__share__) return TRUE;\n";
    dot_c << "        if (!__ptrTable__.add((void*) *__ptr__, (void*) *__ptr__))\n";
    dot_c << "            return FALSE;\n";
    break;
  }

  dot_c << "              return ((*__ptr__)->bundler(__xdrs__));\n";
  dot_c << "          }\n";
  dot_c << "          break;\n";
  dot_c << "       case XDR_FREE:\n";
  dot_c << "          if (!__ptr__)\n";
  dot_c << "              return FALSE;\n";
  dot_c << "          else if (!*__ptr__)\n";
  dot_c << "              return TRUE;\n";
  dot_c << "          else\n";
  dot_c << "              return ((*__ptr__)->bundler(__xdrs__));\n";
  dot_c << "          break;\n";
  dot_c << "     }\n";
  dot_c << "   return FALSE;\n";
  dot_c << "}\n";
}

void classDefn::genPtrBundlerPVM()
{

}

classDefn::classDefn(stringHandle declared_name, List<field *> &f,
		     stringHandle parent_name, char *pt) 
: userDefn (declared_name, TRUE, f)
{
  userDefn *found;
  if (parent_name) {
    if (found = userPool.find(parent_name)) {
      parentName = found->name;
      parent = (classDefn*) found;
      parent->addChild(this);
    } else {
      cout << "Parent " << (char*)parent_name << " of " << (char*)declared_name <<
	" is not known\n, exiting";
      exit(0);
    }
  } else {
    parentName = 0;
    parent = 0;
  }

  userPool.add(this, name);
  type_id = generateClassId();
  passThru = pt;
}

void classDefn::addChild(classDefn *kid)
{
  assert(kid);
  assert(kid->parentName);
  assert(!strcmp((char*)kid->parentName, (char*)name));
  children.add(kid);
}

// this will not get called if generateThread is true
void classDefn::genHeader()
{
    List<field*> fp;

    dot_h << "\n#define " << (char*)name << "_CLASS_ID " << type_id << "\n\n";
    dot_h << "\n#define " << (char*)name << "_PTR " << (char*)name << "* \n";

    if (!parentName)
      dot_h << "class " << (char*)name << " {  \npublic:\n";
    else 
      dot_h << "class " << (char*)name << " : public " << (char*)parentName
	<< " {\npublic:\n";

    for (fp = fields; *fp; fp++)
      (*fp)->genHeader(dot_h);

    dot_h << "\n   // the right bundler will be called if a\n";
    dot_h << "   // base pointer points to a derived class\n";
    dot_h << "     virtual unsigned char get_id() { return "
      << type_id << " ;}\n";

    if (generateXDR)
      dot_h << "   virtual bool_t bundler(XDR*);\n";
    else
      dot_h << "   virtual bool_t bundler(IGEN_PVM_FILTER);\n";

    if (passThru) {
      dot_h << "\n// Begin ignored  ";
      dot_h << endl << passThru << "\n// End ignored\n";
    }

    dot_h << "};\n\n";

    if (generateXDR) {
      dot_h << "extern xdr_" << (char*)name <<
	"(XDR*, " << (char*)name << "*);\n";
      if (do_ptr)
	dot_h << "extern xdr_" << (char*)name << "_PTR" <<
	  "(XDR*, " << (char*)name << "_PTR*);\n";
    } else if (generatePVM) {
      dot_h << "extern IGEN_pvm_" <<
	(char*)name << "(IGEN_PVM_FILTER, " << (char*)name << "*);\n";
      if (do_ptr)
	dot_h << "extern IGEN_pvm_" << (char*)name << "_PTR" <<
	  "(IGEN_PVM_FILTER, " << (char*)name << "_PTR*);\n";
    }
}

void classDefn::genBundlerPVM()
{
  List<field*> fp;
  userDefn *foundType;

  dot_c << "bool_t IGEN_pvm_" << (char*)name << "(IGEN_PVM_FILTER __dir__, " <<
    (char*)name << " *__ptr__) {\n";
  dot_c << "      return(__ptr__->bundler(__dir__)); ";
  dot_c << "}\n";

  // build the handlers for PVM
  dot_c << "bool_t " << (char*)name << "::bundler (IGEN_PVM_FILTER __dir__) {\n";

  dot_c << "    if (__dir__ == IGEN_PVM_FREE) {\n";

  for (fp = fields; *fp; fp++) {
    foundType = userPool.find((*fp)->getType());
    assert (foundType);
    if (foundType->userDefined ||
	foundType->arrayType ||
	!(strcmp("String", (char*)foundType->name)))
      (*fp)->genBundler(dot_c, "&(this->");
  }
  dot_c << "    } else {\n";
}

void classDefn::genBundlerXDR()
{
  List<field*> fp;
  userDefn *foundType;

  dot_c << "\nbool_t xdr_" << (char*)name << "(XDR *__xdrs__, " <<
    (char*)name << " *__ptr__) {\n";
  dot_c << "    if (!__ptr__) return FALSE;\n";
  dot_c << "    else return(__ptr__->bundler(__xdrs__));\n";
  dot_c << "}\n";

  // build the handlers for xdr
  dot_c << "bool_t " << (char*)name << "::bundler(XDR *__xdrs__) {\n";

  // ******************************************
  // handle the free'ing
  dot_c << "    if (__xdrs__->x_op == XDR_FREE) {\n";
  for (fp = fields; *fp; fp++) {
    // xdr - non-array types
    // call the bundler for each element of this user defined type
    // don't call for non-user defined types such as {int, char, bool}
    // since those will not have had memory allocated for them
    foundType = userPool.find((*fp)->getType());
    assert (foundType);
    if (foundType->userDefined ||
	foundType->arrayType ||
	!(strcmp("String", (char*) foundType->name)))
      (*fp)->genBundler(dot_c, "&(this->");
  }

  // handle fields inherited from parent
  handle_parent_fields_free(this);

  // ******************************************
  // handle the encoding/decoding
  dot_c << "    } else if (__xdrs__->x_op == XDR_ENCODE " <<
    "|| __xdrs__->x_op == XDR_DECODE) {\n";

  for (fp = fields; *fp; fp++)
    (*fp)->genBundler(dot_c, "&(this->");

  // handle inherited fields
  handle_parent_fields(this);

  dot_c << "    } else return (FALSE);\n";
  dot_c << "    return(TRUE);\n";
  dot_c << "}\n\n";
}


void classDefn::genBundler()
{
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


// ******************************************
// handle any data items that may have been inherited
void handle_parent_fields_free(classDefn *self)
{
  userDefn *found;
  List<field*> fp;

  classDefn *p = self->parent;
  if (p)
    dot_c << " // do the parents fields\n";

  while (p) {
    for (fp = (p->fields); *fp; fp++) {
      found = userPool.find((*fp)->getType());
      assert(found);
      if (found->userDefined ||
	  found->arrayType ||
	  !(strcmp("String", (char*)found->name)))
	(*fp)->genBundler(dot_c, "&(this->");
    }
    p = p->parent;
  }
}

// ******************************************
// handle any data items that may have been inherited
void handle_parent_fields(classDefn *self)
{
  List<field*> fp;
  classDefn *p = self->parent;
  if (p)
    dot_c << " // do the parents fields\n";

  while (p) {
    for (fp = (p->fields); *fp; fp++)
      (*fp)->genBundler(dot_c, "&(this->");
    p = p->parent;
  }
}

void dump_child_id (classDefn *sibling, ofstream &file_id)
{
  List<classDefn*> iter;
  classDefn *kid;

  file_id << "   case " << (char*)sibling->name << "_CLASS_ID:\n";
  file_id << "       if (!(*__ptr__ = new " << (char*)sibling->name << "))\n";
  file_id << "           return FALSE;\n";
  file_id << "       break;\n";

  for (iter = sibling->children; kid = *iter; iter++) {
    dump_child_id(kid, file_id);
  }
}

// tclTunable.C
// C++ code that provides access to tunable constants from tcl.

/* $Log: tclTunable.C,v $
/* Revision 1.3  1994/12/21 00:44:07  tamches
/* Reduces compiler warnings e.g Bool to bool, char * to const char *
/*
 * Revision 1.2  1994/11/04  15:51:26  tamches
 * Fixed a bug when searching for tunable constants by name.
 * Added some extra error checking.
 *
 * Revision 1.1  1994/10/26  23:12:52  tamches
 * First version of tclTunable.C; provides "tclTunable" tcl command that
 * can access tunable constants.
 * */

#include <assert.h>
#include <stdlib.h> // atoi()
#include <tcl.h>
#include <tk.h>
#include "util/h/tunableConst.h"

#include "tclTunable.h"

struct cmdTabEntry {
   const char *cmdName;
   int   index;
   int   numArgs;
};

const int GETNUMTUNABLES      =0;
const int GETNAME             =1;
const int GETDESCRIPTION      =2;
const int GETVALUEBYNAME      =3;
const int GETVALUEBYINDEX     =4;
const int SETVALUEBYNAME      =5;
const int SETVALUEBYINDEX     =6;
const int GETTYPEBYINDEX      =7;
const int GETTYPEBYNAME       =8;
const int GETUSEBYINDEX       =9;
const int GETUSEBYNAME        =10;
const int GETFLOATRANGEBYINDEX=11;
const int GETFLOATRANGEBYNAME =12; 
const int CMDERROR            =13;

struct cmdTabEntry TclTunableCommands[] = {
  {"getnumtunables", GETNUMTUNABLES, 0},   // size of global tunable list
  {"getname", GETNAME, 1},                 // index # to string
  {"getdescription", GETDESCRIPTION, 1},   // index # to string

  {"getvaluebyname", GETVALUEBYNAME, 1},   // string (getname) to value
  {"getvaluebyindex", GETVALUEBYINDEX, 1}, // index # to value

  {"setvaluebyname", SETVALUEBYNAME, 2},   // string (getname) x value to NULL
  {"setvaluebyindex", SETVALUEBYINDEX, 2}, // index # x value to NULL

  {"gettypebyindex", GETTYPEBYINDEX, 1},   // index # to type (bool, float, etc.)
  {"gettypebyname", GETTYPEBYNAME, 1},     // string (getname) to tunable constant type (bool, float, etc.)

  {"getusebyindex", GETUSEBYINDEX, 1},     // index # to tunableUse (developerConstant, userConstant)
  {"getusebyname", GETUSEBYNAME, 1},       // index # to tunableUse (developerConstant, userConstant)

  {"getfloatrangebyindex", GETFLOATRANGEBYINDEX, 1}, // index # to floating point range (define only for float tunable constants)
  {"getfloatrangebyname", GETFLOATRANGEBYINDEX, 1},

  {NULL, CMDERROR, 0}
};

int findCommand(Tcl_Interp *interp, int argc, char **argv) {
   if (argc==0) {
      sprintf(interp->result, "USAGE: TclTunable <option> args\n");
      return CMDERROR;
   }

   for (cmdTabEntry *cmd = TclTunableCommands; cmd!=NULL; cmd++) {
      if (0==strcmp(argv[0], cmd->cmdName)) {
         // we have a match
         if (argc-1 == cmd->numArgs)
            return cmd->index;

         // wrong # args
         sprintf(interp->result, "%s: wrong # args (%d); should be %d.\n",
		 argv[0], argc-1, cmd->numArgs);
         return CMDERROR;
      }
   }

   // could not find command
   sprintf(interp->result, "TclTunable: unknown option (%s)\n", argv[0]);
   return CMDERROR;
}

tunableConstant *tunableConstantListEntryByIndex(int index) {
   // We'll say that index numbers start at 0
   // If 1 proves more convenient, we'll change it later...

   assert(NULL != tunableConstant::allConstants);

   List<tunableConstant *> iterList = *tunableConstant::allConstants;
      // make a copy of the list for iteration purposes
      // (actually, it just copies the head element, which itself
      // is merely a pointer)

   tunableConstant *tc;
   while (NULL != (tc = *iterList)) {
      if (--index < 0)
         return tc; // normal return

      iterList++;
   }

   cerr << "Tunable constant #" << index << " looks out of range." << endl;
   return NULL;
}

tunableConstant *findTunableConstantListEntryByName(char *name) {
   assert(NULL != tunableConstant::allConstants);

   List<tunableConstant *> iterList = *tunableConstant::allConstants;

   tunableConstant *tc;
   while (NULL != (tc = *iterList)) {
      if (0==strcmp(tc->getName(), name))
         return tc;
      iterList++;
   }

   cerr << "findTunableConstantListEntryByName: could not find tc with name=" << name << endl;
   return NULL;

//   tunableConstant *result = tunableConstant::allConstants->find(name);
//      // will be NULL if not found...
//
//   if (result==NULL)
//
//   return result;
}

int getValue(Tcl_Interp *interp, tunableConstant *tc) {
   if (NULL==tc) {
      sprintf(interp->result, "getValue error: no tunable constant");
      return TCL_ERROR;
   }

   // stick value into interp->result as a string
   if (tc->getType() == tunableBoolean)
      sprintf(interp->result, "%d", ((tunableBooleanConstant *)tc)->getValue() ? 1 : 0);
   else if (tc->getType() == tunableFloat)
      sprintf(interp->result, "%g", ((tunableFloatConstant *)tc)->getValue());
   else
      assert(false);

   return TCL_OK;
}

int setValue(Tcl_Interp *interp, tunableConstant *tc, char *newValString) {
   if (NULL==tc) return TCL_ERROR;

   // stick value into interp->result as a string
   if (tc->getType() == tunableBoolean)
      ((tunableBooleanConstant *)tc)->setValue((bool)atoi(newValString));
   else if (tc->getType() == tunableFloat)
      ((tunableFloatConstant *)tc)->setValue((float)atof(newValString));
   else
      assert(false);

   return TCL_OK;
}

int getType(Tcl_Interp *interp, tunableConstant *tc) {
   if (NULL == tc) return TCL_ERROR;

   // stick value into interp->result as a string
   if (tc->getType() == tunableBoolean)
      sprintf(interp->result, "%s", "bool");
   else if (tc->getType() == tunableFloat)
      sprintf(interp->result, "%s", "float");
   else
      assert(false);

   return TCL_OK;
}

int getUse(Tcl_Interp *interp, tunableConstant *tc) {
   if (NULL == tc) return TCL_ERROR;

   // stick use into interp->result as a string
   if (tc->getUse() == developerConstant)
      sprintf(interp->result, "%s", "developer");
   else if (tc->getUse() == userConstant)
      sprintf(interp->result, "%s", "user");
   else
      assert(false);

   return TCL_OK;
}

int getFloatRange(Tcl_Interp *interp, tunableFloatConstant *tc) {
   if (NULL == tc) return TCL_ERROR;

   // stick float range into interp->result as a string
   if (tc->getUse() == tunableFloat)
      sprintf(interp->result, "%g %g", tc->getMin(), tc->getMax());
   else
      assert(false);

   return TCL_OK;
}

int TclTunableCommand(ClientData cd, Tcl_Interp *interp,
		      int argc, char **argv) {
   // This is the entrypoint for the TclTunable command.
   // i.e. once installed into tcl, a tcl code call to "TclTunable" enters here...

   int commandIndex = findCommand(interp, argc-1, argv+1);
   if (commandIndex == CMDERROR)
      return TCL_ERROR;

   switch (commandIndex) {
      case GETNUMTUNABLES:
  	 assert(tunableConstant::allConstants);
         sprintf(interp->result, "%d", tunableConstant::allConstants->count());
         return TCL_OK;

      case GETNAME: {
         // list index number --> name
         tunableConstant *tc = tunableConstantListEntryByIndex(atoi(argv[2]));
         if (tc==NULL) return TCL_ERROR;

         sprintf(interp->result, "%s", tc->getName());
         return TCL_OK;
      }

      case GETDESCRIPTION: {
         // list index number --> description (if no description, we substitute the name)
         tunableConstant *tc = tunableConstantListEntryByIndex(atoi(argv[2]));
         if (NULL == tc) return TCL_ERROR;

         sprintf(interp->result, "%s", (tc->getDesc()==NULL) ? tc->getName() : tc->getDesc());
         return TCL_OK;
      }

      case GETVALUEBYNAME:
         // string (name) --> string (value)
         return getValue(interp, findTunableConstantListEntryByName(argv[2]));

      case GETVALUEBYINDEX:
         // index --> string (value)
         return getValue(interp, tunableConstantListEntryByIndex(atoi(argv[2])));

      case SETVALUEBYNAME:
         // string (name) x string(value) --> NULL
         return setValue(interp, findTunableConstantListEntryByName(argv[2]), argv[3]);

      case SETVALUEBYINDEX:
         // index x string (value) --> NULL
         return setValue(interp, tunableConstantListEntryByIndex(atoi(argv[2])), argv[3]);

      case GETTYPEBYINDEX:
         // index --> string (value)
         return getType(interp, tunableConstantListEntryByIndex(atoi(argv[2])));

      case GETTYPEBYNAME:
         // string (name) --> string (type)
         return getType(interp, findTunableConstantListEntryByName(argv[2]));

      case GETUSEBYINDEX:
         // index --> string (use)  [developer v. user]
         return getUse(interp, tunableConstantListEntryByIndex(atoi(argv[2])));

      case GETUSEBYNAME:
         // string (name) --> string (use)
         return getUse(interp, findTunableConstantListEntryByName(argv[2]));

      case GETFLOATRANGEBYINDEX:
         // index --> string (float range)
         return getFloatRange(interp, (tunableFloatConstant *)tunableConstantListEntryByIndex(atoi(argv[2])));

      case GETFLOATRANGEBYNAME:
         // name --> string (float range)
         return getFloatRange(interp, (tunableFloatConstant *)findTunableConstantListEntryByName(argv[2]));

      default: assert(false);
   }

   assert(false);
   return TCL_ERROR;
}

void InstallTunableTclCommand(Tcl_Interp *interp) {
   // Call this in your C++ program (presumably near the
   // beginning) to add the "TclTunable" command to your tcl scripts...

   Tcl_CreateCommand(interp,
		     "tclTunable", // name of command
		     TclTunableCommand,
		     NULL, // no client data
		     NULL // no delete command
                    );
}

/* ****************************************************** */
// temporary stuff
#ifdef tcltunabletestprog

extern int main(int argc, char **argv);
void *dummy = (void *)main;

Tcl_Interp *MainInterp;

int Tcl_AppInit(Tcl_Interp *interp) {
   MainInterp=interp;
   // Tk_Window mainWindow = Tk_MainWindow(interp);

   if (TCL_ERROR == Tcl_Init(interp)) return TCL_ERROR;
   if (TCL_ERROR ==  Tk_Init(interp)) return TCL_ERROR;
   InstallTunableTclCommand(interp);

   tcl_RcFileName = "~/.wishrc";
   return TCL_OK;
}

tunableBooleanConstant myConst(true, NULL, userConstant, "myName", "myDescription");
tunableBooleanConstant myConst2(true, NULL, developerConstant, "myNameIsJack", "myDescription");
tunableBooleanConstant myConst3(true, NULL, userConstant, "myNameIsBilly", "myDescription");
tunableBooleanConstant myConst4(false, NULL, userConstant, "myNameIsBob", "myDescription");
tunableBooleanConstant myConst5(true, NULL, userConstant, "myNameIsJoe", "myDescription");

tunableFloatConstant myConst10(1.5, 0.0, 10.0, NULL, userConstant, "Float #1", "floatdescr1");
tunableFloatConstant myConst11(1.6, NULL, NULL, userConstant, "Name of Float #2", "floatdescr2");

tunableBooleanConstant myConst20(false, NULL, developerConstant, "Developer Constant #1", "myDescription");
tunableBooleanConstant myConst21(false, NULL, developerConstant, "Developer Constant #2", "myDescription");
tunableBooleanConstant myConst22(false, NULL, developerConstant, "Developer Constant #3", "myDescription");
tunableBooleanConstant myConst23(false, NULL, developerConstant, "Developer Constant #4", "myDescription");
tunableBooleanConstant myConst24(false, NULL, developerConstant, "Developer Constant #5", "myDescription");
tunableBooleanConstant myConst25(false, NULL, developerConstant, "Developer Constant #6", "myDescription");

#endif

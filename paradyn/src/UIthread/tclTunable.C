// tclTunable.C
// C++ code that provides access to tunable constants from tcl.

/* $Log: tclTunable.C,v $
/* Revision 1.5  1995/02/27 18:57:51  tamches
/* Extensive changes, to reflect equally extensive changes which
/* have been made to tunable constants.
/*
 * Revision 1.4  1994/12/21  07:40:49  tamches
 * Removed uses of tunableConstant::allConstants (which became a protected
 * class variable), replacing them with tunableConstant::beginIteration();
 *
 * Revision 1.3  1994/12/21  00:44:07  tamches
 * Reduces compiler warnings e.g Bool to bool, char * to const char *
 *
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
#include "../TCthread/tunableConst.h"

#include "tclTunable.h"

struct cmdTabEntry {
   const char *cmdName;
   int   index;
   int   numArgs;
};

const int GETBOOLALLNAMES     =0; // return an array of names, representing the associative
                                  // array index values for each boolean tunable constant in
                                  // the registry.
const int GETFLOATALLNAMES    =1; // return an array of names, representing the associative
                                  // array index values for each float tunable constant in
                                  // the registry.
const int GETNUMTUNABLES      =2;
const int GETNUMBOOLTUNABLES  =3;
const int GETNUMFLOATTUNABLES =4;
const int GETDESCRIPTION      =5;
const int GETVALUEBYNAME      =6;
const int SETVALUEBYNAME      =7;
const int GETTYPEBYNAME       =8;
const int GETUSEBYNAME        =9;
const int GETFLOATRANGEBYNAME =10;
const int CMDERROR            =11;

struct cmdTabEntry TclTunableCommands[] = {
  {"getboolallnames", GETBOOLALLNAMES, 0}, // return a copy of all boolean tc names
  {"getfloatallnames", GETFLOATALLNAMES, 0}, // return a copy of all float tc names
  {"getnumtunables", GETNUMTUNABLES, 0},   // size of global tunable lists
  {"getnumbooltunables", GETNUMBOOLTUNABLES, 0},   // size of global bool tunable list
  {"getnumfloattunables", GETNUMFLOATTUNABLES, 0},   // size of global float tunable list
  {"getdescription", GETDESCRIPTION, 1},   // string (name) to string (description)
  {"getvaluebyname", GETVALUEBYNAME, 1},   // string (name) to value
  {"setvaluebyname", SETVALUEBYNAME, 2},   // string (name) x value to NULL
  {"gettypebyname", GETTYPEBYNAME, 1},     // string (name) to tc type (bool, float)
  {"getusebyname", GETUSEBYNAME, 1},       // string (name) to tunableUse (developerConstant, userConstant)
  {"getfloatrangebyname", GETFLOATRANGEBYNAME, 1}, // string (name) to float range

  {NULL, CMDERROR, 0}
};

int findCommand(Tcl_Interp *interp, int argc, char **argv) {
   if (argc==0) {
      sprintf(interp->result, "USAGE: uimpd tclTunable <option> args\n");
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

char *getBoolAllNames() {
   // Tcl_Merge takes in an array of strings, and returns a list
   // string, which MUST eventually be free()'d.
   const int numBoolTunables = tunableConstantRegistry::numBoolTunables();
   vector<tunableBooleanConstant> allBoolConstants = tunableConstantRegistry::getAllBoolTunableConstants();
   assert(allBoolConstants.size() == numBoolTunables);

   char **boolConstantStrings = new char* [numBoolTunables];
   assert(boolConstantStrings);

   for (int lcv=0; lcv<numBoolTunables; lcv++) {
      const string &theName = allBoolConstants[lcv].getName();
      boolConstantStrings[lcv] = theName.string_of();
   }

   char *resultString = Tcl_Merge(numBoolTunables, boolConstantStrings);
   return resultString;
}

char *getFloatAllNames() {
   // Tcl_Merge takes in an array of strings, and returns a list
   // string, which MUST eventually be free()'d.
   const int numFloatTunables = tunableConstantRegistry::numFloatTunables();
   vector<tunableFloatConstant> allFloatConstants = tunableConstantRegistry::getAllFloatTunableConstants();
   assert(allFloatConstants.size() == numFloatTunables);

   char **floatConstantStrings = new char* [numFloatTunables];
   assert(floatConstantStrings);

   for (int lcv=0; lcv<numFloatTunables; lcv++) {
      const string &theName = allFloatConstants[lcv].getName();
      floatConstantStrings[lcv] = theName.string_of();
   }

   char *resultString = Tcl_Merge(numFloatTunables, floatConstantStrings);
   return resultString;
}

int TclTunableCommand(ClientData cd, Tcl_Interp *interp,
		      int argc, char **argv) {
   // This is the entrypoint for the TclTunable command.
   // i.e. once installed into tcl, a tcl code call to "TclTunable" enters here...

   int commandIndex = findCommand(interp, argc-1, argv+1);
   if (commandIndex == CMDERROR) {
      sprintf(interp->result, "uimpd tclTunable: could not parse");
      return TCL_ERROR;
   }

   switch (commandIndex) {
      case GETBOOLALLNAMES: {
         char *resultString = getBoolAllNames();
         strcpy(interp->result, resultString);
         free(resultString);
         return TCL_OK;
      }

      case GETFLOATALLNAMES: {
         char *resultString = getFloatAllNames();
         strcpy(interp->result, resultString);
         free(resultString);
         return TCL_OK;
      }

      case GETNUMTUNABLES:
         sprintf(interp->result, "%d", tunableConstantRegistry::numTunables());
         return TCL_OK;

      case GETNUMBOOLTUNABLES:
         sprintf(interp->result, "%d", tunableConstantRegistry::numBoolTunables());
         return TCL_OK;

      case GETNUMFLOATTUNABLES:
         sprintf(interp->result, "%d", tunableConstantRegistry::numFloatTunables());
         return TCL_OK;

      case GETDESCRIPTION: {
         // string (name) --> description (if no description, we substitute the name)
 	 if (!tunableConstantRegistry::existsTunableConstant(argv[2])) {
            sprintf(interp->result, "tclTunable getdescription: unknown tunable %s\n", argv[2]);
            return TCL_ERROR;
	 }

         tunableConstantBase tcb = tunableConstantRegistry::getGenericTunableConstantByName(argv[2]);
         sprintf(interp->result, "%s", (tcb.getDesc()==NULL) ? tcb.getName().string_of() : tcb.getDesc().string_of());
         return TCL_OK;
      }

      case GETVALUEBYNAME:
         // string (name) --> string (value)
 	 if (!tunableConstantRegistry::existsTunableConstant(argv[2])) {
            sprintf(interp->result, "tclTunable getvaluebyname: unknown tunable %s\n", argv[2]);
            return TCL_ERROR;
	 }

         if (tunableConstantRegistry::getTunableConstantType(argv[2]) == tunableBoolean) {
            tunableBooleanConstant tbc = tunableConstantRegistry::findBoolTunableConstant(argv[2]);
            if (tbc.getValue() == true)
               strcpy(interp->result, "1");
            else
               strcpy(interp->result, "0");
            return TCL_OK;
         }
         else {
            tunableFloatConstant tfc = tunableConstantRegistry::findFloatTunableConstant(argv[2]);
            sprintf(interp->result, "%g", tfc.getValue());
            return TCL_OK;
         }

      case SETVALUEBYNAME:
         // string (name) x string(value) --> NULL
 	 if (!tunableConstantRegistry::existsTunableConstant(argv[2])) {
            sprintf(interp->result, "tclTunable setvaluebyname: unknown tunable %s\n", argv[2]);
            return TCL_ERROR;
	 }

         if (tunableConstantRegistry::getTunableConstantType(argv[2]) == tunableBoolean)
	   tunableConstantRegistry::setBoolTunableConstant(argv[2], (bool)atoi(argv[3]));
         else
	   tunableConstantRegistry::setFloatTunableConstant(argv[2], (float)atof(argv[3]));
         return TCL_OK;

      case GETTYPEBYNAME:
         // string (name) --> string (type)
 	 if (!tunableConstantRegistry::existsTunableConstant(argv[2])) {
            sprintf(interp->result, "tclTunable gettypebyname: unknown tunable %s\n", argv[2]);
            return TCL_ERROR;
	 }

         if (tunableConstantRegistry::getTunableConstantType(argv[2]) == tunableBoolean)
            sprintf(interp->result, "bool");
         else
            sprintf(interp->result, "float");
         return TCL_OK;

      case GETUSEBYNAME: {
         // string (name) --> string (use)
 	 if (!tunableConstantRegistry::existsTunableConstant(argv[2])) {
            sprintf(interp->result, "tclTunable getusebyname: unknown tunable %s\n", argv[2]);
            return TCL_ERROR;
	 }

         tunableConstantBase tcb = tunableConstantRegistry::getGenericTunableConstantByName(argv[2]);
         if (tcb.getUse() == developerConstant)
            sprintf(interp->result, "developer");
         else
            sprintf(interp->result, "user");
         return TCL_OK;
      }

      case GETFLOATRANGEBYNAME: {
         // name --> string (float range)
 	 if (!tunableConstantRegistry::existsFloatTunableConstant(argv[2])) {
            sprintf(interp->result, "tclTunable getfloatrangebyname: unknown float tunable %s\n", argv[2]);
            return TCL_ERROR;
	 }

         assert(tunableFloat == tunableConstantRegistry::getTunableConstantType(argv[2]));
         tunableFloatConstant tfc = tunableConstantRegistry::findFloatTunableConstant(argv[2]);
         sprintf(interp->result, "%g %g", tfc.getMin(), tfc.getMax());
         return TCL_OK;
      }

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

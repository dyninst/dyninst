/*
 * Copyright (c) 1996-1998 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// tclTunable.C
// C++ code that provides access to tunable constants from tcl.

/* $Id: tclTunable.C,v 1.15 1999/04/27 16:03:51 nash Exp $ */

#include <assert.h>
#include <stdlib.h> // atoi()

#include "util/h/headers.h"
#include "tcl.h"
#include "tk.h"
#include "../TCthread/tunableConst.h"

#include "tclTunable.h"

struct cmdTabEntry {
   const char *cmdName;
   int   index;
   int   numArgs;
};

const int GETBOOLALLNAMES     =0;
   // return an array of names, representing the associative
   // array index values for each boolean tunable constant in
   // the registry.
const int GETFLOATALLNAMES    =1;
   // return an array of names, representing the associative
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
  {"getnumtunables", GETNUMTUNABLES, 0},           // size of global tunable lists
  {"getnumbooltunables", GETNUMBOOLTUNABLES, 0},   // size of global bool tunable list
  {"getnumfloattunables", GETNUMFLOATTUNABLES, 0}, // size of global float tunable list
  {"getdescription", GETDESCRIPTION, 1},   // string (name) to string (description)
  {"getvaluebyname", GETVALUEBYNAME, 1},   // string (name) to value
  {"setvaluebyname", SETVALUEBYNAME, 2},   // string (name) x value to NULL
  {"gettypebyname", GETTYPEBYNAME, 1},     // string (name) to tc type (bool, float)
  {"getusebyname", GETUSEBYNAME, 1},       // string (name) to tunableUse (developerConstant vs. userConstant)
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
   const unsigned numBoolTunables = tunableConstantRegistry::numBoolTunables();
   vector<tunableBooleanConstant> allBoolConstants = tunableConstantRegistry::getAllBoolTunableConstants();
   assert(allBoolConstants.size() == numBoolTunables);

   const char **boolConstantStrings = new const char* [numBoolTunables];
   assert(boolConstantStrings);

   for (unsigned lcv=0; lcv<numBoolTunables; lcv++) {
      const string &theName = allBoolConstants[lcv].getName();
      boolConstantStrings[lcv] = theName.string_of();
   }

   char *resultString = Tcl_Merge(numBoolTunables, (char**)boolConstantStrings);
   return resultString;
}

char *getFloatAllNames() {
   // Tcl_Merge takes in an array of strings, and returns a list
   // string, which MUST eventually be free()'d.
   const unsigned numFloatTunables = tunableConstantRegistry::numFloatTunables();
   vector<tunableFloatConstant> allFloatConstants = tunableConstantRegistry::getAllFloatTunableConstants();
   assert(allFloatConstants.size() == numFloatTunables);

   const char **floatConstantStrings = new const char* [numFloatTunables];
   assert(floatConstantStrings);

   for (unsigned lcv=0; lcv<numFloatTunables; lcv++) {
      const string &theName = allFloatConstants[lcv].getName();
      floatConstantStrings[lcv] = theName.string_of();
   }

   char *resultString = Tcl_Merge(numFloatTunables, (char**)floatConstantStrings);
   return resultString;
}

int TclTunableCommand(ClientData, Tcl_Interp *interp,
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
	 Tcl_SetResult(interp, resultString, TCL_DYNAMIC);
            // TCL_DYNAMIC --> "the string was allocated with malloc()
            // and is now the property of the tcl system"
         return TCL_OK;
      }

      case GETFLOATALLNAMES: {
         char *resultString = getFloatAllNames();
	 Tcl_SetResult(interp, resultString, TCL_DYNAMIC);
            // TCL_DYNAMIC --> "the string was allocated with malloc()
            // and is now the property of the tcl system"
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
	 Tcl_SetResult(interp, (char*)((tcb.getDesc().string_of()==NULL) ? tcb.getName().string_of() : tcb.getDesc().string_of()), TCL_VOLATILE);
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

         bool aflag;
	 aflag=(tunableFloat == tunableConstantRegistry::getTunableConstantType(argv[2]));
	 assert(aflag);
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

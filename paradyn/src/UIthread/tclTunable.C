/*
 * Copyright (c) 1996-2003 Barton P. Miller
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

/* $Id: tclTunable.C,v 1.26 2003/09/05 19:14:21 pcroth Exp $ */

#include <assert.h>
#include <stdlib.h> // atoi()

#include "common/h/headers.h"
#include "tcl.h"
#include "tk.h"
#include "../TCthread/tunableConst.h"
#include "pdutil/h/TclTools.h"
#include "ParadynTkGUI.h"


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
  {"getdescription", GETDESCRIPTION, 1},   // pdstring (name) to pdstring (description)
  {"getvaluebyname", GETVALUEBYNAME, 1},   // pdstring (name) to value
  {"setvaluebyname", SETVALUEBYNAME, 2},   // pdstring (name) x value to NULL
  {"gettypebyname", GETTYPEBYNAME, 1},     // pdstring (name) to tc type (bool, float)
  {"getusebyname", GETUSEBYNAME, 1},       // pdstring (name) to tunableUse (developerConstant vs. userConstant)
  {"getfloatrangebyname", GETFLOATRANGEBYNAME, 1}, // pdstring (name) to float range

  {NULL, CMDERROR, 0}
};

int findCommand(Tcl_Interp *interp, int argc, TCLCONST char **argv) {
   std::ostringstream resstr;

   if (argc==0) {
      resstr << "USAGE: uimpd tclTunable <option> args\n" << std::ends;
      SetInterpResult(interp, resstr);
      return CMDERROR;
   }

   for (cmdTabEntry *cmd = TclTunableCommands; cmd!=NULL; cmd++) {
      if (0==strcmp(argv[0], cmd->cmdName)) {
         // we have a match
         if (argc-1 == cmd->numArgs)
            return cmd->index;

         // wrong # args
         resstr << argv[0] << ": wrong # args ("
             << argc-1 << "); should be "
             << cmd->numArgs << ".\n" << std::ends;
         SetInterpResult(interp, resstr);
         return CMDERROR;
      }
   }

   // could not find command
   resstr << "TclTunable: unknown option (" << argv[0] << ")\n" << std::ends;
   SetInterpResult(interp, resstr);
   return CMDERROR;
}



Tcl_Obj*
getBoolAllNames(Tcl_Interp* interp)
{
    unsigned nBoolTunables = tunableConstantRegistry::numBoolTunables();
    pdvector<tunableBooleanConstant> allBoolConstants = tunableConstantRegistry::getAllBoolTunableConstants();
    assert(allBoolConstants.size() == nBoolTunables);
    unsigned int i;

    Tcl_Obj* resultObj = Tcl_NewListObj(0, NULL);
    for( i = 0; i < nBoolTunables; i++ )
    {
        const char* strName = allBoolConstants[i].getName().c_str();
        Tcl_Obj* theName = Tcl_NewStringObj( strName, -1 );
        int appendRet = Tcl_ListObjAppendElement(interp, resultObj, theName );
        if( appendRet != TCL_OK )
        {
            // the error is already reflected in the interpreter's result
            break;
        }
    }
    return resultObj;
}


Tcl_Obj*
getFloatAllNames(Tcl_Interp* interp)
{
    unsigned nFloatTunables = tunableConstantRegistry::numFloatTunables();
    pdvector<tunableFloatConstant> allFloatConstants = tunableConstantRegistry::getAllFloatTunableConstants();
    assert(allFloatConstants.size() == nFloatTunables);
    unsigned int i;

    Tcl_Obj* resultObj = Tcl_NewListObj(0, NULL);
    for( i = 0; i < nFloatTunables; i++ )
    {
        Tcl_Obj* theName = Tcl_NewStringObj( allFloatConstants[i].getName().c_str(), -1 );
        int appendRet = Tcl_ListObjAppendElement(interp, resultObj, theName );
        if( appendRet != TCL_OK )
        {
            // the error is already reflected in the interpreter's result
            break;
        }
    }
    return resultObj;
}


int
ParadynTkGUI::TclTunableCmd( int argc, TCLCONST char **argv)
{
   // This is the entrypoint for the TclTunable command.
   // i.e. once installed into tcl, a tcl code call to "TclTunable" enters here...
   std::ostringstream resstr;
   bool resultSet = false;


   int commandIndex = findCommand(interp, argc-1, argv+1);
   if (commandIndex == CMDERROR) {
      resstr << "uimpd tclTunable: could not parse" << std::ends;
      SetInterpResult(interp, resstr);
      return TCL_ERROR;
   }

   int ret = TCL_OK;
   switch (commandIndex) {
      case GETBOOLALLNAMES: {
        Tcl_SetObjResult( interp, getBoolAllNames(interp) );
        resultSet = true;
        break;
      }

      case GETFLOATALLNAMES: {
        Tcl_SetObjResult( interp, getFloatAllNames(interp) );
        resultSet = true;
        break;
      }

      case GETNUMTUNABLES:
         resstr << tunableConstantRegistry::numTunables() << std::ends;
         break;

      case GETNUMBOOLTUNABLES:
         resstr << tunableConstantRegistry::numBoolTunables() << std::ends;
         break;

      case GETNUMFLOATTUNABLES:
         resstr << tunableConstantRegistry::numFloatTunables() << std::ends;
         break;

      case GETDESCRIPTION: {
         // pdstring (name) --> description (if no description, we substitute the name)
 	     if (!tunableConstantRegistry::existsTunableConstant(argv[2])) {
                resstr << "tclTunable getdescription: unknown tunable "
                    << argv[2] << "\n" << std::ends;
                ret = TCL_ERROR;
                break;
	     }

         tunableConstantBase tcb = tunableConstantRegistry::getGenericTunableConstantByName(argv[2]);
	     resstr << const_cast<char*>((tcb.getDesc().c_str()==NULL) ? 
					 tcb.getName().c_str() : 
					 tcb.getDesc().c_str()) << std::ends;
         break;
      }

      case GETVALUEBYNAME:
         // pdstring (name) --> pdstring (value)
 	     if (!tunableConstantRegistry::existsTunableConstant(argv[2])) {
            resstr << "tclTunable getvaluebyname: unknown tunable "
                << argv[2] << "\n" << std::ends;
            ret = TCL_ERROR;
            break;
	     }

         if (tunableConstantRegistry::getTunableConstantType(argv[2]) == tunableBoolean) {
            tunableBooleanConstant tbc = tunableConstantRegistry::findBoolTunableConstant(argv[2]);
            if (tbc.getValue() == true)
                resstr << "1" << std::ends;
            else
                resstr << "0" << std::ends;
         }
         else {
            tunableFloatConstant tfc = tunableConstantRegistry::findFloatTunableConstant(argv[2]);
            resstr << tfc.getValue() << std::ends;
         }
         break;

      case SETVALUEBYNAME:
         // pdstring (name) x pdstring(value) --> NULL
 	     if (!tunableConstantRegistry::existsTunableConstant(argv[2])) {
            resstr << "tclTunable setvaluebyname: unknown tunable "
                << argv[2] << "\n" << std::ends;
            ret = TCL_ERROR;
            break;
	     }

         if (tunableConstantRegistry::getTunableConstantType(argv[2]) == tunableBoolean)
	       tunableConstantRegistry::setBoolTunableConstant(argv[2], (bool)atoi(argv[3]));
         else
	       tunableConstantRegistry::setFloatTunableConstant(argv[2], (float)atof(argv[3]));
         break;

      case GETTYPEBYNAME:
         // pdstring (name) --> pdstring (type)
 	     if (!tunableConstantRegistry::existsTunableConstant(argv[2])) {
                resstr << "tclTunable gettypebyname: unknown tunable "
                    << argv[2] << "\n" << std::ends;
                ret = TCL_ERROR;
                break;
	     }

         if (tunableConstantRegistry::getTunableConstantType(argv[2]) == tunableBoolean)
             resstr << "bool" << std::ends;
         else
             resstr << "float" << std::ends;
         break;

      case GETUSEBYNAME: {
             // pdstring (name) --> pdstring (use)
 	         if (!tunableConstantRegistry::existsTunableConstant(argv[2])) {
                    resstr << "tclTunable getusebyname: unknown tunable "
                        << argv[2] << "\n" << std::ends;
                    ret = TCL_ERROR;
                    break;
	         }

             tunableConstantBase tcb = tunableConstantRegistry::getGenericTunableConstantByName(argv[2]);
             if (tcb.getUse() == developerConstant)
                 resstr << "developer" << std::ends;
             else
                 resstr << "user" << std::ends;

         }
         break;

      case GETFLOATRANGEBYNAME: {
             // name --> pdstring (float range)
             if (!tunableConstantRegistry::existsFloatTunableConstant(argv[2])) {
                resstr << "tclTunable getfloatrangebyname: unknown float tunable "
                    << argv[2] << "\n" << std::ends;
                ret = TCL_ERROR;
                break;
             }

             bool aflag;
	         aflag=(tunableFloat == tunableConstantRegistry::getTunableConstantType(argv[2]));
	         assert(aflag);
             tunableFloatConstant tfc = tunableConstantRegistry::findFloatTunableConstant(argv[2]);
             resstr << tfc.getMin() << " " << tfc.getMax() << std::ends;
         }
         break;

      default:
          resstr << "unrecognized tclTunable command '" << argv[1] << "'" 
                 << std::ends;
          ret = TCL_ERROR;
   }

   if( !resultSet )
   {
        SetInterpResult(interp, resstr);
   }
   return ret;
}


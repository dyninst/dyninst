/*
 * UIpublic.C : exported services of the User Interface Manager thread 
 *              of Paradyn
 *
 * This file contains the following:
 *      UIMUser::chooseMenuItemREPLY
 *               showErrorREPLY
 *               showMsgREPLY
 *               chooseMetricsandResourcesREPLY
 *
 *      UIM:: 
 */

#include <stdio.h>
#include "UI.CLNT.h"
#include "UI.SRVR.h"
#include "UI.h"

void 
UIMUser::chooseMenuItemREPLY(chooseMenuItemCBFunc cb, int userChoice)
{
  (cb) (userChoice);
}

void
UIMUser::showErrorREPLY(showErrorCBFunc cb,int userChoice)
{
  (cb) (userChoice);
}

void 
UIMUser::showMsgREPLY(showMsgCBFunc cb,int userChoice)
{
  (cb) (userChoice);
}

void
UIMUser::chooseMetricsandResourcesREPLY
          (chooseMandRCBFunc cb,
	   char **metricNames,
	   int numMetrics,
	   resourceList *focusChoice)
{
  (cb) (metricNames, numMetrics, focusChoice);
}
	
void 
UIM::chooseMenuItem(chooseMenuItemCBFunc cb,
	       char *menuItems,
	       char *menuTitle,
	       char *options,
	       int numMenuItems,
	       int flags)
{
  sprintf (stderr, "chooseMenuItem called\n");
}

void 
UIM::showError(showErrorCBFunc cb,
	  char *displayMsg)
{
  sprintf (stderr, "showError: %s\n", displayMsg);
}

void 
UIM::showMsg(showMsgCBFunc cb,
	char *displayMsg,
	int numChoices,
	char **choices)
{
  sprintf (stderr, "showMsg: %s\n", displayMsg);
}

void 
UIM::chooseMetricsandResources(chooseMandRCBFunc cb)
{
  sprintf (stderr, "chooseMetricsandResources\n");
}






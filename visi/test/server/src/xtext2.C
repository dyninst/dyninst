/* $Log: xtext2.C,v $
/* Revision 1.5  2004/04/16 20:41:18  legendre
/* Fixed compilation errors
/*
/* Revision 1.4  1996/01/19 20:56:15  newhall
/* more chages to visiLib interface
/*
 * Revision 1.3  1996/01/17 19:29:11  newhall
 * changes due to new visiLib
 *
 * Revision 1.2  1996/01/17 18:40:26  newhall
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/18  18:27:13  newhall
 * updated test subdirectory, added visilib routine GetMetRes()
 * */

/*
 * $XConsortium: xtext.c,v 1.16 91/05/16 14:56:23 swick Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pdutil/h/pdsocket.h"
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Paned.h>

#include <X11/Xaw/Cardinals.h>

//////////////////////////////////
#include "visualization.h"
//////////////////////////////////

static void ClearText(Widget w,XtPointer text_ptr,XtPointer call_data);
static void PrintText(Widget w,XtPointer text_ptr,XtPointer call_data);
static void QuitProgram(Widget w,void * app_con,XtPointer call_data);
static void Syntax(XtAppContext app_con,char *call);
//////////////////////////////
XtAppContext app_con;
Widget toplevel, paned, clear, print, quit, text;
Widget getMR, stopMR, phaseN;
//////////////////////////////

// extern int fprintf(), bcopy(), read(), perror(), printf();

String fallback_resources[] = { 
    "*background: Grey",
    "*foreground: Black",
    "*font: *-Helvetica-*-r-*-12-*",
    "*input:				True",
    "*showGrip:				off",
    "?.?.text.preferredPaneSize:	200", 
    "?.?.text.width:			200", 
    "?.?.text.textSource.editType:	edit",
    "?.?.text.scrollVertical:		whenNeeded",
    "?.?.text.scrollHorizontal:		whenNeeded",
    "?.?.text.autoFill:			on",
    "*clear*label:            Clear",
    "*print*label:            Print",
    "*quit*label:              Quit",
    NULL,
};

// callback routine for FOLD and DATAVALUES 
int fd_input(int dummy){

  Arg args[1];
  XawTextPosition pos;
  XawTextBlock tb;
  char buf[100];
  int i,j,k;
  int noMetrics,noResources,noBins;
  int size;
  double value;

  XtSetArg(args[0], XtNinsertPosition, &pos);
  XtGetValues(text, args, ONE);


  fprintf(stderr,"@@@@ in callback for datavalues and fold\n");
  noMetrics = visi_NumMetrics();
  noResources = visi_NumResources();
  noBins = visi_NumBuckets();
  for(i=0;i < noMetrics; i++)
   for(j=0;j<noResources;j++){
      k = dummy;
      if(visi_Valid(i,j)){
	  fprintf(stderr,"dataGrid[%d][%d][%d] = %f\n",i,j,k,
					visi_DataValue(i,j,k));
	  value = visi_DataValue(i,j,k);
          if(!(isnan(value))){
          sprintf(&buf[0],"%s%d%s%d%s%d%s%f\n","dataGrid[",i,"][",j,"][",k,
	          "] = ",value); 
          }
          else{
            sprintf(&buf[0],"%s%d%s%d%s%d%s\n","dataGrid[",i,"][",j,"][",k ,
		    "] = NaN"); 
          }
          size = strlen(buf);  
          tb.firstPos = 0;
          tb.length = size;
          tb.ptr = buf;
          tb.format = FMT8BIT;
          if (XawTextReplace(text,pos,pos+size-1,&tb) != XawEditDone) {
              fprintf(stderr,"XawTextReplace returned error\n");
          }
          pos += size;
          XtSetArg(args[0], XtNinsertPosition, pos);
          XtSetValues(text, args, ONE);
    }
  }
  return 1;
}

// callback routine for ADDMETRICSRESOURCES
int fd_input2(int dummy){
  Arg args[1];
  XawTextPosition pos;
  XawTextBlock tb;
  char buf[100];
  int size;
  int noMetrics,noResources,noBins;
  int aggr;
  double value;

  fprintf(stderr,"@@@@ in callback for ADDMETRICSRESOURCES\n");
  XtSetArg(args[0], XtNinsertPosition, &pos);
  XtGetValues(text, args, ONE);
  noMetrics = visi_NumMetrics();
  noResources = visi_NumResources();
  noBins = visi_NumBuckets();
  value  = visi_BucketWidth();
  sprintf(&buf[0],"\n%s%d%s%d%s%d%s%f\n","noMetrics = ",noMetrics,
	 ", no resources = ",noResources,", num Bins = ",noBins,
	 "\nbinWidth = ",value);

  size = strlen(buf);  
  tb.firstPos = 0;
  tb.length = size;
  tb.ptr = buf;
  tb.format = FMT8BIT;
  if (XawTextReplace(text,pos,pos+size-1,&tb) != XawEditDone) {
    fprintf(stderr,"XawTextReplace returned error\n");
  }
  pos += size;
  XtSetArg(args[0], XtNinsertPosition, pos);
  XtSetValues(text, args, ONE);
  return(0);
}

// callback routine for NEWMETRICSRESOURCES, PHASENAME
int fd_input3(int dummy){
  Arg args[1];
  XawTextPosition pos;
  XawTextBlock tb;
  char buf[100];
  int size;

  XtSetArg(args[0], XtNinsertPosition, &pos);
  XtGetValues(text, args, ONE);
  sprintf(&buf[0],"\n%s\n",
  "This operation is not fully supported: only the call is implemented");
  size = strlen(buf);  
  tb.firstPos = 0;
  tb.length = size;
  tb.ptr = buf;
  tb.format = FMT8BIT;
  if (XawTextReplace(text,pos,pos+size-1,&tb) != XawEditDone) {
    fprintf(stderr,"XawTextReplace returned error\n");
  }
  pos += size;
  XtSetArg(args[0], XtNinsertPosition, pos);
  XtSetValues(text, args, ONE);

  return(0);
}

/////////////////////////////////////
static void GetMetsResCallback(Widget w,
			       void *app_con,
			       XtPointer call_data)
{

  fprintf(stderr,"@@@@ in GetMetsResCallback upcall\n"); 
  visi_GetMetsRes(0,0);  
}

static void StopMetsResCallback(Widget w,
				void *app_con,
				XtPointer call_data)
{

  fprintf(stderr,"@@@@ in StopMetsResCallback upcall\n"); 
  visi_StopMetRes(0,0);  
}

static void PName(Widget w, void * app_con,XtPointer call_data){

  fprintf(stderr,"@@@@ in PName upcall\n"); 
  visi_DefinePhase(" ", 0, 0);  
}
/////////////////////////////////////


int main(int argc,char **argv)
{
 //   XtAppContext app_con;
 //  Widget toplevel, paned, clear, print, quit, text;
 Arg args[1];

/////////////////////////////////////
// variables added for paradyn integration 
    int ok, fd;
/////////////////////////////////////

    toplevel = XtAppInitialize(&app_con, "Xtext", NULL, ZERO,
			       &argc, argv, fallback_resources,
			       NULL, ZERO);

    /*
     * Check to see that all arguments were processed, and if not then
     * report an error and exit.
     */

    if (argc != 1)		
	Syntax(app_con, argv[0]);

//////////////////////////////////////
// call visi_Init: step (1) from README file

   if((fd = visi_Init()) < 0){
	 exit(-1);
    }
//////////////////////////////////////

//////////////////////////////////////
// register event callbacks: step (2) from README file

   ok = visi_RegistrationCallback(ADDMETRICSRESOURCES,fd_input2); 
   ok = visi_RegistrationCallback(DATAVALUES,fd_input); 
   ok = visi_RegistrationCallback(FOLD,fd_input); 
   ok = visi_RegistrationCallback(INVALIDMETRICSRESOURCES,fd_input);
//////////////////////////////////////

    paned = XtCreateManagedWidget("paned", panedWidgetClass, toplevel, 
				  NULL, ZERO);
    clear = XtCreateManagedWidget("clear", commandWidgetClass, paned, 
				  NULL, ZERO);
    print = XtCreateManagedWidget("print", commandWidgetClass, paned, 
				  NULL, ZERO);
    quit = XtCreateManagedWidget("quit", commandWidgetClass, paned, 
				  NULL, ZERO);
//////////////////////////////////////
// this is for an upcall to Paradyn: step (3a) from README

    getMR = XtCreateManagedWidget("Get Metric Resource",commandWidgetClass,
				  paned,NULL,ZERO);
    stopMR = XtCreateManagedWidget("Stop Metric Resource",commandWidgetClass,
				  paned,NULL,ZERO);
    phaseN = XtCreateManagedWidget("Name a Phase",commandWidgetClass,
				  paned,NULL,ZERO);

//////////////////////////////////////

    XtSetArg(args[0], XtNstring, "This is a test.\n");

    text = XtCreateManagedWidget("text", asciiTextWidgetClass, paned, 
				 args, ONE);

    XtAddCallback(clear, XtNcallback, ClearText, (XtPointer) text);
    XtAddCallback(print, XtNcallback, PrintText, (XtPointer) text);
    XtAddCallback(quit,  XtNcallback, QuitProgram, (XtPointer) app_con);

///////////////////////////////////////
//  Add callbacks for upcalls to Paradyn: step (3b) from README

    XtAddCallback(getMR, XtNcallback, GetMetsResCallback, (XtPointer) app_con);
    XtAddCallback(stopMR, XtNcallback, StopMetsResCallback, (XtPointer) app_con);
    XtAddCallback(phaseN, XtNcallback, PName, (XtPointer) app_con);
//////////////////////////////////////

//////////////////////////////////////
// register visi_callback routine as callback on events assoc. w/ file desc
// step (4) from README file

   XtAppAddInput(app_con,fd,(XtPointer) XtInputReadMask,
		 (XtInputCallbackProc) visi_callback, text); 
//////////////////////////////////////

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_con);
}

/*	Function Name: ClearText
 *	Description: This function clears all text out of the text widget.
 *	Arguments: w - *** UNUSED ***
 *                 text_ptr - a pointer to the text widget.
 *                 call_data - *** UNUSED ***.
 *	Returns: none.
 */

/* ARGSUSED */
static void ClearText(Widget w,XtPointer text_ptr,XtPointer call_data)
{
    Widget text = (Widget) text_ptr;
    Arg args[1];

    XtSetArg(args[0], XtNstring, "");
    XtSetValues(text, args, ONE);
}

/*	Function Name: PrintText
 *	Description: This function clears all text out of the text widget.
 *	Arguments: w - *** UNUSED ***
 *                 text_ptr - a pointer to the text widget.
 *                 call_data - *** UNUSED ***.
 *	Returns: none.
 */

/* ARGSUSED */
static void PrintText(Widget w,XtPointer text_ptr,XtPointer call_data)
{
    Widget text = (Widget) text_ptr;
    Arg args[1];
    String str;

    XtSetArg(args[0], XtNstring, &str);
    XtGetValues(text, args, ONE);

    fprintf(stderr, "Text String is:\n--------\n%s\n--------\n", str);
}

/*	Function Name: QuitProgram
 *	Description: This function exits the program
 *	Arguments: w - *** UNUSED ***
 *                 text_ptr - a pointer to the text widget.
 *                 call_data - *** UNUSED ***.
 *	Returns: none.
 */

/* ARGSUSED */
static void QuitProgram(Widget w, void *pp_con, XtPointer call_data)
{
    fprintf(stderr, "Bye!\n");
    XtDestroyApplicationContext((XtAppContext) app_con);
    exit(0);
}

/*	Function Name: Syntax
 *	Description: Prints a the calling syntax for this function to stdout.
 *	Arguments: app_con - the application context.
 *                 call - the name of the application.
 *	Returns: none - exits tho.
 */

static void Syntax(XtAppContext app_con,char *call)
{
    XtDestroyApplicationContext(app_con);
    fprintf( stderr, "Usage: %s \n", call);
    exit(1);
}


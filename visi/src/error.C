/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */
/* $Log: error.C,v $
/* Revision 1.3  1994/05/11 17:13:09  newhall
/* changed data type from double to float
/*
 * Revision 1.2  1994/03/14  20:28:47  newhall
 * changed visi subdirectory structure
 *  */ 
#include "visi/h/visiTypes.h"

static char *visi_errmsg[] =
{
	"No Error",
	"Error: realloc",
	"Error: create datagrid",
	"Error: subscript out of range",
	"Error: aggregate datagrid error",
	"Error: no valid element",
	"Error: malloc",
	"Error: strncpy"
	"Error: VisiInit incorrect number of arguments"

};


void visi_ErrorHandler(int errno,char *msg){

  if((errno < (VISI_ERROR_BASE))&&(errno >= VISI_ERROR_MAX))
    fprintf(stderr,"%s : %s\n",visi_errmsg[VISI_ERROR_BASE-errno],msg);

  else
    fprintf(stderr,"ERROR: invalid errno = %d\n",errno);

}


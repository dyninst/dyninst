/* $Log: error.C,v $
/* Revision 1.2  1994/03/14 20:28:47  newhall
/* changed visi subdirectory structure
/*  */ 
#include "visi/h/error.h"

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


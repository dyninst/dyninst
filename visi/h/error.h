/* $Log: error.h,v $
/* Revision 1.1  1994/03/14 20:27:28  newhall
/* changed visi subdirectory structure
/*  */ 
#ifndef _error_h
#define _error_h
#include <stdio.h>
#include <math.h>
#define INVALID            0
#define VALID              1
#define NOVALUE           -1
#define OK                 0
#define VISI_ERROR_BASE   -19
#define ERROR_REALLOC     -20
#define ERROR_CREATEGRID  -21
#define ERROR_SUBSCRIPT   -22
#define ERROR_AGGREGATE   -23
#define ERROR_NOELM       -24
#define ERROR_MALLOC      -25
#define ERROR_STRNCPY     -26
#define ERROR_INIT        -27
#define VISI_ERROR_MAX    -27
#define ERROR              quiet_nan() 

typedef enum {DATAVALUES,INVALIDMETRICSRESOURCES,ADDMETRICSRESOURCES,
	      NEWMETRICSRESOURCES,PHASENAME,FOLD} msgTag;

extern void visi_ErrorHandler(int errno,char *msg);
#endif

/*
 *  tclHack.c -- This file references the procedure main() so that the 
 *     tclMain.o module from libtcl.a will be loaded.  
 *     
 * $Log: tclHack.c,v $
 * Revision 1.1  1994/05/31 21:05:49  rbi
 * Initial version of tclVisi and tabVis
 *
 *
 */
extern int main();
int *tclDummyMainPtr = (int *) main;

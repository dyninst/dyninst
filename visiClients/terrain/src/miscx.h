/* $Id: miscx.h,v 1.2 1998/03/30 01:22:32 wylie Exp $ */

#ifndef MISCX_H
#define MISCX_H



void PopUpInit();
void IFeep();
int IGetGeometry(Window win, WindowInfo *Info);
char *ips_malloc(int count);
void XtGetValue(Widget widget, char *field, caddr_t result);
void XtSetValue(Widget widget, char *field, caddr_t result);


#endif
